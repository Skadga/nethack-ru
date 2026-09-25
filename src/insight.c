/* NetHack 5.0	insight.c	$NHDT-Date: 1777004419 2026/04/23 20:20:19 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.134 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Enlightenment and Conduct+Achievements and Vanquished+Extinct+Geno'd
 * and stethoscope/probing feedback.
 *
 * Most code used to reside in cmd.c, presumably because ^X was originally
 * a wizard mode command and the majority of those are in that file.
 * Some came from end.c where it is used during end of game disclosure.
 * And some came from priest.c that had once been in pline.c.
 */

#include "hack.h"

staticfn void enlght_out(const char *);
staticfn void enlght_line(const char *, const char *, const char *,
                          const char *);
staticfn char *enlght_combatinc(const char *, int, int, char *);
staticfn void enlght_halfdmg(int, int);
staticfn boolean walking_on_water(void);
staticfn boolean cause_known(int);
staticfn char *attrval(int, int, char *);
staticfn char *fmt_elapsed_time(char *, int);
staticfn char *N_times(long, char *) NONNULL NONNULLARG2;
staticfn void background_enlightenment(int, int);
staticfn void basics_enlightenment(int, int);
staticfn void characteristics_enlightenment(int, int);
staticfn void one_characteristic(int, int, int);
staticfn void status_enlightenment(int, int);
staticfn void weapon_insight(int);
staticfn void attributes_enlightenment(int, int);
staticfn void show_achievements(int);
staticfn int QSORTCALLBACK vanqsort_cmp(const genericptr, const genericptr);
staticfn int num_extinct(void);
staticfn int num_gone(int, int *);
staticfn char *size_str(int);
staticfn void item_resistance_message(int, const char *, int);

extern const char *const hu_stat[];  /* hunger status from eat.c */
extern const char *const enc_stat[]; /* encumbrance status from botl.c */

static const char You_[] = "Вы ", are[] = "", were[] = "",
                  have[] = "", had[] = "", can[] = "можете ",
                  could[] = "могли ";
static const char have_been[] = "были ", have_never[] = "никогда не ",
                  never[] = "никогда не ";

/* for livelogging: */
struct ll_achieve_msg {
    long llflag;
    const char *msg;
};
/* ordered per 'enum achievements' in you.h */
/* take care to keep them in sync! */
static struct ll_achieve_msg achieve_msg [] = {
    { 0, "" }, /* actual achievements are numbered from 1 */
    { LL_ACHIEVE, "получил Колокол Открытия" },
    { LL_ACHIEVE, "вошёл в Геенну" },
    { LL_ACHIEVE, "получил Канделябр Исторгания" },
    { LL_ACHIEVE, "получил Книгу Мёртвых" },
    { LL_ACHIEVE, "совершил изгнание" },
    { LL_ACHIEVE, "получил Амулет Йендора" },
    { LL_ACHIEVE, "вошёл на Стихийные Планы" },
    { LL_ACHIEVE, "вошёл на Астральный План" },
    { LL_ACHIEVE, "вознёсся" },
    /* if the type of item isn't discovered yet, disclosing the event
       via #chronicle would be a spoiler (particularly for gray stone);
       the ID'd name for the type of item will be appended to the next
       two messages, for display via livelog and/or dumplog */
    { LL_ACHIEVE | LL_SPOILER, "добрался до конца шахт" }, /* " luckstone" */
    { LL_ACHIEVE | LL_SPOILER, "прошёл Сокобан" }, /* " <item>" */
    { LL_ACHIEVE | LL_UMONST, "убил Медузу" },
     /* these two are not logged */
    { 0, "hero was always blond, no, blind" },
    { 0, "hero never wore armor" },
     /* */
    { LL_MINORAC | LL_DUMP, "вошёл в Гномьи Шахты" },
    { LL_ACHIEVE, "добрался до Шахтёрского Городка" }, /* probably minor, but dnh logs it */
    { LL_MINORAC, "вошёл в лавку" },
    { LL_MINORAC, "вошёл в храм" },
    { LL_ACHIEVE, "посоветовался с Оракулом" }, /* minor, but rare enough */
    { LL_MINORAC | LL_DUMP, "прочитал роман Плоского мира" }, /* even more so */
    { LL_ACHIEVE, "вошёл в Сокобан" }, /* keep as major for turn comparison
                                        * with completed sokoban */
    { LL_ACHIEVE, "вошёл в Большой зал" },
    /* The following 8 are for advancing through the ranks
       and messages differ by role so are created on the fly;
       rank 0 (Xp 1 and 2) isn't an achievement */
    { LL_MINORAC | LL_DUMP, "" }, /* Xp 3 */
    { LL_MINORAC | LL_DUMP, "" }, /* Xp 6 */
    { LL_MINORAC | LL_DUMP, "" }, /* Xp 10 */
    { LL_ACHIEVE, "" }, /* Xp 14, so able to attempt the quest */
    { LL_ACHIEVE, "" }, /* Xp 18 */
    { LL_ACHIEVE, "" }, /* Xp 22 */
    { LL_ACHIEVE, "" }, /* Xp 26 */
    { LL_ACHIEVE, "" }, /* Xp 30 */
    { LL_MINORAC, "выучил мелодию подъёмного моста замка" }, /* achievement #31 */
    { 0, "" } /* keep this one at the end */
};

/* macros to simplify output of enlightenment messages; also used by
   conduct and achievements */
#define enl_msg(prefix, present, past, suffix, ps) \
    enlght_line((prefix), final ? (past) : (present), (suffix), (ps))
#define you_are(attr, ps) enl_msg(You_, are, were, (attr), (ps))
#define you_have(attr, ps) enl_msg(You_, have, had, (attr), (ps))
#define you_can(attr, ps) enl_msg(You_, can, could, (attr), (ps))
#define you_have_been(goodthing) \
    enl_msg(You_, have_been, were, (goodthing), "")
#define you_have_never(badthing) \
    enl_msg(You_, have_never, never, (badthing), "")
#define you_have_X(something) \
    enl_msg(You_, have, (const char *) "", (something), "")

staticfn void
enlght_out(const char *buf)
{
    if (ge.en_via_menu) {
        add_menu_str(ge.en_win, buf);
    } else
        putstr(ge.en_win, 0, buf);
}

staticfn void
enlght_line(
    const char *start,
    const char *middle,
    const char *end,
    const char *ps)
{
#ifndef NO_ENLGHT_CONTRACTIONS
    static const struct contrctn {
        const char *twowords, *contrctn;
    } contra[] = {
        { " are not ", " aren't " },
        { " were not ", " weren't " },
        { " have not ", " haven't " },
        { " had not ", " hadn't " },
        { " can not ", " can't " },
        { " could not ", " couldn't " },
    };
    int i;
#endif
    char buf[BUFSZ];

    Sprintf(buf, " %s%s%s%s.", start, middle, end, ps);
#ifndef NO_ENLGHT_CONTRACTIONS
    if (strstri(buf, " not ")) { /* TODO: switch to libc strstr() */
        for (i = 0; i < SIZE(contra); ++i)
            (void) strsubst(buf, contra[i].twowords, contra[i].contrctn);
    }
#endif
    enlght_out(buf);
}

/* format increased chance to hit or damage or defense (Protection) */
staticfn char *
enlght_combatinc(
    const char *inctyp, /* "to hit" or "damage" or "defense" */
    int incamt,         /* amount of increment (negative if decrement) */
    int final,          /* ENL_{GAMEINPROGRESS,GAMEOVERALIVE,GAMEOVERDEAD} */
    char *outbuf)
{
    const char *modif, *bonus;
    boolean invrt;
    int absamt;

    absamt = abs(incamt);
    /* Protection amount is typically larger than damage or to-hit;
       reduce magnitude by a third in order to stretch modifier ranges
       (small:1..5, moderate:6..10, large:11..19, huge:20+) */
    if (!strcmp(inctyp, "defense"))
        absamt = (absamt * 2) / 3;

    if (absamt <= 3)
        modif = "небольшой";
    else if (absamt <= 6)
        modif = "умеренный";
    else if (absamt <= 12)
        modif = "большой";
    else
        modif = "огромный";

    modif = !incamt ? "нет" : modif; /* ("нет" вряд ли случится) */
    bonus = (incamt >= 0) ? "бонус" : "штраф";
    /* "bonus <foo>" (to hit) vs "<bar> bonus" (damage, defense) */
    invrt = strcmp(inctyp, "to hit") ? TRUE : FALSE;

    Sprintf(outbuf, "%s %s к %s", modif, bonus,
            invrt ? "попаданию" : (inctyp[0] == 'd') ? "урону" : "защите");
    if (final || wizard)
        Sprintf(eos(outbuf), " (%s%d)", (incamt > 0) ? "+" : "", incamt);

    return outbuf;
}

/* report half physical or half spell damage */
staticfn void
enlght_halfdmg(int category, int final)
{
    const char *category_name;
    char buf[BUFSZ];

    switch (category) {
    case HALF_PHDAM:
        category_name = "физический";
        break;
    case HALF_SPDAM:
        category_name = "магический";
        break;
    default:
        category_name = "неизвестный";
        break;
    }
    Sprintf(buf, " %s %s урон", (final || wizard) ? "половинный" : "уменьшенный",
            category_name);
    enl_msg(You_, "получаете", "получали", buf, from_what(category));
}

/* is hero actively using water walking capability on water (or lava)? */
staticfn boolean
walking_on_water(void)
{
    if (u.uinwater || Levitation || Flying)
        return FALSE;
    return (boolean) (Wwalking && is_pool_or_lava(u.ux, u.uy));
}

/* describe u.utraptype; used by status_enlightenment() and self_lookat() */
char *
trap_predicament(char *outbuf, int final, boolean wizxtra)
{
    struct trap *t;

    /* caller has verified u.utrap */
    *outbuf = '\0';
    switch (u.utraptype) {
    case TT_BURIEDBALL:
        Strcpy(outbuf, "привязаны к чему-то закопанному");
        break;
    case TT_LAVA:
        Sprintf(outbuf, "тонете в %s", final ? "лаве" : hliquid("lava"));
        break;
    case TT_INFLOOR:
        Sprintf(outbuf, "застряли в %s", the(surface(u.ux, u.uy)));
        break;
    default: /* TT_BEARTRAP, TT_PIT, or TT_WEB */
        Strcpy(outbuf, "в ловушке");
        if ((t = t_at(u.ux, u.uy)) != 0) /* should never be null */
            Sprintf(eos(outbuf), " в %s", trapname(t->ttyp, FALSE));
        break;
    }
    if (wizxtra) { /* give extra information for wizard mode enlightenment */
        /* curly braces: u.utrap is an escape attempt counter rather than a
           turn timer so use different ornamentation than usual parentheses */
        Sprintf(eos(outbuf), " {%u}", u.utrap);
    }
    return outbuf;
}

/* check whether hero is wearing something that player definitely knows
   confers the target property; item must have been seen and its type
   discovered but it doesn't necessarily have to be fully identified */
staticfn boolean
cause_known(
    int propindx) /* index of a property which can be conveyed by worn item */
{
    struct obj *o;
    long mask = W_ARMOR | W_AMUL | W_RING | W_TOOL;

    /* simpler than from_what()/what_gives(); we don't attempt to
       handle artifacts and we deliberately ignore wielded items */
    for (o = gi.invent; o; o = o->nobj) {
        if (!(o->owornmask & mask))
            continue;
        if ((int) objects[o->otyp].oc_oprop == propindx
            && objects[o->otyp].oc_name_known && o->dknown)
            return TRUE;
    }
    return FALSE;
}

/* format a characteristic value, accommodating Strength's strangeness */
staticfn char *
attrval(
    int attrindx,
    int attrvalue,
    char resultbuf[]) /* should be at least [7] to hold "18/100\0" */
{
    if (attrindx != A_STR || attrvalue <= 18)
        Sprintf(resultbuf, "%d", attrvalue);
    else if (attrvalue > STR18(100)) /* 19 to 25 */
        Sprintf(resultbuf, "%d", attrvalue - 100);
    else /* simplify "18/\**" to be "18/100" */
        Sprintf(resultbuf, "18/%02d", attrvalue - 18);
    return resultbuf;
}

/* format urealtime.realtime as
      " D days, H hours, M minutes and S seconds"
   with any fields having a value of 0 omitted:
      0-00:00:20 => " 20 seconds"
      0-00:15:05 => " 15 minutes and 5 seconds"
      0-00:16:00 => " 16 minutes"
      0-01:15:10 => " 1 hour, 15 minutes and 10 seconds"
      0-02:00:01 => " 2 hours and 1 second"
      3-00:25:40 => " 3 days, 25 minutes and 40 seconds"
   (note: for a list of more than two entries, nethack usually includes the
   [style-wise] optional comma before "and" but in this instance it does not)
 */
staticfn char *
fmt_elapsed_time(char *outbuf, int final)
{
    int fieldcnt;
    long edays, ehours, eminutes, eseconds;
    /* for a game that's over, reallydone() has updated urealtime.realtime
       to its final value before calling us during end of game disclosure;
       for a game that's still in progress, it holds the amount of elapsed
       game time from previous sessions up through most recent save/restore
       (or up through latest level change when 'checkpoint' is On);
       '.start_timing' has a non-zero value even if '.realtime' is 0 */
    long etim = urealtime.realtime;

    if (!final)
        etim += timet_delta(getnow(), urealtime.start_timing);
    /* we could use localtime() to convert the value into a 'struct tm'
       to get date and time fields but this is simple and straightforward */
    eseconds = etim % 60L, etim /= 60L;
    eminutes = etim % 60L, etim /= 60L;
    ehours = etim % 24L;
    edays = etim / 24L;
    fieldcnt = !!edays + !!ehours + !!eminutes + !!eseconds;

    Strcpy(outbuf, fieldcnt ? "" : " нет"); /* 'нет' не должно случиться */
    if (edays) {
        Sprintf(eos(outbuf), " %ld дн.", edays);
        if (fieldcnt > 1) /* hours and/or minutes and/or seconds to follow */
            Strcat(outbuf, (fieldcnt == 2) ? " и" : ",");
        --fieldcnt; /* edays has been processed */
    }
    if (ehours) {
        Sprintf(eos(outbuf), " %ld ч.", ehours);
        if (fieldcnt > 1) /* minutes and/or seconds to follow */
            Strcat(outbuf, (fieldcnt == 2) ? " и" : ",");
        --fieldcnt; /* ehours has been processed */
    }
    if (eminutes) {
        Sprintf(eos(outbuf), " %ld мин.", eminutes);
        if (fieldcnt > 1) /* seconds to follow */
            Strcat(outbuf, " и");
        /* eminutes has been processed but no need to decrement fieldcnt */
    }
    if (eseconds)
        Sprintf(eos(outbuf), " %ld сек.", eseconds);
    return outbuf;
}

/* "once" vs "twice" vs "17 times", used in several places */
staticfn char *
N_times(long n, char *outbuf)
{
    switch (n) {
    case 0:
    default:
        Sprintf(outbuf, "%ld раз", n);
        break;
    case 1:
        Strcpy(outbuf, "один раз");
        break;
    case 2:
        Strcpy(outbuf, "два раза");
        break;
    case 3:
        Strcpy(outbuf, "три раза");
        break;
    }
    return outbuf;
}

void
enlightenment(
    int mode,  /* BASICENLIGHTENMENT | MAGICENLIGHTENMENT (| both) */
    int final) /* ENL_GAMEINPROGRESS:0, ENL_GAMEOVERALIVE, ENL_GAMEOVERDEAD */
{
    char buf[BUFSZ], tmpbuf[BUFSZ];

    ge.en_win = create_nhwindow(NHW_MENU);
    ge.en_via_menu = !final;
    if (ge.en_via_menu)
        start_menu(ge.en_win, MENU_BEHAVE_STANDARD);

    Strcpy(tmpbuf, svp.plname);
    *tmpbuf = highc(*tmpbuf); /* same adjustment as bottom line */
    /* as in background_enlightenment, when poly'd we need to use the saved
       gender in u.mfemale rather than the current you-as-monster gender */
    Snprintf(buf, sizeof(buf), "%s the %s's attributes:", tmpbuf,
             ((Upolyd ? u.mfemale : flags.female) && gu.urole.name.f)
                ? gu.urole.name.f
                : gu.urole.name.m);

    /* title */
    enlght_out(buf); /* "Conan the Archeologist's attributes:" */
    /* background and characteristics; ^X or end-of-game disclosure */
    if (mode & BASICENLIGHTENMENT) {
        /* role, race, alignment, deities, dungeon level, time, experience */
        background_enlightenment(mode, final);
        /* hit points, energy points, armor class, gold */
        basics_enlightenment(mode, final);
        /* strength, dexterity, &c */
        characteristics_enlightenment(mode, final);
    }
    /* expanded status line information, including things which aren't
       included there due to space considerations;
       shown for both basic and magic enlightenment */
    status_enlightenment(mode, final);
    /* remaining attributes; shown for potion,&c or wizard mode and
       explore mode ^X or end of game disclosure */
    if (mode & MAGICENLIGHTENMENT) {
        /* intrinsics and other traditional enlightenment feedback */
        attributes_enlightenment(mode, final);
    }

    enlght_out(""); /* separator */
    enlght_out("Разное:");
    /* reminder to player and/or information for dumplog */
    if ((mode & BASICENLIGHTENMENT) != 0 && (wizard || discover || final)) {
        if (wizard || discover) {
            Sprintf(buf, "работаете в режиме %s", wizard ? "отладки" : "исследования");
            you_are(buf, "");
        }

        if (!flags.bones) {
            /* mention not saving bones iff hero just died */
            Sprintf(buf, "отключена загрузка%s уровней-костей",
                    (final == ENL_GAMEOVERDEAD) ? " и сохранение" : "");
            you_have_X(buf);
        } else if (!u.uroleplay.numbones) {
            enl_msg(You_, "не встречали", "не встречали",
                    " ни одного уровня-костей", "");
        } else {
            Sprintf(buf, "встречали %ld уровней-костей",
                    u.uroleplay.numbones);
            you_have_X(buf);
        }
    }
    (void) fmt_elapsed_time(buf, final);
    enl_msg("Общее время игры ", "", "", buf, "");

    if (!ge.en_via_menu) {
        display_nhwindow(ge.en_win, TRUE);
    } else {
        menu_item *selected = 0;

        end_menu(ge.en_win, (char *) 0);
        if (select_menu(ge.en_win, PICK_NONE, &selected) > 0)
            free((genericptr_t) selected);
        ge.en_via_menu = FALSE;
    }
    destroy_nhwindow(ge.en_win);
    ge.en_win = WIN_ERR;
}

