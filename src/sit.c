/* NetHack 5.0	sit.c	$NHDT-Date: 1718136168 2024/06/11 20:02:48 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.95 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"

staticfn void throne_sit_effect(void);
staticfn int lay_an_egg(void);

/* take away the hero's money */
void
take_gold(void)
{
    struct obj *otmp, *nobj;
    int lost_money = 0;

    for (otmp = gi.invent; otmp; otmp = nobj) {
        nobj = otmp->nobj;
        if (otmp->oclass == COIN_CLASS) {
            lost_money = 1;
            remove_worn_item(otmp, FALSE);
            delobj(otmp);
        }
    }
    if (!lost_money) {
        You_feel("странное ощущение.");
    } else {
        You("замечаете, что у вас нет золота!");
        disp.botl = TRUE;
    }
}

staticfn void special_throne_effect(int effect);

/* maybe do something when hero sits on a throne */
staticfn void
throne_sit_effect(void)
{
    coordxy tx = u.ux, ty = u.uy;

    boolean special_throne = !!In_V_tower(&u.uz);

    if (rnd(6) > 4) { /* [why so convoluted? it's the same as '!rn2(3)'] */
        int effect = rnd(13);

        if (wizard && !iflags.debug_fuzzer) {
            char buf[BUFSZ];
            int which;

            buf[0] = '\0';
            getlin("Эффект сидения на троне (1..13) [0=случайно]", buf);
            if (buf[0] == '\033') {
                pline("%s", Never_mind);
                return; /* caller will still cause a move to elapse */
            }
            which = atoi(buf);
            if (which >= 1 && which <= 13)
                effect = which;
        }

        if (special_throne) {
            special_throne_effect(effect);
            return;
        }

        switch (effect) {
        case 1:
            (void) adjattrib(rn2(A_MAX), -rn1(4, 3), FALSE);
            losehp(rnd(10), "проклятый трон", KILLED_BY_AN);
            break;
        case 2:
            (void) adjattrib(rn2(A_MAX), 1, FALSE);
            break;
        case 3:
            pline("Вас бьёт электрическим разрядом%s!",
                  (Shock_resistance) ? "" : " сильно");
            losehp(Shock_resistance ? rnd(6) : rnd(30), "электрическое кресло",
                   KILLED_BY_AN);
            exercise(A_CON, FALSE);
            break;
        case 4:
            You_feel("намного, намного лучше!");
            if (Upolyd) {
                if (u.mh >= (u.mhmax - 5))
                    u.mhmax += 4;
                u.mh = u.mhmax;
            }
            if (u.uhp >= (u.uhpmax - 5)) {
                u.uhpmax += 4;
                if (u.uhpmax > u.uhppeak)
                    u.uhppeak = u.uhpmax;
            }
            u.uhp = u.uhpmax;
            u.ucreamed = 0;
            make_blinded(0L, TRUE);
            make_sick(0L, (char *) 0, FALSE, SICK_ALL);
            heal_legs(0);
            disp.botl = TRUE;
            break;
        case 5:
            take_gold();
            break;
        case 6:
            if (u.uluck + rn2(5) < 0) {
                You_feel("ваша удача меняется.");
                change_luck(1);
            } else
                makewish();
            break;
        case 7:
            {
                int cnt = rnd(10);

                /* Magical voice not affected by deafness */
                pline("Голос эхом разносится:");
                SetVoice((struct monst *) 0, 0, 80, voice_throne);
                verbalize("Ваша аудиенция созвана, %s!",
                          flags.female ? "госпожа" : "господин");
                while (cnt--)
                    (void) makemon(courtmon(), tx, ty, NO_MM_FLAGS);
                break;
            }
        case 8:
            /* Magical voice not affected by deafness */
            pline("Голос эхом разносится:");
            SetVoice((struct monst *) 0, 0, 80, voice_throne);
            verbalize("По твоему повелительному приказу, %s...",
                      flags.female ? "госпожа" : "господин");
            do_genocide(5); /* REALLY|ONTHRONE, see do_genocide() */
            break;
        case 9:
            /* Magical voice not affected by deafness */
            pline("Голос эхом разносится:");
            SetVoice((struct monst *) 0, 0, 80, voice_throne);
            verbalize(
                 "Проклятие тебе за сидение на этом святейшем троне!");
            if (Luck > 0) {
                make_blinded(BlindedTimeout + rn1(100, 250), TRUE);
                change_luck((Luck > 1) ? -rnd(2) : -1);
            } else
                rndcurse();
            break;
        case 10:
            if (Luck < 0 || (HSee_invisible & INTRINSIC)) {
                if (svl.level.flags.nommap) {
                    pline("Ужасный гул наполняет вашу голову!");
                    make_confused((HConfusion & TIMEOUT) + (long) rnd(30),
                                  FALSE);
                } else {
                    pline("В вашем разуме возникает образ.");
                    do_mapping();
                }
            } else {
                /* avoid "vision clears" if hero can't see */
                if (!Blind) {
                    Your("зрение проясняется.");
                } else {
                    int num_of_eyes = eyecount(gy.youmonst.data);
                    const char *eye = body_part(EYE);

                    /* note: 1 eye case won't actually happen--can't
                       sit on throne when poly'd into always-levitating
                       floating eye and can't polymorph into Cyclops */
                    switch (num_of_eyes) { /* 2, 1, or 0 */
                    default:
                    case 2: /* more than 1 eye */
                        eye = makeplural(eye);
                        FALLTHROUGH;
                        /*FALLTHRU*/
                    case 1: /* one eye (Cyclops, floating eye) */
                        Your("%s %s...", eye, vtense(eye, "tingle"));
                        break;
                    case 0: /* no eyes */
                        You("испытываете очень странное ощущение в %s.",
                            body_part(HEAD));
                        break;
                    }
                }
                HSee_invisible |= FROMOUTSIDE;
                newsym(u.ux, u.uy);
            }
            break;
        case 11:
            if (Luck < 0) {
                You_feel("угрозу.");
                aggravate();
            } else {
                You_feel("резкое смещение.");
                tele(); /* teleport him */
            }
            break;
        case 12:
            You("обретаете озарение!");
            if (gi.invent) {
                /* rn2(5) agrees w/seffects() */
                identify_pack(rn2(5), FALSE);
            }
            break;
        case 13:
            Your("разум скручивается в крендель!");
            make_confused((HConfusion & TIMEOUT) + (long) rn1(7, 16),
                          FALSE);
            break;
        default:
            impossible("эффект трона");
            break;
        }
    } else {
        if (is_prince(gy.youmonst.data) || u.uevent.uhand_of_elbereth)
            You_feel("очень уютно здесь.");
        else
            You_feel("как-то не на месте...");
    }

    /* 5.0: when the random chance for removal is hit, ask for confirmation
       if in wizard mode, and remove the throne even if hero was teleported
       away from it.  [This used to remove a throne at hero's current
       location if there happened to be one, so for the teleport case that
       only happened when teleporting back to the same point where hero
       started from.]  "Analyzing a throne" doesn't really make any sense
       but if the answer is yes than it will vanish in a puff of logic. */
    if (!special_throne &&
        !rn2(3) && (!wizard || y_n("Исследовать трон?") == 'y')) {
        levl[tx][ty].typ = ROOM, levl[tx][ty].flags = 0;
        map_background(tx, ty, FALSE);
        newsym_force(tx, ty);
        /* "[God] promptly vanishes in a puff of logic" is from
           Douglas Adams' _The_Hitchhiker's_Guide_to_the_Galaxy_. */
        pline("Трон %s в облаке логики.",
                  cansee(tx, ty) ? "Исчезает" : "Исчез");
    }
}

