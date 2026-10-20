/* NetHack 5.0	polyself.c	$NHDT-Date: 1772101811 2026/02/26 02:30:11 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.227 $ */
/*      Copyright (C) 1987, 1988, 1989 by Ken Arromdee */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Polymorph self routine.
 *
 * Note:  the light source handling code assumes that gy.youmonst.m_id
 * always remains 1 and gy.youmonst.mx will always remain 0 when it handles
 * the case of the player polymorphed into a light-emitting monster.
 *
 * Transformation sequences:
 *              /-> polymon                 poly into monster form
 *    polyself =
 *              \-> newman -> polyman       fail to poly, get human form
 *
 *    rehumanize -> polyman                 return to original form
 *
 *    polymon (called directly)             usually golem petrification
 */

#include "hack.h"

staticfn void check_strangling(boolean);
staticfn void polyman(const char *, const char *);
staticfn void dropp(struct obj *);
staticfn void break_armor(void);
staticfn void drop_weapon(int);
staticfn int armor_to_dragon(int);
staticfn void newman(void);
staticfn void polysense(void);

static const char no_longer_petrify_resistant[] =
    "Больше не устойчивы к окаменению, вы";

/* update the gy.youmonst.data structure pointer and intrinsics */
void
set_uasmon(void)
{
    struct permonst *mdat = &mons[u.umonnum];
    boolean was_vampshifter = valid_vampshiftform(gy.youmonst.cham, u.umonnum);

    set_mon_data(&gy.youmonst, mdat);
    gy.youmonst.m_id = 1;

    if (Protection_from_shape_changers)
        gy.youmonst.cham = NON_PM;
    else if (is_vampire(gy.youmonst.data))
        gy.youmonst.cham = gy.youmonst.mnum;
    /* assume hero-as-chameleon/doppelganger/sandestin doesn't change shape */
    else if (!was_vampshifter)
        gy.youmonst.cham = NON_PM;
    u.mcham = gy.youmonst.cham; /* for save/restore since youmonst isn't */

#define PROPSET(PropIndx, ON)                          \
    do {                                               \
        if (ON)                                        \
            u.uprops[PropIndx].intrinsic |= FROMFORM;  \
        else                                           \
            u.uprops[PropIndx].intrinsic &= ~FROMFORM; \
    } while (0)
#define resist_from_form(MRtyp) ((gy.youmonst.data->mresists & (MRtyp)) != 0)

    PROPSET(FIRE_RES, resist_from_form(MR_FIRE));
    PROPSET(COLD_RES, resist_from_form( MR_COLD));
    PROPSET(SLEEP_RES, resist_from_form(MR_SLEEP));
    PROPSET(DISINT_RES, resist_from_form(MR_DISINT));
    PROPSET(SHOCK_RES, resist_from_form(MR_ELEC));
    PROPSET(POISON_RES, resist_from_form(MR_POISON));
    PROPSET(ACID_RES, resist_from_form(MR_ACID));
    PROPSET(STONE_RES, resist_from_form(MR_STONE));
    {
        /* resists_drli() takes wielded weapon into account; suppress it */
        struct obj *save_uwep = uwep;

        uwep = 0;
        PROPSET(DRAIN_RES, resists_drli(&gy.youmonst));
        uwep = save_uwep;
    }
    /* resists_magm() takes wielded, worn, and carried equipment into
       into account; cheat and duplicate its monster-specific part */
    PROPSET(ANTIMAGIC, (dmgtype(mdat, AD_MAGM)
                        || mdat == &mons[PM_BABY_GRAY_DRAGON]
                        || dmgtype(mdat, AD_RBRE)));
    PROPSET(SICK_RES, (mdat->mlet == S_FUNGUS || mdat == &mons[PM_GHOUL]));

    PROPSET(STUNNED, (mdat == &mons[PM_STALKER] || is_bat(mdat)));
    PROPSET(HALLUC_RES, dmgtype(mdat, AD_HALU));
    PROPSET(SEE_INVIS, perceives(mdat));
    PROPSET(TELEPAT, telepathic(mdat));
    /* note that Infravision uses mons[race] rather than usual mons[role] */
    PROPSET(INFRAVISION, infravision(Upolyd ? mdat : &mons[gu.urace.mnum]));
    PROPSET(INVIS, pm_invisible(mdat));
    PROPSET(TELEPORT, can_teleport(mdat));
    PROPSET(TELEPORT_CONTROL, control_teleport(mdat));
    PROPSET(LEVITATION, is_floater(mdat));
    /* floating eye is the only 'floater'; it is also flagged as a 'flyer';
       suppress flying for it so that enlightenment doesn't confusingly
       show latent flight capability always blocked by levitation */
    PROPSET(FLYING, (is_flyer(mdat) && !is_floater(mdat)));
    PROPSET(SWIMMING, is_swimmer(mdat));
    /* [don't touch MAGICAL_BREATHING here; both Amphibious and Breathless
       key off of it but include different monster forms...] */
    PROPSET(PASSES_WALLS, passes_walls(mdat));
    PROPSET(REGENERATION, regenerates(mdat));
    PROPSET(REFLECTING, (mdat == &mons[PM_SILVER_DRAGON]));
    PROPSET(BLINDED, !haseyes(mdat));
    PROPSET(BLND_RES, (dmgtype_fromattack(mdat, AD_BLND, AT_EXPL)
                       || dmgtype_fromattack(mdat, AD_BLND, AT_GAZE)));
#undef PROPSET
#undef resist_from_form

    /* whether the player is flying/floating depends on their steed,
       which won't be known during the restore process: but BFlying
       and BStealth should be set correctly already in that case, so
       there's nothing to do */
    if (!program_state.restoring)
        float_vs_flight(); /* maybe toggle (BFlying & I_SPECIAL) */
    polysense();

#ifdef STATUS_HILITES
    if (VIA_WINDOWPORT())
        status_initialize(REASSESS_ONLY);
#endif
    /* we can reset this now, having just done what it is meant to trigger */
    gw.were_changes = 0L;
}

/* Levitation overrides Flying; set or clear BFlying|I_SPECIAL */
void
float_vs_flight(void)
{
    boolean stuck_in_floor = (u.utrap && u.utraptype != TT_PIT);

    /* floating overrides flight; so does being trapped in the floor */
    if ((HLevitation || ELevitation)
        || ((HFlying || EFlying) && stuck_in_floor))
        BFlying |= I_SPECIAL;
    else
        BFlying &= ~I_SPECIAL;
    /* being trapped on the ground (bear trap, web, molten lava survived
       with fire resistance, former lava solidified via cold, tethered
       to a buried iron ball) overrides floating--the floor is reachable */
    if ((HLevitation || ELevitation) && stuck_in_floor)
        BLevitation |= I_SPECIAL;
    else
        BLevitation &= ~I_SPECIAL;

    /* riding blocks stealth unless hero+steed fly, so a change in flying
       might cause a change in stealth */
    steed_vs_stealth();

    disp.botl = TRUE;
}

/* riding blocks stealth unless hero+steed fly */
void
steed_vs_stealth(void)
{
    if (u.usteed && !Flying && !Levitation)
        BStealth |= FROMOUTSIDE;
    else
        BStealth &= ~FROMOUTSIDE;
}

/* for changing into form that's immune to strangulation */
staticfn void
check_strangling(boolean on)
{
    /* on -- maybe resume strangling */
    if (on) {
        boolean was_strangled = (Strangled != 0L);

        /* when Strangled is already set, polymorphing from one
           vulnerable form into another causes the counter to be reset */
        if (uamul && uamul->otyp == AMULET_OF_STRANGULATION
            && can_be_strangled(&gy.youmonst)) {
            Strangled = 6L;
            disp.botl = TRUE;
            Your("%s %s ваша %s!", simpleonames(uamul),
                 was_strangled ? "всё ещё сжимает" : "начинает сжимать",
                 body_part_case(NECK, BP_ACC)); /* "throat" */
            makeknown(AMULET_OF_STRANGULATION);
        }

    /* off -- maybe block strangling */
    } else {
        if (Strangled && !can_be_strangled(&gy.youmonst)) {
            Strangled = 0L;
            disp.botl = TRUE;
            You("больше не подвергаетесь удушению.");
        }
    }
}

DISABLE_WARNING_FORMAT_NONLITERAL

/* make a (new) human out of the player */
staticfn void
polyman(const char *fmt, const char *arg)
{
    boolean sticking = (sticks(gy.youmonst.data) && u.ustuck && !u.uswallow),
            was_mimicking = (U_AP_TYPE != M_AP_NOTHING);
    boolean was_blind = !!Blind,
            had_see_invis = !!See_invisible;

    if (Upolyd) {
        u.acurr = u.macurr; /* restore old attribs */
        u.amax = u.mamax;
        u.umonnum = u.umonster;
        flags.female = u.mfemale;
    }
    set_uasmon();

    u.mh = u.mhmax = 0;
    u.mtimedone = 0;
    skinback(FALSE);
    u.uundetected = 0;

    if (sticking)
        uunstick();
    find_ac();
    if (was_mimicking) {
        if (gm.multi < 0)
            unmul("");
        gy.youmonst.m_ap_type = M_AP_NOTHING;
        gy.youmonst.mappearance = 0;
    }

    newsym(u.ux, u.uy);

    urgent_pline(fmt, arg);
    /* check whether player foolishly genocided self while poly'd */
    if (ugenocided()) {
        /* intervening activity might have clobbered genocide info */
        struct kinfo *kptr = find_delayed_killer(POLYMORPH);

        if (kptr != (struct kinfo *) 0 && kptr->name[0]) {
            svk.killer.format = kptr->format;
            Strcpy(svk.killer.name, kptr->name);
        } else {
            svk.killer.format = KILLED_BY;
            Strcpy(svk.killer.name, "собственного геноцида");
        }
        dealloc_killer(kptr);
        done(GENOCIDED);
    }

    if (!!See_invisible ^ had_see_invis)
        set_mimic_blocking(); /* See_invisible just toggled */

    if (u.twoweap && !could_twoweap(gy.youmonst.data))
        untwoweapon();

    if (u.utrap && u.utraptype == TT_PIT) {
        set_utrap(rn1(6, 2), TT_PIT); /* time to escape resets */
    }
    if (was_blind && !Blind) { /* reverting from eyeless */
        set_itimeout(&HBlinded, 1L);
        make_blinded(0L, TRUE); /* remove blindness */
    }
    check_strangling(TRUE);

    if (!Levitation && !u.ustuck && is_pool_or_lava(u.ux, u.uy))
        spoteffects(TRUE);

    see_monsters();
}

RESTORE_WARNING_FORMAT_NONLITERAL

void
change_sex(void)
{
    /* Some monsters are always of one sex and their sex can't be changed;
     * Succubi/incubi can change, but are handled below.
     *
     * !Upolyd check necessary because is_male() and is_female()
     * may be true for certain roles
     */
    if (!Upolyd
        || (!is_male(gy.youmonst.data) && !is_female(gy.youmonst.data)
            && !is_neuter(gy.youmonst.data)))
        flags.female = !flags.female;
    if (Upolyd) /* poly'd: also change saved sex */
        u.mfemale = !u.mfemale;
    max_rank_sz(); /* [this appears to be superfluous] */
    if ((Upolyd ? u.mfemale : flags.female) && gu.urole.name.f)
        Strcpy(svp.pl_character, gu.urole.name.f);
    else
        Strcpy(svp.pl_character, gu.urole.name.m);
    if (!Upolyd) {
        u.umonnum = u.umonster;
    } else if (u.umonnum == PM_AMOROUS_DEMON) {
        flags.female = !flags.female;
#if 0
        /* change monster type to match new sex; disabled with
           PM_AMOROUS_DEMON */
        u.umonnum = (u.umonnum == PM_SUCCUBUS) ? PM_INCUBUS : PM_SUCCUBUS;
#endif
        set_uasmon();
    }
}

/* log a message if non-poly'd hero's gender has changed */
void
livelog_newform(boolean viapoly, int oldgend, int newgend)
{
    char buf[BUFSZ];
    const char *oldrole, *oldrank, *newrole, *newrank;

    /*
     * TODO?
     *  Give other logging feedback here instead of in newman().
     */

    if (!Upolyd) {
        if (newgend != oldgend) {
            oldrole = (oldgend && gu.urole.name.f) ? gu.urole.name.f
                                                  : gu.urole.name.m;
            newrole = (newgend && gu.urole.name.f) ? gu.urole.name.f
                                                  : gu.urole.name.m;
            oldrank = rank_of(u.ulevel, Role_switch, oldgend);
            newrank = rank_of(u.ulevel, Role_switch, newgend);
            Sprintf(buf, "%.10s %.30s", genders[flags.female].adj, newrank);
            livelog_printf(LL_MINORAC, "%s в %s",
                           viapoly ? "превратился" : "перевоплотился",
                           an(strcmp(newrole, oldrole) ? newrole
                              : strcmp(newrank, oldrank) ? newrank
                                : buf));
        }
    }
}

staticfn void
newman(void)
{
    const char *newform;
    int i, oldlvl, newlvl, oldgend, newgend, hpmax, enmax;

    oldlvl = u.ulevel;
    newlvl = oldlvl + rn1(5, -2);     /* new = old + {-2,-1,0,+1,+2} */
    if (newlvl > 127 || newlvl < 1) { /* level went below 0? */
        goto dead; /* old level is still intact (in case of lifesaving) */
    }
    if (newlvl > MAXULEV)
        newlvl = MAXULEV;
    /* If your level goes down, your peak level goes down by
       the same amount so that you can't simply use blessed
       full healing to undo the decrease.  But if your level
       goes up, your peak level does *not* undergo the same
       adjustment; you might end up losing out on the chance
       to regain some levels previously lost to other causes. */
    if (newlvl < oldlvl)
        u.ulevelmax -= (oldlvl - newlvl);
    if (u.ulevelmax < newlvl)
        u.ulevelmax = newlvl;
    u.ulevel = newlvl;

    oldgend = poly_gender();
    if (gs.sex_change_ok && !rn2(10))
        change_sex();

    adjabil(oldlvl, (int) u.ulevel);

    /* random experience points for the new experience level */
    u.uexp = rndexp(FALSE);

    /* set up new attribute points (particularly Con) */
    redist_attr();

    /*
     * New hit points:
     *  remove "level gain"-based HP from any extra HP accumulated
     *  (the "extra" might actually be negative);
     *  modify the extra, retaining {80%, 90%, 100%, or 110%};
     *  add in newly generated set of level-gain HP.
     *
     * (This used to calculate new HP in direct proportion to old HP,
     * but that was subject to abuse:  accumulate a large amount of
     * extra HP, drain level down to 1, then polyself to level 2 or 3
     * [lifesaving capability needed to handle level 0 and -1 cases]
     * and the extra got multiplied by 2 or 3.  Repeat the level
     * drain and polyself steps until out of lifesaving capability.)
     */
    hpmax = u.uhpmax;
    for (i = 0; i < oldlvl; i++)
        hpmax -= (int) u.uhpinc[i];
    /* hpmax * rn1(4,8) / 10; 0.95*hpmax on average */
    hpmax = rounddiv((long) hpmax * (long) rn1(4, 8), 10);
    for (i = 0; (u.ulevel = i) < newlvl; i++)
        hpmax += newhp();
    if (hpmax < u.ulevel)
        hpmax = u.ulevel; /* min of 1 HP per level */
    /* retain same proportion for current HP; u.uhp * hpmax / u.uhpmax */
    u.uhp = rounddiv((long) u.uhp * (long) hpmax, u.uhpmax);
    setuhpmax(hpmax, TRUE); /* might reduce u.uhp */
    /*
     * Do the same for spell power.
     */
    enmax = u.uenmax;
    for (i = 0; i < oldlvl; i++)
        enmax -= (int) u.ueninc[i];
    enmax = rounddiv((long) enmax * (long) rn1(4, 8), 10);
    for (i = 0; (u.ulevel = i) < newlvl; i++)
        enmax += newpw();
    if (enmax < u.ulevel)
        enmax = u.ulevel;
    u.uen = rounddiv((long) u.uen * (long) enmax,
                     ((u.uenmax < 1) ? 1 : u.uenmax));
    u.uenmax = enmax;
    /* [should alignment record be tweaked too?] */

    u.uhunger = rn1(500, 500);
    if (Sick)
        make_sick(0L, (char *) 0, FALSE, SICK_ALL);
    if (Stoned)
        make_stoned(0L, (char *) 0, 0, (char *) 0);
    if (u.uhp <= 0) {
        if (Polymorph_control) { /* even when Stunned || Unaware */
            if (u.uhp <= 0)
                u.uhp = 1;
        } else {
 dead:      /* we come directly here if experience level went to 0 or less */
            urgent_pline(
                     "Ваша новая форма, похоже, недостаточно здорова, чтобы выжить.");
            svk.killer.format = KILLED_BY_AN;
            Strcpy(svk.killer.name, "неудачный полиморф");
            done(DIED);
            /* must have been life-saved to get here */
            newuhs(FALSE);
            encumber_msg(); /* used to be done by redist_attr() */
            return; /* lifesaved */
        }
    }
    newuhs(FALSE);
    /* use saved gender we're about to revert to, not current */
    newform = ((Upolyd ? u.mfemale : flags.female) && gu.urace.individual.f)
                ? gu.urace.individual.f
                : (gu.urace.individual.m)
                   ? gu.urace.individual.m
                   : gu.urace.noun;
    polyman("Вы чувствуете себя новым %s!", newform);

    newgend = poly_gender();
    /* note: newman() bypasses achievements for new ranks attained and
       doesn't log "new <form>" when that isn't accompanied by level change */
    if (newlvl != oldlvl)
        livelog_printf(LL_MINORAC, "стал %d уровнем опыта в новом облике %s",
                       newlvl, newform);
    else
        livelog_newform(TRUE, oldgend, newgend);

    if (Slimed) {
        Your("тело трансформируется, но на вас всё ещё слизь.");
        make_slimed(10L, (const char *) 0);
    }

    disp.botl = TRUE;
    see_monsters();
    encumber_msg();

    retouch_equipment(2);
    if (!uarmg)
        selftouch(no_longer_petrify_resistant);
}