/*ARGSUSED*/
/* display role, race, alignment and such to en_win */
staticfn void
background_enlightenment(int unused_mode UNUSED, int final)
{
    const char *role_titl, *rank_titl;
    int innategend, difgend, difalgn;
    char buf[BUFSZ], tmpbuf[BUFSZ];

    /* note that if poly'd, we need to use u.mfemale instead of flags.female
       to access hero's saved gender-as-human/elf/&c rather than current */
    innategend = (Upolyd ? u.mfemale : flags.female) ? 1 : 0;
    role_titl = (innategend && gu.urole.name.f) ? gu.urole.name.f
                                                : gu.urole.name.m;
    rank_titl = rank_of(u.ulevel, Role_switch, innategend);

    enlght_out(""); /* separator after title */
    enlght_out("Происхождение:");

    /* if polymorphed, report current shape before underlying role;
       will be repeated as first status: "you are transformed" and also
       among various attributes: "you are in beast form" (after being
       told about lycanthropy) or "you are polymorphed into <a foo>"
       (with countdown timer appended for wizard mode); we really want
       the player to know he's not a samurai at the moment... */
    if (Upolyd) {
        char anbuf[20]; /* includes trailing space; [4] suffices */
        struct permonst *uasmon = gy.youmonst.data;
        boolean altphrasing = vampshifted(&gy.youmonst);

        tmpbuf[0] = '\0';
        /* here we always use current gender, not saved role gender */
        if (!is_male(uasmon) && !is_female(uasmon) && !is_neuter(uasmon))
            Sprintf(tmpbuf, "%s ", genders[flags.female ? 1 : 0].adj);
        if (altphrasing)
            Sprintf(eos(tmpbuf), "%s in ",
                    pmname(&mons[gy.youmonst.cham],
                           flags.female ? FEMALE : MALE));
        Snprintf(buf, sizeof(buf), "%s%s%s%s",
                 !final ? "сейчас " : "",
                 altphrasing ? just_an(anbuf, tmpbuf) : "в форме ",
                 tmpbuf, pmname(uasmon, flags.female ? FEMALE : MALE));
        you_are(buf, "");
    }

    /* report role; omit gender if it's redundant (eg, "female priestess") */
    tmpbuf[0] = '\0';
    if (!gu.urole.name.f
        && ((gu.urole.allow & ROLE_GENDMASK) == (ROLE_MALE | ROLE_FEMALE)
            || innategend != flags.initgend))
        Sprintf(tmpbuf, "%s ", genders[innategend].adj);
    buf[0] = '\0';
    if (Upolyd)
        Strcpy(buf, "на самом деле "); /* "Вы на самом деле ..." */
    if (!strcmpi(rank_titl, role_titl)) {
        /* omit role when rank title matches it */
        Sprintf(eos(buf), "%s, уровень %d %s%s", rank_titl, u.ulevel,
                tmpbuf, gu.urace.noun);
    } else {
        Sprintf(eos(buf), "%s, уровень %d %s%s %s", rank_titl, u.ulevel,
                tmpbuf, gu.urace.adj, role_titl);
    }
    you_are(buf, "");

    /* report alignment (bypass you_are() in order to omit ending period);
       adverb is used to distinguish between temporary change (helm of opp.
       alignment), permanent change (one-time conversion), and original */
    Sprintf(buf, " %s%s%s, %sна миссии для %s",
            You_, !final ? are : were,
            align_str(u.ualign.type),
            /* helm of opposite alignment (might hide conversion) */
            (u.ualign.type != u.ualignbase[A_CURRENT])
               /* what's the past tense of "currently"? if we used "formerly"
                  it would sound like a reference to the original alignment */
               ? (!final ? "сейчас " : "временно ")
               /* permanent conversion */
               : (u.ualign.type != u.ualignbase[A_ORIGINAL])
                  /* and what's the past tense of "now"? certainly not "then"
                     in a context like this...; "belatedly" == weren't that
                     way sooner (in other words, didn't start that way) */
                  ? (!final ? "теперь " : "запоздало ")
                  /* atheist (ignored in very early game) */
                  : (!u.uconduct.gnostic && svm.moves > 1000L)
                     ? "номинально "
                     /* lastly, normal case */
                     : "",
            u_gname());
    enlght_out(buf);
    /* show the rest of this game's pantheon (finishes previous sentence)
       [appending "also Moloch" at the end would allow for straightforward
       trailing "and" on all three aligned entries but looks too verbose] */
    Sprintf(buf, ", которому %s противостоят", !final ? "" : "противостояли");
    if (u.ualign.type != A_LAWFUL)
        Sprintf(eos(buf), " %s (%s) и", align_gname(A_LAWFUL),
                align_str(A_LAWFUL));
    if (u.ualign.type != A_NEUTRAL)
        Sprintf(eos(buf), " %s (%s)%s", align_gname(A_NEUTRAL),
                align_str(A_NEUTRAL),
                (u.ualign.type != A_CHAOTIC) ? " и" : "");
    if (u.ualign.type != A_CHAOTIC)
        Sprintf(eos(buf), " %s (%s)", align_gname(A_CHAOTIC),
                align_str(A_CHAOTIC));
    Strcat(buf, "."); /* terminate sentence */
    enlght_out(buf);

    /* show original alignment,gender,race,role if any have been changed;
       giving separate message for temporary alignment change bypasses need
       for tricky phrasing otherwise necessitated by possibility of having
       helm of opposite alignment mask a permanent alignment conversion */
    difgend = (innategend != flags.initgend);
    difalgn = (((u.ualign.type != u.ualignbase[A_CURRENT]) ? 1 : 0)
               + ((u.ualignbase[A_CURRENT] != u.ualignbase[A_ORIGINAL])
                  ? 2 : 0));
    if (difalgn & 1) { /* have temporary alignment so report permanent one */
        Sprintf(buf, "на самом деле %s", align_str(u.ualignbase[A_CURRENT]));
        you_are(buf, "");
        difalgn &= ~1; /* suppress helm from "started out <foo>" message */
    }
    if (difgend || difalgn) { /* sex change or perm align change or both */
        Sprintf(buf, " Вы начинали как %s%s%s.",
                difgend ? genders[flags.initgend].adj : "",
                (difgend && difalgn) ? " и " : "",
                difalgn ? align_str(u.ualignbase[A_ORIGINAL]) : "");
        enlght_out(buf);
    }

    /* "You are left-handed." won't work well if polymorphed into something
       without hands; use "You are normally left-handed." in that situation */
    Sprintf(buf, "%s%sша",
            !strcmp(body_part(HANDED), "handed") ? "обычно " : "",
            URIGHTY ? "прав" : "лев");
    you_are(buf, "");

    /* As of 3.6.2: dungeon level, so that ^X really has all status info as
       claimed by the comment below; this reveals more information than
       the basic status display, but that's one of the purposes of ^X;
       similar information is revealed by #overview; the "You died in
       <location>" given by really_done() is more rudimentary than this */
    *buf = *tmpbuf = '\0';
    if (In_endgame(&u.uz)) {
        int egdepth = observable_depth(&u.uz);

        (void) endgamelevelname(tmpbuf, egdepth);
        Snprintf(buf, sizeof buf, "в эндшпиле, на %s%s",
                 "", tmpbuf);
    } else if (Is_knox(&u.uz)) {
        /* this gives away the fact that the knox branch is only 1 level */
        Sprintf(buf, "на %s уровне", svd.dungeons[u.uz.dnum].dname);
        /* TODO? maybe phrase it differently when actually inside the fort,
           if we're able to determine that (not trivial) */
    } else {
        char dgnbuf[QBUFSZ];

        Strcpy(dgnbuf, svd.dungeons[u.uz.dnum].dname);
        if (!strncmpi(dgnbuf, "The ", 4))
            *dgnbuf = lowc(*dgnbuf);
        Sprintf(tmpbuf, "уровень %d",
                In_quest(&u.uz) ? dunlev(&u.uz) : depth(&u.uz));
        /* TODO? maybe extend this bit to include various other automatic
           annotations from the dungeon overview code */
        if (Is_rogue_level(&u.uz))
            Strcat(tmpbuf, ", примитивная область");
        else if (Is_bigroom(&u.uz) && !Blind)
            Strcat(tmpbuf, ", очень большая комната");
        Snprintf(buf, sizeof buf, "в %s, на %s", dgnbuf, tmpbuf);
    }
    you_are(buf, "");

    /* this is shown even if the 'time' option is off */
    if (svm.moves == 1L) {
        you_have("только что начали приключение", "");
    } else {
        /* 'turns' grates on the nerves in this context... */
        Sprintf(buf, "%ld ходов назад", svm.moves);
        /* same phrasing for current and final: "entered" is unconditional */
        enlght_line(You_, "вошли в подземелье ", buf, "");
    }

    /* for gameover, these have been obtained in really_done() so that they
       won't vary if user leaves a disclosure prompt or --More-- unanswered
       long enough for the dynamic value to change between then and now */
    if (final ? iflags.at_midnight : midnight()) {
        enl_msg("Сейчас ", "", "", "полночь", "");
    } else if (final ? iflags.at_night : night()) {
        enl_msg("Сейчас ", "", "", "ночь", "");
    }
    /* other environmental factors */
    if (flags.moonphase == FULL_MOON || flags.moonphase == NEW_MOON) {
        /* [This had "tonight" but has been changed to "in effect".
           There is a similar issue to Friday the 13th--it's the value
           at the start of the current session but that session might
           have dragged on for an arbitrary amount of time.  We want to
           report the values that currently affect play--or affected
           play when game ended--rather than actual outside situation.] */
        Sprintf(buf, "сейчас %s%s",
                (flags.moonphase == FULL_MOON) ? "полнолуние"
                : (flags.moonphase == NEW_MOON) ? "новолуние"
                  /* showing these would probably just lead to confusion
                     since they have no effect on game play... */
                  : (flags.moonphase < FULL_MOON) ? "луна в первой четверти"
                    : "луна в последней четверти",
                /* we don't have access to 'how' here--aside from survived
                   vs died--so settle for general platitude */
                final ? ", когда приключение закончилось" : "");
        enl_msg("", "", "", buf, "");
    }
    if (flags.friday13) {
        /* let player know that friday13 penalty is/was in effect;
           we don't say "it is/was Friday the 13th" because that was at
           the start of the session and it might be past midnight (or
           days later if the game has been paused without save/restore),
           so phrase this similar to the start up message */
        Sprintf(buf, " Плохие события %s в пятницу 13-го.",
                !final ? "возможны"
                : (final == ENL_GAMEOVERALIVE) ? "могли случиться"
                  /* there's no may to tell whether -1 Luck made a
                     difference but hero has died... */
                  : "произошли");
        enlght_out(buf);
    }

    if (!Upolyd) {
        int ulvl = (int) u.ulevel;
        /* [flags.showexp currently does not matter; should it?] */

        /* experience level is already shown above */
        Sprintf(buf, "%-1ld очков опыта", u.uexp);
        /* TODO?
         *  Remove wizard-mode restriction since patient players can
         *  determine the numbers needed without resorting to spoilers
         *  (even before this started being disclosed for 'final';
         *  just enable 'showexp' and look at normal status lines
         *  after drinking gain level potions or eating wraith corpses
         *  or being level-drained by vampires).
         */
        if (ulvl < 30 && (final || wizard)) {
            long nxtlvl = newuexp(ulvl), delta = nxtlvl - u.uexp;

            Sprintf(eos(buf), ", ещё %ld очков до %s уровня %d",
                    delta,
                    /* present tense=="needed", past tense=="were needed" */
                    (ulvl < 18) ? "следующего" : "след.",
                    (ulvl + 1));
        }
        you_have(buf, "");
    }
#ifdef SCORE_ON_BOTL
    if (flags.showscore) {
        /* describes what's shown on status line, which is an approximation;
           only show it here if player has the 'showscore' option enabled */
        Sprintf(buf, "%ld%s", botl_score(),
                !final ? "" : " после итоговых корректировок");
        enl_msg("Ваш счёт ", "", "", buf, "");
    }
#endif
}

/* hit points, energy points, armor class -- essential information which
   doesn't fit very well in other categories */
/*ARGSUSED*/
staticfn void
basics_enlightenment(int mode UNUSED, int final)
{
    static char Power[] = "очков энергии (сила заклинаний)";
    char buf[BUFSZ];
    int pw = u.uen, hp = (Upolyd ? u.mh : u.uhp),
        pwmax = u.uenmax, hpmax = (Upolyd ? u.mhmax : u.uhpmax);

    enlght_out(""); /* separator after background */
    enlght_out("Основное:");

    if (hp < 0)
        hp = 0;
    /* "1 out of 1" rather than "all" if max is only 1; should never happen */
    if (hp == hpmax && hpmax > 1)
        Sprintf(buf, "имеете все %d очков здоровья", hpmax);
    else
        Sprintf(buf, "имеете %d из %d очков здоровья", hp, hpmax);
    you_have(buf, "");

    /* low max energy is feasible, so handle couple of extra special cases */
    if (pwmax == 0 || (pw == pwmax && pwmax == 2)) /* both: not "all 2" */
        Sprintf(buf, "%s %s", !pwmax ? "не имеете" : "имеете оба", Power);
    else if (pw == pwmax && pwmax > 2)
        Sprintf(buf, "имеете все %d %s", pwmax, Power);
    else
        Sprintf(buf, "имеете %d из %d %s", pw, pwmax, Power);
    you_have(buf, "");

    if (Upolyd) {
        switch (mons[u.umonnum].mlevel) {
        case 0:
            /* status line currently being explained shows "HD:0" */
            Strcpy(buf, "0 хитов (на деле 1/2)");
            break;
        case 1:
            Strcpy(buf, "1 хит");
            break;
        default:
            Sprintf(buf, "%d хитов", mons[u.umonnum].mlevel);
            break;
        }
        you_have(buf, "");
    }

    find_ac(); /* enforces AC_MAX cap */
    Sprintf(buf, "%d", u.uac);
    if (abs(u.uac) == AC_MAX)
        Sprintf(eos(buf), ", %s из возможных",
                (u.uac < 0) ? "лучший" : "худший");
    enl_msg("Ваш класс доспеха ", "", "", buf, "");

    /* gold; similar to doprgold (#showgold) but without shop billing info;
       includes container contents, unlike status line but like doprgold */
    {
        long umoney = money_cnt(gi.invent), hmoney = hidden_gold(final);

        if (!umoney) {
            Sprintf(buf, " Ваш кошелёк %s пуст", !final ? "" : "был");
        } else {
            Sprintf(buf, " В вашем кошельке %s %ld %s", !final ? "" : "было",
                    umoney, currency(umoney));
        }
        /* terminate the wallet line if appropriate, otherwise add an
           introduction to subsequent continuation; output now either way */
        Strcat(buf, !hmoney ? "." : !umoney ? ", но" : ", и");
        enlght_out(buf);

        /* put contained gold on its own line to avoid excessive width; it's
           phrased as a continuation of the wallet line so not capitalized */
        if (hmoney) {
            Sprintf(buf, "%ld %s спрятано в рюкзаке",
                    hmoney, umoney ? "дополнительно" : currency(hmoney));
            enl_msg("у вас ", "есть ", "было ", buf, "");
        }
    }

    if (flags.pickup) {
        char ocl[MAXOCLASSES + 1];

        Strcpy(buf, "включён");
        if (costly_spot(u.ux, u.uy)) {
            /* being in a shop inhibits autopickup, even 'pickup_thrown' */
            Strcat(buf, ", но временно отключён внутри лавки");
        } else {
            oc_to_str(flags.pickup_types, ocl);
            Sprintf(eos(buf), " для %s%s%s", *ocl ? "'" : "",
                    *ocl ? ocl : "всех типов", *ocl ? "'" : "");
            if (flags.pickup_thrown && *ocl)
                Strcat(buf, " плюс брошенное"); /* show when not 'all types' */
            if (ga.apelist)
                Strcat(buf, ", с исключениями");
        }
    } else
        Strcpy(buf, "выключен");
    enl_msg("Автоподбор ", "", "", buf, "");
}