/* special throne in Vlad's tower: effect is 1 to 13 inclusive */
staticfn void
special_throne_effect(int effect) {
    coordxy tx = u.ux, ty = u.uy;

    switch (effect) {
    case 1:
    case 2:
    case 3:
    case 4:
        /* 4 chances of a wish, but then the throne disappears.

           This is the only way the throne can disappear from sitting
           on it, so if you sit on it enough (enduring the negative
           effects) you are guaranteed an eventual wish. */
        makewish();
        levl[tx][ty].typ = ROOM, levl[tx][ty].flags = 0;
        map_background(tx, ty, FALSE);
        newsym_force(tx, ty);
        pline("Трон распадается, истощив свою силу.");
        break;
    case 5:
        /* permanent level drain */
        pline("Сидение на троне оказалось ужасным.");
        if (!Drain_resistance) {
            losexp("плохой опыт сидения на троне");
            if (u.ulevelmax > u.ulevel)
                u.ulevelmax -= 1;
        }
        break;
    case 6:
    {
        /* grease hands and inventory

           Same rules for which items can be affected as grease_ok in apply.c */
        struct obj *otmp;

        pline("Жирная жидкость обдаёт вас!");
        for (otmp = gi.invent; otmp; otmp = otmp->nobj)
            if (otmp->oclass != COIN_CLASS)
                otmp->greased = 1;
        make_glib(rn1(101, 100));
        update_inventory();
        break;
    }
    case 7:
        /* lose an intrinsic */
        attrcurse();
        pline("Трон, кажется, чем-то позабавился.");
        break;
    case 8:
    {
        /* level teleport to Vibrating Square level */
        d_level vs_level;
        find_hell(&vs_level);
        vs_level.dlevel = svd.dungeons[vs_level.dnum].num_dunlevs - 1;
        if (u.uhave.amulet)
            You_feel("крайне дезориентированы на мгновение.");
        else
            schedule_goto(
                &vs_level, UTOTYPE_NONE, (char *) 0,
                "Вы чувствуете себя крайне не на месте.");
        break;
    }
    case 9:
    {
        /* summon demons; a NULL argument to msummon summons demons as
           though they were summoned by the Wizard of Yendor */
        pline("Трон, кажется, зовёт на помощь!");
        msummon(NULL);
        msummon(NULL);
        msummon(NULL);
        break;
    }
    case 10:
    {
        /* confused blessed remove curse effect */
        struct obj fake_spellbook;
        long save_confusion = HConfusion;

        fake_spellbook = cg.zeroobj;
        fake_spellbook.otyp = SPE_REMOVE_CURSE;
        fake_spellbook.oclass = SPBOOK_CLASS;
        fake_spellbook.blessed = 1;
        HConfusion = 1L;
        (void) seffects(&fake_spellbook);
        HConfusion = save_confusion;
        break;
    }
    case 11:
        /* polymorph effect (not blocked by magic resistance, but other things
           that protect from polymorphs work) */
        if (is_vampire(gy.youmonst.data)) {
            You_feel("недостойным.");
        } else {
            pline("Этот трон не предназначен для таких, как вы!");
            You_feel("перемены наступают в вас.");
            polyself(POLY_NOFLAGS);
        }
        break;
    case 12:
        /* acid damage */
        pline("Трон покрыт кислотой!");
        losehp(Acid_resistance ? rnd(16) : rnd(80), "кислотное кресло",
               KILLED_BY_AN);
        exercise(A_CON, FALSE);
        break;
    case 13:
    {
        /* ability shuffle */
        int ability;
        pline("Когда вы сидите на троне, ваше тело и разум начинают искажаться.");
        for (ability = 0; ability < A_MAX; ++ability) {
            adjattrib(ability, rn2(5) - 2, -1);
        }
        break;
    }
    }
}