void
polyself(int psflags)
{
    char buf[BUFSZ];
    int old_light, new_light, mntmp, class, tryct, gvariant = NEUTRAL;
    boolean forcecontrol = ((psflags & POLY_CONTROLLED) != 0),
            low_control = ((psflags & POLY_LOW_CTRL) != 0),
            monsterpoly = ((psflags & POLY_MONSTER) != 0),
            formrevert = ((psflags & POLY_REVERT) != 0),
            draconian = (uarm && Is_dragon_armor(uarm)),
            iswere = (ismnum(u.ulycn)),
            isvamp = (is_vampire(gy.youmonst.data)
                      || is_vampshifter(&gy.youmonst)),
            controllable_poly = Polymorph_control && !(Stunned || Unaware);

    if (Unchanging) {
        You("не удаётся трансформироваться!");
        return;
    }
    /* being Stunned|Unaware doesn't negate this aspect of Poly_control */
    if (!Polymorph_control && !forcecontrol && !draconian && !iswere
        && !isvamp) {
        if (rn2(20) > ACURR(A_CON)) {
            You1(shudder_for_moment);
            losehp(rnd(30), "системного шока", KILLED_BY_AN);
            exercise(A_CON, FALSE);
            return;
        }
    }
    old_light = emits_light(gy.youmonst.data);
    mntmp = NON_PM;

    if (formrevert) {
        mntmp = gy.youmonst.cham;
        monsterpoly = TRUE;
        controllable_poly = FALSE;
    }

    if (forcecontrol && low_control
        && (draconian || monsterpoly || isvamp || iswere))
        forcecontrol = FALSE;

    if (monsterpoly && isvamp)
        goto do_vampyr;

    if (controllable_poly || forcecontrol) {
        buf[0] = '\0';
        tryct = 5;

        do {
            mntmp = NON_PM;
            getlin("В кого превратиться? [введите название]", buf);
            (void) mungspaces(buf);
            if (*buf == '\033') {
                /* user is cancelling controlled poly */
                if (forcecontrol) { /* wizard mode #polyself */
                    pline1(Never_mind);
                    return;
                }
                Strcpy(buf, "*"); /* resort to random */
            }
            if (!strcmp(buf, "*") || !strcmp(buf, "случайно")) {
                /* explicitly requesting random result */
                tryct = 0; /* will skip thats_enough_tries */
                continue;  /* end do-while(--tryct > 0) loop */
            }
            class = 0;
            mntmp = name_to_mon(buf, &gvariant);
            if (mntmp < LOW_PM) {
 by_class:
                class = name_to_monclass(buf, &mntmp);
                if (class && mntmp == NON_PM)
                    mntmp = (draconian && class == S_DRAGON)
                            ? armor_to_dragon(uarm->otyp)
                            : mkclass_poly(class);

            /* placeholder monsters are for corpses and all flagged
               M2_NOPOLY but they are reasonable polymorph targets;
               pick a suitable substitute (which might be geno'd) */
            } else if (is_placeholder(&mons[mntmp])
                       /* when your own race, fall to !polyok() case */
                       && !your_race(&mons[mntmp])
                       /* same for generic human, even if hero isn't human */
                       && mntmp != PM_HUMAN) {
                /* far less general than mkclass() */
                if (mntmp == PM_ORC)
                    mntmp = rn2(3) ? PM_HILL_ORC : PM_MORDOR_ORC;
                else if (mntmp == PM_ELF)
                    mntmp = rn2(3) ? PM_GREEN_ELF : PM_GREY_ELF;
                else if (mntmp == PM_GIANT)
                    mntmp = rn2(3) ? PM_STONE_GIANT : PM_HILL_GIANT;
                /* note: PM_DWARF and PM_GNOME are ordinary monsters and
                   no longer flagged no-poly so have no need for placeholder
                   handling; PM_HUMAN is a placeholder without a suitable
                   substitute so gets handled differently below */
            }

            if (mntmp < LOW_PM) {
                if (!class)
                    pline("Я никогда не слышал о таких монстрах.");
                else
                    You_cant("превратиться ни в кого из них.");
            } else if (wizard && Upolyd
                       && (mntmp == u.umonster
                           /* "priest" and "priestess" match the monster
                              rather than the role; override that unless
                              the text explicitly contains "aligned" */
                           || (u.umonster == PM_CLERIC
                               && mntmp == PM_ALIGNED_CLERIC
                               && !strstri(buf, "aligned")))) {
                /* in wizard mode, picking own role while poly'd reverts to
                   normal without newman()'s chance of level or sex change */
                rehumanize();
                old_light = 0; /* rehumanize() extinguishes u-as-mon light */
                goto made_change;
            } else if (iswere && (were_beastie(mntmp) == u.ulycn
                                  || mntmp == counter_were(u.ulycn)
                                  || (Upolyd && mntmp == PM_HUMAN))) {
                goto do_shift;
            } else if (!polyok(&mons[mntmp])
                       /* Note:  humans are illegal as monsters, but an
                          illegal monster forces newman(), which is what
                          we want if they specified a human.... (unless
                          they specified a unique monster) */
                       && !(mntmp == PM_HUMAN
                            || (your_race(&mons[mntmp])
                                && (mons[mntmp].geno & G_UNIQ) == 0)
                            || mntmp == gu.urole.mnum)) {
                const char *pm_name;

                /* mkclass_poly() can pick a !polyok()
                   candidate; if so, usually try again */
                if (class) {
                    if (rn2(3) || --tryct > 0)
                        goto by_class;
                    /* no retries left; put one back on counter
                       so that end of loop decrement will yield
                       0 and trigger thats_enough_tries message */
                    ++tryct;
                }
                pm_name = pmname(&mons[mntmp], flags.female ? FEMALE : MALE);
                if (the_unique_pm(&mons[mntmp]))
                    pm_name = the(pm_name);
                else if (!type_is_pname(&mons[mntmp]))
                    pm_name = an(pm_name);
                You_cant("превратиться в %s.", pm_name);
            } else
                break;
        } while (--tryct > 0);

        if (!tryct)
            pline1(thats_enough_tries);
        /* allow skin merging, even when polymorph is controlled */
        if (draconian && (tryct <= 0 || mntmp == armor_to_dragon(uarm->otyp)))
            goto do_merge;
        if (isvamp && (tryct <= 0 || mntmp == PM_WOLF || mntmp == PM_FOG_CLOUD
                       || is_bat(&mons[mntmp])))
            goto do_vampyr;
    } else if (draconian || iswere || isvamp) {
        /* special changes that don't require polyok() */
        if (draconian) {
 do_merge:
            mntmp = armor_to_dragon(uarm->otyp);
            if (!(svm.mvitals[mntmp].mvflags & G_GENOD)) {
                unsigned was_lit = uarm->lamplit;
                int arm_light = artifact_light(uarm) ? arti_light_radius(uarm)
                                                     : 0;

                /* allow G_EXTINCT */
                if (Is_dragon_scales(uarm)) {
                    /* dragon scales remain intact as uskin */
                    You("сливаетесь со своей чешуйчатой бронёй.");
                } else { /* dragon scale mail reverts to scales */
                    /* similar to noarmor(invent.c),
                       shorten to "<color> scale mail" */
                    Strcpy(buf, simpleonames(uarm));
                    strsubst(buf, " dragon ", " ");
                    /* tricky phrasing; dragon scale mail is singular, dragon
                       scales are plural (note: we don't use "set of scales",
                       which usually overrides the distinction, here) */
                    Your("%s превращается в чешую, когда вы сливаетесь с ней.", buf);
                    /* uarm->spe enchantment remains unchanged;
                       re-converting scales to mail poses risk
                       of evaporation due to over enchanting */
                    uarm->otyp += GRAY_DRAGON_SCALES - GRAY_DRAGON_SCALE_MAIL;
                    observe_object(uarm);
                    disp.botl = TRUE; /* AC is changing */
                }
                uskin = uarm;
                uarm = (struct obj *) 0;
                /* save/restore hack */
                uskin->owornmask |= I_SPECIAL;
                if (was_lit)
                    maybe_adjust_light(uskin, arm_light);
                update_inventory();
            }
        } else if (iswere) {
 do_shift:
            if (Upolyd && were_beastie(mntmp) != u.ulycn)
                mntmp = PM_HUMAN; /* Illegal; force newman() */
            else
                mntmp = u.ulycn;
        } else if (isvamp) {
 do_vampyr:
            if (mntmp < LOW_PM || (mons[mntmp].geno & G_UNIQ)) {
                mntmp = (gy.youmonst.data == &mons[PM_VAMPIRE_LEADER]
                         && !rn2(10)) ? PM_WOLF
                                      : !rn2(4) ? PM_FOG_CLOUD
                                                : PM_VAMPIRE_BAT;
                if (ismnum(gy.youmonst.cham)
                    && !is_vampire(gy.youmonst.data) && !rn2(2))
                    mntmp = gy.youmonst.cham;
            }
            if (controllable_poly) {
                Sprintf(buf, "Превратиться в %s?",
                        an(pmname(&mons[mntmp], gvariant)));
                if (y_n(buf) != 'y')
                    return;
            }
        }
        /* if polymon fails, "you feel" message has been given
           so don't follow up with another polymon or newman;
           sex_change_ok left disabled here */
        if (mntmp == PM_HUMAN)
            newman(); /* werecritter */
        else
            (void) polymon(mntmp);
        goto made_change; /* maybe not, but this is right anyway */
    }

    if (mntmp < LOW_PM) {
        tryct = 200;
        do {
            /* randomly pick an "ordinary" monster */
            mntmp = rn1(SPECIAL_PM - LOW_PM, LOW_PM);
            if (polyok(&mons[mntmp]) && !is_placeholder(&mons[mntmp]))
                break;
        } while (--tryct > 0);
    }

    /* The below polyok() fails either if everything is genocided, or if
     * we deliberately chose something illegal to force newman().
     */
    gs.sex_change_ok++;
    if (!polyok(&mons[mntmp]) || (!forcecontrol && !rn2(5))
        || your_race(&mons[mntmp])) {
        newman();
    } else {
        (void) polymon(mntmp);
    }
    gs.sex_change_ok--; /* reset */

 made_change:
    new_light = emits_light(gy.youmonst.data);
    if (old_light != new_light) {
        if (old_light)
            del_light_source(LS_MONSTER, monst_to_any(&gy.youmonst));
        if (new_light == 1)
            ++new_light; /* otherwise it's undetectable */
        if (new_light)
            new_light_source(u.ux, u.uy, new_light, LS_MONSTER,
                             monst_to_any(&gy.youmonst));
    }
}

/* (try to) make a mntmp monster out of the player; return 1 if successful */
int
polymon(int mntmp)
{
    char buf[BUFSZ], ustuckNam[BUFSZ];
    boolean sticking = sticks(gy.youmonst.data) && u.ustuck && !u.uswallow,
            was_blind = !!Blind, dochange = FALSE, was_expelled = FALSE,
            was_hiding_under = u.uundetected && hides_under(gy.youmonst.data);
    int mlvl, newMaxStr;

    if (svm.mvitals[mntmp].mvflags & G_GENOD) { /* allow G_EXTINCT */
        You_feel("чувствуете себя довольно %s-образно.",
                 pmname(&mons[mntmp], flags.female ? FEMALE : MALE));
        exercise(A_WIS, TRUE);
        return 0;
    }

    /* KMH, conduct */
    if (!u.uconduct.polyselfs++)
        livelog_printf(LL_CONDUCT,
                       "впервые изменил форму, став %s",
                       an(pmname(&mons[mntmp], flags.female ? FEMALE : MALE)));

    /* exercise used to be at the very end but only Wis was affected
       there since the polymorph was always in effect by then */
    exercise(A_CON, FALSE);
    exercise(A_WIS, TRUE);

    if (!Upolyd) {
        /* Human to monster; save human stats */
        u.macurr = u.acurr;
        u.mamax = u.amax;
        u.mfemale = flags.female;
    } else {
        /* Monster to monster; restore human stats, to be
         * immediately changed to provide stats for the new monster
         */
        u.acurr = u.macurr;
        u.amax = u.mamax;
        flags.female = u.mfemale;
    }

    /* if stuck mimicking gold, stop immediately */
    if (gm.multi < 0 && U_AP_TYPE == M_AP_OBJECT
        && gy.youmonst.data->mlet != S_MIMIC)
        unmul("");
    /* if becoming a non-mimic, stop mimicking anything */
    if (mons[mntmp].mlet != S_MIMIC) {
        /* as in polyman() */
        gy.youmonst.m_ap_type = M_AP_NOTHING;
        gy.youmonst.mappearance = 0;
    }
    if (is_male(&mons[mntmp])) {
        if (flags.female)
            dochange = TRUE;
    } else if (is_female(&mons[mntmp])) {
        if (!flags.female)
            dochange = TRUE;
    } else if (!is_neuter(&mons[mntmp]) && mntmp != u.ulycn) {
        if (gs.sex_change_ok && !rn2(10))
            dochange = TRUE;
    }

    Strcpy(ustuckNam, u.ustuck ? Some_Monnam(u.ustuck) : "");

    Strcpy(buf, (u.umonnum != mntmp) ? "" : "новый ");
    if (dochange) {
        flags.female = !flags.female;
        Strcat(buf, (is_male(&mons[mntmp]) || is_female(&mons[mntmp]))
                       ? "" : flags.female ? "самка " : "самец ");
    }
    Strcat(buf, pmname(&mons[mntmp], flags.female ? FEMALE : MALE));
    You("%s %s!", (u.umonnum != mntmp) ? "превращаетесь в" : "чувствуете себя", an(buf));

    if (Stoned && poly_when_stoned(&mons[mntmp])) {
        /* poly_when_stoned already checked stone golem genocide */
        mntmp = PM_STONE_GOLEM;
        make_stoned(0L, "Вы превращаетесь в камень!", 0, (char *) 0);
    }

    u.mtimedone = rn1(500, 500);
    u.umonnum = mntmp;
    set_uasmon();

    /* New stats for monster, to last only as long as polymorphed.
     * Currently only strength gets changed.
     */
    newMaxStr = uasmon_maxStr();
    if (strongmonst(&mons[mntmp])) {
        ABASE(A_STR) = AMAX(A_STR) = (schar) newMaxStr;
    } else {
        /* not a strongmonst(); if hero has exceptional strength, remove it
           (note: removal is temporary until returning to original form);
           we don't attempt to enforce lower maximum for wimpy forms;
           unlike for strongmonst, current strength does not get set to max */
        AMAX(A_STR) = (schar) newMaxStr;
        /* make sure current is not higher than max (strip exceptional Str) */
        if (ABASE(A_STR) > AMAX(A_STR))
            ABASE(A_STR) = AMAX(A_STR);
    }

    if (Stone_resistance && Stoned) { /* parnes@eniac.seas.upenn.edu */
        make_stoned(0L, "Вы, кажется, больше не окаменеваете.", 0,
                    (char *) 0);
    }
    if (Sick_resistance && Sick) {
        make_sick(0L, (char *) 0, FALSE, SICK_ALL);
        You("больше не чувствуете тошноты.");
    }
    if (Slimed) {
        if (flaming(gy.youmonst.data)) {
            make_slimed(0L, "Слизь сгорает!");
        } else if (mntmp == PM_GREEN_SLIME) {
            /* do it silently */
            make_slimed(0L, (char *) 0);
        }
    }
    check_strangling(FALSE); /* maybe stop strangling */
    if (nohands(gy.youmonst.data))
        make_glib(0);

    /*
    mlvl = adj_lev(&mons[mntmp]);
     * We can't do the above, since there's no such thing as an
     * "experience level of you as a monster" for a polymorphed character.
     */
    mlvl = (int) mons[mntmp].mlevel;
    if (gy.youmonst.data->mlet == S_DRAGON && mntmp >= PM_GRAY_DRAGON) {
        u.mhmax = In_endgame(&u.uz) ? (8 * mlvl) : (4 * mlvl + d(mlvl, 4));
    } else if (is_golem(gy.youmonst.data)) {
        u.mhmax = golemhp(mntmp);
    } else {
        if (!mlvl)
            u.mhmax = rnd(4);
        else
            u.mhmax = d(mlvl, 8);
        if (is_home_elemental(&mons[mntmp]))
            u.mhmax *= 3;
    }
    u.mh = u.mhmax;

    if (u.ulevel < mlvl) {
        /* Low level characters can't become high level monsters for long */
#ifdef DUMB
        /* DRS/NS 2.2.6 messes up -- Peter Kendell */
        int mtd = u.mtimedone, ulv = u.ulevel;

        u.mtimedone = mtd * ulv / mlvl;
#else
        u.mtimedone = u.mtimedone * u.ulevel / mlvl;
#endif
    }

    if (uskin && mntmp != armor_to_dragon(uskin->otyp))
        skinback(FALSE);
    break_armor();
    drop_weapon(1);
    find_ac(); /* (repeated below) */
    /* if hiding under something and can't hide anymore, unhide now;
       but don't auto-hide when not already hiding-under */
    if (was_hiding_under)
        (void) hideunder(&gy.youmonst);

    if (u.utrap && u.utraptype == TT_PIT) {
        set_utrap(rn1(6, 2), TT_PIT); /* time to escape resets */
    }
    if (was_blind && !Blind) { /* previous form was eyeless */
        set_itimeout(&HBlinded, 1L);
        make_blinded(0L, TRUE); /* remove blindness */
    }
    newsym(u.ux, u.uy); /* Change symbol */

    /* you now know what an egg of your type looks like; [moved from
       below in case expels() -> spoteffects() drops hero onto any eggs] */
    if (lays_eggs(gy.youmonst.data)) {
        learn_egg_type(u.umonnum);
        /* make queen bees recognize killer bee eggs */
        learn_egg_type(egg_type_from_parent(u.umonnum, TRUE));
    }

    if (u.uswallow) {
        uchar usiz;

        /* if new form can't be swallowed, make engulfer expel hero */
        if (unsolid(gy.youmonst.data)
            /* subset of engulf_target() */
            || (usiz = gy.youmonst.data->msize) >= MZ_HUGE
            || (u.ustuck->data->msize < usiz && !is_whirly(u.ustuck->data))) {
            boolean expels_mesg = TRUE;

            if (unsolid(gy.youmonst.data)) {
                if (canspotmon(u.ustuck)) /* [see below for explanation] */
                    Strcpy(ustuckNam, Monnam(u.ustuck));
                pline("%s больше не может сдержать вас.", ustuckNam);
                expels_mesg = FALSE;
            }
            expels(u.ustuck, u.ustuck->data, expels_mesg);
            was_expelled = TRUE;
            /* FIXME? if expels() triggered rehumanize then we should
               return early */
        }

    /* [note:  this 'sticking' handling is only sufficient for changing from
       grabber to engulfer or vice versa because engulfing by poly'd hero
       always ends immediately so won't be in effect during a polymorph] */
    } else if (u.ustuck && !sticking /* && !u.uswallow */
               /* being held; if now capable of holding, make holder
                  release so that hero doesn't automagically start holding
                  it; or, release if no longer capable of being held */
               && (sticks(gy.youmonst.data) || unsolid(gy.youmonst.data))) {
        /* u.ustuck name was saved above in case we're changing from can-see
           to can't-see; but might have changed from can't-see to can-see so
           override here if hero knows who u.ustuck is */
        if (canspotmon(u.ustuck))
            Strcpy(ustuckNam, Monnam(u.ustuck));
        set_ustuck((struct monst *) 0);
        pline("%s ослабляет хватку на вас.", ustuckNam);
    } else if (sticking && !sticks(gy.youmonst.data)) {
        /* was holding onto u.ustuck but no longer capable of that */
        uunstick();
    }

    if (u.usteed) {
        if (touch_petrifies(u.usteed->data) && !Stone_resistance && rnl(3)) {
            pline("%s touch %s.", no_longer_petrify_resistant,
                  mon_nam(u.usteed));
            Sprintf(buf, "riding %s",
                    an(pmname(u.usteed->data, Mgender(u.usteed))));
            instapetrify(buf);
        }
        if (!can_ride(u.usteed))
            dismount_steed(DISMOUNT_POLY);
    }

    find_ac();
    if (((!Levitation && !u.ustuck && !Flying && is_pool_or_lava(u.ux, u.uy))
         || (Underwater && !Swimming))
        /* if expelled above, expels() already called spoteffects() */
        && !was_expelled) {
        spoteffects(TRUE);
        /* FIXME? if spoteffects() triggered rehumanize then we should
           return early */
    }
    if (Passes_walls && u.utrap
        && (u.utraptype == TT_INFLOOR || u.utraptype == TT_BURIEDBALL)) {
        if (u.utraptype == TT_INFLOOR) {
            pline("Камень, кажется, больше не держит вас.");
        } else {
            pline("Закопанный шар больше не прикован к вам.");
            buried_ball_to_freedom();
        }
        reset_utrap(TRUE);
    } else if (likes_lava(gy.youmonst.data) && u.utrap
               && u.utraptype == TT_LAVA) {
        pline("%s Теперь ощущается успокаивающе.", hliquid("lava"));
        reset_utrap(TRUE);
    }
    if (amorphous(gy.youmonst.data) || is_whirly(gy.youmonst.data)
        || unsolid(gy.youmonst.data)) {
        if (Punished) {
            You("выскальзываете из железной цепи.");
            unpunish();
        } else if (u.utrap && u.utraptype == TT_BURIEDBALL) {
            You("высвобождаетесь из закопанного шара и цепи.");
            buried_ball_to_freedom();
        }
    }
    if (u.utrap && (u.utraptype == TT_WEB || u.utraptype == TT_BEARTRAP)
        && (amorphous(gy.youmonst.data) || is_whirly(gy.youmonst.data)
            || unsolid(gy.youmonst.data)
            || (gy.youmonst.data->msize <= MZ_SMALL
                && u.utraptype == TT_BEARTRAP))) {
        You("больше не застряли в %s.",
            u.utraptype == TT_WEB ? "паутине" : "капкане");
        /* probably should burn webs too if PM_FIRE_ELEMENTAL */
        reset_utrap(TRUE);
    }
    if (webmaker(gy.youmonst.data) && u.utrap && u.utraptype == TT_WEB) {
        You("ориентируетесь на паутине.");
        reset_utrap(TRUE);
    }
    check_strangling(TRUE); /* maybe start strangling */

    disp.botl = TRUE;
    gv.vision_full_recalc = 1;
    see_monsters();
    encumber_msg();

    retouch_equipment(2);
    /* this might trigger a recursive call to polymon() [stone golem
       wielding cockatrice corpse and hit by stone-to-flesh, becomes
       flesh golem above, now gets transformed back into stone golem;
       fortunately neither form uses #monster] */
    if (!uarmg)
        selftouch(no_longer_petrify_resistant);

    /* the explanation of '#monster' used to be shown sooner, but there are
       possible fatalities above and it isn't useful unless hero survives */
    if (flags.verbose) {
        static const char use_thec[] = "Используйте команду #%s, чтобы %s.";
        static const char monsterc[] = "монстр";
        struct permonst *uptr = gy.youmonst.data;
        boolean might_hide = (is_hider(uptr) || hides_under(uptr));

        if (can_breathe(uptr))
            pline(use_thec, monsterc, "использовать своё дыхательное оружие");
        if (attacktype(uptr, AT_SPIT))
            pline(use_thec, monsterc, "плеваться ядом");
        if (uptr->mlet == S_NYMPH)
            pline(use_thec, monsterc, "снять железный шар");
        if (attacktype(uptr, AT_GAZE))
            pline(use_thec, monsterc, "смотреть взглядом на монстров");
        if (might_hide && webmaker(uptr))
            pline(use_thec, monsterc, "спрятаться или сплести паутину");
        else if (might_hide)
            pline(use_thec, monsterc, "спрятаться");
        else if (webmaker(uptr))
            pline(use_thec, monsterc, "сплести паутину");
        if (is_were(uptr))
            pline(use_thec, monsterc, "призвать помощь");
        if (u.umonnum == PM_GREMLIN)
            pline(use_thec, monsterc, "размножиться в фонтане");
        if (is_unicorn(uptr))
            pline(use_thec, monsterc, "использовать свой рог");
        if (is_mind_flayer(uptr))
            pline(use_thec, monsterc, "испустить ментальный всплеск");
        if (uptr->msound == MS_SHRIEK) /* worthless, actually */
            pline(use_thec, monsterc, "пронзительно закричать");
        if (is_vampire(uptr) || is_vampshifter(&gy.youmonst))
            pline(use_thec, monsterc, "изменить облик");

        if (lays_eggs(uptr) && flags.female
            && !(uptr == &mons[PM_GIANT_EEL]
                 || uptr == &mons[PM_ELECTRIC_EEL]))
            pline(use_thec, "sit",
                  eggs_in_water(uptr) ? "метать икру в воде" : "отложить яйцо");
    }
    return 1;
}