/* characteristics: expanded version of bottom line strength, dexterity, &c */
staticfn void
characteristics_enlightenment(int mode, int final)
{
    char buf[BUFSZ];

    enlght_out("");
    Sprintf(buf, "%sХарактеристики:", !final ? "" : "Итоговые ");
    enlght_out(buf);

    /* bottom line order */
    one_characteristic(mode, final, A_STR); /* strength */
    one_characteristic(mode, final, A_DEX); /* dexterity */
    one_characteristic(mode, final, A_CON); /* constitution */
    one_characteristic(mode, final, A_INT); /* intelligence */
    one_characteristic(mode, final, A_WIS); /* wisdom */
    one_characteristic(mode, final, A_CHA); /* charisma */
}

/* display one attribute value for characteristics_enlightenment() */
staticfn void
one_characteristic(int mode, int final, int attrindx)
{
    extern const char *const attrname[]; /* attrib.c */
    boolean hide_innate_value = FALSE, interesting_alimit;
    int acurrent, abase, apeak, alimit;
    const char *paren_pfx;
    char subjbuf[BUFSZ], valubuf[BUFSZ], valstring[32];

    /* being polymorphed or wearing certain cursed items prevents
       hero from reliably tracking changes to characteristics so
       we don't show base & peak values then; when the items aren't
       cursed, hero could take them off to check underlying values
       and we show those in such case so that player doesn't need
       to actually resort to doing that */
    if (Upolyd) {
        hide_innate_value = TRUE;
    } else if (Fixed_abil) {
        if (stuck_ring(uleft, RIN_SUSTAIN_ABILITY)
            || stuck_ring(uright, RIN_SUSTAIN_ABILITY))
            hide_innate_value = TRUE;
    }
    switch (attrindx) {
    case A_STR:
        if (uarmg && uarmg->otyp == GAUNTLETS_OF_POWER && uarmg->cursed)
            hide_innate_value = TRUE;
        break;
    case A_DEX:
        break;
    case A_CON:
        if (u_wield_art(ART_OGRESMASHER) && uwep->cursed)
            hide_innate_value = TRUE;
        break;
    case A_INT:
        if (uarmh && uarmh->otyp == DUNCE_CAP && uarmh->cursed)
            hide_innate_value = TRUE;
        break;
    case A_WIS:
        if (uarmh && uarmh->otyp == DUNCE_CAP && uarmh->cursed)
            hide_innate_value = TRUE;
        break;
    case A_CHA:
        break;
    default:
        return; /* impossible */
    };
    /* note: final disclosure includes MAGICENLIGHTENTMENT */
    if ((mode & MAGICENLIGHTENMENT) && !Upolyd)
        hide_innate_value = FALSE;

    acurrent = ACURR(attrindx);
    (void) attrval(attrindx, acurrent, valubuf); /* Sprintf(valubuf,"%d",) */
    Sprintf(subjbuf, "%s: ", attrname[attrindx]);

    if (!hide_innate_value) {
        /* show abase, amax, and/or attrmax if acurr doesn't match abase
           (a magic bonus or penalty is in effect) or abase doesn't match
           amax (some points have been lost to poison or exercise abuse
           and are restorable) or attrmax is different from normal human
           (while game is in progress; trying to reduce dependency on
           spoilers to keep track of such stuff) or attrmax was different
           from abase (at end of game; this attribute wasn't maxed out) */
        abase = ABASE(attrindx);
        apeak = AMAX(attrindx);
        alimit = ATTRMAX(attrindx);
        /* criterium for whether the limit is interesting varies */
        interesting_alimit =
            final ? TRUE /* was originally `(abase != alimit)' */
                  : (alimit != (attrindx != A_STR ? 18 : STR18(100)));
        paren_pfx = final ? " (" : " (текущее; ";
        if (acurrent != abase) {
            Sprintf(eos(valubuf), "%sбазовое:%s", paren_pfx,
                    attrval(attrindx, abase, valstring));
            paren_pfx = ", ";
        }
        if (abase != apeak) {
            Sprintf(eos(valubuf), "%sмакс.:%s", paren_pfx,
                    attrval(attrindx, apeak, valstring));
            paren_pfx = ", ";
        }
        if (interesting_alimit) {
            Sprintf(eos(valubuf), "%s%sпредел:%s", paren_pfx,
                    /* more verbose if exceeding 'limit' due to magic bonus */
                    (acurrent > alimit) ? "врождённое " : "",
                    attrval(attrindx, alimit, valstring));
            /* paren_pfx = ", "; */
        }
        if (acurrent != abase || abase != apeak || interesting_alimit)
            Strcat(valubuf, ")");
    }
    enl_msg(subjbuf, "", "", valubuf, "");
}

/* status: selected obvious capabilities, assorted troubles */
staticfn void
status_enlightenment(int mode, int final)
{
    boolean magic = (mode & MAGICENLIGHTENMENT) ? TRUE : FALSE;
    int cap;
    char buf[BUFSZ], youtoo[BUFSZ], heldmon[BUFSZ];
    boolean Riding = (u.usteed
                      /* if hero dies while dismounting, u.usteed will still
                         be set; we want to ignore steed in that situation */
                      && !(final == ENL_GAMEOVERDEAD
                           && !strcmp(svk.killer.name, "riding accident")));
    const char *steedname = (!Riding ? (char *) 0
                      : x_monnam(u.usteed,
                                 u.usteed->mtame ? ARTICLE_YOUR : ARTICLE_THE,
                                 (char *) 0,
                                 (SUPPRESS_SADDLE | SUPPRESS_HALLUCINATION),
                                 FALSE));

    /*\
     * Status (many are abbreviated on bottom line; others are or
     *     should be discernible to the hero hence to the player)
    \*/
    enlght_out(""); /* separator after title or characteristics */
    enlght_out(final ? "Итоговый статус:" : "Статус:");

    Strcpy(youtoo, You_);
    /* not a traditional status but inherently obvious to player; more
       detail given below (attributes section) for magic enlightenment */
    if (Upolyd) {
        Strcpy(buf, "превращены");
        if (ugenocided())
            Sprintf(eos(buf), " и %s %s внутри",
                    final ? "чувствовали" : "чувствуете", udeadinside());
        you_are(buf, "");
    }
    /* not a trouble, but we want to display riding status before maybe
       reporting steed as trapped or hero stuck to cursed saddle */
    if (Riding) {
        Sprintf(buf, "верхом на %s", steedname);
        you_are(buf, "");
        Sprintf(eos(youtoo), "и %s ", steedname);
    }
    /* other movement situations that hero should always know */
    if (Levitation) {
        if (Lev_at_will && magic)
            you_are("левитируете, когда захотите", "");
        else
            enl_msg(youtoo, are, were, "левитируете", from_what(LEVITATION));
    } else if (Flying) { /* can only fly when not levitating */
        enl_msg(youtoo, are, were, "летите", from_what(FLYING));
    }
    if (Underwater) {
        you_are("под водой", "");
    } else if (u.uinwater) {
        you_are(Swimming ? "плывёте" : "в воде", from_what(SWIMMING));
    } else if (walking_on_water()) {
        /* show active Wwalking here, potential Wwalking elsewhere */
        Sprintf(buf, "идёте по %s",
                is_pool(u.ux, u.uy) ? "воде"
                : is_lava(u.ux, u.uy) ? "лаве"
                  : surface(u.ux, u.uy)); /* catchall; shouldn't happen */
        you_are(buf, from_what(WWALKING));
    }
    if (Upolyd && (u.uundetected || U_AP_TYPE != M_AP_NOTHING))
        youhiding(TRUE, final);

    /* internal troubles, mostly in the order that prayer ranks them */
    if (Stoned) {
        if (final && (Stoned & I_SPECIAL))
            enlght_out(" Вы превратились в камень.");
        else
            you_are("превращаетесь в камень", "");
    }
    if (Slimed) {
        if (final && (Slimed & I_SPECIAL))
            enlght_out(" Вы превратились в слизь.");
        else
            you_are("превращаетесь в слизь", "");
    }
    if (Strangled) {
        if (u.uburied) {
            you_are("закопаны", "");
        } else {
            if (final && (Strangled & I_SPECIAL)) {
                enlght_out(" Вы погибли от удушения.");
            } else {
                Strcpy(buf, "вас душат");
                if (wizard)
                    Sprintf(eos(buf), " (%ld)", (Strangled & TIMEOUT));
                you_are(buf, from_what(STRANGLED));
            }
        }
    }
    if (Sick) {
        /* the two types of sickness are lumped together; hero can be
           afflicted by both but there is only one timeout; botl status
           puts TermIll before FoodPois and death due to timeout reports
           terminal illness if both are in effect, so do the same here */
        if (final && (Sick & I_SPECIAL)) {
            Sprintf(buf, " %sумерли от %s.", You_, /* has trailing space */
                    (u.usick_type & SICK_NONVOMITABLE)
                    ? "неизлечимой болезни" : "пищевого отравления");
            enlght_out(buf);
        } else {
            /* unlike death due to sickness, report the two cases separately
               because it is possible to cure one without curing the other */
            if (u.usick_type & SICK_NONVOMITABLE)
                you_are("неизлечимо больны", "");
            if (u.usick_type & SICK_VOMITABLE)
                you_are("неизлечимо больны из-за пищевого отравления", "");
        }
    }
    if (Vomiting)
        you_are("испытываете тошноту", "");
    if (Stunned)
        you_are("оглушены", "");
    if (Confusion)
        you_are("сбиты с толку", "");
    if (Hallucination)
        you_are("галлюцинируете", "");
    if (Blind) {
        /* check the reasons in same order as from_what() */
        Sprintf(buf, "%s слепы",
                (HBlinded & FROMOUTSIDE) != 0L ? "постоянно"
                : (HBlinded & FROMFORM) ? "от природы"
                  /* better phrasing desperately wanted... */
                  : Blindfolded_only ? "намеренно"
                    /* timed, possibly combined with blindfold */
                    : "временно");
        if (wizard && (HBlinded == BlindedTimeout && !Blindfolded))
            Sprintf(eos(buf), " (%ld)", BlindedTimeout);
        /* !haseyes: avoid "you are innately blind innately" */
        you_are(buf, !haseyes(gy.youmonst.data) ? "" : from_what(BLINDED));
    }
    if (Deaf)
        you_are("глухи", from_what(DEAF));

    /* external troubles, more or less */
    if (Punished) {
        if (uball) {
            Sprintf(buf, "прикованы к %s", ansimpleoname(uball));
        } else {
            impossible("Punished without uball?");
            Strcpy(buf, "наказаны");
        }
        you_are(buf, "");
    }
    if (u.utrap) {
        char predicament[BUFSZ];
        boolean anchored = (u.utraptype == TT_BURIEDBALL);

        (void) trap_predicament(predicament, final, wizard);
        if (u.usteed) { /* not `Riding' here */
            Sprintf(buf, "%s%s ", anchored ? "вы и " : "", steedname);
            *buf = highc(*buf);
            enl_msg(buf, "", "", predicament, "");
        } else
            you_are(predicament, "");
    } /* (u.utrap) */
    heldmon[0] = '\0'; /* lint suppression */
    if (u.ustuck) { /* includes u.uswallow */
        Strcpy(heldmon, a_monnam(u.ustuck));
        if (!strcmp(heldmon, "it")
            && (!has_mgivenname(u.ustuck)
                || strcmp(MGIVENNAME(u.ustuck), "it") != 0))
            Strcpy(heldmon, "невидимое существо");
    }
    if (u.uswallow) {
        assert(u.ustuck != NULL); /* implied by u.uswallow */
        Snprintf(buf, sizeof buf, "%s %s",
                digests(u.ustuck->data) ? "вас проглотил" : "вас поглотил",
                heldmon);
        if (dmgtype(u.ustuck->data, AD_DGST)) {
            /* if final, death via digestion can be deduced by u.uswallow
               still being True and u.uswldtim having been decremented to 0 */
            if (final && !u.uswldtim)
                Strcat(buf, " и полностью переварен");
            else
                Sprintf(eos(buf), " и %s переварен",
                        final ? "был" : "будет");
        }
        if (wizard)
            Sprintf(eos(buf), " (%u)", u.uswldtim);
        you_are(buf, "");
    } else if (u.ustuck) {
        boolean ustick = (Upolyd && sticks(gy.youmonst.data));
        int dx = u.ustuck->mx - u.ux, dy = u.ustuck->my - u.uy;

        Snprintf(buf, sizeof buf, "%s %s (%s)",
                 ustick ? "держите" : "скованы с",
                 heldmon, dxdy_to_dist_descr(dx, dy, TRUE));
        you_are(buf, "");
    }
    if (Riding) {
        struct obj *saddle = which_armor(u.usteed, W_SADDLE);

        if (saddle && saddle->cursed) {
            Sprintf(buf, "прилипли к %s %s", s_suffix(steedname),
                    simpleonames(saddle));
            you_are(buf, "");
        }
    }
    if (Wounded_legs) {
        /* EWounded_legs is used to track left/right/both rather than some
           form of extrinsic impairment; HWounded_legs is used for timeout;
           both apply to steed instead of hero when mounted */
        long whichleg = (EWounded_legs & BOTH_SIDES);
        const char *bp = u.usteed ? mbodypart(u.usteed, LEG) : body_part(LEG),
            *article = "раненая ", /* precedes "wounded", so never "an " */
            *leftright = "";

        if (whichleg == BOTH_SIDES)
            bp = makeplural(bp), article = "раненые ";
        else
            leftright = (whichleg == LEFT_SIDE) ? "левая " : "правая ";
        Sprintf(buf, "%s%s %s", article, leftright, bp);

        /* when mounted, Wounded_legs applies to steed rather than to
           hero; we only report steed's wounded legs in wizard mode */
        if (u.usteed) { /* not `Riding' here */
            if (wizard && steedname) {
                char steednambuf[BUFSZ];

                Strcpy(steednambuf, steedname);
                *steednambuf = highc(*steednambuf);
                enl_msg(steednambuf, "", "", buf, "");
            }
        } else {
            you_have(buf, "");
        }
    }
    if (Glib) {
        Sprintf(buf, "имеете скользкие %s", fingers_or_gloves(TRUE));
        if (wizard)
            Sprintf(eos(buf), " (%ld)", (Glib & TIMEOUT));
        you_have(buf, "");
    }
    if (Fumbling) {
        if (magic || cause_known(FUMBLING))
            enl_msg(You_, "роняете предметы", "роняли предметы", "", from_what(FUMBLING));
    }
    if (Sleepy) {
        if (magic || cause_known(SLEEPY)) {
            Strcpy(buf, from_what(SLEEPY));
            if (wizard)
                Sprintf(eos(buf), " (%ld)", (HSleepy & TIMEOUT));
            enl_msg("Вы ", "засыпаете", "уснули", " против своей воли", buf);
        }
    }
    /* hunger/nutrition */
    if (Hunger) {
        if (magic || cause_known(HUNGER))
            enl_msg(You_, "голодаете", "голодали", " быстро",
                    from_what(HUNGER));
    }
    Strcpy(buf, hu_stat[u.uhs]); /* hunger status; omitted if "normal" */
    mungspaces(buf);             /* strip trailing spaces */
    /* status line doesn't show hunger when state is "not hungry", we do;
       needed for wizard mode's reveal of u.uhunger but add it for everyone */
    if (!*buf)
        Strcpy(buf, "не голодны");
    if (*buf) { /* (since "not hungry" was added, this will always be True) */
        *buf = lowc(*buf); /* override capitalization */
        if (!strcmp(buf, "weak"))
            Strcat(buf, " из-за сильного голода");
        else if (!strncmp(buf, "faint", 5)) /* fainting, fainted */
            Strcat(buf, " из-за голодания");
        if (wizard)
            Sprintf(eos(buf), " <%d>", u.uhunger);
        you_are(buf, "");
    }
    /* encumbrance */
    if ((cap = near_capacity()) > UNENCUMBERED) {
        const char *adj = "?_?"; /* (should always get overridden) */

        Strcpy(buf, enc_stat[cap]);
        *buf = lowc(*buf);
        switch (cap) {
        case SLT_ENCUMBER:
            adj = "слегка";
            break; /* burdened */
        case MOD_ENCUMBER:
            adj = "умеренно";
            break; /* stressed */
        case HVY_ENCUMBER:
            adj = "сильно";
            break; /* strained */
        case EXT_ENCUMBER:
            adj = "чрезвычайно";
            break; /* overtaxed */
        case OVERLOADED:
            adj = "невозможно";
            break;
        }
        if (wizard)
            Sprintf(eos(buf), " <%d>", inv_weight());
        Sprintf(eos(buf), "; движение %s%s", adj,
                (cap < OVERLOADED) ? " замедлено" : "");
        you_are(buf, "");
    } else {
        /* last resort entry, guarantees Status section is non-empty
           (no longer needed for that purpose since weapon status added;
           still useful though) */
        Strcpy(buf, "не перегружены");
        if (wizard)
            Sprintf(eos(buf), " <%d>", inv_weight());
        you_are(buf, "");
    }
    /* current weapon(s) and corresponding skill level(s) */
    weapon_insight(final);
    /* unlike ring of increase accuracy's effect, the monk's suit penalty
       is too blatant to be restricted to magical enlightenment */
    if (iflags.tux_penalty && !Upolyd) {
        (void) enlght_combatinc("to hit", -gu.urole.spelarmr, final, buf);
        /* if from_what() ever gets extended from wizard mode to normal
           play, it could be adapted to handle this */
        Sprintf(eos(buf), " из-за вашего доспеха %s", suit_simple_name(uarm));
        you_have(buf, "");
    }
    /* report 'nudity' */
    if (!uarm && !uarmu && !uarmc && !uarms && !uarmg && !uarmf && !uarmh) {
        if (u.uroleplay.nudist)
            enl_msg(You_, "не носите", "не носили", " никаких доспехов", "");
        else
            you_are("не носите никаких доспехов", "");
    }
}