/* hero lays an egg */
staticfn int
lay_an_egg(void)
{
    struct obj *uegg;

    if (!flags.female) {
        pline("%s не может откладывать яйца!",
              Hallucination
              ? "Вы можете думать, что вы утконос, но самец всё равно"
              : "Самцы");
        return ECMD_OK;
    } else if (u.uhunger < (int) objects[EGG].oc_nutrition) {
        You("недостаточно энергии, чтобы снести яйцо.");
        return ECMD_OK;
    } else if (eggs_in_water(gy.youmonst.data)) {
        if (!(Underwater || Is_waterlevel(&u.uz))) {
            pline("Вы не плавающая тетра.");
            return ECMD_OK;
        }
        if (Upolyd
            && (gy.youmonst.data == &mons[PM_GIANT_EEL]
                || gy.youmonst.data == &mons[PM_ELECTRIC_EEL])) {
            You("тоскуете по Саргассову морю.");
            return ECMD_OK;
        }
    }
    uegg = mksobj(EGG, FALSE, FALSE);
    uegg->spe = 1;
    uegg->quan = 1L;
    uegg->owt = weight(uegg);
    /* this sets hatch timers if appropriate */
    set_corpsenm(uegg, egg_type_from_parent(u.umonnum, FALSE));
    uegg->known = 1;
    observe_object(uegg);
    You("%s яйцо.", eggs_in_water(gy.youmonst.data) ? "порождаете" : "несёте");
    dropy(uegg);
    stackobj(uegg);
    morehungry((int) objects[EGG].oc_nutrition);
    return ECMD_TIME;
}