/* determine hero's temporary strength value used while polymorphed;
   hero poly'd into M2_STRONG monster usually gets 18/100 strength but
   there are exceptions; non-M2_STRONG get maximum strength set to 18 */
schar
uasmon_maxStr(void)
{
    const struct Race *R;
    int newMaxStr;
    int mndx = u.umonnum;
    struct permonst *ptr = &mons[mndx];

    if (is_orc(ptr)) {
        if (mndx != PM_URUK_HAI && mndx != PM_ORC_CAPTAIN)
            mndx = PM_ORC;
    } else if (is_elf(ptr)) {
        mndx = PM_ELF;
    } else if (is_dwarf(ptr)) {
        mndx = PM_DWARF;
    } else if (is_gnome(ptr)) {
        mndx = PM_GNOME;
#if 0   /* use the mons[] value for humans */
    } else if (is_human(ptr)) {
        mndx = PM_HUMAN;
#endif
    }
    R = character_race(mndx);

    if (strongmonst(ptr)) {
        /* ettins, titans and minotaurs don't pass the is_giant() test;
           giant mummies and giant zombies do but we throttle those */
        boolean live_H = is_giant(ptr) && !is_undead(ptr);

        /* hero orcs are limited to 18/50 for maximum strength, so treat
           hero poly'd into an orc the same; goblins, orc shamans, and orc
           zombies don't have strongmonst() attribute so won't get here;
           hobgoblins and orc mummies do get here and are limited to 18/50
           like normal orcs; however, orc captains and Uruk-hai retain 18/100
           strength; hero gnomes are also limited to 18/50; hero elves are
           limited to 18/00 regardless of whether they're strongmonst, but
           the two strongmonst types (monarchs and nobles) have current
           strength set to 18 [by polymon()], the others don't */
        newMaxStr = R ? R->attrmax[A_STR] : live_H ? STR19(19) : STR18(100);
    } else {
        newMaxStr = R ? R->attrmax[A_STR] : 18; /* 18 is same as STR18(0) */
    }
    return (schar) newMaxStr;
}

/* dropx() jacket for break_armor() */
staticfn void
dropp(struct obj *obj)
{
    struct obj *otmp;

    /*
     * Dropping worn armor while polymorphing might put hero into water
     * (loss of levitation boots or water walking boots that the new
     * form can't wear), where emergency_disrobe() could remove it from
     * inventory.  Without this, dropx() could trigger an 'object lost'
     * panic.  Right now, boots are the only armor which might encounter
     * this situation, but handle it for all armor.
     *
     * Hypothetically, 'obj' could have merged with something (not
     * applicable for armor) and no longer be a valid pointer, so scan
     * inventory for it instead of trusting obj->where.
     */
    for (otmp = gi.invent; otmp; otmp = otmp->nobj) {
        if (otmp == obj) {
            dropx(obj);
            /* Note that otmp->nobj is pointing at fobj now,
             * as a result of:
             * dropx() -> dropy() -> dropz() -> place_object(),
             * and no longer pointing at the next obj in inventory.
             * That would be an issue if this loop were allowed
             * to continue, but the break statement that
             * follows prevents the loop from continuing on with
             * objects on the floor.
             */
            break;
        }
    }
}

staticfn void
break_armor(void)
{
    struct obj *otmp;
    struct permonst *uptr = gy.youmonst.data;

    if (breakarm(uptr)) {
        if ((otmp = uarm) != 0) {
            if (donning(otmp))
                cancel_don();
            /* for gold DSM, we don't want Armor_gone() to report that it
               stops shining _after_ we've been told that it is destroyed */
            if (otmp->lamplit)
                end_burn(otmp, FALSE);

            You("вырываетесь из своей брони!");
            exercise(A_STR, FALSE);
            (void) Armor_gone();
            useup(otmp);
        }
        if ((otmp = uarmc) != 0
            /* mummy wrapping adapts to small and very big sizes */
            && (otmp->otyp != MUMMY_WRAPPING || !WrappingAllowed(uptr))) {
            if (otmp->otyp == MUMMY_WRAPPING) {
                /* doesn't have a clasp to break open */
                Your("%s разрывается на части!", cloak_simple_name(otmp));
                (void) Cloak_off();
                useup(otmp);
            } else if (otmp->otyp == ALCHEMY_SMOCK) {
                pline("Узел на вашем %s развязывается!", cloak_simple_name(otmp));
                (void) Cloak_off();
                dropp(otmp);
            } else {
                pline("Застёжка на вашем %s распадается!", cloak_simple_name(otmp));
                (void) Cloak_off();
                dropp(otmp);
            }
        }
        if (uarmu) {
            Your("рубашка разрывается в клочья!");
            useup(uarmu);
        }
    } else if (sliparm(uptr)) {
        if ((otmp = uarm) != 0 && racial_exception(&gy.youmonst, otmp) < 1) {
            if (donning(otmp))
                cancel_don();
            Your("броня спадает с вас!");
            /* [note: _gone() instead of _off() dates to when life-saving
               could force fire resisting armor back on if hero burned in
               hell (3.0, predating Gehennom); the armor isn't actually
               gone here but also isn't available to be put back on] */
            (void) Armor_gone();
            dropp(otmp);
        }
        if ((otmp = uarmc) != 0
            /* mummy wrapping adapts to small and very big sizes */
            && (otmp->otyp != MUMMY_WRAPPING || !WrappingAllowed(uptr))) {
            if (is_whirly(uptr))
                Your("%s падает, ничем не удерживаемый!", cloak_simple_name(otmp));
            else
                You("уменьшаетесь в %s!", cloak_simple_name(otmp));
            (void) Cloak_off();
            dropp(otmp);
        }
        if ((otmp = uarmu) != 0) {
            if (is_whirly(uptr))
                You("просачиваетесь сквозь свою рубашку!");
            else
                You("становитесь слишком малы для своей рубашки!");
            setworn((struct obj *) 0, otmp->owornmask & W_ARMU);
            dropp(otmp);
        }
    }
    if (has_horns(uptr)) {
        if ((otmp = uarmh) != 0) {
            if (is_flimsy(otmp) && !donning(otmp)) {
                /* Future possibilities: This could damage/destroy helmet */
                Your("Ваш%s %s %s %s!",
                     num_horns(uptr) == 1 ? "" : "и",
                     num_horns(uptr) == 1 ? "рог" : "рога",
                     num_horns(uptr) == 1 ? "пробивает" : "пробивают",
                     the(xname(otmp)));
            } else {
                if (donning(otmp))
                    cancel_don();
                Your("%s падает на %s!", helm_simple_name(otmp),
                     surface(u.ux, u.uy));
                (void) Helmet_off();
                dropp(otmp);
            }
        }
    }
    if (nohands(uptr) || verysmall(uptr)) {
        if ((otmp = uarmg) != 0) {
            if (donning(otmp))
                cancel_don();
            /* Drop weapon along with gloves */
            You("бросаете свои перчатки%s!", uwep ? " и оружие" : "");
            drop_weapon(0);
            (void) Gloves_off();
            /* Glib manipulation (ends immediately) handled by Gloves_off */
            dropp(otmp);
        }
        if ((otmp = uarms) != 0) {
            You("больше не можете держать свой щит!");
            (void) Shield_off();
            dropp(otmp);
        }
        if ((otmp = uarmh) != 0) {
            if (donning(otmp))
                cancel_don();
            Your("%s падает на %s!", helm_simple_name(otmp),
                 surface(u.ux, u.uy));
            (void) Helmet_off();
            dropp(otmp);
        }
    }
    if (nohands(uptr) || verysmall(uptr)
        || slithy(uptr) || uptr->mlet == S_CENTAUR) {
        if ((otmp = uarmf) != 0) {
            if (donning(otmp))
                cancel_don();
            if (is_whirly(uptr))
                Your("сапоги спадают!");
            else
                Your("сапоги %s с ваших ног!",
                     verysmall(uptr) ? "соскальзывают" : "слетают");
            (void) Boots_off();
            dropp(otmp);
        }
    }
    /* not armor, but eyewear shouldn't stay worn without a head to wear
       it/them on (should also come off if head is too tiny or too huge,
       but putting accessories on doesn't reject those cases [yet?]);
       amulet stays worn */
    if ((otmp = ublindf) != 0 && !has_head(uptr)) {
        const char *eyewear = simpleonames(otmp); /* blindfold|towel|lenses */

        Your("%s %s!", eyewear,
             otmp->otyp == LENSES ? "спадают" : "спадает");
        (void) Blindf_off((struct obj *) 0); /* Null: skip usual off mesg */
        dropp(otmp);
    }
    /* rings stay worn even when no hands */
}

staticfn void
drop_weapon(int alone)
{
    struct obj *otmp;
    const char *what, *which, *whichtoo;
    boolean candropwep, candropswapwep, updateinv = TRUE;

    if (uwep) {
        /* !alone check below is currently superfluous but in the
         * future it might not be so if there are monsters which cannot
         * wear gloves but can wield weapons
         */
        if (!alone || cantwield(gy.youmonst.data)) {
            candropwep = canletgo(uwep, "");
            candropswapwep = !u.twoweap || canletgo(uswapwep, "");
            if (alone) {
                what = (candropwep && candropswapwep) ? "бросить" : "отпустить";
                which = is_sword(uwep) ? "меч" : weapon_descr(uwep);
                if (u.twoweap) {
                    whichtoo =
                        is_sword(uswapwep) ? "меч" : weapon_descr(uswapwep);
                    if (strcmp(which, whichtoo))
                        which = "оружие";
                }
                if (uwep->quan != 1L || u.twoweap)
                    which = makeplural(which);

                You("обнаруживаете, что должны %s %s!", what, which);
            }
            /* if either uwep or wielded uswapwep is flagged as 'in_use'
               then don't drop it or explicitly update inventory; leave
               those actions to caller (or caller's caller, &c) */
            if (u.twoweap) {
                otmp = uswapwep;
                uswapwepgone();
                if (otmp->in_use)
                    updateinv = FALSE;
                else if (candropswapwep)
                    dropx(otmp);
            }
            otmp = uwep;
            uwepgone();
            if (otmp->in_use)
                updateinv = FALSE;
            else if (candropwep)
                dropx(otmp);
            /* [note: dropp vs dropx -- if heart of ahriman is wielded, we
               might be losing levitation by dropping it; but that won't
               happen until the drop, unlike Boots_off() dumping hero into
               water and triggering emergency_disrobe() before dropx()] */

            if (updateinv)
                update_inventory();
        } else if (!could_twoweap(gy.youmonst.data)) {
            untwoweapon();
        }
    }
}

/* return to original form, usually either due to polymorph timing out
   or dying from loss of hit points while being polymorphed */