/* extracted from status_enlightenment() to reduce clutter there */
staticfn void
weapon_insight(int final)
{
    char buf[BUFSZ];
    int wtype;

    /* report being weaponless; distinguish whether gloves are worn
       [perhaps mention silver ring(s) when not wearing gloves?] */
    if (!uwep) {
        you_are(empty_handed(), "");

    /* two-weaponing implies hands and
       a weapon or wep-tool (not other odd stuff) in each hand */
    } else if (u.twoweap) {
        you_are("сражаетесь двумя оружиями сразу", "");

    /* report most weapons by their skill class (so a katana will be
       described as a long sword, for instance; mattock, hook, and aklys
       are exceptions), or wielded non-weapon item by its object class */
    } else {
        const char *what = weapon_descr(uwep);

        /* [what about other silver items?] */
        if (uwep->otyp == SHIELD_OF_REFLECTION)
            what = shield_simple_name(uwep); /* silver|smooth shield */
        else if (is_wet_towel(uwep))
            what = /* (uwep->spe < 3) ? "moist towel" : */ "мокрое полотенце";

        if (!strcmpi(what, "armor") || !strcmpi(what, "food")
            || !strcmpi(what, "venom"))
            Sprintf(buf, "держите %s", what);
        else
            /* [maybe include known blessed?] */
            Sprintf(buf, "держите %s",
                    (uwep->quan == 1L) ? what : makeplural(what));
        you_are(buf, "");
    }

    /*
     * Skill with current weapon.  Might help players who've never
     * noticed #enhance or decided that it was pointless.
     */
    if ((wtype = weapon_type(uwep)) != P_NONE && (!uwep || !is_ammo(uwep))) {
        char sklvlbuf[20];
        int sklvl = P_SKILL(wtype);
        boolean hav = (sklvl != P_UNSKILLED && sklvl != P_SKILLED);

        if (sklvl == P_ISRESTRICTED)
            Strcpy(sklvlbuf, "нет");
        else
            (void) lcase(skill_level_name(wtype, sklvlbuf));
        /* "you have no/basic/expert/master/grand-master skill with <skill>"
           or "you are unskilled/skilled in <skill>" */
        Sprintf(buf, "%s %s %s", skill_name(wtype),
                hav ? " — навык " : " — уровень ", sklvlbuf);

        if (!u.twoweap) {
            if (can_advance(wtype, FALSE))
                Sprintf(eos(buf), " и %s этот навык",
                        !final ? "можете улучшить" : "могли бы улучшить");
            if (hav)
                you_have(buf, "");
            else
                you_are(buf, "");

        } else { /* two-weapon */
            static const char also_[] = "также ";
            char pfx[QBUFSZ], sfx[QBUFSZ],
                sknambuf2[20], sklvlbuf2[20], twobuf[20];
            const char *also = "", *also2 = "", *also3 = (char *) 0,
                       *verb_present, *verb_past;
            int wtype2 = weapon_type(uswapwep),
                sklvl2 = P_SKILL(wtype2),
                twoskl = P_SKILL(P_TWO_WEAPON_COMBAT);
            boolean a1, a2, ab,
                    hav2 = (sklvl2 != P_UNSKILLED && sklvl2 != P_SKILLED);

            /* normally hero must have access to two-weapon skill in
               order to initiate u.twoweap, but not if polymorphed into
               a form which has multiple weapon attacks, so we need to
               avoid getting bitten by unexpected skill value */
            if (twoskl == P_ISRESTRICTED) {
                twoskl = P_UNSKILLED;
                /* restricted is the same as unskilled as far as bonus
                   or penalty goes, and it isn't ordinarily seen so
                   skill_level_name() returns "Unknown" for it */
                Strcpy(twobuf, "ограничен");
            } else {
                (void) lcase(skill_level_name(P_TWO_WEAPON_COMBAT, twobuf));
            }

            /* keep buf[] from above in case skill levels match */
            pfx[0] = sfx[0] = '\0';
            if (twoskl < sklvl) {
                /* twoskil won't be restricted so sklvl is at least basic */
                Sprintf(pfx, "Ваш навык в %s ", skill_name(wtype));
                Sprintf(sfx, " ограничено уровнем %s при двух оружиях", twobuf);
                also = also_;
            } else if (twoskl > sklvl) {
                /* sklvl might be restricted */
                Strcpy(pfx, "Ваш навык двух оружий ");
                Strcpy(sfx, " ограничено ");
                if (sklvl > P_ISRESTRICTED)
                    Sprintf(eos(sfx), "уровнем %s", sklvlbuf);
                else
                    Sprintf(eos(sfx), "отсутствием навыка");
                Sprintf(eos(sfx), " с %s", skill_name(wtype));
                also2 = also_;
            } else {
                Strcat(buf, " и два оружия");
                also3 = also_;
            }
            if (*pfx)
                enl_msg(pfx, "", "", sfx, "");
            else if (hav)
                you_have(buf, "");
            else
                you_are(buf, "");

            /* skip comparison between secondary and two-weapons if it is
               identical to the comparison between primary and twoweap */
            if (wtype2 != wtype) {
                Strcpy(sknambuf2, skill_name(wtype2));
                (void) lcase(skill_level_name(wtype2, sklvlbuf2));
                verb_present = "is", verb_past = "was";
                pfx[0] = sfx[0] = buf[0] = '\0';
                if (twoskl < sklvl2) {
                    /* twoskil is at least unskilled, sklvl2 at least basic */
                    Sprintf(pfx, "Ваш навык в %s ", sknambuf2);
                    Sprintf(sfx, " %sограничено уровнем %s при двух оружиях",
                            also, twobuf);
                } else if (twoskl > sklvl2) {
                    /* sklvl2 might be restricted */
                    Strcpy(pfx, "Ваш навык двух оружий ");
                    Sprintf(sfx, " %sограничено ", also2);
                    if (sklvl2 > P_ISRESTRICTED)
                        Sprintf(eos(sfx), "уровнем %s", sklvlbuf2);
                    else
                        Strcat(eos(sfx), "отсутствием навыка");
                    Sprintf(eos(sfx), " с %s", sknambuf2);
                } else {
                    /* equal; two-weapon is at least unskilled, so sklvl2 is
                       too; "you [also] have basic/expert/master/grand-master
                       skill with <skill>" or "you [also] are unskilled/
                       skilled in <skill> */
                    Sprintf(buf, "%s %s %s", sknambuf2,
                            hav2 ? " — навык " : " — уровень ", sklvlbuf2);
                    Strcat(buf, " и два оружия");
                    if (also3) {
                        Strcpy(pfx, "Вы также ");
                        Snprintf(sfx, sizeof(sfx), " %s", buf), buf[0] = '\0';
                        verb_present = "";
                        verb_past = "";
                    }
                }
                if (*pfx)
                    enl_msg(pfx, verb_present, verb_past, sfx, "");
                else if (hav2)
                    you_have(buf, "");
                else
                    you_are(buf, "");
            } /* wtype2 != wtype */

            /* if training and available skill credits already allow
               #enhance for any of primary, secondary, or two-weapon,
               tell the player; avoid attempting figure out whether
               spending skill credits enhancing one might make either
               or both of the others become ineligible for enhancement */
            a1 = can_advance(wtype, FALSE);
            a2 = (wtype2 != wtype) ? can_advance(wtype2, FALSE) : FALSE;
            ab = can_advance(P_TWO_WEAPON_COMBAT, FALSE);
            if (a1 || a2 || ab) {
                static const char also_wik_[] = " и также с ";

                /* for just one, the conditionals yield
                   1) "skill with <that one>"; for more than one:
                   2) "skills with <primary> and also with <secondary>" or
                   3) "skills with <primary> and also with two-weapons" or
                   4) "skills with <secondary> and also with two-weapons" or
                   5) "skills with <primary>, <secondary>, and two-weapons"
                   (no 'also's or extra 'with's for case 5); when primary
                   and secondary use the same skill, only cases 1 and 3 are
                   possible because 'a2' gets forced to False above */
                Sprintf(sfx, " навык%s: %s%s%s%s%s",
                        ((int) a1 + (int) a2 + (int) ab > 1) ? "и" : "",
                        a1 ? skill_name(wtype) : "",
                        ((a1 && a2 && ab) ? ", "
                         : (a1 && (a2 || ab)) ? also_wik_ : ""),
                        a2 ? skill_name(wtype2) : "",
                        ((a1 && a2 && ab) ? ", and "
                         : (a2 && ab) ? also_wik_ : ""),
                        ab ? "двумя оружиями" : "");
                enl_msg(You_, "можете улучшить", "могли бы улучшить", sfx, "");
            }
        } /* two-weapon */
    } /* skill applies */
}

staticfn void
item_resistance_message(
    int adtyp,
    const char *prot_message,
    int final)
{
    int protection = u_adtyp_resistance_obj(adtyp);

    if (protection) {
        boolean somewhat = protection < 99;

        enl_msg("Ваши предметы ",
                "",
                "",
                prot_message, item_what(adtyp));
    }
}