/* #sit command */
int
dosit(void)
{
    static const char sit_message[] = "садитесь на %s.";
    struct trap *trap = t_at(u.ux, u.uy);
    int typ = levl[u.ux][u.uy].typ;

    if (u.usteed) {
        You("уже сидите на %s.", mon_nam(u.usteed));
        return ECMD_OK;
    }
    if (u.uundetected && is_hider(gy.youmonst.data)
        && u.umonnum != PM_TRAPPER) /* trapper can stay hidden on floor */
        u.uundetected = 0; /* no longer on the ceiling */

    if (!can_reach_floor(FALSE)) {
        if (u.uswallow)
            There("внутри нет сидений!");
        else if (Levitation)
            You("кувыркаетесь на месте.");
        else
            You("сидите на воздухе.");
        return ECMD_OK;
    } else if (u.ustuck && !sticks(gy.youmonst.data)) {
        /* holding monster is next to hero rather than beneath, but
           hero is in no condition to actually sit at has/her own spot */
        if (humanoid(u.ustuck->data))
            pline("%s не предложит %s колени.", Monnam(u.ustuck), mhis(u.ustuck));
        else
            pline("%s не имеет коленей.", Monnam(u.ustuck));
        return ECMD_OK;
    } else if (is_pool(u.ux, u.uy) && !Underwater) { /* water walking */
        goto in_water;
    } else if (Upolyd && u.umonnum == PM_GREMLIN
               && (levl[u.ux][u.uy].typ == FOUNTAIN || is_pool(u.ux, u.uy))) {
        goto in_water;
    }

    if (OBJ_AT(u.ux, u.uy)
        /* ensure we're not standing on the precipice */
        && !(uteetering_at_seen_pit(trap) || uescaped_shaft(trap))) {
        struct obj *obj;

        obj = svl.level.objects[u.ux][u.uy];
        if (gy.youmonst.data->mlet == S_DRAGON && obj->oclass == COIN_CLASS) {
            You("сворачиваетесь вокруг вашей %sкладовой.",
                (obj->quan + money_cnt(gi.invent) < u.ulevel * 1000)
                ? "скудной " : "");
        } else if (obj->otyp == TOWEL) {
            pline("Вероятно, сейчас не лучшее время для пикника...");
        } else {
            if (slithy(gy.youmonst.data))
                You("сворачиваетесь вокруг %s.", the(xname(obj)));
            else
                You("садитесь на %s.", the(xname(obj)));
            if (obj->otyp == CORPSE && amorphous(&mons[obj->corpsenm]))
                pline("Это мягкое...");
            else if (obj->otyp == CREAM_PIE) {
                 if (!Deaf) {
                   Soundeffect(se_squelch, 30);
                   pline("Хлюп!");
                }
                useupf(obj, obj->quan);
            } else if (!(Is_box(obj)
                         || objects[obj->otyp].oc_material == CLOTH))
                pline("Это не очень удобно...");
        }
    } else if (trap != 0 || (u.utrap && (u.utraptype >= TT_LAVA))) {
        if (u.utrap) {
            exercise(A_WIS, FALSE); /* you're getting stuck longer */
            if (u.utraptype == TT_BEARTRAP) {
                You_cant("опуститься, когда ваш %s в медвежьем капкане.",
                         body_part(FOOT));
                u.utrap++;
            } else if (u.utraptype == TT_PIT) {
                if (trap && trap->ttyp == SPIKED_PIT) {
                    You("садитесь на шип.  Ой!");
                    losehp(Half_physical_damage ? rn2(2) : 1,
                           "сидение на железном шипе", KILLED_BY);
                    exercise(A_STR, FALSE);
                } else
                    You("садитесь в яму.");
                u.utrap += rn2(5);
            } else if (u.utraptype == TT_WEB) {
                You("садитесь в паутину и ещё больше запутываетесь!");
                u.utrap += rn1(10, 5);
            } else if (u.utraptype == TT_LAVA) {
                /* Must have fire resistance or they'd be dead already */
                You("садитесь в %s!", hliquid("lava"));
                if (Slimed)
                    burn_away_slime();
                u.utrap += rnd(4);
                losehp(d(2, 10), "сидение в лаве",
                       KILLED_BY); /* lava damage */
            } else if (u.utraptype == TT_INFLOOR
                       || u.utraptype == TT_BURIEDBALL) {
                You_cant("maneuver to sit!");
                u.utrap++;
            }
        } else {
            /* when flying, "you land" might need some refinement; it sounds
               as if you're staying on the ground but you will immediately
               take off again unless you become stuck in a holding trap */
            You("%s.", Flying ? "приземляетесь" : "садитесь");
            dotrap(trap, VIASITTING);
        }
    } else if ((Underwater || Is_waterlevel(&u.uz))
                && !eggs_in_water(gy.youmonst.data)) {
        if (Is_waterlevel(&u.uz))
            There("поблизости нет плавающих подушек.");
        else
            You("садитесь на илистое дно.");
    } else if (is_pool(u.ux, u.uy) && !eggs_in_water(gy.youmonst.data)) {
 in_water:
        You("садитесь в %s.", hliquid("water"));
        if (Upolyd && u.umonnum == PM_GREMLIN) {
            if (split_mon(&gy.youmonst, (struct monst *) 0)) {
                if (levl[u.ux][u.uy].typ == FOUNTAIN)
                    dryup(u.ux, u.uy, TRUE);
            }
            /* splitting--or failing to do so--protects gear from the water */
        } else {
            if (!rn2(10) && uarm)
                (void) water_damage(uarm, "доспех", TRUE);
            if (!rn2(10) && uarmf && uarmf->otyp != WATER_WALKING_BOOTS)
                (void) water_damage(uarm, "доспех", TRUE);
        }
    } else if (IS_SINK(typ)) {
        You(sit_message, defsyms[S_sink].explanation);
        Your("%s намокает.",
             humanoid(gy.youmonst.data) ? "ягодицы" : "нижняя сторона");
    } else if (IS_ALTAR(typ)) {
        You(sit_message, defsyms[S_altar].explanation);
        altar_wrath(u.ux, u.uy);
    } else if (IS_GRAVE(typ)) {
        You(sit_message, defsyms[S_grave].explanation);
    } else if (typ == STAIRS) {
        You(sit_message, "лестницу");
    } else if (typ == LADDER) {
        You(sit_message, "лестницу");
    } else if (is_lava(u.ux, u.uy)) {
        /* must be WWalking */
        You(sit_message, hliquid("лава"));
        burn_away_slime();
        if (likes_lava(gy.youmonst.data)) {
            pline("%s Тёплая.", hliquid("lava"));
            return ECMD_TIME;
        }
        pline("%s Обжигает вас!", hliquid("lava"));
        losehp(d((Fire_resistance ? 2 : 10), 10), /* lava damage */
               "сидение на лаве", KILLED_BY);
    } else if (is_ice(u.ux, u.uy)) {
        You(sit_message, defsyms[S_ice].explanation);
        if (!Cold_resistance)
            pline("Лёд холодный.");
    } else if (typ == DRAWBRIDGE_DOWN) {
        You(sit_message, "подъёмный мост");
    } else if (IS_THRONE(typ)) {
        You(sit_message, defsyms[S_throne].explanation);
        throne_sit_effect();
    } else if (lays_eggs(gy.youmonst.data)) {
        return lay_an_egg();
    } else {
        pline("Вам весело сидеть на %s?", surface(u.ux, u.uy));
    }
    return ECMD_TIME;
}