void
rehumanize(void)
{
    boolean was_flying = (Flying != 0);

    /* You can't revert back while unchanging */
    if (Unchanging) {
        if (u.mh < 1) {
            svk.killer.format = NO_KILLER_PREFIX;
            Strcpy(svk.killer.name, "убит, застряв в форме существа");
            done(DIED);
            /* can get to here if declining to die in explore or wizard
               mode; since we're wearing an amulet of unchanging we can't
               be wearing an amulet of life-saving */
            return; /* don't rehumanize after all */
        } else if (uamul && uamul->otyp == AMULET_OF_UNCHANGING) {
            Your("%s %s!", simpleonames(uamul), otense(uamul, "fail"));
            observe_object(uamul);
            makeknown(AMULET_OF_UNCHANGING);
        }
    }

    /*
     * Right now, dying while being a shifted vampire (bat, cloud, wolf)
     * reverts to human rather than to vampire.
     */

    if (emits_light(gy.youmonst.data))
        del_light_source(LS_MONSTER, monst_to_any(&gy.youmonst));
    polyman("Вы возвращаетесь к %s форме!", gu.urace.adj);

    if (u.uhp < 1) {
        /* can only happen if some bit of code reduces u.uhp
           instead of u.mh while poly'd */
        Your("старая форма была недостаточно здорова, чтобы выжить.");
        Sprintf(svk.killer.name, "возврат к нездоровой %s форме",
                gu.urace.adj);
        svk.killer.format = KILLED_BY;
        done(DIED);
    }
    nomul(0);

    disp.botl = TRUE;
    gv.vision_full_recalc = 1;
    encumber_msg();
    update_inventory();
    if (was_flying && !Flying && u.usteed)
        You("и %s мягко возвращаетесь на %s.",
            mon_nam(u.usteed), surface(u.ux, u.uy));
    retouch_equipment(2);
    if (!uarmg)
        selftouch(no_longer_petrify_resistant);
}

int
dobreathe(void)
{
    struct attack *mattk;

    if (Strangled) {
        You_cant("дышать. Простите.");
        return ECMD_OK;
    }
    if (u.uen < 15) {
        You("недостаточно энергии, чтобы дышать!");
        return ECMD_OK;
    }
    u.uen -= 15;
    disp.botl = TRUE;

    if (!getdir((char *) 0))
        return ECMD_CANCEL;

    mattk = attacktype_fordmg(gy.youmonst.data, AT_BREA, AD_ANY);
    if (!mattk)
        impossible("bad breath attack?"); /* mouthwash needed... */
    else if (!u.dx && !u.dy && !u.dz)
        ubreatheu(mattk);
    else
        ubuzz(BZ_U_BREATH(BZ_OFS_AD(mattk->adtyp)), (int) mattk->damn);
    return ECMD_TIME;
}

int
dospit(void)
{
    struct obj *otmp;
    struct attack *mattk;

    if (!getdir((char *) 0))
        return ECMD_CANCEL;
    mattk = attacktype_fordmg(gy.youmonst.data, AT_SPIT, AD_ANY);
    if (!mattk) {
        impossible("bad spit attack?");
    } else {
        switch (mattk->adtyp) {
        case AD_BLND:
        case AD_DRST:
            otmp = mksobj(BLINDING_VENOM, TRUE, FALSE);
            break;
        default:
            impossible("bad attack type in dospit");
            FALLTHROUGH;
            /*FALLTHRU*/
        case AD_ACID:
            otmp = mksobj(ACID_VENOM, TRUE, FALSE);
            break;
        }
        otmp->spe = 1; /* to indicate it's yours */
        throwit(otmp, 0L, FALSE, (struct obj *) 0);
    }
    return ECMD_TIME;
}

int
doremove(void)
{
    if (!Punished) {
        if (u.utrap && u.utraptype == TT_BURIEDBALL) {
            pline("Шар и цепь прочно закопаны в %s.",
                      surface(u.ux, u.uy));
            return ECMD_OK;
        }
        You("не прикованы ни к чему!");
        return ECMD_OK;
    }
    unpunish();
    return ECMD_TIME;
}

int
dospinweb(void)
{
    coordxy x = u.ux, y = u.uy;
    struct trap *ttmp = t_at(x, y);
    /* disallow webs on water, lava, air & cloud */
    boolean reject_terrain = is_pool_or_lava(x, y) || IS_AIR(levl[x][y].typ);

    /* [at the time this was written, it was not possible to be both a
       webmaker and a flyer, but with the advent of amulet of flying that
       became a possibility; at present hero can spin a web while flying] */
    if (Levitation || reject_terrain) {
        You("должны стоять на %s земле, чтобы сплести паутину.",
            reject_terrain ? "твёрдой" : "той");
        return ECMD_OK;
    }
    if (u.uswallow) {
        You("выпускаете паутинную жидкость внутри %s.", mon_nam(u.ustuck));
        if (is_animal(u.ustuck->data)) {
            expels(u.ustuck, u.ustuck->data, TRUE);
            return ECMD_OK;
        }
        if (is_whirly(u.ustuck->data)) {
            int i;

            for (i = 0; i < NATTK; i++)
                if (u.ustuck->data->mattk[i].aatyp == AT_ENGL)
                    break;
            if (i == NATTK)
                impossible("Swallower has no engulfing attack?");
            else {
                char sweep[30];

                sweep[0] = '\0';
                switch (u.ustuck->data->mattk[i].adtyp) {
                case AD_FIRE:
                    Strcpy(sweep, "воспламеняется и ");
                    break;
                case AD_ELEC:
                    Strcpy(sweep, "жарится и ");
                    break;
                case AD_COLD:
                    Strcpy(sweep, "замерзает, разлетается и ");
                    break;
                }
                pline("Паутина %sуносится прочь!", sweep);
            }
            return ECMD_OK;
        } /* default: a nasty jelly-like creature */
        pline("Паутина растворяется в %s.", mon_nam(u.ustuck));
        return ECMD_OK;
    }
    if (u.utrap) {
        You("не можете плести паутину, застряв в ловушке.");
        return ECMD_OK;
    }
    exercise(A_DEX, TRUE);
    if (ttmp) {
        switch (ttmp->ttyp) {
        case PIT:
        case SPIKED_PIT:
            You("плетёте паутину, прикрывая яму.");
            deltrap(ttmp);
            bury_objs(x, y);
            newsym(x, y);
            return ECMD_TIME;
        case SQKY_BOARD:
            pline("Скрипучая доска заглушена.");
            deltrap(ttmp);
            newsym(x, y);
            return ECMD_TIME;
        case TELEP_TRAP:
        case LEVEL_TELEP:
        case MAGIC_PORTAL:
        case VIBRATING_SQUARE:
            Your("паутина исчезает!");
            return ECMD_OK;
        case WEB:
            You("делаете паутину толще.");
            return ECMD_TIME;
        case HOLE:
        case TRAPDOOR:
            You("плетёте паутину над %s.",
                (ttmp->ttyp == TRAPDOOR) ? "люком" : "дырой");
            deltrap(ttmp);
            newsym(x, y);
            return ECMD_TIME;
        case ROLLING_BOULDER_TRAP:
            You("плетёте паутину, заклинивая спусковой механизм.");
            deltrap(ttmp);
            newsym(x, y);
            return ECMD_TIME;
        case ARROW_TRAP:
        case DART_TRAP:
        case BEAR_TRAP:
        case ROCKTRAP:
        case FIRE_TRAP:
        case LANDMINE:
        case SLP_GAS_TRAP:
        case RUST_TRAP:
        case MAGIC_TRAP:
        case ANTI_MAGIC:
        case POLY_TRAP:
            You("активировали ловушку!");
            dotrap(ttmp, NO_TRAP_FLAGS);
            return ECMD_TIME;
        default:
            impossible("Webbing over trap type %d?", ttmp->ttyp);
            return ECMD_OK;
        }
    } else if (On_stairs(x, y)) {
        /* cop out: don't let them hide the stairs */
        Your("паутина не мешает доступу к %s.",
             (levl[x][y].typ == STAIRS) ? "лестнице" : "стремянке");
        return ECMD_TIME;
    }
    ttmp = maketrap(x, y, WEB);
    if (ttmp) {
        You("плетёте паутину.");
        ttmp->madeby_u = 1;
        feeltrap(ttmp);
        if (*in_rooms(x, y, SHOPBASE))
            add_damage(x, y, SHOP_WEB_COST);
    }
    return ECMD_TIME;
}

int
dosummon(void)
{
    int placeholder;
    if (u.uen < 10) {
        You("недостаточно энергии, чтобы послать призыв о помощи!");
        return ECMD_OK;
    }
    u.uen -= 10;
    disp.botl = TRUE;

    You("призываете своих сородичей на помощь!");
    exercise(A_WIS, TRUE);
    if (!were_summon(gy.youmonst.data, TRUE, &placeholder, (char *) 0))
        pline("Но никто не приходит.");
    return ECMD_TIME;
}

int
dogaze(void)
{
    struct monst *mtmp;
    int looked = 0;
    char qbuf[QBUFSZ];
    int i;
    uchar adtyp = 0;

    for (i = 0; i < NATTK; i++) {
        if (gy.youmonst.data->mattk[i].aatyp == AT_GAZE) {
            adtyp = gy.youmonst.data->mattk[i].adtyp;
            break;
        }
    }
    if (adtyp != AD_CONF && adtyp != AD_FIRE) {
        impossible("gaze attack %d?", adtyp);
        return ECMD_OK;
    }

    if (Blind) {
        You_cant("не видите, на что смотреть.");
        return ECMD_OK;
    } else if (Hallucination) {
        You_cant("смотреть на что-либо, что видите.");
        return ECMD_OK;
    }
    if (u.uen < 15) {
        You("недостаточно энергии, чтобы использовать свой особый взгляд!");
        return ECMD_OK;
    }
    u.uen -= 15;
    disp.botl = TRUE;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp))
            continue;
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)) {
            looked++;
            if (Invis && !perceives(mtmp->data)) {
                pline("%s, кажется, не замечает вашего взгляда.", Monnam(mtmp));
            } else if (mtmp->minvis && !See_invisible) {
                You_cant("увидеть, куда смотреть на %s.", Monnam(mtmp));
            } else if (M_AP_TYPE(mtmp) == M_AP_FURNITURE
                       || M_AP_TYPE(mtmp) == M_AP_OBJECT) {
                looked--;
                continue;
            } else if (flags.safe_dog && mtmp->mtame && !Confusion) {
                You("избегаете смотреть на %s.", y_monnam(mtmp));
            } else {
                if (flags.confirm && mtmp->mpeaceful && !Confusion) {
                    Sprintf(qbuf, "Действительно %s %s?",
                            (adtyp == AD_CONF) ? "сбить с толку" : "атаковать",
                            mon_nam(mtmp));
                    if (y_n(qbuf) != 'y')
                        continue;
                }
                setmangry(mtmp, TRUE);
                if (helpless(mtmp) || mtmp->mstun
                    || !mtmp->mcansee || !haseyes(mtmp->data)) {
                    looked--;
                    continue;
                }
                /* No reflection check for consistency with when a monster
                 * gazes at *you*--only medusa gaze gets reflected then.
                 */
                if (adtyp == AD_CONF) {
                    if (!mtmp->mconf)
                        Your("взгляд сбивает с толку %s!", mon_nam(mtmp));
                    else
                        pline("%s становится всё более и более сбитым с толку.",
                              Monnam(mtmp));
                    mtmp->mconf = 1;
                } else if (adtyp == AD_FIRE) {
                    int dmg = d(2, 6), orig_dmg = dmg, lev = (int) u.ulevel;

                    You("атакуете %s огненным взглядом!", mon_nam(mtmp));
                    if (resists_fire(mtmp)) {
                        pline("Огонь не сжигает %s!", mon_nam(mtmp));
                        dmg = 0;
                    }
                    if (lev > rn2(20)) {
                        dmg += destroy_items(mtmp, AD_FIRE, orig_dmg);
                        ignite_items(mtmp->minvent);
                    }
                    if (dmg)
                        mtmp->mhp -= dmg;
                    if (DEADMONSTER(mtmp))
                        killed(mtmp);
                }
                /* For consistency with passive() in uhitm.c, this only
                 * affects you if the monster is still alive.
                 */
                if (DEADMONSTER(mtmp))
                    continue;

                if (mtmp->data == &mons[PM_FLOATING_EYE] && !mtmp->mcan) {
                    if (!Free_action) {
                        You("застываете от взгляда %s!",
                            mon_gen(mtmp, BP_GEN));
                        nomul((u.ulevel > 6 || rn2(4))
                                  ? -d((int) mtmp->m_lev + 1,
                                       (int) mtmp->data->mattk[0].damd)
                                  : -200);
                        gm.multi_reason = "застывший от взгляда монстра";
                        gn.nomovemsg = 0;
                        return ECMD_TIME;
                    } else
                        You("на мгновение деревенеете под взглядом %s.",
                            mon_gen(mtmp, BP_GEN));
                }
                /* Technically this one shouldn't affect you at all because
                 * the Medusa gaze is an active monster attack that only
                 * works on the monster's turn, but for it to *not* have an
                 * effect would be too weird.
                 */
                if (mtmp->data == &mons[PM_MEDUSA] && !mtmp->mcan) {
                    pline("Смотреть на бодрствующего %s не самая лучшая идея.",
                          l_monnam(mtmp));
                    /* as if gazing at a sleeping anything is fruitful... */
                    urgent_pline("Вы превращаетесь в камень...");
                    svk.killer.format = KILLED_BY;
                    Strcpy(svk.killer.name,
                           "намеренно встретившись взглядом с Медузой");
                    done(STONING);
                }
            }
        }
    }
    if (!looked)
        You("смотрите в никуда.");
    return ECMD_TIME;
}

/* called by domonability() for #monster */
int
dohide(void)
{
    boolean ismimic = gy.youmonst.data->mlet == S_MIMIC,
            on_ceiling = is_clinger(gy.youmonst.data) || Flying;

    /* can't hide while being held (or holding) or while trapped
       (except for floor hiders [trapper or mimic] in pits) */
    if (u.ustuck || (u.utrap && (u.utraptype != TT_PIT || on_ceiling))) {
        You_cant("спрятаться, пока вы %s.",
                 !u.ustuck ? "в ловушке"
                   : u.uswallow ? (digests(u.ustuck->data) ? "проглочены"
                                                           : "поглощены")
                     : !sticks(gy.youmonst.data) ? "схвачены"
                       : (humanoid(u.ustuck->data) ? "держите кого-то"
                                                   : "держите этого монстра"));
        if (u.uundetected || (ismimic && U_AP_TYPE != M_AP_NOTHING)) {
            u.uundetected = 0;
            gy.youmonst.m_ap_type = M_AP_NOTHING;
            newsym(u.ux, u.uy);
        }
        return ECMD_OK;
    }
    /* note: hero-as-eel handling is incomplete but unnecessary;
       such critters aren't offered the option of hiding via #monster */
    if (gy.youmonst.data->mlet == S_EEL && !is_pool(u.ux, u.uy)) {
        if (IS_FOUNTAIN(levl[u.ux][u.uy].typ))
            pline("Фонтан недостаточно глубок, чтобы в нём спрятаться.");
        else
            There("нет %s, в котором можно спрятаться.", hliquid("water"));
        u.uundetected = 0;
        return ECMD_OK;
    }
    if (hides_under(gy.youmonst.data)) {
        long ct = 0L;
        struct obj *otmp, *otop = svl.level.objects[u.ux][u.uy];

        if (!otop) {
            There("нет ничего, под чем можно спрятаться.");
            u.uundetected = 0;
            return ECMD_OK;
        }
        for (otmp = otop;
             otmp && otmp->otyp == CORPSE
                  && touch_petrifies(&mons[otmp->corpsenm]);
             otmp = otmp->nexthere)
            ct += otmp->quan;
        /* otmp will be Null iff the entire pile consists of 'trice corpses */
        if (!otmp && !Stone_resistance) {
            char kbuf[BUFSZ];
            const char *corpse_name = cxname(otop);

            /* for the plural case, we'll say "cockatrice corpses" or
               "chickatrice corpses" depending on the top of the pile
               even if both types are present */
            if (ct == 1)
                corpse_name = an(corpse_name);
            /* no need to check poly_when_stoned(); no hide-underers can
               turn into stone golems instead of becoming petrified */
            pline("Прятаться под %s%s — роковая ошибка...",
                  corpse_name, plur(ct));
            Sprintf(kbuf, "прятался под %s%s", corpse_name, plur(ct));
            instapetrify(kbuf);
            /* only reach here if life-saved */
            u.uundetected = 0;
            return ECMD_TIME;
        }
    }
    /* Planes of Air and Water */
    if (on_ceiling && !has_ceiling(&u.uz)) {
        There("нет места, где спрятаться над вами.");
        u.uundetected = 0;
        return ECMD_OK;
    }
    if ((is_hider(gy.youmonst.data) && !Flying) /* floor hider */
        && (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz))) {
        There("нет места, где спрятаться под вами.");
        u.uundetected = 0;
        return ECMD_OK;
    }
    /* TODO? inhibit floor hiding at furniture locations, or
     * else make youhiding() give smarter messages at such spots.
     */

    if (u.uundetected || (ismimic && U_AP_TYPE != M_AP_NOTHING)) {
        youhiding(FALSE, 1); /* "you are already hiding" */
        return ECMD_OK;
    }

    if (ismimic) {
        /* should bring up a dialog "what would you like to imitate?" */
        gy.youmonst.m_ap_type = M_AP_OBJECT;
        gy.youmonst.mappearance = STRANGE_OBJECT;
    } else
        u.uundetected = 1;
    newsym(u.ux, u.uy);
    youhiding(FALSE, 0); /* "you are now hiding" */
    return ECMD_TIME;
}

int
dopoly(void)
{
    struct permonst *savedat = gy.youmonst.data;

    if (is_vampire(gy.youmonst.data) || is_vampshifter(&gy.youmonst)) {
        polyself(POLY_MONSTER);
        if (savedat != gy.youmonst.data) {
            You("превращаетесь в %s.",
                an(pmname(gy.youmonst.data, Ugender)));
            newsym(u.ux, u.uy);
        }
    }
    return ECMD_TIME;
}