/* attributes: intrinsics and the like, other non-obvious capabilities */
staticfn void
attributes_enlightenment(
    int unused_mode UNUSED,
    int final)
{
    static NEARDATA const char
        if_surroundings_permitted[] = " если бы окружение позволяло";
    int ltmp, armpro, warnspecies;
    char buf[BUFSZ];

    /*\
     *  Attributes
    \*/
    enlght_out("");
    enlght_out(final ? "Итоговые атрибуты:" : "Атрибуты:");

    if (u.uevent.uhand_of_elbereth) {
        static const char *const hofe_titles[3] = { "Рука Элберета",
                                                    "Посланник Равновесия",
                                                    "Слава Ариоха" };
        you_are(hofe_titles[u.uevent.uhand_of_elbereth - 1], "");
    }

    Sprintf(buf, "%s", piousness(TRUE, "привержены своему выравниванию"));
    if (u.ualign.record >= 0)
        you_are(buf, "");
    else
        you_have(buf, "");

    if (wizard) {
        Sprintf(buf, " %d", u.ualign.record);
        enl_msg("Ваше выравнивание ", "", "", buf, "");
    }

    /*** Resistances to troubles ***/
    if (Invulnerable)
        you_are("неуязвимы", from_what(INVULNERABLE));
    if (Antimagic)
        you_are("защищены от магии", from_what(ANTIMAGIC));
    if (Fire_resistance)
        you_are("устойчивы к огню", from_what(FIRE_RES));
    item_resistance_message(AD_FIRE, "дают защиту от огня", final);
    if (Cold_resistance)
        you_are("устойчивы к холоду", from_what(COLD_RES));
    item_resistance_message(AD_COLD, "дают защиту от холода", final);
    if (Sleep_resistance)
        you_are("устойчивы ко сну", from_what(SLEEP_RES));
    if (Disint_resistance)
        you_are("устойчивы к дезинтеграции", from_what(DISINT_RES));
    item_resistance_message(AD_DISN, "дают защиту от дезинтеграции", final);
    if (Shock_resistance)
        you_are("устойчивы к электричеству", from_what(SHOCK_RES));
    item_resistance_message(AD_ELEC, "дают защиту от электричества",
                            final);
    if (Poison_resistance)
        you_are("устойчивы к яду", from_what(POISON_RES));
    if (Acid_resistance) {
        Sprintf(buf, "%.20s%.30s",
                temp_resist(ACID_RES) ? "временно " : "",
                "устойчивы к кислоте");
        you_are(buf, from_what(ACID_RES));
    }
    item_resistance_message(AD_ACID, "дают защиту от кислоты", final);
    if (Drain_resistance)
        you_are("устойчивы к потере уровня", from_what(DRAIN_RES));
    if (Sick_resistance)
        you_are("невосприимчивы к болезням", from_what(SICK_RES));
    if (Stone_resistance) {
        Sprintf(buf, "%.20s%.30s",
                temp_resist(STONE_RES) ? "временно " : "",
                "устойчивы к окаменению");
        you_are(buf, from_what(STONE_RES));
    }
    if (Halluc_resistance)
        enl_msg(You_, "устойчивы", "были устойчивы", " к галлюцинациям",
                from_what(HALLUC_RES));
    if (u.uedibility)
        you_can("распознавать вредную пищу", "");

    /*** Vision and senses ***/
    if ((HBlinded || EBlinded) && BBlinded) /* blind w/ blindness blocked */
        you_can("видеть", from_what(-BLINDED)); /* Eyes of the Overworld */
    if (Blnd_resist && !Blind) /* skip if no eyes or blindfolded */
        you_are("не слепнете от света",
                from_what(BLND_RES));
    if (See_invisible) {
        if (!Blind)
            enl_msg(You_, "видите", "видели", " невидимое", from_what(SEE_INVIS));
        else if (!PermaBlind)
            enl_msg(You_, "увидите", "увидели бы",
                    " невидимое, когда не слепы", "");
        else
            enl_msg(You_, "видели бы", "увидели бы",
                    " невидимое, если бы не были слепы", "");
    }
    if (Blind_telepat)
        you_are("телепат", from_what(TELEPAT));
    if (Warning)
        you_are("предупреждены", from_what(WARNING));
    if (Warn_of_mon && svc.context.warntype.obj) {
        Sprintf(buf, "ощущаете присутствие %s",
                (svc.context.warntype.obj & M2_ORC) ? "орков"
                : (svc.context.warntype.obj & M2_ELF) ? "эльфов"
                  : (svc.context.warntype.obj & M2_DEMON) ? "демонов"
                    : something);
        you_are(buf, from_what(WARN_OF_MON));
    }
    if (Warn_of_mon && svc.context.warntype.polyd) {
        Sprintf(buf, "ощущаете присутствие %s",
                ((svc.context.warntype.polyd & (M2_HUMAN | M2_ELF))
                 == (M2_HUMAN | M2_ELF)) ? "людей и эльфов"
                    : (svc.context.warntype.polyd & M2_HUMAN) ? "людей"
                      : (svc.context.warntype.polyd & M2_ELF) ? "эльфов"
                        : (svc.context.warntype.polyd & M2_ORC) ? "орков"
                          : (svc.context.warntype.polyd & M2_DEMON) ? "демонов"
                            : "определённых монстров");
        you_are(buf, "");
    }
    warnspecies =  svc.context.warntype.speciesidx;
    if (Warn_of_mon && ismnum(warnspecies)) {
        Sprintf(buf, "ощущаете присутствие %s",
                makeplural(mons[warnspecies].pmnames[NEUTRAL]));
        you_are(buf, from_what(WARN_OF_MON));
    }
    if (Undead_warning)
        you_are("предупреждены о нежити", from_what(WARN_UNDEAD));
    if (Searching)
        you_have("имеете автоматический поиск", from_what(SEARCHING));
    if (Clairvoyant) {
        you_are("ясновидящий", from_what(CLAIRVOYANT));
    } else if ((HClairvoyant || EClairvoyant) && BClairvoyant) {
        Strcpy(buf, from_what(-CLAIRVOYANT));
        (void) strsubst(buf, " because of ", " if not for ");
        enl_msg(You_, "были бы", "были бы", " ясновидящим", buf);
    }
    if (Infravision)
        you_have("имеете инфравидение", from_what(INFRAVISION));
    if (Detect_monsters) {
        Strcpy(buf, "ощущаете присутствие монстров");
        if (wizard) {
            long detectmon_timeout = (HDetect_monsters & TIMEOUT);

            if (detectmon_timeout)
                Sprintf(eos(buf), " (%ld)", detectmon_timeout);
        }
        you_are(buf, "");
    }
    if (u.umconf) { /* 'u.umconf' is a counter rather than a timeout */
        Strcpy(buf, " монстров при ударе по ним");
        if (wizard && !final) {
            if (u.umconf == 1)
                Strcat(buf, " (только следующая атака)");
            else /* u.umconf > 1 */
                Sprintf(eos(buf), " (следующие %u ударов)", u.umconf);
        }
        enl_msg(You_, "собьёте с толку", "собили бы с толку", buf, "");
    }

    /*** Appearance and behavior ***/
    if (Adornment) {
        int adorn = 0;

        if (uleft && uleft->otyp == RIN_ADORNMENT)
            adorn += uleft->spe;
        if (uright && uright->otyp == RIN_ADORNMENT)
            adorn += uright->spe;
        /* the sum might be 0 (+0 ring or two which negate each other);
           that yields "you are charismatic" (which isn't pointless
           because it potentially impacts seduction attacks) */
        Sprintf(buf, "%sобаятельны",
                (adorn > 0) ? "более " : (adorn < 0) ? "менее " : "");
        you_are(buf, from_what(ADORNED));
    }
    if (Invisible)
        you_are("невидимы", from_what(INVIS));
    else if (Invis)
        you_are("невидимы для других", from_what(INVIS));
    /* ordinarily "visible" is redundant; this is a special case for
       the situation when invisibility would be an expected attribute */
    else if ((HInvis || EInvis) && BInvis)
        you_are("видимы", from_what(-INVIS));
    if (Displaced)
        you_are("смещены", from_what(DISPLACED));
    if (Stealth) {
        you_are("незаметны", from_what(STEALTH));
    } else if (BStealth && (HStealth || EStealth)) {
        Sprintf(buf, " незаметны%s",
                (BStealth == FROMOUTSIDE) ? " если не верхом" : "");
        enl_msg(You_, "были бы", "были бы", buf, "");
    }
    if (Aggravate_monster)
        enl_msg("Вы раздражаете", "", "", " монстров",
                from_what(AGGRAVATE_MONSTER));
    if (Conflict)
        enl_msg("Вы провоцируете", "", "", " конфликт", from_what(CONFLICT));

    /*** Transportation ***/
    if (Jumping)
        you_can("прыгать", from_what(JUMPING));
    if (Teleportation)
        you_can("телепортироваться", from_what(TELEPORT));
    if (Teleport_control)
        you_have("имеете контроль телепортации", from_what(TELEPORT_CONTROL));
    /* actively levitating handled earlier as a status condition */
    if (BLevitation) { /* levitation is blocked */
        long save_BLev = BLevitation;

        BLevitation = 0L;
        if (Levitation) {
            /* either trapped in the floor or inside solid rock
               (or both if chained to buried iron ball and have
               moved one step into solid rock somehow) */
            boolean trapped = (save_BLev & I_SPECIAL) != 0L,
                    terrain = (save_BLev & FROMOUTSIDE) != 0L;

            Sprintf(buf, "%s%s%s",
                    trapped ? " если бы не были в ловушке" : "",
                    (trapped && terrain) ? " и" : "",
                    terrain ? if_surroundings_permitted : "");
            enl_msg(You_, "левитировали бы", "левитировали бы", buf, "");
        }
        BLevitation = save_BLev;
    }
    /* actively flying handled earlier as a status condition */
    if (BFlying) { /* flight is blocked */
        long save_BFly = BFlying;

        BFlying = 0L;
        if (Flying) {
            enl_msg(You_, "летали бы", "летали бы",
                    /* wording quibble: for past tense, "hadn't been"
                       would sound better than "weren't" (and
                       "had permitted" better than "permitted"), but
                       "weren't" and "permitted" are adequate so the
                       extra complexity to handle that isn't worth it */
                    Levitation
                       ? " если бы вы не левитировали"
                       : (save_BFly == I_SPECIAL)
                          /* this is an oversimplification; being trapped
                             might also be blocking levitation so flight
                             would still be blocked after escaping trap */
                          ? " если бы вы не были в ловушке"
                          : (save_BFly == FROMOUTSIDE)
                             ? if_surroundings_permitted
                             /* two or more of levitation, surroundings,
                                and being trapped in the floor */
                             : " если бы обстоятельства позволили",
                    "");
        }
        BFlying = save_BFly;
    }
    /* including this might bring attention to the fact that ceiling
       clinging has inconsistencies... */
    if (is_clinger(gy.youmonst.data)) {
        boolean has_lid = has_ceiling(&u.uz);

        if (has_lid && !u.uinwater) {
            you_can("цепляться за потолок", "");
        } else {
            Sprintf(buf, " к потолку, если %s%s%s",
                    !has_lid ? "был бы потолок" : "",
                    (!has_lid && u.uinwater) ? " и " : "",
                    u.uinwater ? (Underwater ? "вы не под водой"
                                  : "вы не в воде") : "");
            /* past tense is applicable for death while Unchanging */
            enl_msg(You_, "могли бы цепляться", "могли бы цепляться", buf, "");
        }
    }
    /* actively walking on water handled earlier as a status condition */
    if (Wwalking && !walking_on_water())
        you_can("ходить по воде", from_what(WWALKING));
    /* actively swimming (in water but not under it) handled earlier */
    if (Swimming && (Underwater || !u.uinwater))
        you_can("плавать", from_what(SWIMMING));
    if (Breathless)
        you_can("выживать без воздуха", from_what(MAGICAL_BREATHING));
    else if (Amphibious)
        you_can("дышать под водой", from_what(MAGICAL_BREATHING));
    if (Passes_walls)
        you_can("проходить сквозь стены", from_what(PASSES_WALLS));

    /*** Physical attributes ***/
    if (Regeneration)
        enl_msg("Вы регенерируете", "", "", "", from_what(REGENERATION));
    if (Slow_digestion)
        you_have("медленное переваривание", from_what(SLOW_DIGESTION));
    if (u.uhitinc) {
        (void) enlght_combatinc("to hit", u.uhitinc, final, buf);
        if (iflags.tux_penalty && !Upolyd)
            Sprintf(eos(buf), " %s штраф вашей брони",
                    (u.uhitinc < 0) ? "увеличивая"
                    : (u.uhitinc < 4 * gu.urole.spelarmr / 5)
                      ? "частично компенсируя"
                      : (u.uhitinc < gu.urole.spelarmr) ? "почти компенсируя"
                        : "преодолевая");
        you_have(buf, "");
    }
    if (u.udaminc)
        you_have(enlght_combatinc("damage", u.udaminc, final, buf), "");
    if (u.uspellprot || Protection) {
        int prot = 0;

        if (uleft && uleft->otyp == RIN_PROTECTION)
            prot += uleft->spe;
        if (uright && uright->otyp == RIN_PROTECTION)
            prot += uright->spe;
        if (uamul && uamul->otyp == AMULET_OF_GUARDING)
            prot += 2;
        if (HProtection & INTRINSIC)
            prot += u.ublessed;
        prot += u.uspellprot;
        if (prot)
            you_have(enlght_combatinc("defense", prot, final, buf), "");
    }
    if ((armpro = magic_negation(&gy.youmonst)) > 0) {
        /* magic cancellation factor, conferred by worn armor */
        static const char *const mc_types[] = {
            "" /*обычный*/, "защищён", "усиленно защищён", "полностью защищён",
        };
        /* sanity check */
        if (armpro >= SIZE(mc_types))
            armpro = SIZE(mc_types) - 1;
        you_are(mc_types[armpro], "");
    }
    if (Half_physical_damage)
        enlght_halfdmg(HALF_PHDAM, final);
    if (Half_spell_damage)
        enlght_halfdmg(HALF_SPDAM, final);
    if (Half_gas_damage)
        enl_msg(You_, "получаете", "получали", " сниженный урон от ядовитого газа", "");
    if (spellid(0) > NO_SPELL) { /* skip if no spells are known yet */
        /* greatly simplified edition of percent_success(spell.c)--may need
           to be suppressed if oversimplification leads to player confusion */
        char cast_adj[QBUFSZ];
        boolean suit = uarm && is_metallic(uarm),
                robe = uarmc && uarmc->otyp == ROBE;

        *cast_adj = '\0';
        if (suit) /* omit "wearing" to shorten the text */
            Sprintf(cast_adj, " ослаблено металлической бронёй%s",
                    robe ? ", смягчено вашей мантией" : "");
        else if (robe)
            Strcpy(cast_adj, " усилено ношением мантии");

        if (*cast_adj)
            enl_msg("Ваша способность колдовать ", "", "", cast_adj, "");
    }
    /* polymorph and other shape change */
    if (Protection_from_shape_changers)
        you_are("защищены от изменяющих форму",
                from_what(PROT_FROM_SHAPE_CHANGERS));
    if (Unchanging) {
        const char *what = 0;

        if (!Upolyd) /* Upolyd handled below after current form */
            you_can("не менять свою текущую форму",
                    from_what(UNCHANGING));
        /* blocked shape changes */
        if (Polymorph)
            what = !final ? "превращались бы" : "превратились бы";
        else if (ismnum(u.ulycn))
            what = !final ? "меняли бы форму" : "сменили бы форму";
        if (what) {
            Sprintf(buf, "%s периодически", what);
            /* omit from_what(UNCHANGING); too verbose */
            enl_msg(You_, buf, buf, ", если бы не были заперты в своей форме",
                    "");
        }
    } else if (Polymorph) {
        you_are("превращаетесь периодически", from_what(POLYMORPH));
    }
    if (Polymorph_control)
        you_have("имеете контроль превращений", from_what(POLYMORPH_CONTROL));
    if (Upolyd && u.umonnum != u.ulycn
        /* if we've died from turning into slime, we're polymorphed
           right now but don't want to list it as a temporary attribute
           [we need a more reliable way to detect this situation] */
        && !(final == ENL_GAMEOVERDEAD
             && u.umonnum == PM_GREEN_SLIME && !Unchanging)) {
        /* foreign shape (except were-form which is handled below) */
        if (!vampshifted(&gy.youmonst))
            Sprintf(buf, "превращены в %s",
                    pmname(gy.youmonst.data,
                              flags.female ? FEMALE : MALE));
        else
            Sprintf(buf, "превращены в %s, форма %s",
                    pmname(&mons[gy.youmonst.cham],
                              flags.female ? FEMALE : MALE),
                    pmname(gy.youmonst.data, flags.female ? FEMALE : MALE));
        if (wizard)
            Sprintf(eos(buf), " (%d)", u.mtimedone);
        you_are(buf, "");
    }
    if (lays_eggs(gy.youmonst.data) && flags.female) /* Upolyd */
        you_can("откладывать яйца", "");
    if (ismnum(u.ulycn)) {
        /* "you are a werecreature [in beast form]" */
        Strcpy(buf, an(pmname(&mons[u.ulycn],
               flags.female ? FEMALE : MALE)));
        if (u.umonnum == u.ulycn) {
            Strcat(buf, " в звериной форме");
            if (wizard)
                Sprintf(eos(buf), " (%d)", u.mtimedone);
        }
        you_are(buf, "");
    }
    if (Unchanging && Upolyd) /* !Upolyd handled above */
        you_can("не менять свою текущую форму", from_what(UNCHANGING));
    if (Hate_silver)
        you_are("страдаете от серебра", "");
    /* movement and non-armor-based protection */
    if (Fast)
        you_are(Very_fast ? "очень быстры" : "быстры", from_what(FAST));
    if (Reflecting)
        you_have("имеете отражение", from_what(REFLECTING));
    if (Free_action)
        you_have("свободно двигаетесь", from_what(FREE_ACTION));
    if (Fixed_abil)
        you_have("фиксированные способности", from_what(FIXED_ABIL));
    if (Lifesaved)
        enl_msg("Ваша жизнь ", "будет", "была бы", " спасена", "");

    /*** Miscellany ***/
    if (Luck) {
        ltmp = abs((int) Luck);
        Sprintf(buf, "%s%sудачливы",
                ltmp >= 10 ? "чрезвычайно " : ltmp >= 5 ? "очень " : "",
                Luck < 0 ? "не" : "");
        if (wizard)
            Sprintf(eos(buf), " (%d)", Luck);
        you_are(buf, "");
    } else if (wizard)
        enl_msg("Ваша удача ", "равна", "равнялась", " нулю", "");
    if (u.moreluck > 0)
        you_have("дополнительная удача", "");
    else if (u.moreluck < 0)
        you_have("пониженная удача", "");
    if (carrying(LUCKSTONE) || stone_luck(TRUE)) {
        ltmp = stone_luck(FALSE);
        if (ltmp <= 0)
            enl_msg("Плохая удача ", "", "", " не исчезает", "");
        if (ltmp >= 0)
            enl_msg("Хорошая удача ", "", "", " не исчезает", "");
    }

    if (u.ugangr) {
        Sprintf(buf, " %sгневается на вас",
                u.ugangr > 6 ? "чрезвычайно " : u.ugangr > 3 ? "очень " : "");
        if (wizard)
            Sprintf(eos(buf), " (%d)", u.ugangr);
        enl_msg(u_gname(), "", "", buf, "");
    } else {
        /*
         * We need to suppress this when the game is over, because death
         * can change the value calculated by can_pray(), potentially
         * resulting in a false claim that you could have prayed safely.
         */
        if (!final) {
#if 0
            /* "can [not] safely pray" vs "could [not] have safely prayed" */
            Sprintf(buf, "%s%ssafely pray%s", can_pray(FALSE) ? "" : "not ",
                    final ? "have " : "", final ? "ed" : "");
#else
            Sprintf(buf, "%sмолиться", can_pray(FALSE) ? "безопасно " : "не ");
#endif
            if (wizard)
                Sprintf(eos(buf), " (%d)", u.ublesscnt);
            you_can(buf, "");
        }
    }

#ifdef DEBUG
    /* named fruit debugging (doesn't really belong here...); to enable,
       include 'fruit' in DEBUGFILES list (even though it isn't a file...) */
    if (wizard && explicitdebug("fruit")) {
        struct fruit *f;

        reorder_fruit(TRUE); /* sort by fruit index, from low to high;
                              * this modifies the gf.ffruit chain, so could
                              * possibly mask or even introduce a problem,
                              * but it does useful sanity checking */
        for (f = gf.ffruit; f; f = f->nextf) {
            Sprintf(buf, "Fruit #%d ", f->fid);
            enl_msg(buf, "is ", "was ", f->fname, "");
        }
        enl_msg("The current fruit ", "is ", "was ", svp.pl_fruit, "");
        Sprintf(buf, "%d", flags.made_fruit);
        enl_msg("The made fruit flag ", "is ", "was ", buf, "");
    }
#endif

    {
        const char *p;

        buf[0] = '\0';
        if (final < 2) { /* still in progress, or quit/escaped/ascended */
            p = "выжили после гибели ";
            if (!u.umortality)
                p = !final ? (char *) 0 : "выжили";
            else
                (void) N_times((long) u.umortality, buf);
        } else { /* game ended in character's death */
            p = "мертвы";
            switch (u.umortality) {
            case 0:
                impossible("dead without dying?");
                FALLTHROUGH;
                /* FALLTHRU */
            case 1:
                break; /* just "are dead" */
            default:
                Sprintf(buf, " (%d раз!)", u.umortality);
                break;
            }
        }
        if (p)
            enl_msg(You_, "были убиты ", p, buf, "");
    }
}

/* ^X command */
int
doattributes(void)
{
    int mode = BASICENLIGHTENMENT;

    /* show more--as if final disclosure--for wizard and explore modes */
    if (wizard || discover)
        mode |= MAGICENLIGHTENMENT;

    enlightenment(mode, ENL_GAMEINPROGRESS);
    return ECMD_OK;
}

void
youhiding(boolean via_enlghtmt, /* enlightenment line vs topl message */
          int msgflag)          /* for variant message phrasing */
{
    char *bp, buf[BUFSZ];

    Strcpy(buf, "прячетесь");
    if (U_AP_TYPE != M_AP_NOTHING) {
        /* mimic; hero is only able to mimic a strange object or gold
           or hallucinatory alternative to gold, so we skip the details
           for the hypothetical furniture and monster cases */
        bp = eos(strcpy(buf, "имитируете"));
        if (U_AP_TYPE == M_AP_OBJECT) {
            Sprintf(bp, " %s", simple_typename(gy.youmonst.mappearance));
        } else if (U_AP_TYPE == M_AP_FURNITURE) {
            Strcpy(bp, " что-то");
        } else if (U_AP_TYPE == M_AP_MONSTER) {
            Strcpy(bp, " кого-то");
        } else {
            ; /* something unexpected; leave 'buf' as-is */
        }
    } else if (u.uundetected) {
        bp = eos(buf); /* points past "hiding" */
        if (gy.youmonst.data->mlet == S_EEL) {
            if (is_pool(u.ux, u.uy))
                Sprintf(bp, " в %s", waterbody_name(u.ux, u.uy));
        } else if (hides_under(gy.youmonst.data)) {
            struct obj *o = svl.level.objects[u.ux][u.uy];

            if (o)
                Sprintf(bp, " под %s", ansimpleoname(o));
        } else if (is_clinger(gy.youmonst.data) || Flying) {
            /* Flying: 'lurker above' hides on ceiling but doesn't cling */
            Sprintf(bp, " на %s", ceiling(u.ux, u.uy));
        } else {
            /* on floor; is_hider() but otherwise not special: 'trapper' */
            if (u.utrap && u.utraptype == TT_PIT) {
                struct trap *t = t_at(u.ux, u.uy);

                Sprintf(bp, " в %s яме",
                        (t && t->ttyp == SPIKED_PIT) ? "шипастой " : "");
            } else
                Sprintf(bp, " на %s", surface(u.ux, u.uy));
        }
    } else {
        ; /* shouldn't happen; will result in generic "you are hiding" */
    }

    if (via_enlghtmt) {
        int final = msgflag; /* 'final' is used by you_are() macro */

        you_are(buf, "");
    } else {
        /* for dohide(), when player uses '#monster' command */
        You("вы %s %s.", msgflag ? "уже" : "сейчас", buf);
    }
}