/* curse a few inventory items at random! */
void
rndcurse(void)
{
    int nobj = 0;
    int cnt, onum;
    struct obj *otmp;
    static const char mal_aura[] = "чувствуете злую ауру, окружающую %s.";

    if (u_wield_art(ART_MAGICBANE) && rn2(20)) {
        You(mal_aura, "лезвие, поглощающее магию");
        return;
    }

    if (Antimagic) {
        shieldeff(u.ux, u.uy);
    }

    You(mal_aura, "вас");

    for (otmp = gi.invent; otmp; otmp = otmp->nobj) {
        /* gold isn't subject to being cursed or blessed */
        if (otmp->oclass == COIN_CLASS)
            continue;
        nobj++;
    }
    cnt = rnd(6 / ((!!Antimagic) + (!!Half_spell_damage) + 1));
    if (nobj) {
        for (; cnt > 0; cnt--) {
            onum = rnd(nobj);
            for (otmp = gi.invent; otmp; otmp = otmp->nobj) {
                /* as above */
                if (otmp->oclass == COIN_CLASS)
                    continue;
                if (--onum == 0)
                    break; /* found the target */
            }
            /* the !otmp case should never happen; picking an already
               cursed item happens--avoid "resists" message in that case */
            if (!otmp || otmp->cursed)
                continue; /* next target */

            if (otmp->oartifact && spec_ability(otmp, SPFX_INTEL)
                && rn2(10) < 8) {
                pline("%s!", Tobjnam(otmp, "resist"));
                continue;
            }

            if (otmp->blessed)
                unbless(otmp);
            else
                curse(otmp);
        }
        update_inventory();
    }

    /* treat steed's saddle as extended part of hero's inventory */
    if (u.usteed && !rn2(4) && (otmp = which_armor(u.usteed, W_SADDLE)) != 0
        && !otmp->cursed) { /* skip if already cursed */
        if (otmp->blessed)
            unbless(otmp);
        else
            curse(otmp);
        if (!Blind) {
            pline("%s %s.", Yobjnam2(otmp, "glow"),
                  hcolor(otmp->cursed ? NH_BLACK : (const char *) "коричневым"));
            otmp->bknown = Hallucination ? 0 : 1; /* bypass set_bknown() */
        } else {
            otmp->bknown = 0; /* bypass set_bknown() */
        }
    }
}