/* #monster for hero-as-mind_flayer giving psychic blast */
int
domindblast(void)
{
    struct monst *mtmp, *nmon;
    int dmg;

    if (u.uen < 10) {
        You("сосредотачиваетесь, но для поддержания этого не хватает энергии.");
        return ECMD_OK;
    }
    u.uen -= 10;
    disp.botl = TRUE;

    You("сосредотачиваетесь.");
    pline("Волна психической энергии выплёскивается наружу.");
    for (mtmp = fmon; mtmp; mtmp = nmon) {
        int u_sen;

        nmon = mtmp->nmon;
        if (DEADMONSTER(mtmp))
            continue;
        if (mdistu(mtmp) > BOLT_LIM * BOLT_LIM)
            continue;
        if (mtmp->mpeaceful)
            continue;
        if (mindless(mtmp->data))
            continue;
        u_sen = telepathic(mtmp->data) && !mtmp->mcansee;
        if (u_sen || (telepathic(mtmp->data) && rn2(2)) || !rn2(10)) {
            dmg = rnd(15);
            /* wake it up first, to bring hidden monster out of hiding;
               but in case it is currently peaceful, don't make it hostile
               unless it will survive the psychic blast, otherwise hero
               would avoid the penalty for killing it while peaceful */
            wakeup(mtmp, (dmg > mtmp->mhp) ? TRUE : FALSE);
            You("наводитесь на %s %s.", u_sen ? "телепатию"
                : telepathic(mtmp->data) ? "скрытую телепатию"
                  : "разум", mon_gen(mtmp, BP_GEN));
            mtmp->mhp -= dmg;
            if (DEADMONSTER(mtmp))
                killed(mtmp);
        }
    }
    return ECMD_TIME;
}

void
uunstick(void)
{
    struct monst *mtmp = u.ustuck;

    if (!mtmp) {
        impossible("uunstick: no ustuck?");
        return;
    }
    set_ustuck((struct monst *) 0); /* before pline() */
    pline("%s больше не в ваших тисках.", Monnam(mtmp));
}

void
skinback(boolean silently)
{
    if (uskin) {
        int old_light = arti_light_radius(uskin);

        if (!silently)
            Your("кожа возвращается к своему первоначальному виду.");
        uarm = uskin;
        uskin = (struct obj *) 0;
        /* undo save/restore hack */
        uarm->owornmask &= ~I_SPECIAL;

        if (artifact_light(uarm))
            maybe_adjust_light(uarm, old_light);
    }
}

/* Russian case forms for the nouns used by the *_parts[] arrays below.
   The table is keyed by the nominative form mbodypart() returns, so one entry
   serves every index spelled the same way: ARM and HAND are both "рука",
   FOOT and LEG both "нога", FINGER and TOE both "палец".
   s[] is the singular nom/gen/dat/acc/ins/prep, p[] the plural
   nom/gen/dat/acc/ins/prep.  Russian accusative plural equals nominative
   plural, so p[BP_ACC] is a NULL placeholder that bp_pl_slot[] never selects.
   A NULL means the form doesn't exist and the caller falls back to the
   nominative.  That covers the mass noun "кровь" (no plural); an entry whose
   s[BP_NOM] is NULL is plural-only and takes every case from p[] ("волосы"). */
struct bp_paradigm {
    const char *s[6], *p[6];
};

/* BP_ACC has no p[] slot of its own: plural accusative = plural nominative */
static const int bp_pl_slot[6] = { BP_NOM, BP_GEN, BP_DAT, BP_NOM,
                                   BP_INS, BP_PREP };