/* #conduct command [KMH]; shares enlightenment's tense handling */
int
doconduct(void)
{
    show_conduct(ENL_GAMEINPROGRESS);
    return ECMD_OK;
}

/* display conducts; for doconduct(), also disclose() and dump_everything() */
void
show_conduct(int final)
{
    char buf[BUFSZ], bufN[40];
    int ngenocided;

    /* Create the conduct window */
    ge.en_win = create_nhwindow(NHW_MENU);
    putstr(ge.en_win, 0, "Добровольные ограничения:");

    /* rerolling; "You <this or that>" is about the character, rerolling
       is about the player so phrase it differently;
       also, always use past tense since the chance to do something with it
       is gone by time player can issue #conduct command or see disclosure */
    if (!u.uroleplay.reroll)
        Strcpy(buf, " Перевыбор персонажа не был включён.");
    else if (!u.uroleplay.numrerolls)
        Strcpy(buf, " Ваш персонаж не был перевыбран.");
    else
        Sprintf(buf, " Ваш персонаж был перевыбран %s.",
                N_times(u.uroleplay.numrerolls, bufN));
    enlght_out(buf);

    if (u.uroleplay.blind)
        you_have_been("слеп от рождения");
    if (u.uroleplay.deaf)
        you_have_been("глух от рождения");
    /* note: we don't report "you are without possessions" unless the
       game started with the pauper option set */
    if (u.uroleplay.pauper)
        enl_msg(You_, "начали", "начинали",
                " без имущества", "");
    /* nudist is far more than a subset of possessionless, and a much
       more impressive accomplishment, but showing "started out without
       possessions" before "faithfully nudist" looks more logical */
    if (u.uroleplay.nudist)
        you_have_been("преданы нудизму");

    if (!u.uconduct.food)
        enl_msg(You_, "не ели", "не ели", " никакой пищи", "");
        /* but beverages are okay */
    else if (!u.uconduct.unvegan)
        you_have_X("придерживались строгой веганской диеты");
    else if (!u.uconduct.unvegetarian)
        you_have_been("вегетарианцем");

    if (!u.uconduct.gnostic)
        you_have_been("атеистом");

    if (!u.uconduct.weaphit) {
        you_have_never("билы с оружием в руках");
    } else if (wizard) {
        Sprintf(buf, "били оружием %ld раз",
                u.uconduct.weaphit);
        you_have_X(buf);
    }
    if (!u.uconduct.killer)
        you_have_been("пацифистом");

    if (!u.uconduct.literate) {
        you_have_been("неграмотным");
    } else if (wizard) {
        Sprintf(buf, "читали предметы и надписи %ld раз", u.uconduct.literate);
        you_have_X(buf);
    }

    if (!u.uconduct.pets)
        you_have_never("имели питомца");

    ngenocided = num_genocides();
    if (ngenocided == 0) {
        you_have_never("истребляли монстров");
    } else {
        Sprintf(buf, "истребили %d видов монстров", ngenocided);
        you_have_X(buf);
    }

    if (!u.uconduct.polypiles) {
        you_have_never("превращали предмет");
    } else if (wizard) {
        Sprintf(buf, "превратили %ld предметов", u.uconduct.polypiles);
        you_have_X(buf);
    }

    if (!u.uconduct.polyselfs) {
        you_have_never("меняли форму");
    } else if (wizard) {
        Sprintf(buf, "меняли форму %ld раз", u.uconduct.polyselfs);
        you_have_X(buf);
    }

    if (!u.uconduct.wishes) {
        you_have_X("не использовали желаний");
    } else {
        Sprintf(buf, "использовали %ld желание", u.uconduct.wishes);
        if (u.uconduct.wisharti) {
            /* if wisharti == wishes
             *  1 wish (for an artifact)
             *  2 wishes (both for artifacts)
             *  N wishes (all for artifacts)
             * else (N is at least 2 in order to get here; M < N)
             *  N wishes (1 for an artifact)
             *  N wishes (M for artifacts)
             */
            if (u.uconduct.wisharti == u.uconduct.wishes)
                Sprintf(eos(buf), " (%s",
                        (u.uconduct.wisharti > 2L) ? "все "
                          : (u.uconduct.wisharti == 2L) ? "оба " : "");
            else
                Sprintf(eos(buf), " (%ld ", u.uconduct.wisharti);

            Sprintf(eos(buf), "за %s)",
                    (u.uconduct.wisharti == 1L) ? "артефакт"
                                                : "артефакты");
        }
        you_have_X(buf);

        if (!u.uconduct.wisharti)
            enl_msg(You_, "не желали", "не желали",
                    " артефактов", "");
    }

    /* only report Sokoban conduct if the Sokoban branch has been entered */
    if (sokoban_in_play()) {
        const char *presentverb = "нарушили", *pastverb = "нарушили";

        if (!u.uconduct.sokocheat) {
            presentverb = "не нарушили";
            pastverb = "не нарушили";
            Strcpy(buf, " ни одного особого правила Сокобана");
        } else {
            Strcpy(buf, " особые правила Сокобана ");
            Strcat(buf, N_times(u.uconduct.sokocheat, bufN));
        }
        enl_msg(You_, presentverb, pastverb, buf, "");
    }

    show_achievements(final);

    /* Pop up the window and wait for a key */
    display_nhwindow(ge.en_win, TRUE);
    destroy_nhwindow(ge.en_win);
    ge.en_win = WIN_ERR;
}

/*
 *      Achievements (see 'enum achievements' in you.h).
 */

staticfn void
show_achievements(
    int final) /* 'final' is used "behind the curtain" by enl_foo() macros */
{
    int i, achidx, absidx, acnt;
    char title[QBUFSZ], buf[QBUFSZ];
    winid awin = WIN_ERR;

    /* unfortunately we can't show the achievements (at least not all of
       them) while the game is in progress because it would give away the
       ID of luckstone (at Mine's End) and of real Amulet of Yendor */
    if (!final && !wizard)
        return;

    /* first, figure whether any achievements have been accomplished
       so that we don't show the header for them if the resulting list
       below it would be empty */
    if ((acnt = count_achievements()) == 0)
        return;

    if (ge.en_win != WIN_ERR) {
        awin = ge.en_win; /* end of game disclosure window */
        putstr(awin, 0, "");
    } else {
        awin = create_nhwindow(NHW_MENU);
    }
    Sprintf(title, "Достижения:");
    putstr(awin, 0, title);

    /* display achievements in the order in which they were recorded;
       lone exception is to defer the Amulet if we just ascended;
       it warrants alternate wording when given away during ascension,
       but the Amulet achievement is always attained before entering
       endgame and the alternate wording looks strange if shown before
       "reached endgame" and "reached Astral" */
    if (remove_achievement(ACH_UWIN)) { /* UWIN == Ascended! */
        /* for ascension, force it to be last and Amulet next to last
           by taking them out and then adding them back */
        if (remove_achievement(ACH_AMUL)) /* should always be True here */
            record_achievement(ACH_AMUL);
        record_achievement(ACH_UWIN);
    }
    for (i = 0; i < acnt; ++i) {
        achidx = u.uachieved[i];
        absidx = abs(achidx);

        switch (absidx) {
        case ACH_BLND:
            enl_msg(You_, "исследуете", "исследовали",
                    " без возможности видеть", "");
            break;
        case ACH_NUDE:
            enl_msg(You_, "ходили", "ходили", " без доспехов", "");
            break;
        case ACH_MINE:
            you_have_X("посетили Гномьи шахты");
            break;
        case ACH_TOWN:
            you_have_X("посетили Шахтёрский городок");
            break;
        case ACH_SHOP:
            you_have_X("посетили лавку");
            break;
        case ACH_TMPL:
            you_have_X("посетили храм");
            break;
        case ACH_ORCL:
            you_have_X("советовались с Оракулом Дельф");
            break;
        case ACH_NOVL:
            you_have_X("читали роман Плоского мира");
            break;
        case ACH_SOKO:
            you_have_X("посетили Сокобан");
            break;
        case ACH_SOKO_PRIZE: /* hard to reach guaranteed bag or amulet */
            you_have_X("прошли Сокобан");
            break;
        case ACH_MINE_PRIZE: /* hidden guaranteed luckstone */
            you_have_X("прошли Гномьи шахты");
            break;
        case ACH_BGRM:
            you_have_X("посетили Большую комнату");
            break;
        case ACH_MEDU:
            you_have_X("победили Медузу");
            break;
        case ACH_TUNE:
            you_have_X(
                "выучили мелодию подъёмного моста Замка");
            break;
        case ACH_BELL:
            /* alternate phrasing for present vs past and also for
               possessing the item vs once held it */
            enl_msg(You_,
                    u.uhave.bell ? "имеете" : "держали",
                    u.uhave.bell ? "имели" : "держали",
                    " Колокол Открытия", "");
            break;
        case ACH_HELL:
            enl_msg(You_, "вошли", "", " в Геенну", "");
            break;
        case ACH_CNDL:
            enl_msg(You_,
                    u.uhave.menorah ? "имеете" : "держали",
                    u.uhave.menorah ? "имели" : "держали",
                    " Канделябр Призыва", "");
            break;
        case ACH_BOOK:
            enl_msg(You_,
                    u.uhave.book ? "имеете" : "держали",
                    u.uhave.book ? "имели" : "держали",
                    " Книгу Мёртвых", "");
            break;
        case ACH_INVK:
            you_have_X("получили доступ к Святилищу Молоха");
            break;
        case ACH_AMUL:
            /* alternate wording for ascended (always past tense) since
               hero had it until #offer forced it to be relinquished */
            enl_msg(You_,
                    u.uhave.amulet ? "имеете" : "получили",
                    u.uevent.ascended ? "доставили"
                     : u.uhave.amulet ? "имели" : "получали",
                    " Амулет Йендора", "");
            break;

        /* reaching Astral makes feedback about reaching the Planes
           be redundant and ascending makes both be redundant, but
           we display all that apply */
        case ACH_ENDG:
            you_have_X("достигли Элементальных Планов");
            break;
        case ACH_ASTR:
            you_have_X("достигли Астрального Плана");
            break;
        case ACH_UWIN:
            /* the ultimate achievement... */
            enlght_out(" Вы вознеслись!");
            break;

        /* rank 0 is the starting condition, not an achievement; 8 is Xp 30 */
        case ACH_RNK1: case ACH_RNK2: case ACH_RNK3: case ACH_RNK4:
        case ACH_RNK5: case ACH_RNK6: case ACH_RNK7: case ACH_RNK8:
            Sprintf(buf, "достигли ранга %s",
                    rank_of(rank_to_xlev(absidx - (ACH_RNK1 - 1)),
                            Role_switch, (achidx < 0) ? TRUE : FALSE));
            you_have_X(buf);
            break;

        default:
            Sprintf(buf, " [Неожиданное достижение #%d.]", achidx);
            enlght_out(buf);
            break;
        } /* switch */
    } /* for */

    if (awin != ge.en_win) {
        display_nhwindow(awin, TRUE);
        destroy_nhwindow(awin);
    }
}

/* record an achievement (add at end of list unless already present) */
void
record_achievement(schar achidx)
{
    int i, absidx;
    int repeat_achievement = 0;

    absidx = abs(achidx);
    /* valid achievements range from 1 to N_ACH-1; however, ranks can be
       stored as the complement (ie, negative) to track gender */
    if ((achidx < 1 && (absidx < ACH_RNK1 || absidx > ACH_RNK8))
        || achidx >= N_ACH) {
        impossible("Achievement #%d is out of range.", achidx);
        return;
    }

    /* the list has an extra slot so there is always at least one 0 at
       its end (more than one unless all N_ACH-1 possible achievements
       have been recorded); find first empty slot or achievement #achidx;
       an attempt to duplicate an achievement can happen if any of Bell,
       Candelabrum, Book, or Amulet is dropped then picked up again */
    for (i = 0; u.uachieved[i]; ++i)
        if (abs(u.uachieved[i]) == absidx) {
            repeat_achievement = 1;
            break;
        }

    /*
     * We do the sound for an achievement, even if it has already been
     * achieved before. Some players might have set up level-based
     * theme music or something. We do let the sound interface know
     * that it's not the original achievement though.
     */
    SoundAchievement(achidx, 0, repeat_achievement);

    if (repeat_achievement)
        return; /* already recorded, don't duplicate it */
    u.uachieved[i] = achidx;

    /* avoid livelog for achievements recorded during final disclosure:
       nudist and blind-from-birth; also ascension which is suppressed
       by this gets logged separately in really_done() */
    if (program_state.gameover)
        return;

    if (absidx >= ACH_RNK1 && absidx <= ACH_RNK8) {
        livelog_printf(achieve_msg[absidx].llflag,
                       "attained the rank of %s (level %d)",
                       rank_of(rank_to_xlev(absidx - (ACH_RNK1 - 1)),
                               Role_switch, (achidx < 0) ? TRUE : FALSE),
                       u.ulevel);
    } else if (achidx == ACH_SOKO_PRIZE
               || achidx == ACH_MINE_PRIZE) {
        /* need to supply extra information for these two */
        short otyp = ((achidx == ACH_SOKO_PRIZE)
                      ? svc.context.achieveo.soko_prize_otyp
                      : svc.context.achieveo.mines_prize_otyp);

        /* note: OBJ_NAME() works here because both "bag of holding" and
           "amulet of reflection" are fully named in their objects[] entry
           but that's not true in the general case */
        livelog_printf(achieve_msg[achidx].llflag, "%s %s",
                       achieve_msg[achidx].msg, OBJ_NAME(objects[otyp]));
    } else {
        livelog_printf(achieve_msg[absidx].llflag, "%s",
                       achieve_msg[absidx].msg);
    }
}

/* discard a recorded achievement; return True if removed, False otherwise */
boolean
remove_achievement(schar achidx)
{
    int i;

    for (i = 0; u.uachieved[i]; ++i)
        if (abs(u.uachieved[i]) == abs(achidx))
            break; /* stop when found */
    if (!u.uachieved[i]) /* not found */
        return FALSE;
    /* list is 0 terminated so any beyond the removed one move up a slot */
    do {
        u.uachieved[i] = u.uachieved[i + 1];
    } while (u.uachieved[++i]);
    return TRUE;
}

/* used to decide whether there are any achievements to display */
int
count_achievements(void)
{
    int i, acnt = 0;

    for (i = 0; u.uachieved[i]; ++i)
        ++acnt;
    return acnt;
}

/* convert a rank index to an achievement number; encode it when female
   in order to subsequently report gender-specific ranks accurately */
schar
achieve_rank(int rank) /* 1..8 */
{
    schar achidx = (schar) ((rank - 1) + ACH_RNK1);

    if (flags.female)
        achidx = -achidx;
    return achidx;
}

/* return True if sokoban branch has been entered, False otherwise */
boolean
sokoban_in_play(void)
{
    int achidx;

    /* TODO? move this to dungeon.c and test furthest level reached of the
       sokoban branch instead of relying on the entered-sokoban achievement */

    for (achidx = 0; u.uachieved[achidx]; ++achidx)
        if (u.uachieved[achidx] == ACH_SOKO)
            return TRUE;
    return FALSE;
}

/* #chronicle command */
int
do_gamelog(void)
{
#ifdef CHRONICLE
    if (gg.gamelog) {
        show_gamelog(ENL_GAMEINPROGRESS);
    } else {
        pline("Событий для летописи нет.");
    }
#else
    pline("Летопись была отключена при компиляции.");
#endif /* !CHRONICLE */
    return ECMD_OK;
}

/* 'major' events for dumplog; inclusion or exclusion here may need tuning */
#define LL_majors (0L \
                   | LL_WISH            \
                   | LL_ACHIEVE         \
                   | LL_UMONST          \
                   | LL_DIVINEGIFT      \
                   | LL_LIFESAVE        \
                   | LL_ARTIFACT        \
                   | LL_GENOCIDE        \
                   | LL_DUMP) /* explicitly for dumplog */
#define majorevent(llmsg) (((llmsg)->flags & LL_majors) != 0)
#define spoilerevent(llmsg) (((llmsg)->flags & LL_SPOILER) != 0)

/* #chronicle details */
void
show_gamelog(int final)
{
#ifdef CHRONICLE
    struct gamelog_line *llmsg;
    winid win;
    char buf[BUFSZ];
    int eventcnt = 0;

    win = create_nhwindow(NHW_TEXT);
    Sprintf(buf, "%s события:", final ? "Главные" : "Зафиксированные");
    putstr(win, 0, buf);
    for (llmsg = gg.gamelog; llmsg; llmsg = llmsg->next) {
        if (final && !majorevent(llmsg))
            continue;
        if (!final && !wizard && spoilerevent(llmsg))
            continue;
        if (!eventcnt++)
            putstr(win, 0, " Ход");
        Snprintf(buf, sizeof buf, "%5ld: %s", llmsg->turn, llmsg->text);
        putstr(win, 0, buf);
    }
    /* since start of game is logged as a major event, 'eventcnt' should
       never end up as 0; for 'final', end of game is a major event too */
    if (!eventcnt)
        putstr(win, 0, " нет");

    display_nhwindow(win, TRUE);
    destroy_nhwindow(win);
#else
    nhUse(final);
#endif /* !CHRONICLE */
    return;
}

/*
 *      Vanquished monsters.
 */