/* remove a random INTRINSIC ability from hero.
   returns the intrinsic property which was removed,
   or 0 if nothing was removed. */
int
attrcurse(void)
{
    int ret = 0;

    switch (rnd(11)) {
    case 1:
        if (HFire_resistance & INTRINSIC) {
            HFire_resistance &= ~INTRINSIC;
            You_feel("теплее.");
            ret = FIRE_RES;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 2:
        if (HTeleportation & INTRINSIC) {
            HTeleportation &= ~INTRINSIC;
            You_feel("менее нервозно.");
            ret = TELEPORT;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 3:
        if (HPoison_resistance & INTRINSIC) {
            HPoison_resistance &= ~INTRINSIC;
            You_feel("немного дурно!");
            ret = POISON_RES;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 4:
        if (HTelepat & INTRINSIC) {
            HTelepat &= ~INTRINSIC;
            if (Blind && !Blind_telepat)
                see_monsters(); /* Can't sense mons anymore! */
            Your("чувства покидают вас!");
            ret = TELEPAT;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 5:
        if (HCold_resistance & INTRINSIC) {
            HCold_resistance &= ~INTRINSIC;
            You_feel("прохладнее.");
            ret = COLD_RES;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 6:
        if (HInvis & INTRINSIC) {
            HInvis &= ~INTRINSIC;
            You_feel("паранойю.");
            ret = INVIS;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 7:
        if (HSee_invisible & INTRINSIC) {
            HSee_invisible &= ~INTRINSIC;
            if (!See_invisible) {
                set_mimic_blocking();
                see_monsters();
                /* might not be able to see self anymore */
                newsym(u.ux, u.uy);
            }
            You("%s!", Hallucination ? "думал, будто видел ки-кис"
                                     : "подумали, что что-то увидели");
            ret = SEE_INVIS;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 8:
        if (HFast & INTRINSIC) {
            HFast &= ~INTRINSIC;
            You_feel("медленнее.");
            ret = FAST;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 9:
        if (HStealth & INTRINSIC) {
            HStealth &= ~INTRINSIC;
            You_feel("неуклюже.");
            ret = STEALTH;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 10:
        /* intrinsic protection is just disabled, not set back to 0 */
        if (HProtection & INTRINSIC) {
            HProtection &= ~INTRINSIC;
            You_feel("уязвимо.");
            ret = PROTECTION;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 11:
        if (HAggravate_monster & INTRINSIC) {
            HAggravate_monster &= ~INTRINSIC;
            You_feel("менее привлекательно.");
            ret = AGGRAVATE_MONSTER;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    default:
        break;
    }
    return ret;
}

/*sit.c*/