static const struct bp_paradigm bp_paradigms[] = {
    /* рука -- ARM, HAND */
    { "рука", "руки", "руке",
      "руку", "рукой", "руке",
      "руки", "рук", "рукам",
      NULL, "руками", "руках" },
    /* глаз -- EYE */
    { "глаз", "глаза", "глазу",
      "глаз", "глазом", "глазе",
      "глаза", "глаз", "глазам",
      NULL, "глазами", "глазах" },
    /* лицо -- FACE */
    { "лицо", "лица", "лицу",
      "лицо", "лицом", "лице",
      "лица", "лиц", "лицам",
      NULL, "лицами", "лицах" },
    /* палец -- FINGER, TOE */
    { "палец", "пальца", "пальцу",
      "палец", "пальцем", "пальце",
      "пальцы", "пальцев", "пальцам",
      NULL, "пальцами", "пальцах" },
    /* кончик пальца -- FINGERTIP */
    { "кончик пальца", "кончика пальца",
      "кончику пальца",
      "кончик пальца", "кончиком пальца",
      "кончике пальца",
      "кончики пальцев", "кончиков пальцев",
      "кончикам пальцев",
      NULL,
      "кончиками пальцев", "кончиках пальцев" },
    /* нога -- FOOT, LEG */
    { "нога", "ноги", "ноге",
      "ногу", "ногой", "ноге",
      "ноги", "ног", "ногам",
      NULL, "ногами", "ногах" },
    /* ладонный -- HANDED (прилагательное) */
    { "ладонный", "ладонного",
      "ладонному",
      "ладонный", "ладонным",
      "ладонном",
      "ладонные", "ладонных", "ладонным",
      NULL, "ладонными", "ладонных" },
    /* голова -- HEAD */
    { "голова", "головы", "голове",
      "голову", "головой", "голове",
      "головы", "голов", "головам",
      NULL, "головами", "головах" },
    /* легкоголовый -- LIGHT_HEADED (прилагательное) */
    { "легкоголовый",
      "легкоголового",
      "легкоголовому",
      "легкоголового", "легкоголовым",
      "легкоголовом",
      "легкоголовые", "легкоголовых",
      "легкоголовым",
      NULL,
      "легкоголовыми", "легкоголовых" },
    /* шея -- NECK */
    { "шея", "шеи", "шее",
      "шею", "шеей", "шее",
      "шеи", "шей", "шеям",
      NULL, "шеями", "шеях" },
    /* позвоночник -- SPINE */
    { "позвоночник",
      "позвоночника",
      "позвоночнику",
      "позвоночник", "позвоночником",
      "позвоночнике",
      "позвоночники", "позвоночников",
      "позвоночникам",
      NULL,
      "позвоночниками", "позвоночниках" },
    /* волосы -- HAIR (только множественное) */
    { NULL, NULL, NULL,
      NULL, NULL, NULL,
      "волосы", "волос", "волосам",
      NULL, "волосами", "волосах" },
    /* кровь -- BLOOD (без множественного) */
    { "кровь", "крови", "крови",
      "кровь", "кровью", "крови",
      NULL, NULL, NULL,
      NULL, NULL, NULL },
    /* лёгкое -- LUNG */
    { "лёгкое", "лёгкого", "лёгкому",
      "лёгкое", "лёгким", "лёгком",
      "лёгкие", "лёгких", "лёгким",
      NULL, "лёгкими", "лёгких" },
    /* нос -- NOSE */
    { "нос", "носа", "носу",
      "нос", "носом", "носу",
      "носы", "носов", "носам",
      NULL, "носами", "носах" },
    /* желудок -- STOMACH */
    { "желудок", "желудка",
      "желудку",
      "желудок", "желудком",
      "желудке",
      "желудки", "желудков", "желудкам",
      NULL, "желудками", "желудках" },
    /* передняя лапа -- animal ARM(0)/HAND(6)-замена */
    { "передняя лапа", "передней лапы", "передней лапе",
      "переднюю лапу", "передней лапой", "передней лапе",
      "передние лапы", "передних лап", "передним лапам",
      NULL, "передними лапами", "передних лапах" },
    /* морда -- animal FACE (зверь, не лицо) */
    { "морда", "морды", "морде",
      "морду", "мордой", "морде",
      "морды", "морд", "мордам",
      NULL, "мордами", "мордах" },
    /* передняя когтистая лапа -- animal FINGER(3)/CLAWED(6) */
    { "передняя когтистая лапа",
      "передней когтистой лапы",
      "передней когтистой лапе",
      "переднюю когтистую лапу",
      "передней когтистой лапой",
      "передней когтистой лапе",
      "передние когтистые лапы",
      "передних когтистых лап",
      "передним когтистым лапам",
      NULL,
      "передними когтистыми лапами",
      "передних когтистых лапах" },
    /* кончик когтя -- animal FINGERTIP(4) */
    { "кончик когтя", "кончика когтя",
      "кончику когтя",
      "кончик когтя", "кончиком когтя",
      "кончике когтя",
      "кончики когтей", "кончиков когтей",
      "кончикам когтей",
      NULL,
      "кончиками когтей", "кончиках когтей" },
    /* задняя когтистая лапа -- animal CLAW_TIP(5) */
    { "задняя когтистая лапа",
      "задней когтистой лапы",
      "задней когтистой лапе",
      "заднюю когтистую лапу",
      "задней когтистой лапой",
      "задней когтистой лапе",
      "задние когтистые лапы",
      "задних когтистых лап",
      "задним когтистым лапам",
      NULL,
      "задними когтистыми лапами",
      "задних когтистых лапах" },
    /* когтистая -- animal CLAWED(7), прилагательное ж.р. */
    { "когтистая", "когтистой",
      "когтистой",
      "когтистую", "когтистой",
      "когтистой",
      "когтистые", "когтистых", "когтистым",
      NULL, "когтистыми", "когтистых" },
    /* задняя лапа -- animal LEG(9)-замена */
    { "задняя лапа", "задней лапы", "задней лапе",
      "заднюю лапу", "задней лапой", "задней лапе",
      "задние лапы", "задних лап", "задним лапам",
      NULL, "задними лапами", "задних лапах" },
    /* кончик заднего когтя -- animal REAR_CLAW_TIP(13) */
    { "кончик заднего когтя",
      "кончика заднего когтя",
      "кончику заднего когтя",
      "кончик заднего когтя",
      "кончиком заднего когтя",
      "кончике заднего когтя",
      "кончики задних когтей",
      "кончиков задних когтей",
      "кончикам задних когтей",
      NULL,
      "кончиками задних когтей",
      "кончиках задних когтей" },
    /* шерсть -- animal FUR(14), как кровь: без мн.ч. */
    { "шерсть", "шерсти", "шерсти",
      "шерсть", "шерстью", "шерсти",
      NULL, NULL, NULL,
      NULL, NULL, NULL },
    /* крыло -- bird ARM(0)/HAND(3)/HAND(6) */
    { "крыло", "крыла", "крылу",
      "крыло", "крылом", "крыле",
      "крылья", "крыльев", "крыльям",
      NULL, "крыльями", "крыльях" },
    /* кончик крыла -- bird FINGERTIP(4) */
    { "кончик крыла", "кончика крыла",
      "кончику крыла",
      "кончик крыла", "кончиком крыла",
      "кончике крыла",
      "кончики крыльев", "кончиков крыльев",
      "кончикам крыльев",
      NULL,
      "кончиками крыльев",
      "кончиках крыльев" },
    /* окрылая -- bird WINGED(7), прилагательное ж.р. */
    { "окрылая", "окрылой",
      "окрылой",
      "окрылую", "окрылой",
      "окрылой",
      "окрылые", "окрылых", "окрылым",
      NULL, "окрылыми", "окрылых" },
    /* перья -- bird HAIR(14), только мн.ч. */
    { NULL, NULL, NULL,
      NULL, NULL, NULL,
      "перья", "перьев", "перьям",
      NULL, "перьями", "перьях" },
    /* клюв -- bird BEAK/NOSE(16) */
    { "клюв", "клюва", "клюву",
      "клюв", "клювом", "клюве",
      "клювы", "клювов", "клювам",
      NULL, "клювами", "клювах" },
    /* передняя нога -- horse ARM(0) */
    { "передняя нога", "передней ноги", "передней ноге",
      "переднюю ногу", "передней ногой", "передней ноге",
      "передние ноги", "передних ног", "передним ногам",
      NULL, "передними ногами", "передних ногах" },
    /* переднее копыто -- horse FINGER(3)/FOREHOOF(6) */
    { "переднее копыто", "переднего копыта",
      "переднему копыту",
      "переднее копыто", "передним копытом",
      "переднем копыте",
      "передние копыта", "передних копыт",
      "передним копытам",
      NULL,
      "передними копытами", "передних копытах" },
    /* кончик копыта -- horse FINGERTIP(4) */
    { "кончик копыта", "кончика копыта",
      "кончику копыта",
      "кончик копыта", "кончиком копыта",
      "кончике копыта",
      "кончики копыт", "кончиков копыт",
      "кончикам копыт",
      NULL,
      "кончиками копыт", "кончиках копыт" },
    /* заднее копыто -- horse CLAW_TIP(5) */
    { "заднее копыто", "заднего копыта",
      "заднему копыту",
      "заднее копыто", "задним копытом",
      "заднем копыте",
      "задние копыта", "задних копыт",
      "задним копытам",
      NULL,
      "задними копытами", "задних копытах" },
    /* копытная -- horse HOOVED(7), прилагательное ж.р. */
    { "копытная", "копытной",
      "копытной",
      "копытную", "копытной",
      "копытной",
      "копытные", "копытных", "копытным",
      NULL, "копытными", "копытных" },
    /* задняя нога -- horse LEG(9) */
    { "задняя нога", "задней ноги", "задней ноге",
      "заднюю ногу", "задней ногой", "задней ноге",
      "задние ноги", "задних ног", "задним ногам",
      NULL, "задними ногами", "задних ногах" },
    /* кончик заднего копыта -- horse REAR_HOOF_TIP(13) */
    { "кончик заднего копыта",
      "кончика заднего копыта",
      "кончику заднего копыта",
      "кончик заднего копыта",
      "кончиком заднего копыта",
      "кончике заднего копыта",
      "кончики задних копыт",
      "кончиков задних копыт",
      "кончикам задних копыт",
      NULL,
      "кончиками задних копыт",
      "кончиках задних копыт" },
    /* грива -- horse MANE(14) */
    { "грива", "гривы", "гриве",
      "гриву", "гривой", "гриве",
      "гривы", "грив", "гривам",
      NULL, "гривами", "гривах" },
    /* псевдоподия -- jelly ARM(0) */
    { "псевдоподия", "псевдоподии", "псевдоподии",
      "псевдоподию", "псевдоподией", "псевдоподии",
      "псевдоподии", "псевдоподий", "псевдоподиям",
      NULL, "псевдоподиями", "псевдоподиях" },
    /* пятно -- jelly EYE(1), тёмность не выводится из кода */
    { "пятно", "пятна", "пятну",
      "пятно", "пятном", "пятне",
      "пятна", "пятен", "пятнам",
      NULL, "пятнами", "пятнах" },
    /* передняя часть -- jelly FACE(2) */
    { "передняя часть", "передней части",
      "передней части",
      "переднюю часть", "передней частью",
      "передней части",
      "передние части", "передних частей",
      "передним частям",
      NULL,
      "передними частями", "передних частях" },
    /* отросток псевдоподии -- jelly FINGER(3) */
    { "отросток псевдоподии",
      "отростка псевдоподии",
      "отростку псевдоподии",
      "отросток псевдоподии",
      "отростком псевдоподии",
      "отростке псевдоподии",
      "отростки псевдоподий",
      "отростков псевдоподий",
      "отросткам псевдоподий",
      NULL,
      "отростками псевдоподий",
      "отростках псевдоподий" },
    /* конец псевдоподии -- jelly FINGERTIP(4)/FOOT(13) */
    { "конец псевдоподии",
      "конца псевдоподии",
      "концу псевдоподии",
      "конец псевдоподии",
      "концом псевдоподии",
      "конце псевдоподии",
      "концы псевдоподий",
      "концов псевдоподий",
      "концам псевдоподий",
      NULL,
      "концами псевдоподий",
      "концах псевдоподий" },
    /* основа псевдоподии -- jelly CLAW_TIP(5) */
    { "основа псевдоподии",
      "основы псевдоподии",
      "основе псевдоподии",
      "основу псевдоподии",
      "основой псевдоподии",
      "основе псевдоподии",
      "основы псевдоподий",
      "основ псевдоподий",
      "основам псевдоподий",
      NULL,
      "основами псевдоподий",
      "основах псевдоподий" },
    /* захват -- jelly HAND(6) */
    { "захват", "захвата", "захвату",
      "захват", "захватом", "захвате",
      "захваты", "захватов", "захватам",
      NULL, "захватами", "захватах" },
    /* схваченное -- jelly CLAWED(7), прилагательное ср.р. */
    { "схваченное", "схваченного",
      "схваченному",
      "схваченное", "схваченным",
      "схваченном",
      "схваченные", "схваченных", "схваченным",
      NULL, "схваченными", "схваченных" },
    /* мозговая область -- jelly HEAD(8) */
    { "мозговая область", "мозговой области",
      "мозговой области",
      "мозговую область", "мозговой областью",
      "мозговой области",
      "мозговые области", "мозговых областей",
      "мозговым областям",
      NULL,
      "мозговыми областями", "мозговых областях" },
    /* нижняя псевдоподия -- jelly LEG(9) */
    { "нижняя псевдоподия", "нижней псевдоподии",
      "нижней псевдоподии",
      "нижнюю псевдоподию", "нижней псевдоподией",
      "нижней псевдоподии",
      "нижние псевдоподии", "нижних псевдоподий",
      "нижним псевдоподиям",
      NULL,
      "нижними псевдоподиями", "нижних псевдоподиях" },
    /* вязкая -- jelly LIGHT_HEADED(10), прилагательное ж.р. */
    { "вязкая", "вязкой",
      "вязкой",
      "вязкую", "вязкой",
      "вязкой",
      "вязкие", "вязких", "вязким",
      NULL, "вязкими", "вязких" },
    /* середина -- jelly NECK(11) */
    { "середина", "середины", "середине",
      "середину", "серединой", "середине",
      "середины", "середин", "серединам",
      NULL, "серединами", "серединах" },
    /* поверхность -- jelly SPINE(12)/HAIR(16) */
    { "поверхность", "поверхности", "поверхности",
      "поверхность", "поверхностью", "поверхности",
      "поверхности", "поверхностей", "поверхностям",
      NULL, "поверхностями", "поверхностях" },
    /* рябь -- jelly TAIL(14), ед.ч. ж.р., мн.ч. нет */
    { "рябь", "ряби", "ряби",
      "рябь", "рябью", "ряби",
      NULL, NULL, NULL,
      NULL, NULL, NULL },
    /* соки -- jelly BLOOD(15), только мн.ч. */
    { NULL, NULL, NULL,
      NULL, NULL, NULL,
      "соки", "соков", "сокам",
      NULL, "соками", "соках" },
    /* сенсор -- jelly NOSE(17) */
    { "сенсор", "сенсора", "сенсору",
      "сенсор", "сенсором", "сенсоре",
      "сенсоры", "сенсоров", "сенсорам",
      NULL, "сенсорами", "сенсорах" },
    /* отросток -- sphere ARM(0), не путать с jelly "отросток псевдоподии" */
    { "отросток", "отростка", "отростку",
      "отросток", "отростком", "отростке",
      "отростки", "отростков", "отросткам",
      NULL, "отростками", "отростках" },
    /* зрительный нерв -- sphere EYE(1) */
    { "зрительный нерв", "зрительного нерва",
      "зрительному нерву",
      "зрительный нерв", "зрительным нервом",
      "зрительном нерве",
      "зрительные нервы", "зрительных нервов",
      "зрительным нервам",
      NULL,
      "зрительными нервами", "зрительных нервах" },
    /* тело -- sphere FACE(2)/HEAD(8)/SPINE(12) */
    { "тело", "тела", "телу",
      "тело", "телом", "теле",
      "тела", "тел", "телам",
      NULL, "телами", "телах" },
    /* щупальце -- sphere FINGER(3)/HAND(6) */
    { "щупальце", "щупальца", "щупальцу",
      "щупальце", "щупальцем", "щупальце",
      "щупальца", "щупалец", "щупальцам",
      NULL, "щупальцами", "щупальцах" },
    /* конец щупальца -- sphere FINGERTIP(4) */
    { "конец щупальца", "конца щупальца",
      "концу щупальца",
      "конец щупальца", "концом щупальца",
      "конце щупальца",
      "концы щупалец", "концов щупалец",
      "концам щупалец",
      NULL,
      "концами щупалец", "концах щупалец" },
    /* нижний отросток -- sphere CLAW_TIP(5) */
    { "нижний отросток", "нижнего отростка",
      "нижнему отростку",
      "нижний отросток", "нижним отростком",
      "нижнем отростке",
      "нижние отростки", "нижних отростков",
      "нижним отросткам",
      NULL,
      "нижними отростками", "нижних отростках" },
    /* щупальценосная -- sphere CLAWED(7), прилагательное ж.р. */
    { "щупальценосная", "щупальценосной",
      "щупальценосному",
      "щупальценосную", "щупальценосным",
      "щупальценосном",
      "щупальценосные", "щупальценосных",
      "щупальценосным",
      NULL,
      "щупальценосными", "щупальценосных" },
    /* нижнее щупальце -- sphere LEG(9) */
    { "нижнее щупальце", "нижнего щупальца",
      "нижнему щупальцу",
      "нижнее щупальце", "нижним щупальцем",
      "нижнем щупальце",
      "нижние щупальца", "нижних щупалец",
      "нижним щупальцам",
      NULL,
      "нижними щупальцами", "нижних щупальцах" },
    /* вращательная -- sphere LIGHT_HEADED(10), прилагательное ж.р. */
    { "вращательная", "вращательной",
      "вращательной",
      "вращательную", "вращательной",
      "вращательной",
      "вращательные", "вращательных",
      "вращательным",
      NULL, "вращательными", "вращательных" },
    /* экватор -- sphere NECK(11) */
    { "экватор", "экватора", "экватору",
      "экватор", "экватором", "экваторе",
      "экваторы", "экваторов", "экваторам",
      NULL, "экваторами", "экваторах" },
    /* конец нижнего щупальца -- sphere TAIL(13) */
    { "конец нижнего щупальца",
      "конца нижнего щупальца",
      "концу нижнего щупальца",
      "конец нижнего щупальца",
      "концом нижнего щупальца",
      "конце нижнего щупальца",
      "концы нижних щупалец",
      "концов нижних щупалец",
      "концам нижних щупалец",
      NULL,
      "концами нижних щупалец",
      "концах нижних щупалец" },
    /* реснички -- sphere BLOOD(14), только мн.ч. */
    { NULL, NULL, NULL,
      NULL, NULL, NULL,
      "реснички", "ресничек", "ресничкам",
      NULL, "ресничками", "ресничках" },
    /* сила жизни -- sphere LUNG(15) */
    { "сила жизни", "силы жизни", "силе жизни",
      "силу жизни", "силой жизни", "силе жизни",
      "силы жизни", "сил жизни", "силам жизни",
      NULL, "силами жизни", "силах жизни" },
    /* сетчатка -- sphere NOSE(16) */
    { "сетчатка", "сетчатки", "сетчатке",
      "сетчатку", "сетчаткой", "сетчатке",
      "сетчатки", "сетчаток", "сетчаткам",
      NULL, "сетчатками", "сетчатках" },
    /* обонятельный нерв -- sphere STOMACH(17) */
    { "обонятельный нерв", "обонятельного нерва",
      "обонятельному нерву",
      "обонятельный нерв", "обонятельным нервом",
      "обонятельном нерве",
      "обонятельные нервы", "обонятельных нервов",
      "обонятельным нервам",
      NULL,
      "обонятельными нервами", "обонятельных нервах" },
    /* внутренность -- sphere INTERIOR(18) */
    { "внутренность", "внутренности", "внутренности",
      "внутренность", "внутренностью", "внутренности",
      "внутренности", "внутренностей", "внутренностям",
      NULL, "внутренностями", "внутренностях" },
    /* мицелий -- fungus ARM(0) */
    { "мицелий", "мицелия", "мицелию",
      "мицелий", "мицелием", "мицелии",
      "мицелии", "мицелиев", "мицелиям",
      NULL, "мицелиями", "мицелиях" },
    /* зрительная область -- fungus EYE(1) */
    { "зрительная область", "зрительной области",
      "зрительной области",
      "зрительную область", "зрительной областью",
      "зрительной области",
      "зрительные области", "зрительных областей",
      "зрительным областям",
      NULL,
      "зрительными областями", "зрительных областях" },
    /* гифа -- fungus FINGER(3)/CLAW_TIP(4) */
    { "гифа", "гифы", "гифе",
      "гифу", "гифой", "гифе",
      "гифы", "гиф", "гифам",
      NULL, "гифами", "гифах" },
    /* корень -- fungus CLAWED(5)/LEG(12) */
    { "корень", "корня", "корню",
      "корень", "корнем", "корне",
      "корни", "корней", "корням",
      NULL, "корнями", "корнях" },
    /* нить -- fungus HAND(6) */
    { "нить", "нити", "нити",
      "нить", "нитью", "нити",
      "нити", "нитей", "нитям",
      NULL, "нитями", "нитях" },
    /* нитевидная -- fungus HEAD(7), прилагательное ж.р. */
    { "нитевидная", "нитевидной",
      "нитевидному",
      "нитевидную", "нитевидным",
      "нитевидном",
      "нитевидные", "нитевидных",
      "нитевидным",
      NULL,
      "нитевидными", "нитевидных" },
    /* область шляпки -- fungus SPINE(8) */
    { "область шляпки", "области шляпки",
      "области шляпки",
      "область шляпки", "областью шляпки",
      "области шляпки",
      "области шляпок", "областей шляпок",
      "областям шляпок",
      NULL,
      "областями шляпок", "областях шляпок" },
    /* ризома -- fungus LEG(9) */
    { "ризома", "ризомы", "ризоме",
      "ризому", "ризомой", "ризоме",
      "ризомы", "ризом", "ризомам",
      NULL, "ризомами", "ризомах" },
    /* спороносная -- fungus LIGHT_HEADED(10), прилагательное ж.р. */
    { "спороносная", "спороносной",
      "спороносному",
      "спороносную", "спороносным",
      "спороносном",
      "спороносные", "спороносных",
      "спороносным",
      NULL,
      "спороносными", "спороносных" },
    /* ножка -- fungus NECK(11) */
    { "ножка", "ножки", "ножке",
      "ножку", "ножкой", "ножке",
      "ножки", "ножек", "ножкам",
      NULL, "ножками", "ножках" },
    /* конец ризомы -- fungus TAIL(13) */
    { "конец ризомы", "конца ризомы",
      "концу ризомы",
      "конец ризомы", "концом ризомы",
      "конце ризомы",
      "концы ризом", "концов ризом",
      "концам ризом",
      NULL,
      "концами ризом", "концах ризом" },
    /* споры -- fungus BLOOD(14), только мн.ч. */
    { NULL, NULL, NULL,
      NULL, NULL, NULL,
      "споры", "спор", "спорам",
      NULL, "спорами", "спорах" },
    /* пластинка -- fungus NOSE(16)/STOMACH(17) */
    { "пластинка", "пластинки", "пластинке",
      "пластинку", "пластинкой", "пластинке",
      "пластинки", "пластинок", "пластинкам",
      NULL, "пластинками", "пластинках" },
    /* область -- vortex ARM(0) */
    { "область", "области", "области",
      "область", "областью", "области",
      "области", "областей", "областям",
      NULL, "областями", "областях" },
    /* малый поток -- vortex FINGER(3)/CLAW_TIP(4) */
    { "малый поток", "малого потока",
      "малому потоку",
      "малый поток", "малым потоком",
      "малом потоке",
      "малые потоки", "малых потоков",
      "малым потокам",
      NULL,
      "малыми потоками", "малых потоках" },
    /* нижний поток -- vortex LEG(5)/SPINE(9) */
    { "нижний поток", "нижнего потока",
      "нижнему потоку",
      "нижний поток", "нижним потоком",
      "нижнем потоке",
      "нижние потоки", "нижних потоков",
      "нижним потокам",
      NULL,
      "нижними потоками", "нижних потоках" },
    /* завихрение -- vortex CLAWED(6) */
    { "завихрение", "завихрения", "завихрению",
      "завихрение", "завихрением", "завихрении",
      "завихрения", "завихрений", "завихрениям",
      NULL, "завихрениями", "завихрениях" },
    /* завихрённая -- vortex HEAD(7), прилагательное ж.р. */
    { "завихрённая", "завихрённой",
      "завихрённому",
      "завихрённую", "завихрённым",
      "завихрённом",
      "завихрённые", "завихрённых",
      "завихрённым",
      NULL,
      "завихрёнными", "завихрённых" },
    /* центральное ядро -- vortex LIGHT_HEADED(8) */
    { "центральное ядро", "центрального ядра",
      "центральному ядру",
      "центральное ядро", "центральным ядром",
      "центральном ядре",
      "центральные ядра", "центральных ядер",
      "центральным ядрам",
      NULL,
      "центральными ядрами", "центральных ядрах" },
    /* одурманённая -- vortex NECK(10), прилагательное ж.р. */
    { "одурманённая", "одурманённой",
      "одурманённому",
      "одурманённую", "одурманённым",
      "одурманённом",
      "одурманённые", "одурманённых",
      "одурманённым",
      NULL,
      "одурманёнными", "одурманённых" },
    /* потоки -- vortex BLOOD(12)/HAIR(14), обычная парадигма */
    { "потоки", "потоков", "потокам",
      "потоки", "потоками", "потоках",
      "потоки", "потоков", "потокам",
      NULL, "потоками", "потоках" },
    /* центр -- vortex SPINE(11)/LUNG(16) */
    { "центр", "центра", "центру",
      "центр", "центром", "центре",
      "центры", "центров", "центрам",
      NULL, "центрами", "центрах" },
    /* край -- vortex NOSE(13) */
    { "край", "края", "краю",
      "край", "краем", "крае",
      "края", "краёв", "краям",
      NULL, "краями", "краях" },
    /* передний край -- vortex TAIL(17) */
    { "передний край", "переднего края",
      "переднему краю",
      "передний край", "передним краем",
      "переднем крае",
      "передние края", "передних краёв",
      "передним краям",
      NULL,
      "передними краями", "передних краях" },
    /* рудимент конечности -- snake ARM(0) */
    { "рудимент конечности",
      "рудимента конечности",
      "рудименту конечности",
      "рудимент конечности",
      "рудиментом конечности",
      "рудименте конечности",
      "рудименты конечностей",
      "рудиментов конечностей",
      "рудиментам конечностей",
      NULL,
      "рудиментами конечностей",
      "рудиментах конечностей" },
    /* крупная чешуя -- snake FINGERTIP(3) */
    { "крупная чешуя", "крупной чешуи",
      "крупной чешуе",
      "крупную чешую", "крупной чешуёй",
      "крупной чешуе",
      "крупные чешуи", "крупных чешуй",
      "крупным чешуям",
      NULL,
      "крупными чешуями", "крупных чешуях" },
    /* конец крупной чешуи -- snake CLAW_TIP(4) */
    { "конец крупной чешуи",
      "конца крупной чешуи",
      "концу крупной чешуи",
      "конец крупной чешуи",
      "концом крупной чешуи",
      "конце крупной чешуи",
      "концы крупных чешуй",
      "концов крупных чешуй",
      "концам крупных чешуй",
      NULL,
      "концами крупных чешуй",
      "концах крупных чешуй" },
    /* задняя область -- snake CLAWED(5)/LEG(9) */
    { "задняя область", "задней области",
      "задней области",
      "заднюю область", "задней областью",
      "задней области",
      "задние области", "задних областей",
      "задним областям",
      NULL,
      "задними областями", "задних областях" },
    /* чешуйная щель -- snake HAND(6) */
    { "чешуйная щель", "чешуйной щели",
      "чешуйной щели",
      "чешуйную щель", "чешуйной щелью",
      "чешуйной щели",
      "чешуйные щели", "чешуйных щелей",
      "чешуйным щелям",
      NULL,
      "чешуйными щелями", "чешуйных щелях" },
    /* чешуйчатая -- snake HEAD(7), прилагательное ж.р. */
    { "чешуйчатая", "чешуйчатой",
      "чешуйчатому",
      "чешуйчатую", "чешуйчатой",
      "чешуйчатой",
      "чешуйчатые", "чешуйчатых",
      "чешуйчатым",
      NULL, "чешуйчатыми", "чешуйчатых" },
    /* длина -- snake NECK(12) */
    { "длина", "длины", "длине",
      "длину", "длиной", "длине",
      "длины", "длин", "длинам",
      NULL, "длинами", "длинах" },
    /* задняя чешуя -- snake TAIL(13) */
    { "задняя чешуя", "задней чешуи",
      "задней чешуе",
      "заднюю чешую", "задней чешуёй",
      "задней чешуе",
      "задние чешуи", "задних чешуй",
      "задним чешуям",
      NULL,
      "задними чешуями", "задних чешуях" },
    /* чешуи -- snake BLOOD(14), чередование чешуя/чешуи/чешуёй */
    { "чешуи", "чешуй", "чешуям",
      "чешуи", "чешуёй", "чешуе",
      "чешуи", "чешуй", "чешуям",
      NULL, "чешуями", "чешуях" },
    /* раздвоенный язык -- snake NOSE(17) */
    { "раздвоенный язык", "раздвоенного языка",
      "раздвоенному языку",
      "раздвоенный язык", "раздвоенным языком",
      "раздвоенном языке",
      "раздвоенные языки", "раздвоенных языков",
      "раздвоенным языкам",
      NULL,
      "раздвоенными языками", "раздвоенных языках" },
    /* передний сегмент -- worm ARM(0)/HEAD(8) */
    { "передний сегмент", "переднего сегмента",
      "переднему сегменту",
      "передний сегмент", "передним сегментом",
      "переднем сегменте",
      "передние сегменты", "передних сегментов",
      "передним сегментам",
      NULL,
      "передними сегментами", "передних сегментах" },
    /* светочувствительная клетка -- worm EYE(1) */
    { "светочувствительная клетка",
      "светочувствительной клетки",
      "светочувствительной клетке",
      "светочувствительную клетку",
      "светочувствительной клеткой",
      "светочувствительной клетке",
      "светочувствительные клетки",
      "светочувствительных клеток",
      "светочувствительным клеткам",
      NULL,
      "светочувствительными клетками",
      "светочувствительных клетках" },
    /* поясок -- worm FACE(2)/NECK(11), рус. clitellum */
    { "поясок", "пояска", "пояску",
      "поясок", "пояском", "пояске",
      "пояски", "поясков", "пояскам",
      NULL, "поясками", "поясках" },
    /* щетинки -- worm FINGER(3)/CLAW_TIP(4)/BLOOD(14) */
    { "щетинки", "щетинок", "щетинкам",
      "щетинку", "щетинкой", "щетинке",
      "щетинки", "щетинок", "щетинкам",
      NULL, "щетинками", "щетинках" },
    /* задний сегмент -- worm LEG(5) */
    { "задний сегмент", "заднего сегмента",
      "заднему сегменту",
      "задний сегмент", "задним сегментом",
      "заднем сегменте",
      "задние сегменты", "задних сегментов",
      "задним сегментам",
      NULL,
      "задними сегментами", "задних сегментах" },
    /* сегмент -- worm HAND(6) */
    { "сегмент", "сегмента", "сегменту",
      "сегмент", "сегментом", "сегменте",
      "сегменты", "сегментов", "сегментам",
      NULL, "сегментами", "сегментах" },
    /* членистая -- worm CLAWED(7), прилагательное ж.р. */
    { "членистая", "членистой",
      "членистому",
      "членистую", "членистой",
      "членистой",
      "членистые", "членистых",
      "членистым",
      NULL, "членистыми", "членистых" },
    /* задняя часть -- worm LIGHT_HEADED(9), не путать с "задняя область" */
    { "задняя часть", "задней части",
      "задней части",
      "заднюю часть", "задней частью",
      "задней части",
      "задние части", "задних частей",
      "задним частям",
      NULL,
      "задними частями", "задних частях" },
    /* перерастянутая -- worm SPINE(10), прилагательное ж.р. */
    { "перерастянутая", "перерастянутой",
      "перерастянутому",
      "перерастянутую", "перерастянутым",
      "перерастянутом",
      "перерастянутые", "перерастянутых",
      "перерастянутым",
      NULL,
      "перерастянутыми", "перерастянутых" },
    /* задние щетинки -- worm TAIL(13) */
    { "задние щетинки", "задних щетинок",
      "задним щетинкам",
      "задние щетинки", "задними щетинками",
      "задних щетинках",
      "задние щетинки", "задних щетинок",
      "задним щетинкам",
      NULL,
      "задними щетинками", "задних щетинках" },
    /* кожа -- worm LUNG(16) */
    { "кожа", "кожи", "коже",
      "кожу", "кожей", "коже",
      "кожи", "кож", "кожам",
      NULL, "кожами", "кожах" },
    /* головная лопасть -- worm NOSE(17), рус. prostomium */
    { "головная лопасть", "головной лопасти",
      "головной лопасти",
      "головную лопасть", "головной лопастью",
      "головной лопасти",
      "головные лопасти", "головных лопастей",
      "головным лопастям",
      NULL,
      "головными лопастями", "головных лопастях" },
    /* педипальп -- spider ARM(0)/FINGER(3)/HAND(6) */
    { "педипальп", "педипальпа", "педипальпу",
      "педипальп", "педипальпом", "педипальпе",
      "педипальпы", "педипальпов", "педипальпам",
      NULL, "педипальпами", "педипальпах" },
    /* тарзус -- spider CLAW_TIP(4) */
    { "тарзус", "тарзуса", "тарзусу",
      "тарзус", "тарзусом", "тарзусе",
      "тарзусы", "тарзусов", "тарзусам",
      NULL, "тарзусами", "тарзусах" },
    /* коготь -- spider CLAWED(5)/TAIL(13), чередование когтем */
    { "коготь", "когтя", "когтю",
      "коготь", "когтем", "когте",
      "когти", "когтей", "когтям",
      NULL, "когтями", "когтях" },
    /* осязаемая -- spider HEAD(7), прилагательное ж.р. */
    { "осязаемая", "осязаемой",
      "осязаемому",
      "осязаемую", "осязаемым",
      "осязаемом",
      "осязаемые", "осязаемых",
      "осязаемым",
      NULL, "осязаемыми", "осязаемых" },
    /* головогрудь -- spider SPINE(8)/NECK(11) */
    { "головогрудь", "головогруди", "головогруди",
      "головогрудь", "головогрудью", "головогруди",
      "головогруди", "головогрудей", "головогрудям",
      NULL, "головогрудями", "головогрудях" },
    /* спрядённая -- spider LIGHT_HEADED(10), прилагательное ж.р. */
    { "спрядённая", "спрядённой",
      "спрядённому",
      "спрядённую", "спрядённым",
      "спрядённом",
      "спрядённые", "спрядённых",
      "спрядённым",
      NULL, "спрядёнными", "спрядённых" },
    /* брюшко -- spider HAIR(12), средний род */
    { "брюшко", "брюшка", "брюшку",
      "брюшко", "брюшком", "брюшке",
      "брюшки", "брюшек", "брюшкам",
      NULL, "брюшками", "брюшках" },
    /* гемолимфа -- spider BLOOD(15) */
    { "гемолимфа", "гемолимфы", "гемолимфе",
      "гемолимфу", "гемолимфой", "гемолимфе",
      "гемолимфы", "гемолимф", "гемолимфам",
      NULL, "гемолимфами", "гемолимфах" },
    /* книжное лёгкое -- spider LUNG(16) */
    { "книжное лёгкое", "книжного лёгкого",
      "книжному лёгкому",
      "книжное лёгкое", "книжным лёгким",
      "книжном лёгком",
      "книжные лёгкие", "книжных лёгких",
      "книжным лёгким",
      NULL,
      "книжными лёгкими", "книжных лёгких" },
    /* верхняя губа -- spider NOSE(17), рус. labrum */
    { "верхняя губа", "верхней губы",
      "верхней губе",
      "верхнюю губу", "верхней губой",
      "верхней губе",
      "верхние губы", "верхних губ",
      "верхним губам",
      NULL,
      "верхними губами", "верхних губах" },
    /* пищеварительный тракт -- spider STOMACH(18) */
    { "пищеварительный тракт",
      "пищеварительного тракта",
      "пищеварительному тракту",
      "пищеварительный тракт",
      "пищеварительным трактом",
      "пищеварительном тракте",
      "пищеварительные тракты",
      "пищеварительных трактов",
      "пищеварительным трактам",
      NULL,
      "пищеварительными трактами",
      "пищеварительных трактах" },
    /* плавник -- fish ARM(0) */
    { "плавник", "плавника", "плавнику",
      "плавник", "плавником", "плавнике",
      "плавники", "плавников", "плавникам",
      NULL, "плавниками", "плавниках" },
    /* предчелюстная -- fish FACE(2), прилагательное ж.р. */
    { "предчелюстная", "предчелюстной",
      "предчелюстному",
      "предчелюстную", "предчелюстным",
      "предчелюстном",
      "предчелюстные", "предчелюстных",
      "предчелюстным",
      NULL,
      "предчелюстными", "предчелюстных" },
    /* тазовая подмышечная -- fish CLAW_TIP(3) */
    { "тазовая подмышечная",
      "тазовой подмышечной",
      "тазовой подмышечной",
      "тазовую подмышечную",
      "тазовой подмышечной",
      "тазовой подмышечной",
      "тазовые подмышечные",
      "тазовых подмышечных",
      "тазовым подмышечным",
      NULL,
      "тазовыми подмышечными",
      "тазовых подмышечных" },
    /* тазовый плавник -- fish FINGER(4) */
    { "тазовый плавник", "тазового плавника",
      "тазовому плавнику",
      "тазовый плавник", "тазовым плавником",
      "тазовом плавнике",
      "тазовые плавники", "тазовых плавников",
      "тазовым плавникам",
      NULL,
      "тазовыми плавниками", "тазовых плавниках" },
    /* анальный плавник -- fish CLAWED(5) */
    { "анальный плавник", "анального плавника",
      "анальному плавнику",
      "анальный плавник", "анальным плавником",
      "анальном плавнике",
      "анальные плавники", "анальных плавников",
      "анальным плавникам",
      NULL,
      "анальными плавниками", "анальных плавниках" },
    /* грудной плавник -- fish HAND(6) */
    { "грудной плавник", "грудного плавника",
      "грудному плавнику",
      "грудной плавник", "грудным плавником",
      "грудном плавнике",
      "грудные плавники", "грудных плавников",
      "грудным плавникам",
      NULL,
      "грудными плавниками", "грудных плавниках" },
    /* плавниковая -- fish HEAD(7), прилагательное ж.р. */
    { "плавниковая", "плавниковой",
      "плавниковому",
      "плавниковую", "плавниковым",
      "плавниковом",
      "плавниковые", "плавниковых",
      "плавниковым",
      NULL, "плавниковыми", "плавниковых" },
    /* черешок -- fish LIGHT_HEADED(9) */
    { "черешок", "черешка", "черешку",
      "черешок", "черешком", "черешке",
      "черешки", "черешков", "черешкам",
      NULL, "черешками", "черешках" },
    /* истощённая -- fish NECK(10), прилагательное ж.р. */
    { "истощённая", "истощённой",
      "истощённому",
      "истощённую", "истощённым",
      "истощённом",
      "истощённые", "истощённых",
      "истощённым",
      NULL,
      "истощёнными", "истощённых" },
    /* жабры -- fish SPINE(11), чередование жабра/жабры/жабрами */
    { "жабры", "жабр", "жабрам",
      "жабру", "жабрами", "жабре",
      "жабры", "жабр", "жабрам",
      NULL, "жабрами", "жабрах" },
    /* спинной плавник -- fish TAIL(12) */
    { "спинной плавник", "спинного плавника",
      "спинному плавнику",
      "спинной плавник", "спинным плавником",
      "спинном плавнике",
      "спинные плавники", "спинных плавников",
      "спинным плавникам",
      NULL,
      "спинными плавниками", "спинных плавниках" },
    /* хвостовой плавник -- fish BLOOD(13) */
    { "хвостовой плавник", "хвостового плавника",
      "хвостовому плавнику",
      "хвостовой плавник", "хвостовым плавником",
      "хвостовом плавнике",
      "хвостовые плавники", "хвостовых плавников",
      "хвостовым плавникам",
      NULL,
      "хвостовыми плавниками", "хвостовых плавниках" },
    /* жабра -- fish NOSE(16), отдельный ключ от "жабры" */
    { "жабра", "жабры", "жабре",
      "жабру", "жаброй", "жабре",
      "жабры", "жабр", "жабрам",
      NULL, "жабрами", "жабрах" },
    /* ноздря -- fish NOSE(17), ноздрёй/ноздрей */
    { "ноздря", "ноздри", "ноздре",
      "ноздрю", "ноздрёй", "ноздре",
      "ноздри", "ноздрей", "ноздрям",
      NULL, "ноздрями", "ноздрях" },
    /* объектные существительные для noun_case(), не части тела */
    /* щит */
    { "щит", "щита", "щиту",
      "щит", "щитом", "щите",
      "щиты", "щитов", "щитам",
      NULL, "щитами", "щитах" },
    /* оружие */
    { "оружие", "оружия", "оружию",
      "оружие", "оружием", "оружии",
      "оружие", "оружия", "оружиям",
      NULL, "оружиями", "оружиях" },
    /* амулет */
    { "амулет", "амулета", "амулету",
      "амулет", "амулетом", "амулете",
      "амулеты", "амулетов", "амулетам",
      NULL, "амулетами", "амулетах" },
    /* броня */
    { "броня", "брони", "броне",
      "броню", "бронёй", "броне",
      "брони", "броней", "броням",
      NULL, "бронями", "бронях" },
    /* чешуя */
    { "чешуя", "чешуи", "чешуе",
      "чешую", "чешуёй", "чешуе",
      "чешуи", "чешуй", "чешуям",
      NULL, "чешуями", "чешуях" },
    /* перчатки -- ARM_GLOVES (plural-only) */
    { NULL, NULL, NULL, NULL, NULL, NULL,
      "перчатки", "перчаток", "перчаткам", NULL,
      "перчатками", "перчатках" },
    /* рукавицы -- ARM_GLOVES gauntlets (plural-only) */
    { NULL, NULL, NULL, NULL, NULL, NULL,
      "рукавицы", "рукавиц", "рукавицам", NULL,
      "рукавицами", "рукавицах" },
    /* ботинки -- ARM_BOOTS (plural-only) */
    { NULL, NULL, NULL, NULL, NULL, NULL,
      "ботинки", "ботинок", "ботинкам", NULL,
      "ботинками", "ботинках" },
    /* шлем -- ARM_HELM (м.р.) */
    { "шлем", "шлема", "шлему", "шлем", "шлемом", "шлеме",
      "шлемы", "шлемов", "шлемам", NULL,
      "шлемами", "шлемах" },
    /* шляпа -- ARM_HELM (ж.р.) */
    { "шляпа", "шляпы", "шляпе", "шляпу", "шляпой", "шляпе",
      "шляпы", "шляп", "шляпам", NULL,
      "шляпами", "шляпах" },
};
const char *
mbodypart(struct monst *mon, int part)
{
    static NEARDATA const char
        *humanoid_parts[] = { "рука",  "глаз", "лицо", "палец",
                              "кончик пальца", "нога", "рука",
                              "ладонный", "голова", "нога", "легкоголовый",
                              "шея", "позвоночник", "палец",
                              "волосы", "кровь",
                              "лёгкое", "нос", "желудок" },
        *jelly_parts[] = { "псевдоподия", "пятно", "передняя часть",
                           "отросток псевдоподии", "конец псевдоподии",
                           "основа псевдоподии", "захват", "схваченное",
                           "мозговая область", "нижняя псевдоподия", "вязкая",
                           "середина", "поверхность", "конец псевдоподии",
                           "рябь", "соки", "поверхность", "сенсор",
                           "желудок" },
        *animal_parts[] = { "передняя лапа", "глаз", "морда",
                            "передняя когтистая лапа", "кончик когтя",
                            "задняя когтистая лапа",
                            "передняя когтистая лапа", "когтистая", "голова",
                            "задняя лапа", "легкоголовый", "шея",
                            "позвоночник", "кончик заднего когтя", "шерсть",
                            "кровь", "лёгкое", "нос", "желудок" },
        *bird_parts[] = { "крыло", "глаз", "лицо", "крыло",
                          "кончик крыла", "нога", "крыло", "окрылая",
                          "голова", "нога", "легкоголовый", "шея",
                          "позвоночник", "палец", "перья",
                          "кровь", "лёгкое", "клюв", "желудок" },
        *horse_parts[] = { "передняя нога", "глаз", "морда",
                            "переднее копыто", "кончик копыта",
                            "заднее копыто",
                            "переднее копыто", "копытная", "голова",
                            "задняя нога", "легкоголовый", "шея",
                            "позвоночник", "кончик заднего копыта", "грива",
                            "кровь", "лёгкое", "нос", "желудок" },
        *sphere_parts[] = { "отросток", "зрительный нерв", "тело",
                            "щупальце", "конец щупальца",
                            "нижний отросток",
                            "щупальце", "щупальценосная",
                            "тело", "нижнее щупальце",
                            "вращательная", "экватор",
                            "тело", "конец нижнего щупальца",
                            "реснички", "сила жизни",
                            "сетчатка", "обонятельный нерв",
                            "внутренность" },
        *fungus_parts[] = { "мицелий", "зрительная область",
                            "передняя часть",
                            "гифа",    "гифа",       "корень",
                            "нить",    "нитевидная", "область шляпки",
                            "ризома",  "спороносная", "ножка",
                            "корень",  "конец ризомы", "споры",
                            "соки",    "пластинка",  "пластинка",
                            "внутренность" },
        *vortex_parts[] = { "область", "глаз",
                            "передняя часть",
                            "малый поток", "малый поток", "нижний поток",
                            "завихрение", "завихрённая", "центральное ядро",
                            "нижний поток", "одурманённая", "центр",
                            "потоки", "край", "потоки",
                            "сила жизни", "центр", "передний край",
                            "внутренность" },
        *snake_parts[] = { "рудимент конечности", "глаз", "лицо",
                           "крупная чешуя",
                           "конец крупной чешуи", "задняя область",
                           "чешуйная щель",
                           "чешуйчатая", "голова", "задняя область",
                           "легкоголовый", "шея", "длина", "задняя чешуя",
                           "чешуи", "кровь", "лёгкое",
                           "раздвоенный язык",
                           "желудок" },
        *worm_parts[] = { "передний сегмент",
                          "светочувствительная клетка",
                          "поясок", "щетинки", "щетинки",
                          "задний сегмент",
                          "сегмент", "членистая", "передний сегмент",
                          "задняя часть",
                          "перерастянутая", "поясок",
                          "длина", "задние щетинки", "щетинки", "кровь",
                          "кожа", "головная лопасть", "желудок" },
        *spider_parts[] = { "педипальп", "глаз", "лицо",
                            "педипальп", "тарзус",
                            "коготь", "педипальп", "осязаемая",
                            "головогрудь",
                            "нога", "спрядённая", "головогрудь", "брюшко",
                            "коготь", "волосы", "гемолимфа",
                            "книжное лёгкое",
                            "верхняя губа", "пищеварительный тракт" },
        *fish_parts[] = { "плавник", "глаз", "предчелюстная",
                          "тазовая подмышечная",
                          "тазовый плавник", "анальный плавник",
                          "грудной плавник", "плавниковая",
                          "голова", "черешок", "истощённая", "жабры",
                          "спинной плавник", "хвостовой плавник",
                          "чешуи", "кровь",
                          "жабра", "ноздря", "желудок" };
    /* claw attacks are overloaded in mons[]; most humanoids with
       such attacks should still reference hands rather than claws */
    static const char not_claws[] = {
        S_HUMAN,     S_MUMMY,   S_ZOMBIE, S_ANGEL, S_NYMPH, S_LEPRECHAUN,
        S_QUANTMECH, S_VAMPIRE, S_ORC,    S_GIANT, /* quest nemeses */
        '\0' /* string terminator; assert( S_xxx != 0 ); */
    };
    struct permonst *mptr = mon->data;

    if (part <= NO_PART) {
        impossible("mbodypart: bad part %d", part);
        return "mystery part";
    }

    /* some special cases */
    if (mptr->mlet == S_DOG || mptr->mlet == S_FELINE
        || mptr->mlet == S_RODENT || mptr == &mons[PM_OWLBEAR]) {
        switch (part) {
        case HAND:
            return "paw";
        case HANDED:
            return "pawed";
        case FOOT:
            return "rear paw";
        case ARM:
        case LEG:
            return horse_parts[part]; /* "foreleg", "rear leg" */
        default:
            break; /* for other parts, use animal_parts[] below */
        }
    } else if (mptr->mlet == S_YETI) { /* excl. owlbear due to 'if' above */
        /* opposable thumbs, hence "hands", "arms", "legs", &c */
        return humanoid_parts[part]; /* yeti/sasquatch, monkey/ape */
    }
    if ((part == HAND || part == HANDED)
        && (humanoid(mptr) && attacktype(mptr, AT_CLAW)
            && !strchr(not_claws, mptr->mlet) && mptr != &mons[PM_STONE_GOLEM]
            && mptr != &mons[PM_AMOROUS_DEMON]))
        return (part == HAND) ? "claw" : "clawed";
    if ((mptr == &mons[PM_MUMAK] || mptr == &mons[PM_MASTODON])
        && part == NOSE)
        return "trunk";
    if (mptr == &mons[PM_SHARK] && part == HAIR)
        return "skin"; /* sharks don't have scales */
    if ((mptr == &mons[PM_JELLYFISH] || mptr == &mons[PM_KRAKEN])
        && (part == ARM || part == FINGER || part == HAND || part == FOOT
            || part == TOE))
        return "tentacle";
    if (mptr == &mons[PM_FLOATING_EYE] && part == EYE)
        return "cornea";
    if (humanoid(mptr) && (part == ARM || part == FINGER || part == FINGERTIP
                           || part == HAND || part == HANDED))
        return humanoid_parts[part];
    if (mptr->mlet == S_COCKATRICE)
        return (part == HAIR) ? snake_parts[part] : bird_parts[part];
    if (mptr == &mons[PM_RAVEN])
        return bird_parts[part];
    if (mptr->mlet == S_CENTAUR || mptr->mlet == S_UNICORN
        || mptr == &mons[PM_KI_RIN]
        || (mptr == &mons[PM_ROTHE] && part != HAIR))
        return horse_parts[part];
    if (mptr->mlet == S_LIGHT) {
        if (part == HANDED)
            return "rayed";
        else if (part == ARM || part == FINGER || part == FINGERTIP
                 || part == HAND)
            return "ray";
        else
            return "beam";
    }
    if (mptr == &mons[PM_STALKER] && part == HEAD)
        return "head";
    if (mptr->mlet == S_EEL && mptr != &mons[PM_JELLYFISH])
        return fish_parts[part];
    if (mptr->mlet == S_WORM)
        return worm_parts[part];
    if (mptr->mlet == S_SPIDER)
        return spider_parts[part];
    if (slithy(mptr) || (mptr->mlet == S_DRAGON && part == HAIR))
        return snake_parts[part];
    if (mptr->mlet == S_EYE)
        return sphere_parts[part];
    if (mptr->mlet == S_JELLY || mptr->mlet == S_PUDDING
        || mptr->mlet == S_BLOB || mptr == &mons[PM_JELLYFISH])
        return jelly_parts[part];
    if (mptr->mlet == S_VORTEX || mptr->mlet == S_ELEMENTAL)
        return vortex_parts[part];
    if (mptr->mlet == S_FUNGUS)
        return fungus_parts[part];
    if (humanoid(mptr))
        return humanoid_parts[part];
    return animal_parts[part];
}