/* the two uppercase choices are implemented but suppressed from menu.
   also used in options.c */
const char *const vanqorders[NUM_VANQ_ORDER_MODES][3] = {
    { "t", "классический: по уровню монстра",
           "классический: по уровню монстра, по индексу" },
    { "d", "по сложности монстра",
           "по сложности монстра, по индексу" },
    { "a", "по алфавиту, уникумы отдельно",
           "по алфавиту: сначала уникумы, затем остальные" },
    { "A", "по алфавиту, уникумы вперемешку",
           "по алфавиту: уникумы и остальные вперемешку" },
    { "C", "по классу монстра, уровень по убыванию",
           "по классу монстра: уровень по убыванию в классе" },
    { "c", "по классу монстра, уровень по возрастанию",
           "по классу монстра: уровень по возрастанию в классе" },
    { "n", "по количеству, много–мало",
           "по количеству: много–мало, по индексу при равенстве" },
    { "z", "по количеству, мало–много",
           "по количеству: мало–много, по индексу при равенстве" },
};

staticfn int QSORTCALLBACK
vanqsort_cmp(
    const genericptr vptr1,
    const genericptr vptr2)
{
    int indx1 = *(short *) vptr1, indx2 = *(short *) vptr2,
        mlev1, mlev2, mstr1, mstr2, uniq1, uniq2, died1, died2, res;
    const char *name1, *name2, *punct;
    schar mcls1, mcls2;

    switch (flags.vanq_sortmode) {
    default:
    case VANQ_MLVL_MNDX:
        /* sort by monster level */
        mlev1 = mons[indx1].mlevel;
        mlev2 = mons[indx2].mlevel;
        res = mlev2 - mlev1; /* mlevel high to low */
        break;
    case VANQ_MSTR_MNDX:
        /* sort by monster toughness */
        mstr1 = mons[indx1].difficulty;
        mstr2 = mons[indx2].difficulty;
        res = mstr2 - mstr1; /* monstr high to low */
        break;
    case VANQ_ALPHA_SEP:
        uniq1 = ((mons[indx1].geno & G_UNIQ) && indx1 != PM_HIGH_CLERIC);
        uniq2 = ((mons[indx2].geno & G_UNIQ) && indx2 != PM_HIGH_CLERIC);
        if (uniq1 ^ uniq2) { /* one or other uniq, but not both */
            res = uniq2 - uniq1;
            break;
        } /* else both unique or neither unique */
        FALLTHROUGH;
        /*FALLTHRU*/
    case VANQ_ALPHA_MIX:
        name1 = mons[indx1].pmnames[NEUTRAL];
        name2 = mons[indx2].pmnames[NEUTRAL];
        res = strcmpi(name1, name2); /* caseblind alpha, low to high */
        break;
    case VANQ_MCLS_HTOL:
    case VANQ_MCLS_LTOH:
        /* mons[].mlet is a small integer, 1..N, of type plain char;
           if 'char' happens to be unsigned, (mlet1 - mlet2) would yield
           an inappropriate result when mlet2 is greater than mlet1,
           so force our copies (mcls1, mcls2) to be signed */
        mcls1 = (schar) mons[indx1].mlet;
        mcls2 = (schar) mons[indx2].mlet;
        /* S_ANT through S_ZRUTY correspond to lowercase monster classes,
           S_ANGEL through S_ZOMBIE correspond to uppercase, and various
           punctuation characters are used for classes beyond those */
        if (mcls1 > S_ZOMBIE && mcls2 > S_ZOMBIE) {
            /* force a specific order to the punctuation classes that's
               different from the internal order;
               internal order is ok if neither or just one is punctuation
               since letters have lower values so come out before punct */
            static const char punctclasses[] = {
                S_LIZARD, S_EEL, S_GOLEM, S_GHOST, S_DEMON, S_HUMAN, '\0'
            };

            if ((punct = strchr(punctclasses, mcls1)) != 0)
                mcls1 = (schar) (S_ZOMBIE + 1 + (int) (punct - punctclasses));
            if ((punct = strchr(punctclasses, mcls2)) != 0)
                mcls2 = (schar) (S_ZOMBIE + 1 + (int) (punct - punctclasses));
        }
        res = mcls1 - mcls2; /* class */
        if (res == 0) {
            /* Riders are in the same class as major demons, yielding res==0
               above when both mcls1 and mcls2 are either Riders or demons or
               one of each; force Riders to be sorted before demons */
            res = is_rider(&mons[indx2]) - is_rider(&mons[indx1]);
            /* res -1 => #1 is a Rider, #2 isn't;
                    0 => both Riders or neither;
                   +1 => #2 is a Rider, #1 isn't */
            if (res)
                break;
            mlev1 = mons[indx1].mlevel;
            mlev2 = mons[indx2].mlevel;
            res = mlev1 - mlev2; /* mlevel low to high */
            if (flags.vanq_sortmode == VANQ_MCLS_HTOL)
                res = -res; /* mlevel high to low */
        }
        break;
    case VANQ_COUNT_H_L:
    case VANQ_COUNT_L_H:
        died1 = svm.mvitals[indx1].died;
        died2 = svm.mvitals[indx2].died;
        res = died2 - died1; /* dead count high to low */
        if (flags.vanq_sortmode == VANQ_COUNT_L_H)
            res = -res; /* dead count low to high */
        break;
    }
    /* tiebreaker: internal mons[] index */
    if (res == 0)
        res = indx1 - indx2; /* mndx low to high */
    return res;
}

/* returns -1 if cancelled via ESC */
int
set_vanq_order(boolean for_vanq)
{
    winid tmpwin;
    menu_item *selected;
    anything any;
    char buf[BUFSZ];
    const char *desc;
    int i, n, choice,
        clr = NO_COLOR;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin, MENU_BEHAVE_STANDARD);
    any = cg.zeroany; /* zero out all bits */
    for (i = 0; i < SIZE(vanqorders); i++) {
        if (i == VANQ_ALPHA_MIX || i == VANQ_MCLS_HTOL) /* skip these */
            continue;
        /* suppress some orderings if this menu if for 'm #genocided' */
        if (!for_vanq && (i == VANQ_COUNT_H_L || i == VANQ_COUNT_L_H))
            continue;
        desc = vanqorders[i][2];
        /* unique monsters can't be genocided so "alpha, unique separate"
           and "alpha, unique intermixed" are confusing descriptions when
           this menu is for #genocided rather than for #vanquished */
        if (!for_vanq && i == VANQ_ALPHA_SEP)
            desc = "по алфавиту";
        any.a_int = i + 1;
        add_menu(tmpwin, &nul_glyphinfo, &any, *vanqorders[i][0], 0,
                 ATR_NONE, clr, desc,
                 (i == flags.vanq_sortmode) ? MENU_ITEMFLAGS_SELECTED
                                            : MENU_ITEMFLAGS_NONE);
    }
    Sprintf(buf, "Порядок сортировки списка %s",
            for_vanq ? "поверженных монстров (также истреблённые типы)"
                     : "истреблённых типов (также счётчики поверженных)");
    end_menu(tmpwin, buf);

    n = select_menu(tmpwin, PICK_ONE, &selected);
    destroy_nhwindow(tmpwin);
    if (n > 0) {
        choice = selected[0].item.a_int - 1;
        /* skip preselected entry if we have more than one item chosen */
        if (n > 1 && choice == flags.vanq_sortmode)
            choice = selected[1].item.a_int - 1;
        free((genericptr_t) selected);
        flags.vanq_sortmode = choice;
    }
    return (n < 0) ? -1 : flags.vanq_sortmode;
}

/* #vanquished command */
int
dovanquished(void)
{
    list_vanquished(iflags.menu_requested ? 'A' : 'y', FALSE);
    iflags.menu_requested = FALSE;
    return ECMD_OK;
}

/* high priests aren't unique but are flagged as such to simplify something */
#define UniqCritterIndx(mndx) \
    ((mons[mndx].geno & G_UNIQ) != 0 && mndx != PM_HIGH_CLERIC)

#define done_stopprint program_state.stopprint

/* used for #vanquished and end of game disclosure and end of game dumplog */
void
list_vanquished(char defquery, boolean ask)
{
    int i;
    int pfx, nkilled;
    unsigned ntypes, ni;
    long total_killed = 0L;
    winid klwin;
    short mindx[NUMMONS];
    char c, buf[BUFSZ], buftoo[BUFSZ];
    /* 'A' is only supplied by 'm #vanquished'; 'd' is only supplied by
       dump_everything() when writing dumplog, so won't happen if built
       without '#define DUMPLOG' but there's no need for conditionals here */
    boolean force_sort = (defquery == 'A'),
            dumping = (defquery == 'd');

    /* normally we don't ask about sort order for the vanquished list unless
       it contains at least two entries; however, if player has used explicit
       'm #vanquished', choose order no matter what it contains so far */
    if (force_sort) { /* iflags.menu_requested via dovanquished() */
        /* choose value for vanq_sortmode via menu; ESC cancels choosing
           sort order but continues with vanquishd monsters display */
        (void) set_vanq_order(TRUE);
    }
    if (dumping || force_sort) {
        /* switch from 'A' or 'd' to 'y'; 'ask' is already False for the
           cases that might supply 'A' or 'd' */
        defquery = 'y';
        ask = FALSE; /* redundant */
    }

    /* get totals first */
    ntypes = 0;
    for (i = LOW_PM; i < NUMMONS; i++) {
        if ((nkilled = (int) svm.mvitals[i].died) == 0)
            continue;
        mindx[ntypes++] = i;
        total_killed += (long) nkilled;
    }

    /* vanquished creatures list;
     * includes all dead monsters, not just those killed by the player
     */
    if (ntypes != 0) {
        char mlet, prev_mlet = 0; /* used as small integer, not character */
        boolean class_header, uniq_header, Rider,
                was_uniq = FALSE, special_hdr = FALSE;

        if (ask) {
            char allow_yn[10];

            if (ntypes > 1) {
                Strcpy(allow_yn, ynaqchars);
            } else {
                Strcpy(allow_yn, ynqchars); /* don't include 'a', but */
                Strcat(allow_yn, "\033a");  /* allow user to answer 'a' */
                if (defquery == 'a') /* potential default from 'disclose' */
                    defquery = 'y';
            }
            c = yn_function("Хотите ли вы получить отчёт о поверженных существах?",
                            allow_yn, defquery, TRUE);
        } else {
            c = defquery;
        }

        if (c == 'q')
            done_stopprint++;
        if (c == 'y' || c == 'a') {
            if (c == 'a' && ntypes > 1) { /* ask user to choose sort order */
                /* choose value for vanq_sortmode via menu; ESC cancels list
                   of vanquished monsters but does not set 'done_stopprint' */
                if (set_vanq_order(TRUE) < 0)
                    return;
            }
            uniq_header = (flags.vanq_sortmode == VANQ_ALPHA_SEP);
            class_header = ((flags.vanq_sortmode == VANQ_MCLS_LTOH
                             || flags.vanq_sortmode == VANQ_MCLS_HTOL)
                            && ntypes > 1);

            klwin = create_nhwindow(NHW_MENU);
            putstr(klwin, 0, "Поверженные существа:");
            if (!dumping)
                putstr(klwin, 0, "");

            qsort((genericptr_t) mindx, ntypes, sizeof *mindx, vanqsort_cmp);
            for (ni = 0; ni < ntypes; ni++) {
                i = mindx[ni];
                nkilled = svm.mvitals[i].died;
                Rider = is_rider(&mons[i]);
                mlet = mons[i].mlet;
                if (class_header
                    && (mlet != prev_mlet || (special_hdr && !Rider))) {
                    if (!Rider) {
                        Strcpy(buf, def_monsyms[(int) mlet].explain);
                        special_hdr = FALSE;
                    } else {
                        Strcpy(buf, "Всадник");
                        special_hdr = TRUE;
                    }
                    /* 'ask' implies final disclosure, where highlighting
                       of various header lines is suppressed */
                    putstr(klwin, ask ? ATR_NONE : iflags.menu_headings.attr,
                           upstart(buf));
                    prev_mlet = mlet;
                }
                if (UniqCritterIndx(i)) {
                    Sprintf(buf, "%s%s",
                            !type_is_pname(&mons[i]) ? "" : "",
                            mons[i].pmnames[NEUTRAL]);
                    if (nkilled > 1)
                        Sprintf(eos(buf), " (%s)",
                                N_times((long) nkilled, buftoo));
                    was_uniq = TRUE;
                } else {
                    if (uniq_header && was_uniq) {
                        putstr(klwin, 0, "");
                        was_uniq = FALSE;
                    }
                    /* trolls or undead might have come back,
                       but we don't keep track of that */
                    if (nkilled == 1)
                        Strcpy(buf, mons[i].pmnames[NEUTRAL]);
                    else
                        Sprintf(buf, "%3d %s", nkilled,
                                makeplural(mons[i].pmnames[NEUTRAL]));
                }
                /* number of leading spaces to match 3 digit prefix */
                pfx = !strncmpi(buf, "the ", 4) ? 0
                      : !strncmpi(buf, "an ", 3) ? 1
                        : !strncmpi(buf, "a ", 2) ? 2
                          : !digit(buf[2]) ? 4 : 0;
                if (class_header)
                    ++pfx;
                Snprintf(buftoo, sizeof buftoo, "%*s%s", pfx, "", buf);
                putstr(klwin, 0, buftoo);
            }
            /*
             * if (Hallucination)
             *     putstr(klwin, 0, "and a partridge in a pear tree");
             */
            if (ntypes > 1) {
                if (!dumping)
                    putstr(klwin, 0, "");
                Sprintf(buf, "Существ повержено: %ld.", total_killed);
                putstr(klwin, 0, buf);
            }
            display_nhwindow(klwin, TRUE);
            destroy_nhwindow(klwin);
        }

    /*
     * For end-of-game disclosure, we're only called when some monsters
     * were vanquished and won't reach these 'else-if's.
     *
     * If no monsters have been vanquished, we're either called for game
     * still in progress, so use present tense via pline(), or for dumplog
     * which needs putstr() and past tense.
     */
    } else if (!program_state.gameover) {
        /* #vanquished rather than final disclosure, so pline() is ok */
        pline("Ни одно существо не было повержено.");
#ifdef DUMPLOG
    } else if (dumping) {
        putstr(0, 0, "Ни одно существо не было повержено."); /* not pline() */
#endif
    }
}

/* number of monster species which have been genocided */
int
num_genocides(void)
{
    int i, n = 0;

    for (i = LOW_PM; i < NUMMONS; ++i) {
        if (svm.mvitals[i].mvflags & G_GENOD) {
            ++n;
            if (UniqCritterIndx(i))
                impossible("unique creature '%d: %s' genocided?",
                           i, mons[i].pmnames[NEUTRAL]);
        }
    }
    return n;
}

/* return a count of the number of extinct species */
staticfn int
num_extinct(void)
{
    int i, n = 0;

    for (i = LOW_PM; i < NUMMONS; ++i) {
        if (UniqCritterIndx(i))
            continue;
        if ((svm.mvitals[i].mvflags & G_GONE) == G_EXTINCT)
            ++n;
    }
    return n;
}

/* collect both genocides and extinctions, skipping uniques */
staticfn int
num_gone(int mvflags, int *mindx)
{
    uchar mflg = (uchar) mvflags;
    int i, n = 0;

    (void) memset((genericptr_t) mindx, 0, NUMMONS * sizeof *mindx);

    for (i = LOW_PM; i < NUMMONS; ++i) {
        /* uniques can't be genocided but can become extinct;
           however, they're never reported as extinct, so skip them */
        if (UniqCritterIndx(i))
            continue;

        if ((svm.mvitals[i].mvflags & mflg) != 0)
            mindx[n++] = i;
    }
    return n;
}

/* show genocided and extinct monster types for final disclosure/dumplog
   or for the #genocided command */