const char *
body_part(int part)
{
    return mbodypart(&gy.youmonst, part);
}

/* Case-inflected form of a body part noun; 'cse' is an enum bp_case value
   optionally combined with BP_PLURAL.  All 228 cells of the 12 *_parts[]
   arrays (19 enum bodypart_types x 12 species) are covered by
   bp_paradigms[], so the nominative fallback below is unreachable. */
const char *
mbodypart_case(struct monst *mon, int part, int cse)
{
    const char *nom = mbodypart(mon, part);
    const struct bp_paradigm *pg = bp_paradigms;
    int cs = cse & BP_CASE_MASK, plural = !!(cse & BP_PLURAL), i;

    if (cs < 0 || cs > BP_PREP)
        return nom;
    for (i = 0; i < SIZE(bp_paradigms); i++) {
        const char *key = pg[i].s[BP_NOM] ? pg[i].s[BP_NOM] : pg[i].p[BP_NOM];

        if (!strcmp(key, nom))
            break;
    }
    if (i == SIZE(bp_paradigms))
        return nom;
    if (!pg[i].s[BP_NOM]) /* plural-only noun, e.g. "волосы" */
        return pg[i].p[bp_pl_slot[cs]];
    if (plural)
        return pg[i].p[bp_pl_slot[cs]] ? pg[i].p[bp_pl_slot[cs]] : nom;
    return pg[i].s[cs] ? pg[i].s[cs] : nom;
}

/* Case-inflected form of an arbitrary noun; same bp_paradigms[] lookup as
   mbodypart_case() but keyed by the nominative string itself. */
const char *
noun_case(const char *nom, int cse)
{
    const struct bp_paradigm *pg = bp_paradigms;
    int cs = cse & BP_CASE_MASK;
    int i;

    if (cs < 0 || cs > BP_PREP)
        return nom;
    for (i = 0; i < SIZE(bp_paradigms); i++) {
        const char *key = pg[i].s[BP_NOM] ? pg[i].s[BP_NOM] : pg[i].p[BP_NOM];

        if (key && !strcmp(key, nom))
            break;
    }
    if (i == SIZE(bp_paradigms))
        return nom;
    if (!pg[i].s[BP_NOM])
        return pg[i].p[bp_pl_slot[cs]];
    return pg[i].s[cs] ? pg[i].s[cs] : nom;
}

/* Object names for obj_case(); the otyp-keyed table covers ordinary
   objects, the ART_-keyed table covers artifacts, which xname() takes
   from artilist[] rather than from objects[otyp].oc_name.  Forms repeat
   the capitalization of the name xname() returns. */
static const struct obj_paradigm obj_paradigms[] = {
    { TALLOW_CANDLE,                 /* сальная свеча */
      { "сальная свеча", "сальной свечи",
       "сальной свече", "сальную свечу",
       "сальной свечой", "сальной свече" },
      { "сальные свечи", "сальных свечей",
       "сальным свечам", "сальные свечи",
       NULL, "сальных свечах" } },
    { WAX_CANDLE,                    /* восковая свеча */
      { "восковая свеча", "восковой свечи",
       "восковой свече", "восковую свечу",
       "восковой свечой", "восковой свече" },
      { "восковые свечи", "восковых свечей",
       "восковым свечам", "восковые свечи",
       NULL, "восковых свечах" } },
};

static const struct obj_paradigm arti_paradigms[] = {
    { ART_ORB_OF_DETECTION,          /* Сфера Обнаружения */
      { "Сфера Обнаружения", "Сферы Обнаружения",
       "Сфере Обнаружения", "Сферу Обнаружения",
       "Сферой Обнаружения", "Сфере Обнаружения" },
      { "Сферы Обнаружения", "Сфер Обнаружения",
       "Сферам Обнаружения", "Сферы Обнаружения",
       NULL, "Сферах Обнаружения" } },
    { ART_HEART_OF_AHRIMAN,          /* Сердце Ахримана */
      { "Сердце Ахримана", "Сердца Ахримана",
       "Сердцу Ахримана", "Сердце Ахримана",
       "Сердцем Ахримана", "Сердце Ахримана" },
      { "Сердца Ахримана", "Сердец Ахримана",
       "Сердцам Ахримана", "Сердца Ахримана",
       NULL, "Сердцах Ахримана" } },
    { ART_SCEPTRE_OF_MIGHT,          /* Скипетр Мощи */
      { "Скипетр Мощи", "Скипетра Мощи",
       "Скипетру Мощи", "Скипетр Мощи",
       "Скипетром Мощи", "Скипетре Мощи" },
      { "Скипетры Мощи", "Скипетров Мощи",
       "Скипетрам Мощи", "Скипетры Мощи",
       NULL, "Скипетрах Мощи" } },
    { ART_STAFF_OF_AESCULAPIUS,      /* Посох Эскулапа */
      { "Посох Эскулапа", "Посоха Эскулапа",
       "Посоху Эскулапа", "Посох Эскулапа",
       "Посохом Эскулапа", "Посохе Эскулапа" },
      { "Посохи Эскулапа", "Посохов Эскулапа",
       "Посохам Эскулапа", "Посохи Эскулапа",
       NULL, "Посохах Эскулапа" } },
    { ART_MAGIC_MIRROR_OF_MERLIN,    /* Волшебное Зеркало Мерлина */
      { "Волшебное Зеркало Мерлина", "Волшебного Зеркала Мерлина",
       "Волшебному Зеркалу Мерлина", "Волшебное Зеркало Мерлина",
       "Волшебным Зеркалом Мерлина", "Волшебном Зеркале Мерлина" },
      { "Волшебные Зеркала Мерлина", "Волшебных Зеркал Мерлина",
       "Волшебным Зеркалам Мерлина", "Волшебные Зеркала Мерлина",
       NULL, "Волшебных Зеркалах Мерлина" } },
    { ART_EYES_OF_THE_OVERWORLD,     /* Очи Мира Иного */
      { "Очи Мира Иного", "Очей Мира Иного",
       "Очам Мира Иного", "Очи Мира Иного",
       "Очами Мира Иного", "Очах Мира Иного" },
      { "Очи Мира Иного", "Очей Мира Иного",
       "Очам Мира Иного", "Очи Мира Иного",
       NULL, "Очах Мира Иного" } },
    { ART_MITRE_OF_HOLINESS,         /* Митра Святости */
      { "Митра Святости", "Митры Святости",
       "Митре Святости", "Митру Святости",
       "Митрой Святости", "Митре Святости" },
      { "Митры Святости", "Митр Святости",
       "Митрам Святости", "Митры Святости",
       NULL, "Митрах Святости" } },
    { ART_LONGBOW_OF_DIANA,          /* Длинный Лук Дианы */
      { "Длинный Лук Дианы", "Длинного Лука Дианы",
       "Длинному Луку Дианы", "Длинный Лук Дианы",
       "Длинным Луком Дианы", "Длинном Луке Дианы" },
      { "Длинные Луки Дианы", "Длинных Луков Дианы",
       "Длинным Лукам Дианы", "Длинные Луки Дианы",
       NULL, "Длинных Луках Дианы" } },
    { ART_MASTER_KEY_OF_THIEVERY,    /* Главный Ключ Воровства */
      { "Главный Ключ Воровства", "Главного Ключа Воровства",
       "Главному Ключу Воровства", "Главный Ключ Воровства",
       "Главным Ключом Воровства", "Главном Ключе Воровства" },
      { "Главные Ключи Воровства", "Главных Ключей Воровства",
       "Главным Ключам Воровства", "Главные Ключи Воровства",
       NULL, "Главных Ключах Воровства" } },
    { ART_TSURUGI_OF_MURAMASA,       /* Цуруги Мурамасы */
      { "Цуруги Мурамасы", "Цуруги Мурамасы",
       "Цуруги Мурамасы", "Цуруги Мурамасы",
       "Цуруги Мурамасы", "Цуруги Мурамасы" },
      { "Цуруги Мурамасы", "Цуруги Мурамасы",
       "Цуруги Мурамасы", "Цуруги Мурамасы",
       NULL, "Цуруги Мурамасы" } },
    { ART_YENDORIAN_EXPRESS_CARD,    /* Платиновая Йендорианская Кредитка */
      { "Платиновая Йендорианская Кредитка", "Платиновой Йендорианской Кредитки",
       "Платиновой Йендорианской Кредитке", "Платиновую Йендорианскую Кредитку",
       "Платиновой Йендорианской Кредиткой", "Платиновой Йендорианской Кредитке" },
      { "Платиновые Йендорианские Кредитки", "Платиновых Йендорианских Кредиток",
       "Платиновым Йендорианским Кредиткам", "Платиновые Йендорианские Кредитки",
       NULL, "Платиновых Йендорианских Кредитках" } },
    { ART_ORB_OF_FATE,               /* Сфера Судьбы */
      { "Сфера Судьбы", "Сферы Судьбы",
       "Сфере Судьбы", "Сферу Судьбы",
       "Сферой Судьбы", "Сфере Судьбы" },
      { "Сферы Судьбы", "Сфер Судьбы",
       "Сферам Судьбы", "Сферы Судьбы",
       NULL, "Сферах Судьбы" } },
    { ART_EYE_OF_THE_AETHIOPICA,     /* Око Эфиопики */
      { "Око Эфиопики", "Ока Эфиопики",
       "Оку Эфиопики", "Око Эфиопики",
       "Оком Эфиопики", "Оке Эфиопики" },
      { "Ока Эфиопики", "Ок Эфиопики",
       "Окам Эфиопики", "Ока Эфиопики",
       NULL, "Оках Эфиопики" } },
};

/* Case-inflected name of an object; no article, no possessive, no
   pluralizing "s".  Plural forms are chosen by obj->quan the same way
   xname() picks them.  Callers must not pass an object whose type the
   player has not identified, or this will reveal it. */
const char *
obj_case(struct obj *obj, int cse)
{
    const struct obj_paradigm *pg;
    int cs = cse & BP_CASE_MASK;
    int key, i;

    if (cs < 0 || cs > BP_PREP)
        return xname(obj);
    if (obj->oartifact > 0) {
        pg = arti_paradigms;
        key = (int) obj->oartifact;
        for (i = 0; i < SIZE(arti_paradigms); i++)
            if (arti_paradigms[i].key == key)
                break;
    } else {
        pg = obj_paradigms;
        key = obj->otyp;
        for (i = 0; i < SIZE(obj_paradigms); i++)
            if (obj_paradigms[i].key == key)
                break;
    }
    if (i == SIZE(pg))
        return xname(obj);
    if (obj->quan != 1L && pg[i].p[cs])
        return pg[i].p[cs];
    return pg[i].s[cs] ? pg[i].s[cs] : xname(obj);
}

const char *
body_part_case(int part, int cse)
{
    return mbodypart_case(&gy.youmonst, part, cse);
}

int
poly_gender(void)
{
    /* Returns gender of polymorphed player;
     * 0/1=same meaning as flags.female, 2=none.
     */
    if (is_neuter(gy.youmonst.data) || !humanoid(gy.youmonst.data))
        return 2;
    return flags.female;
}

void
ugolemeffects(int damtype, int dam)
{
    int heal = 0;

    /* We won't bother with "slow"/"haste" since players do not
     * have a monster-specific slow/haste so there is no way to
     * restore the old velocity once they are back to human.
     */
    if (u.umonnum != PM_FLESH_GOLEM && u.umonnum != PM_IRON_GOLEM)
        return;
    switch (damtype) {
    case AD_ELEC:
        if (u.umonnum == PM_FLESH_GOLEM)
            heal = (dam + 5) / 6; /* Approx 1 per die */
        break;
    case AD_FIRE:
        if (u.umonnum == PM_IRON_GOLEM)
            heal = dam;
        break;
    }
    if (heal && (u.mh < u.mhmax)) {
        u.mh += heal;
        if (u.mh > u.mhmax)
            u.mh = u.mhmax;
        disp.botl = TRUE;
        pline("Странно, но вы чувствуете себя лучше, чем раньше.");
        exercise(A_STR, TRUE);
    }
}

staticfn int
armor_to_dragon(int atyp)
{
    switch (atyp) {
    case GRAY_DRAGON_SCALE_MAIL:
    case GRAY_DRAGON_SCALES:
        return PM_GRAY_DRAGON;
    case SILVER_DRAGON_SCALE_MAIL:
    case SILVER_DRAGON_SCALES:
        return PM_SILVER_DRAGON;
    case GOLD_DRAGON_SCALE_MAIL:
    case GOLD_DRAGON_SCALES:
        return PM_GOLD_DRAGON;
#if 0 /* DEFERRED */
    case SHIMMERING_DRAGON_SCALE_MAIL:
    case SHIMMERING_DRAGON_SCALES:
        return PM_SHIMMERING_DRAGON;
#endif
    case RED_DRAGON_SCALE_MAIL:
    case RED_DRAGON_SCALES:
        return PM_RED_DRAGON;
    case ORANGE_DRAGON_SCALE_MAIL:
    case ORANGE_DRAGON_SCALES:
        return PM_ORANGE_DRAGON;
    case WHITE_DRAGON_SCALE_MAIL:
    case WHITE_DRAGON_SCALES:
        return PM_WHITE_DRAGON;
    case BLACK_DRAGON_SCALE_MAIL:
    case BLACK_DRAGON_SCALES:
        return PM_BLACK_DRAGON;
    case BLUE_DRAGON_SCALE_MAIL:
    case BLUE_DRAGON_SCALES:
        return PM_BLUE_DRAGON;
    case GREEN_DRAGON_SCALE_MAIL:
    case GREEN_DRAGON_SCALES:
        return PM_GREEN_DRAGON;
    case YELLOW_DRAGON_SCALE_MAIL:
    case YELLOW_DRAGON_SCALES:
        return PM_YELLOW_DRAGON;
    default:
        return NON_PM;
    }
}

/* some species have awareness of other species */
staticfn void
polysense(void)
{
    short warnidx = NON_PM;

    svc.context.warntype.speciesidx = NON_PM;
    svc.context.warntype.species = 0;
    svc.context.warntype.polyd = 0;
    HWarn_of_mon &= ~FROMRACE;

    switch (u.umonnum) {
    case PM_PURPLE_WORM:
    case PM_BABY_PURPLE_WORM:
        warnidx = PM_SHRIEKER;
        break;
    case PM_VAMPIRE:
    case PM_VAMPIRE_LEADER:
        svc.context.warntype.polyd = M2_HUMAN | M2_ELF;
        HWarn_of_mon |= FROMRACE;
        return;
    }
    if (ismnum(warnidx)) {
        svc.context.warntype.speciesidx = warnidx;
        svc.context.warntype.species = &mons[warnidx];
        HWarn_of_mon |= FROMRACE;
    }
}

/* True iff hero's role or race has been genocided */
boolean
ugenocided(void)
{
    return ((svm.mvitals[gu.urole.mnum].mvflags & G_GENOD)
            || (svm.mvitals[gu.urace.mnum].mvflags & G_GENOD));
}

/* how hero feels "inside" after self-genocide of role or race */
const char *
udeadinside(void)
{
    /* self-genocide used to always say "you feel dead inside" but that
       seems silly when you're polymorphed into something undead;
       monkilled() distinguishes between living (killed) and non (destroyed)
       for monster death message; we refine the nonliving aspect a bit */
    return !nonliving(gy.youmonst.data)
             ? "мёртвым"          /* living, including demons */
             : !weirdnonliving(gy.youmonst.data)
                 ? "проклятым" /* undead plus manes */
                 : "пустым";    /* golems plus vortices */
}

/*polyself.c*/