void
list_genocided(char defquery, boolean ask)
{
    int i, mndx;
    int ngenocided, nextinct, ngone, mvflags, mindx[NUMMONS];
    char c;
    winid klwin;
    char buf[BUFSZ];
    boolean genoing, /* prompting for genocide or class genocide */
            dumping; /* for DUMPLOG; doesn't need to be conditional */
    boolean both = (program_state.gameover || wizard || discover);

    dumping = (defquery == 'd');
    genoing = (defquery == 'g');
    if (dumping || genoing)
        defquery = 'y';
    if (genoing)
        both = FALSE; /* genocides only, not extinctions */

    /* this goes through the whole monster list up to three times but will
       happen rarely and is simpler than a more general single pass check;
       extinctions are only revealed during end of game disclosure or when
       running in wizard or explore mode */
    ngenocided = num_genocides();
    nextinct = both ? num_extinct() : 0;
    mvflags = G_GENOD | (both ? G_EXTINCT : 0);
    ngone = num_gone(mvflags, mindx);

    /* genocided or extinct species list */
    if (ngone > 0) {
        Sprintf(buf, "Нужен ли список %sвидов%s%s?",
                (nextinct && !ngenocided) ? "вымерших " : "",
                (ngenocided) ? "истреблённых" : "",
                (nextinct && ngenocided) ? " и вымерших" : "");
        c = ask ? yn_function(buf, (ngone > 1) ? "ynaq" : "ynq\033a",
                              defquery, TRUE)
                : defquery;
        if (c == 'q')
            done_stopprint++;
        if (c == 'y' || c == 'a') {
            int save_sortmode;
            char mlet, prev_mlet = 0;
            boolean class_header = FALSE;

            if (ngone > 1) {
                if (c == 'a') { /* ask player to choose sort order */
                    /* #genocided shares #vanquished's sort order */
                    if (set_vanq_order(FALSE) < 0)
                        return;
                }
                /* sort orderings count-high-to-low or count-low-to-high
                   don't make sense for genocides; if the preferred order
                   to set to either of those, use alphabetical instead;
                   note: the tie breaker for by-class is level-high-to-low
                   or level-low-to-high rather than count so is ok as-is */
                save_sortmode = flags.vanq_sortmode;
                if (flags.vanq_sortmode == VANQ_COUNT_H_L
                    || flags.vanq_sortmode == VANQ_COUNT_L_H)
                    flags.vanq_sortmode = VANQ_ALPHA_MIX;
                qsort((genericptr_t) mindx, ngone,
                      sizeof *mindx, vanqsort_cmp);
                class_header = (flags.vanq_sortmode == VANQ_MCLS_LTOH
                                || flags.vanq_sortmode == VANQ_MCLS_HTOL);
                flags.vanq_sortmode = save_sortmode;
            }

            klwin = create_nhwindow(NHW_MENU);
            Sprintf(buf, "%s%s виды:",
                    (ngenocided) ? "Истреблённые" : "Вымершие",
                    (nextinct && ngenocided) ? " или вымершие" : "");
            putstr(klwin, 0, buf);
            if (!dumping)
                putstr(klwin, 0, "");

            for (i = 0; i < ngone; ++i) {
                mndx = mindx[i];
                mlet = mons[mndx].mlet;
                if (class_header && mlet != prev_mlet) {
                    Strcpy(buf, def_monsyms[(int) mlet].explain);
                    /* 'ask' implies final disclosure, where highlighting
                       of various header lines is suppressed */
                    putstr(klwin, ask ? ATR_NONE : iflags.menu_headings.attr,
                           upstart(buf));
                    prev_mlet = mlet;
                }
                Sprintf(buf, " %s", makeplural(mons[mndx].pmnames[NEUTRAL]));
                /*
                 * "Extinct" is unfortunate terminology.  A species
                 * is marked extinct when its birth limit is reached,
                 * but there might be members of the species still
                 * alive, contradicting the meaning of the word.
                 *
                 * We only append "(extinct)" if the G_GENOD bit is
                 * clear.  During normal play, 'mndx' won't be in the
                 * collected list unless that bit is set.
                 */
                if ((svm.mvitals[mndx].mvflags & G_GONE) == G_EXTINCT)
                    Strcat(buf, " (вымерли)");
                putstr(klwin, 0, buf);
            }
            if (!dumping)
                putstr(klwin, 0, "");
            if (ngenocided > 0) {
                Sprintf(buf, "Истреблено видов: %d.", ngenocided);
                putstr(klwin, 0, buf);
            }
            if (nextinct > 0) {
                Sprintf(buf, "Вымерло видов: %d.", nextinct);
                putstr(klwin, 0, buf);
            }

            display_nhwindow(klwin, TRUE);
            destroy_nhwindow(klwin);
        }

    /* See the comment for similar code near the end of list_vanquished(). */
    } else if (!program_state.gameover) {
        /* #genocided rather than final disclosure, so pline() is ok and
           extinction has been ignored */
        pline("Ни один вид не истреблён%s.", genoing ? " пока" : "");
#ifdef DUMPLOG
    } else if (dumping) { /* 'gameover' is True if we make it here */
        putstr(0, 0, "Виды не истреблялись и не вымирали.");
#endif
    }
}

/* M-g - #genocided command */
int
dogenocided(void)
{
    list_genocided(iflags.menu_requested ? 'a' : 'y', FALSE);
    return ECMD_OK;
}

DISABLE_WARNING_FORMAT_NONLITERAL

/* #wizborn extended command */
int
doborn(void)
{
    static const char fmt[] = "%4i %4i %c %-30s";
    int i;
    winid datawin = create_nhwindow(NHW_TEXT);
    char buf[BUFSZ];
    int nborn = 0, ndied = 0;

    putstr(datawin, 0, "умерло родилось");
    for (i = LOW_PM; i < NUMMONS; i++)
        if (svm.mvitals[i].born || svm.mvitals[i].died
            || (svm.mvitals[i].mvflags & G_GONE) != 0) {
            Sprintf(buf, fmt,
                    svm.mvitals[i].died, svm.mvitals[i].born,
                    ((svm.mvitals[i].mvflags & G_GONE) == G_EXTINCT) ? 'E'
                    : ((svm.mvitals[i].mvflags & G_GONE) == G_GENOD) ? 'G'
                      : ((svm.mvitals[i].mvflags & G_GONE) != 0) ? 'X'
                        : ' ',
                    mons[i].pmnames[NEUTRAL]);
            putstr(datawin, 0, buf);
            nborn += svm.mvitals[i].born;
            ndied += svm.mvitals[i].died;
        }

    putstr(datawin, 0, "");
    Sprintf(buf, fmt, ndied, nborn, ' ', "");

    display_nhwindow(datawin, FALSE);
    destroy_nhwindow(datawin);

    return ECMD_OK;
}

RESTORE_WARNING_FORMAT_NONLITERAL

/*
 * align_str(), piousness(), mstatusline() and ustatusline() once resided
 * in pline.c, then got moved to priest.c just to be out of there.  They
 * fit better here.
 */

const char *
align_str(aligntyp alignment)
{
    switch ((int) alignment) {
    case A_CHAOTIC:
        return "хаотичный";
    case A_NEUTRAL:
        return "нейтральный";
    case A_LAWFUL:
        return "законный";
    case A_NONE:
        return "неприсоединённый";
    }
    return "неизвестный";
}

staticfn char *
size_str(int msize)
{
    static char outbuf[40];

    switch (msize) {
    case MZ_TINY:
        Strcpy(outbuf, "крошечный");
        break;
    case MZ_SMALL:
        Strcpy(outbuf, "маленький");
        break;
    case MZ_MEDIUM:
        Strcpy(outbuf, "средний");
        break;
    case MZ_LARGE:
        Strcpy(outbuf, "большой");
        break;
    case MZ_HUGE:
        Strcpy(outbuf, "огромный");
        break;
    case MZ_GIGANTIC:
        Strcpy(outbuf, "гигантский");
        break;
    default:
        Sprintf(outbuf, "неизвестный размер (%d)", msize);
        break;
    }
    return outbuf;
}

/* used for self-probing */
char *
piousness(boolean showneg, const char *suffix)
{
    static char buf[32]; /* bigger than "insufficiently neutral" */
    const char *pio;

    /* note: piousness 20 matches MIN_QUEST_ALIGN (quest.h) */
    if (u.ualign.record >= 20)
        pio = "благочестиво";
    else if (u.ualign.record > 13)
        pio = "набожно";
    else if (u.ualign.record > 8)
        pio = "пылко";
    else if (u.ualign.record > 3)
        pio = "настойчиво";
    else if (u.ualign.record == 3)
        pio = "";
    else if (u.ualign.record > 0)
        pio = "неуверенно";
    else if (u.ualign.record == 0)
        pio = "номинально";
    else if (!showneg)
        pio = "недостаточно";
    else if (u.ualign.record >= -3)
        pio = "отклонился";
    else if (u.ualign.record >= -8)
        pio = "согрешивший";
    else
        pio = "преступивший";

    Sprintf(buf, "%s", pio);
    if (suffix && (!showneg || u.ualign.record >= 0)) {
        if (u.ualign.record != 3)
            Strcat(buf, " ");
        Strcat(buf, suffix);
    }
    return buf;
}

/* stethoscope or probing applied to monster -- one-line feedback */
void
mstatusline(struct monst *mtmp)
{
    aligntyp alignment = mon_aligntyp(mtmp);
    char info[BUFSZ], monnambuf[BUFSZ];

    info[0] = 0;
    if (mtmp->mtame) {
        Strcat(info, ", приручён");
        if (wizard) {
            Sprintf(eos(info), " (%d", mtmp->mtame);
            if (!mtmp->isminion)
                Sprintf(eos(info), "; голод %ld; аппортность %d",
                        EDOG(mtmp)->hungrytime, EDOG(mtmp)->apport);
            Strcat(info, ")");
        }
    } else if (mtmp->mpeaceful)
        Strcat(info, ", мирный");

    if (mtmp->data == &mons[PM_LONG_WORM]) {
        int segndx, nsegs = count_wsegs(mtmp);

        /* the worm code internals don't consider the head to be one of
           the worm's segments, but we count it as such when presenting
           worm feedback to the player */
        if (!nsegs) {
            Strcat(info, ", одиночный сегмент");
        } else {
            ++nsegs; /* include head in the segment count */
            segndx = wseg_at(mtmp, gb.bhitpos.x, gb.bhitpos.y);
            Sprintf(eos(info), ", %d из %d сегментов",
                    segndx, nsegs);
        }
    }
    if (ismnum(mtmp->cham) && mtmp->data != &mons[mtmp->cham])
        /* don't reveal the innate form (chameleon, vampire, &c),
           just expose the fact that this current form isn't it */
        Strcat(info, ", меняет форму");
    /* pets eating mimic corpses mimic while eating, so this comes first */
    if (mtmp->meating)
        Strcat(info, ", ест");
    /* a stethoscope exposes mimic before getting here so this
       won't be relevant for it, but wand of probing doesn't */
    if (mtmp->mundetected || mtmp->m_ap_type
        || visible_region_at(gb.bhitpos.x, gb.bhitpos.y))
        mhidden_description(mtmp,
                       MHID_PREFIX | MHID_ARTICLE | MHID_ALTMON | MHID_REGION,
                            eos(info));
    if (mtmp->mcan)
        Strcat(info, ", нейтрализован");
    if (mtmp->mconf)
        Strcat(info, ", сбит с толку");
    if (mtmp->mblinded || !mtmp->mcansee)
        Strcat(info, ", слеп");
    if (mtmp->mstun)
        Strcat(info, ", оглушён");
    if (mtmp->msleeping)
        Strcat(info, ", спит");
#if 0 /* unfortunately mfrozen covers temporary sleep and being busy
       * (donning armor, for instance) as well as paralysis */
    else if (mtmp->mfrozen)
        Strcat(info, ", paralyzed");
#else
    else if (mtmp->mfrozen || !mtmp->mcanmove)
        Strcat(info, ", не двигается");
#endif
    /* [arbitrary reason why it isn't moving] */
    else if ((mtmp->mstrategy & STRAT_WAITMASK) != 0)
        Strcat(info, ", медитирует");
    if (mtmp->mflee)
        Strcat(info, ", напуган");
    if (mtmp->mtrapped)
        Strcat(info, ", в ловушке");
    if (mtmp->mspeed)
        Strcat(info, (mtmp->mspeed == MFAST) ? ", быстрый"
                      : (mtmp->mspeed == MSLOW) ? ", медленный"
                         : ", [? скорость]");
    if (mtmp->minvis)
        Strcat(info, ", невидим");
    if (mtmp == u.ustuck) {
        struct permonst *pm = u.ustuck->data;

        /* being swallowed/engulfed takes priority over sticks(youmonst);
           this used to have that backwards and checked sticks() first */
        Strcat(info, u.uswallow ? (digests(pm)
                                   ? ", переваривает вас"
                                   /* note: the "swallowing you" case won't
                                      happen because all animal engulfers
                                      either digest their victims (purple
                                      worm) or enfold them (trappers and
                                      lurkers above) */
                                   : (is_animal(pm) && !enfolds(pm))
                                     ? ", заглатывает вас"
                                     : ", поглощает вас")
                     /* !u.uswallow; if both youmonst and ustuck are holders,
                        youmonst wins */
                     : (!sticks(gy.youmonst.data) ? ", держит вас"
                                                 : ", сдерживается вами"));
    }
    if (mtmp == u.usteed) {
        Strcat(info, ", несёт вас");
        if (Wounded_legs) {
            /* EWounded_legs is used to track left/right/both rather than
               some form of extrinsic impairment; HWounded_legs is used for
               timeout; both apply to steed instead of hero when mounted */
            long legs = (EWounded_legs & BOTH_SIDES);
            const char *what = mbodypart(mtmp, LEG);

            if (legs == BOTH_SIDES)
                what = makeplural(what);
            Sprintf(eos(info), ", повреждено: %s", what);
        }
    }
    if (mtmp->mleashed)
        Strcat(info, ", на поводке");

    /* avoid "Status of the invisible newt ..., invisible" */
    /* and unlike a normal mon_nam, use "saddled" even if it has a name */
    Strcpy(monnambuf, x_monnam(mtmp, ARTICLE_YOUR, (char *) 0,
                               (SUPPRESS_IT | SUPPRESS_INVISIBLE), FALSE));

    pline("Статус %s (%s, %s):  Уровень %d  HP %d(%d)  AC %d%s.",
          monnambuf, align_str(alignment), size_str(mtmp->data->msize),
          mtmp->m_lev, mtmp->mhp, mtmp->mhpmax, find_mac(mtmp), info);
}

/* stethoscope or probing applied to hero -- one-line feedback */
void
ustatusline(void)
{
    NhRegion *reg;
    char info[BUFSZ];
    size_t ln;

    info[0] = '\0';
    if (Sick) {
        Strcat(info, ", умирает от");
        if (u.usick_type & SICK_VOMITABLE)
            Strcat(info, " пищевого отравления");
        if (u.usick_type & SICK_NONVOMITABLE) {
            if (u.usick_type & SICK_VOMITABLE)
                Strcat(info, " и");
            Strcat(info, " болезни");
        }
    }
    if (Stoned)
        Strcat(info, ", каменеет");
    if (Slimed)
        Strcat(info, ", покрывается слизью");
    if (Strangled)
        Strcat(info, ", удушаем");
    if (Vomiting)
        Strcat(info, ", тошнота"); /* !"nauseous" */
    if (Confusion)
        Strcat(info, ", сбит с толку");
    if (Blind) {
        Strcat(info, ", слеп");
        if (u.ucreamed) {
            if ((long) u.ucreamed < BlindedTimeout || Blindfolded
                || !haseyes(gy.youmonst.data))
                Strcat(info, ", покрыт");
            Strcat(info, " липкой слизью");
        } /* note: "goop" == "glop"; variation is intentional */
    }
    if (Stunned)
        Strcat(info, ", оглушён");
    if (Wounded_legs && !u.usteed) {
        /* EWounded_legs is used to track left/right/both rather than some
           form of extrinsic impairment; HWounded_legs is used for timeout;
           both apply to steed instead of hero when mounted */
        long legs = (EWounded_legs & BOTH_SIDES);
        const char *what = body_part(LEG);

        if (legs == BOTH_SIDES)
            what = makeplural(what);
        /* when it's just one leg, ^X reports which, left or right;
           ustatusline() doesn't, in order to keep the output a bit shorter */
        Sprintf(eos(info), ", повреждено: %s", what);
    }
    if (Glib)
        Sprintf(eos(info), ", скользкие %s", fingers_or_gloves(TRUE));
    if (u.utrap)
        Strcat(info, ", в ловушке");
    if (Fast)
        Strcat(info, Very_fast ? ", очень быстр" : ", быстр");
    if (u.uundetected)
        Strcat(info, ", скрыт");
    else if (U_AP_TYPE != M_AP_NOTHING)
        Strcat(info, ", замаскирован");
    if (Invis)
        Strcat(info, ", невидим");
    if (u.ustuck) {
        if (u.uswallow)
            Strcat(info, digests(u.ustuck->data) ? ", переваривается "
                                                 : ", поглощён ");
        else if (!sticks(gy.youmonst.data))
            Strcat(info, ", удерживается ");
        else
            Strcat(info, ", держит ");
        /* FIXME? a_monnam() uses x_monnam() which has a special case that
           forces "the" instead of "a" when formatting u.ustuck while hero
           is swallowed; we don't really want that here but it isn't worth
           fiddling with just for self-probing while engulfed */
        Strcat(info, a_monnam(u.ustuck));
    }
    if (!u.uswallow
        && (reg = visible_region_at(u.ux, u.uy)) != 0
        && (ln = strlen(info)) < sizeof info)
        Snprintf(eos(info), sizeof info - ln, ", в облаке %s",
                 reg_damg(reg) ? "ядовитого газа" : "пара");

    pline("Статус %s (%s):  Уровень %d  HP %d(%d)  AC %d%s.", svp.plname,
          piousness(FALSE, align_str(u.ualign.type)),
          Upolyd ? mons[u.umonnum].mlevel : u.ulevel, Upolyd ? u.mh : u.uhp,
          Upolyd ? u.mhmax : u.uhpmax, u.uac, info);
}

/* for 'onefile' processing where end of this file isn't necessarily the
   end of the source code seen by the compiler */
#undef enl_msg
#undef you_are
#undef you_have
#undef you_can
#undef you_have_been
#undef you_have_never
#undef you_have_X
#undef LL_majors
#undef majorevent
#undef spoilerevent
#undef UniqCritterIndx
#undef done_stopprint

/*insight.c*/
