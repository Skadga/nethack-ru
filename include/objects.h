/* NetHack 5.0	objects.h	$NHDT-Date: 1749097644 2025/06/04 20:27:24 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.30 $ */
/* Copyright (c) Mike Threepoint, 1989.                           */
/* NetHack may be freely redistributed.  See license for details. */

/*
   The data in this file is processed multiple times by its inclusion
   in several places in the code. The results depend on the definition
   of the following:
     OBJECTS_ENUM        to construct object onames enum entries (decl.h).
     OBJECTS_DESCR_INIT to construct obj_descr[] array entries (objects.c).
     OBJECTS_INIT       to construct objects[] array entries (objects.c).
*/

#ifndef NoDes
#define NoDes (char *) 0 /* less visual distraction for 'no description' */
#endif

#ifndef lint
#define HARDGEM(n) (n >= 8)
#else
#define HARDGEM(n) (0)
#endif

/*
 * Note...
 *  OBJECTS() currently has 15 parameters; it more become needed, some
 *  will need to be combined the way BITS() is used, because compilers
 *  are allowed to impose a limit of 15.
 */

#if defined(OBJECTS_DESCR_INIT)
#define OBJ(name,desc)  name, desc
#define OBJECT(obj,bits,prp,sym,prob,dly,wt, \
               cost,sdam,ldam,oc1,oc2,nut,color,sn)  { obj }
#define MARKER(tag,sn) /*empty*/

#elif defined(OBJECTS_INIT)
/* notes: 'sub' was once a bitfield but got changed to separate schar when
   it was overloaded to hold negative weapon skill indices; the first zero
   is padding for oc_prediscovered which has variable init at run-time;
   the second zero is oc_spare1 for padding between oc_tough and oc_dir */
#define BITS(nmkn,mrg,uskn,ctnr,mgc,chrg,uniq,nwsh,big,tuf,dir,sub,mtrl) \
    nmkn,mrg,uskn,0,mgc,chrg,uniq,nwsh,big,tuf,0,dir,mtrl,sub /*cpp fodder*/
/* note: 0UL-1UL is a method of expressing the largest possible
   unsigned long value whilst working around a false-positive warning
   in Microsoft Visual C (which assumes that a negative number was
   intended despite the explicit U suffix) */
#define OBJECT(obj,bits,prp,sym,prob,dly,wt,        \
               cost,sdam,ldam,oc1,oc2,nut,color,sn) \
  { 0, 0, (char *) 0, bits, prp, sym, dly, color, prob, wt, \
    cost, sdam, ldam, oc1, oc2, nut, (0UL-1UL), 0, (0UL-1UL), 0 }
#define MARKER(tag,sn) /*empty*/

#elif defined(OBJECTS_ENUM)
#define OBJ(name,desc)
#define OBJECT(obj,bits,prp,sym,prob,dly,wt,        \
               cost,sdam,ldam,oc1,oc2,nut,color,sn) \
    sn
#define MARKER(tag,sn) tag = sn,

#elif defined(DUMP_ENUMS)
#define OBJ(name,desc)
#define OBJECT(obj,bits,prp,sym,prob,dly,wt,        \
               cost,sdam,ldam,oc1,oc2,nut,color,sn) \
  { sn, #sn }
#define MARKER(tag,sn) /*empty*/

#else
#error Unproductive inclusion of objects.h
#endif  /* OBJECTS_DESCR_INIT || OBJECTS_INIT || OBJECTS_ENUM */

#define GENERIC(desc, class, gen_enum) \
    OBJECT(OBJ("обычное " desc, desc),                                  \
           BITS(0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, 0),            \
           0, class, 0, 0, 0, 0, 0, 0, 0, 0, 0, CLR_GRAY, gen_enum)

/* dummy object[0] -- description [2nd arg] *must* be NULL */
OBJECT(OBJ("странный предмет", NoDes),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, 0),
       0, ILLOBJ_CLASS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, STRANGE_OBJECT),
/* slots [1] through [MAXOCLASSES-1] are indexed by class; some are
   used for display purposes, most aren't used; none are actual objects;
   note that 'real' strange object is in slot [0] but ILLOBJ_CLASS is 1
   so we add a dummy for it in slot [1] to simplify accessing the rest;
   there isn't any entry for RANDOM_CLASS (0) */
GENERIC("странный",    ILLOBJ_CLASS,  GENERIC_ILLOBJ),  /* [1] */
GENERIC("оружие",     WEAPON_CLASS,  GENERIC_WEAPON),  /* [2] */
GENERIC("броня",      ARMOR_CLASS,   GENERIC_ARMOR),   /* [3] */
GENERIC("кольцо",       RING_CLASS,    GENERIC_RING),    /* [4] */
GENERIC("амулет",     AMULET_CLASS,  GENERIC_AMULET),  /* [5] */
GENERIC("инструмент",       TOOL_CLASS,    GENERIC_TOOL),    /* [6] */
GENERIC("еда",       FOOD_CLASS,    GENERIC_FOOD),    /* [7] */
GENERIC("зелье",     POTION_CLASS,  GENERIC_POTION),  /* [8] */
GENERIC("свиток",     SCROLL_CLASS,  GENERIC_SCROLL),  /* [9] */
GENERIC("книга заклинаний",  SPBOOK_CLASS,  GENERIC_SPBOOK),  /* [10] */
GENERIC("палочка",       WAND_CLASS,    GENERIC_WAND),    /* [11] */
GENERIC("монета",       COIN_CLASS,    GENERIC_COIN),    /* [12] */
GENERIC("самоцвет",        GEM_CLASS,     GENERIC_GEM),     /* [13] */
GENERIC("большой валун", ROCK_CLASS,    GENERIC_ROCK),    /* [14] bldr+statue */
GENERIC("железный шар",  BALL_CLASS,    GENERIC_BALL),    /* [15] */
GENERIC("железная цепь", CHAIN_CLASS,   GENERIC_CHAIN),   /* [16] */
GENERIC("яд",      VENOM_CLASS,   GENERIC_VENOM),   /* [17] */
#undef GENERIC
/* FIRST_OBJECT: it would be simpler just to use MARKER(FIRST_OBJECT,ARROW)
   below but that is vulnerable to neglecting to update the marker enum
   after inserting something in front of arrow */
MARKER(LAST_GENERIC, GENERIC_VENOM)
MARKER(FIRST_OBJECT, LAST_GENERIC + 1)
/* this definition of FIRST_OBJECT advances the default value for next enum;
   backtrack to fix that, otherwise ARROW and the rest would be off by 1 */
MARKER(OBJCLASS_HACK, FIRST_OBJECT - 1)

/* weapons ... */
#define WEAPON(name,desc,kn,mg,bi,prob,wt,                          \
               cost,sdam,ldam,hitbon,typ,sub,metal,color,sn)        \
    OBJECT(OBJ(name,desc),                                          \
           BITS(kn, mg, 1, 0, 0, 1, 0, 0, bi, 0, typ, sub, metal),  \
           0, WEAPON_CLASS, prob, 0, wt,                            \
           cost, sdam, ldam, hitbon, 0, wt, color,sn)
#define PROJECTILE(name,desc,kn,prob,wt,                            \
                   cost,sdam,ldam,hitbon,metal,sub,color,sn)        \
    OBJECT(OBJ(name,desc),                                          \
           BITS(kn, 1, 1, 0, 0, 1, 0, 0, 0, 0, PIERCE, sub, metal), \
           0, WEAPON_CLASS, prob, 0, wt,                            \
           cost, sdam, ldam, hitbon, 0, wt, color, sn)
#define BOW(name,desc,kn,prob,wt,cost,hitbon,metal,sub,color,sn)    \
    OBJECT(OBJ(name,desc),                                          \
           BITS(kn, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, sub, metal),      \
           0, WEAPON_CLASS, prob, 0, wt,                            \
           cost, 2, 2, hitbon, 0, wt, color, sn)

/* Note: for weapons that don't do an even die of damage (ex. 2-7 or 3-18)
   the extra damage is added on in weapon.c, not here! */

/* weapon strike mode overloads the oc_dir field */
#define P PIERCE
#define S SLASH
#define B WHACK

/* missiles; materiel reflects the arrowhead, not the shaft */
PROJECTILE("стрела", NoDes,
           1, 55, 1, 2, 6, 6, 0,        IRON, -P_BOW, HI_METAL,
                                                        ARROW),
PROJECTILE("эльфийская стрела", "руническая стрела",
           0, 20, 1, 2, 7, 6, 0,        WOOD, -P_BOW, HI_WOOD,
                                                        ELVEN_ARROW),
PROJECTILE("оркская стрела", "грубая стрела",
           0, 20, 1, 2, 5, 6, 0,        IRON, -P_BOW, CLR_BLACK,
                                                        ORCISH_ARROW),
PROJECTILE("серебряная стрела", NoDes,
           1, 12, 1, 5, 6, 6, 0,        SILVER, -P_BOW, HI_SILVER,
                                                        SILVER_ARROW),
PROJECTILE("я", "бамбуковая стрела",
           0, 15, 1, 4, 7, 7, 1,        METAL, -P_BOW, HI_METAL, YA),
PROJECTILE("арбалетный болт", NoDes,
           1, 55, 1, 2, 4, 6, 0,        IRON, -P_CROSSBOW, HI_METAL,
                                                        CROSSBOW_BOLT),

/* missiles that don't use a launcher */
WEAPON("дротик", NoDes,
       1, 1, 0, 60,   1,   2,  3,  2, 0, P,   -P_DART, IRON, HI_METAL,
                                                        DART),
WEAPON("сюрикен", "метательная звезда",
       0, 1, 0, 35,   1,   5,  8,  6, 2, P,   -P_SHURIKEN, IRON, HI_METAL,
                                                        SHURIKEN),
WEAPON("бумеранг", NoDes,
       1, 1, 0, 15,   5,  20,  9,  9, 0, 0,   -P_BOOMERANG, WOOD, HI_WOOD,
                                                        BOOMERANG),

/* spears [note: javelin used to have a separate skill from spears,
   because the latter are primarily stabbing weapons rather than
   throwing ones; but for playability, they've been merged together
   under spear skill and spears can now be thrown like javelins] */
WEAPON("копьё", NoDes,
       1, 1, 0, 50,  30,   3,  6,  8, 0, P,   P_SPEAR, IRON, HI_METAL,
                                                        SPEAR),
WEAPON("эльфийское копьё", "руническое копьё",
       0, 1, 0, 10,  30,   3,  7,  8, 0, P,   P_SPEAR, WOOD, HI_WOOD,
                                                        ELVEN_SPEAR),
WEAPON("оркское копьё", "грубое копьё",
       0, 1, 0, 13,  30,   3,  5,  8, 0, P,   P_SPEAR, IRON, CLR_BLACK,
                                                        ORCISH_SPEAR),
WEAPON("дварфийское копьё", "крепкое копьё",
       0, 1, 0, 12,  35,   3,  8,  8, 0, P,   P_SPEAR, IRON, HI_METAL,
                                                        DWARVISH_SPEAR),
WEAPON("серебряное копьё", NoDes,
       1, 1, 0,  2,  36,  40,  6,  8, 0, P,   P_SPEAR, SILVER, HI_SILVER,
                                                        SILVER_SPEAR),
WEAPON("метательное копьё", "метательное копьё",
       0, 1, 0, 10,  20,   3,  6,  6, 0, P,   P_SPEAR, IRON, HI_METAL,
                                                        JAVELIN),

/* spearish; doesn't stack, not intended to be thrown */
WEAPON("трезубец", NoDes,
       1, 0, 0,  8,  25,   5,  6,  4, 0, P,   P_TRIDENT, IRON, HI_METAL,
                                                        TRIDENT),
        /* +1 small, +2d4 large */

/* blades; all stack */
WEAPON("кинжал", NoDes,
       1, 1, 0, 30,  10,   4,  4,  3, 2, P,   P_DAGGER, IRON, HI_METAL,
                                                        DAGGER),
WEAPON("эльфийский кинжал", "рунический кинжал",
       0, 1, 0, 10,  10,   4,  5,  3, 2, P,   P_DAGGER, WOOD, HI_WOOD,
                                                        ELVEN_DAGGER),
WEAPON("оркский кинжал", "грубый кинжал",
       0, 1, 0, 12,  10,   4,  3,  3, 2, P,   P_DAGGER, IRON, CLR_BLACK,
                                                        ORCISH_DAGGER),
WEAPON("серебряный кинжал", NoDes,
       1, 1, 0,  3,  12,  40,  4,  3, 2, P,   P_DAGGER, SILVER, HI_SILVER,
                                                        SILVER_DAGGER),
WEAPON("кержамес", NoDes,
       1, 1, 0,  0,  10,   4,  4,  3, 2, S,   P_DAGGER, IRON, HI_METAL,
                                                        ATHAME),
WEAPON("скальпель", NoDes,
       1, 1, 0,  0,   5,   6,  3,  3, 2, S,   P_KNIFE, METAL, HI_METAL,
                                                        SCALPEL),
WEAPON("нож", NoDes,
       1, 1, 0, 20,   5,   4,  3,  2, 0, P|S, P_KNIFE, IRON, HI_METAL,
                                                        KNIFE),
WEAPON("стилет", NoDes,
       1, 1, 0,  5,   5,   4,  3,  2, 0, P|S, P_KNIFE, IRON, HI_METAL,
                                                        STILETTO),
/* 3.6: worm teeth and crysknives now stack;
   when a stack of teeth is enchanted at once, they fuse into one crysknife;
   when a stack of crysknives drops, the whole stack reverts to teeth */
/* 5.0: change crysknife from MINERAL to BONE and worm tooth from 0 to BONE */
WEAPON("зуб червя", NoDes,
       1, 1, 0,  0,  20,   2,  2,  2, 0, 0,   P_KNIFE, BONE, CLR_WHITE,
                                                        WORM_TOOTH),
WEAPON("криснож", NoDes,
       1, 1, 0,  0,  20, 100, 10, 10, 3, P,   P_KNIFE, BONE, CLR_WHITE,
                                                        CRYSKNIFE),

/* axes */
WEAPON("топор", NoDes,
       1, 0, 0, 40,  60,   8,  6,  4, 0, S,   P_AXE, IRON, HI_METAL,
                                                        AXE),
WEAPON("боевой топор", "двухлезвийный топор",       /* "double-bitted"? */
       0, 0, 1, 10, 120,  40,  8,  6, 0, S,   P_AXE, IRON, HI_METAL,
                                                        BATTLE_AXE),

/* swords */
WEAPON("короткий меч", NoDes,
       1, 0, 0,  8,  30,  10,  6,  8, 0, P,   P_SHORT_SWORD, IRON, HI_METAL,
                                                        SHORT_SWORD),
WEAPON("эльфийский короткий меч", "рунический короткий меч",
       0, 0, 0,  2,  30,  10,  8,  8, 0, P,   P_SHORT_SWORD, WOOD, HI_WOOD,
                                                        ELVEN_SHORT_SWORD),
WEAPON("оркский короткий меч", "грубый короткий меч",
       0, 0, 0,  3,  30,  10,  5,  8, 0, P,   P_SHORT_SWORD, IRON, CLR_BLACK,
                                                        ORCISH_SHORT_SWORD),
WEAPON("дварфийский короткий меч", "широкий короткий меч",
       0, 0, 0,  2,  30,  10,  7,  8, 0, P,   P_SHORT_SWORD, IRON, HI_METAL,
                                                        DWARVISH_SHORT_SWORD),
WEAPON("ятаган", "изогнутый меч",
       0, 0, 0, 15,  40,  15,  8,  8, 0, S,   P_SABER, IRON, HI_METAL,
                                                        SCIMITAR),
WEAPON("серебряная сабля", NoDes,
       1, 0, 0,  6,  40,  75,  8,  8, 0, S,   P_SABER, SILVER, HI_SILVER,
                                                        SILVER_SABER),
WEAPON("палаш", NoDes,
       1, 0, 0,  8,  70,  10,  4,  6, 0, S,   P_BROAD_SWORD, IRON, HI_METAL,
                                                        BROADSWORD),
        /* +d4 small, +1 large */
WEAPON("эльфийский палаш", "рунический палаш",
       0, 0, 0,  4,  70,  10,  6,  6, 0, S,   P_BROAD_SWORD, WOOD, HI_WOOD,
                                                        ELVEN_BROADSWORD),
        /* +d4 small, +1 large */
WEAPON("длинный меч", NoDes,
       1, 0, 0, 50,  40,  15,  8, 12, 0, S,   P_LONG_SWORD, IRON, HI_METAL,
                                                        LONG_SWORD),
WEAPON("двуручный меч", NoDes,
       1, 0, 1, 22, 150,  50, 12,  6, 0, S,   P_TWO_HANDED_SWORD,
                                                            IRON, HI_METAL,
                                                        TWO_HANDED_SWORD),
        /* +2d6 large */
WEAPON("катана", "меч самурая",
       0, 0, 0,  4,  40,  80, 10, 12, 1, S,   P_LONG_SWORD, IRON, HI_METAL,
                                                        KATANA),
/* special swords set up for artifacts */
WEAPON("цуруги", "длинный меч самурая",
       0, 0, 1,  0,  60, 500, 16,  8, 2, S,   P_TWO_HANDED_SWORD,
                                                            METAL, HI_METAL,
                                                        TSURUGI),
        /* +2d6 large */
WEAPON("рунический меч", "рунический палаш",
       0, 0, 0,  0,  40, 300,  4,  6, 0, S,   P_BROAD_SWORD, IRON, CLR_BLACK,
                                                        RUNESWORD),
        /* +d4 small, +1 large; Stormbringer: +5d2 +d8 from level drain */

/* polearms */
/* spear-type */
WEAPON("протазан", "простое древковое оружие",
       0, 0, 1,  5,  80,  10,  6,  6, 0, P,   P_POLEARMS, IRON, HI_METAL,
                                                        PARTISAN),
        /* +1 large */
WEAPON("ронсар", "древковое оружие с рукоятью",
       0, 0, 1,  5,  50,   6,  4,  4, 0, P,   P_POLEARMS, IRON, HI_METAL,
                                                        RANSEUR),
        /* +d4 both */
WEAPON("спетум", "развильчатое древковое оружие",
       0, 0, 1,  5,  50,   5,  6,  6, 0, P,   P_POLEARMS, IRON, HI_METAL,
                                                        SPETUM),
        /* +1 small, +d6 large */
WEAPON("глефа", "древковое оружие с односторонним лезвием",
       0, 0, 1,  8,  75,   6,  6, 10, 0, S,   P_POLEARMS, IRON, HI_METAL,
                                                        GLAIVE),
/* axe-type */
WEAPON("алебарда", "изогнутая секира",
       0, 0, 1,  8, 150,  10, 10,  6, 0, P|S, P_POLEARMS, IRON, HI_METAL,
                                                        HALBERD),
        /* +1d6 large */
WEAPON("бардиш", "длинная секира",
       0, 0, 1,  4, 120,   7,  4,  4, 0, S,   P_POLEARMS, IRON, HI_METAL,
                                                        BARDICHE),
        /* +1d4 small, +2d4 large */
WEAPON("вуж", "древковый тесак",
       0, 0, 1,  4, 125,   5,  4,  4, 0, S,   P_POLEARMS, IRON, HI_METAL,
                                                        VOULGE),
        /* +d4 both */
/* curved/hooked */
WEAPON("фошар", "древковый серп",
       0, 0, 1,  6,  60,   5,  6,  8, 0, P|S, P_POLEARMS, IRON, HI_METAL,
                                                        FAUCHARD),
WEAPON("гизарма", "садовая коса",
       0, 0, 1,  6,  80,   5,  4,  8, 0, S,   P_POLEARMS, IRON, HI_METAL,
                                                        GUISARME),
        /* +1d4 small */
WEAPON("биль-гизарма", "крюкообразное древковое оружие",
       0, 0, 1,  4, 120,   7,  4, 10, 0, P|S, P_POLEARMS, IRON, HI_METAL,
                                                        BILL_GUISARME),
        /* +1d4 small */
/* other */
WEAPON("люцернский молот", "зубчатое древковое оружие",
       0, 0, 1,  5, 150,   7,  4,  6, 0, B|P, P_POLEARMS, IRON, HI_METAL,
                                                        LUCERN_HAMMER),
        /* +1d4 small */
WEAPON("бек де корбен", "древковое оружие с клювом",
       0, 0, 1,  4, 100,   8,  8,  6, 0, B|P, P_POLEARMS, IRON, HI_METAL,
                                                        BEC_DE_CORBIN),

/* formerly grouped with the polearms but don't use polearms skill;
   lance isn't even two-handed */
WEAPON("дварфийская кирка", "широкое кайло",
       0, 0, 1, 13, 120,  50, 12,  8, -1, B,  P_PICK_AXE, IRON, HI_METAL,
                                                        DWARVISH_MATTOCK),
WEAPON("пика", NoDes,
       1, 0, 0,  4, 180,  10,  6,  8, 0, P,   P_LANCE, IRON, HI_METAL,
                                                        LANCE),
        /* +2d10 when jousting with lance as primary weapon,
           +2d2 when jousting with it as secondary when dual wielding */

/* bludgeons */
WEAPON("булава", NoDes,
       1, 0, 0, 40,  30,   5,  6,  6, 0, B,   P_MACE, IRON, HI_METAL,
                                                        MACE),
        /* +1 small */
WEAPON("серебряная булава", NoDes,
       1, 0, 0,  2,  36,  60,  6,  6, 0, B,   P_MACE, SILVER, HI_SILVER,
                                                        SILVER_MACE),
        /* +1 small */
WEAPON("моргенштерн", NoDes,
       1, 0, 0, 12, 120,  10,  4,  6, 0, B,   P_MORNING_STAR, IRON, HI_METAL,
                                                        MORNING_STAR),
        /* +d4 small, +1 large */
WEAPON("боевой молот", NoDes,
       1, 0, 0, 15,  50,   5,  4,  4, 0, B,   P_HAMMER, IRON, HI_METAL,
                                                        WAR_HAMMER),
        /* +1 small */
WEAPON("дубина", NoDes,
       1, 0, 0, 12,  30,   3,  6,  3, 0, B,   P_CLUB, WOOD, HI_WOOD,
                                                        CLUB),
WEAPON("резиновый шланг", NoDes,
       1, 0, 0,  0,  20,   3,  4,  3, 0, B,   P_WHIP, PLASTIC, CLR_BROWN,
                                                        RUBBER_HOSE),
WEAPON("посох", "посох",
       0, 0, 1, 11,  40,   5,  6,  6, 0, B,   P_QUARTERSTAFF, WOOD, HI_WOOD,
                                                        QUARTERSTAFF),
/* two-piece */
WEAPON("аклис", "дубина на ремне",
       0, 0, 0,  8,  15,   4,  6,  3, 0, B,   P_CLUB, IRON, HI_METAL,
                                                        AKLYS),
WEAPON("цеп", NoDes,
       1, 0, 0, 40,  15,   4,  6,  4, 0, B,   P_FLAIL, IRON, HI_METAL,
                                                        FLAIL),
        /* +1 small, +1d4 large */

/* misc */
WEAPON("бычий бич", NoDes,
       1, 0, 0,  2,  20,   4,  2,  1, 0, 0,   P_WHIP, LEATHER, CLR_BROWN,
                                                        BULLWHIP),

/* bows */
BOW("лук", NoDes,               1, 24, 30, 60, 0, WOOD, P_BOW, HI_WOOD,
                                                        BOW),
BOW("эльфийский лук", "рунический лук",  0, 12, 30, 60, 0, WOOD, P_BOW, HI_WOOD,
                                                        ELVEN_BOW),
BOW("оркский лук", "грубый лук", 0, 12, 30, 60, 0, WOOD, P_BOW, CLR_BLACK,
                                                        ORCISH_BOW),
BOW("юми", "длинный лук",        0,  0, 30, 60, 0, WOOD, P_BOW, HI_WOOD,
                                                        YUMI),
BOW("праща", NoDes,             1, 40,  3, 20, 0, LEATHER, P_SLING, HI_LEATHER,
                                                        SLING),
BOW("арбалет", NoDes,          1, 45, 50, 40, 0, WOOD, P_CROSSBOW, HI_WOOD,
                                                        CROSSBOW),

#undef P
#undef S
#undef B

#undef WEAPON
#undef PROJECTILE
#undef BOW

/* armor ... */
        /* IRON denotes ferrous metals, including steel.
         * Only IRON weapons and armor can rust.
         * Only COPPER (including brass) corrodes.
         * Some creatures are vulnerable to SILVER.
         */
#define ARMOR(name,desc,kn,mgc,blk,power,prob,delay,wt,  \
              cost,ac,can,sub,metal,c,sn)                   \
    OBJECT(OBJ(name, desc),                                         \
           BITS(kn, 0, 1, 0, mgc, 1, 0, 0, blk, 0, 0, sub, metal),  \
           power, ARMOR_CLASS, prob, delay, wt,                     \
           cost, 0, 0, 10 - ac, can, wt, c, sn)
#define HELM(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c,sn)  \
    ARMOR(name, desc, kn, mgc, 0, power, prob, delay, wt,  \
          cost, ac, can, ARM_HELM, metal, c, sn)
#define CLOAK(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c,sn)  \
    ARMOR(name, desc, kn, mgc, 0, power, prob, delay, wt,  \
          cost, ac, can, ARM_CLOAK, metal, c,sn)
#define SHIELD(name,desc,kn,mgc,blk,pow,prob,delay,wt,cost,ac,can,metal,c,sn) \
    ARMOR(name, desc, kn, mgc, blk, pow, prob, delay, wt, \
          cost, ac, can, ARM_SHIELD, metal, c,sn)
#define GLOVES(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c,sn)  \
    ARMOR(name, desc, kn, mgc, 0, power, prob, delay, wt,  \
          cost, ac, can, ARM_GLOVES, metal, c,sn)
#define BOOTS(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c,sn)  \
    ARMOR(name, desc, kn, mgc, 0, power, prob, delay, wt,  \
          cost, ac, can, ARM_BOOTS, metal, c,sn)

/* helmets */
HELM("эльфийский кожаный шлем", "кожаная шляпа",
     0, 0,           0,  6, 1,  3,  8,  9, 0, LEATHER, HI_LEATHER,
                                                        ELVEN_LEATHER_HELM),
HELM("оркский шлем", "железная шапочка-череп",
     0, 0,           0,  6, 1, 30, 10,  9, 0, IRON, CLR_BLACK,
                                                        ORCISH_HELM),
HELM("дварфийский железный шлем", "каска",
     0, 0,           0,  6, 1, 40, 20,  8, 0, IRON, HI_METAL,
                                                        DWARVISH_IRON_HELM),
HELM("федора", NoDes,
     1, 0,           0,  0, 0,  3,  1, 10, 0, CLOTH, CLR_BROWN,
                                                        FEDORA),
HELM("корнуфаум", "коническая шляпа",
     0, 1, CLAIRVOYANT,  5, 1,  4, 80, 10, 1, CLOTH, CLR_BLUE,
        /* name coined by devteam; confers clairvoyance for wizards,
           blocks clairvoyance if worn by role other than wizard */
                                                        CORNUTHAUM),
HELM("колпак дурака", "коническая шляпа",
     0, 1,           0,  5, 1,  4,  1, 10, 0, CLOTH, CLR_BLUE,
        /* sets Int and Wis to fixed value of 6, so actually provides
           protection against death caused by Int being drained below 3 */
                                                        DUNCE_CAP),
HELM("помятый горшок", NoDes,
     1, 0,           0,  2, 0, 10,  8,  9, 0, IRON, CLR_BLACK,
                                                        DENTED_POT),
HELM("шлем блеска", "хрустальный шлем",
     0, 1,           0,  6, 1, 40, 50,  9, 0, GLASS, CLR_WHITE,
        /* used to be iron and shuffled as "гравированный шлем" but required
           special case for the effect of iron armor on spell casting */
                                                        HELM_OF_BRILLIANCE),
/* with shuffled appearances... */
HELM("шлем", "шлем с плюмажем",
     0, 0,           0, 10, 1, 30, 10,  9, 0, IRON, HI_METAL,
                                                        HELMET),
HELM("шлем осторожности", "гравированный шлем",
     0, 1,     WARNING,  6, 1, 50, 50,  9, 0, IRON, CLR_GREEN,
                                                        HELM_OF_CAUTION),
HELM("шлем противоположной ориентации", "шлем с гребнем",
     0, 1,           0, 10, 1, 50, 50,  9, 0, IRON, HI_METAL,
                                                 HELM_OF_OPPOSITE_ALIGNMENT),
HELM("шлем телепатии", "шлем с забралом",
     0, 1,     TELEPAT,  4, 1, 50, 50,  9, 0, IRON, HI_METAL,
                                                 HELM_OF_TELEPATHY),

/* suits of armor */
/*
 * There is code in polyself.c that assumes (1) and (2).
 * There is code in obj.h, objnam.c, mon.c, read.c that assumes (2).
 *      (1) The dragon scale mails and the dragon scales are together.
 *      (2) That the order of the dragon scale mail and dragon scales
 *          is the same as order of dragons defined in monst.c.
 */
#define DRGN_ARMR(name,mgc,power,cost,ac,color,snam)  \
    ARMOR(name, NoDes, 1, mgc, 1, power, 0, 5, 40,  \
          cost, ac, 0, ARM_SUIT, DRAGON_HIDE, color,snam)
/* 3.4.1: dragon scale mail reclassified as "magic" since magic is
   needed to create them */
DRGN_ARMR("серая кольчуга из чешуи дракона",    1, ANTIMAGIC,  1200, 1, CLR_GRAY,
                                                    GRAY_DRAGON_SCALE_MAIL),
    /* gold DSM is a light source; there's no property for that */
DRGN_ARMR("золотая кольчуга из чешуи дракона",    1, 0,           900, 1, HI_GOLD,
                                                    GOLD_DRAGON_SCALE_MAIL),
DRGN_ARMR("серебряная кольчуга из чешуи дракона",  1, REFLECTING, 1200, 1, DRAGON_SILVER,
                                                    SILVER_DRAGON_SCALE_MAIL),
#if 0 /* DEFERRED */
DRGN_ARMR("мерцающая кольчуга из чешуи дракона", 1, DISPLACED, 1200, 1, CLR_CYAN,
                                                SHIMMERING_DRAGON_SCALE_MAIL),
#endif
DRGN_ARMR("красная кольчуга из чешуи дракона",     1, FIRE_RES,    900, 1, CLR_RED,
                                                    RED_DRAGON_SCALE_MAIL),
DRGN_ARMR("белая кольчуга из чешуи дракона",   1, COLD_RES,    900, 1, CLR_WHITE,
                                                    WHITE_DRAGON_SCALE_MAIL),
DRGN_ARMR("оранжевая кольчуга из чешуи дракона",  1, SLEEP_RES,   900, 1, CLR_ORANGE,
                                                    ORANGE_DRAGON_SCALE_MAIL),
DRGN_ARMR("чёрная кольчуга из чешуи дракона",   1, DISINT_RES, 1200, 1, CLR_BLACK,
                                                    BLACK_DRAGON_SCALE_MAIL),
DRGN_ARMR("синяя кольчуга из чешуи дракона",    1, SHOCK_RES,   900, 1, CLR_BLUE,
                                                    BLUE_DRAGON_SCALE_MAIL),
DRGN_ARMR("зелёная кольчуга из чешуи дракона",   1, POISON_RES,  900, 1, CLR_GREEN,
                                                    GREEN_DRAGON_SCALE_MAIL),
DRGN_ARMR("жёлтая кольчуга из чешуи дракона",  1, ACID_RES,    900, 1, CLR_YELLOW,
                                                    YELLOW_DRAGON_SCALE_MAIL),
/* For now, only dragons leave these. */
/* 3.4.1: dragon scales left classified as "non-magic"; they confer magical
   properties but are produced "naturally"; affects use as polypile fodder */
DRGN_ARMR("серая чешуя дракона",        0, ANTIMAGIC,   700, 7, CLR_GRAY,
                                                        GRAY_DRAGON_SCALES),
DRGN_ARMR("золотая чешуя дракона",        0, 0,           500, 7, HI_GOLD,
                                                        GOLD_DRAGON_SCALES),
DRGN_ARMR("серебряная чешуя дракона",      0, REFLECTING,  700, 7, DRAGON_SILVER,
                                                        SILVER_DRAGON_SCALES),
#if 0 /* DEFERRED */
DRGN_ARMR("мерцающая чешуя дракона",  0, DISPLACED,   700, 7, CLR_CYAN,
                                                    SHIMMERING_DRAGON_SCALES),
#endif
DRGN_ARMR("красная чешуя дракона",         0, FIRE_RES,    500, 7, CLR_RED,
                                                        RED_DRAGON_SCALES),
DRGN_ARMR("белая чешуя дракона",       0, COLD_RES,    500, 7, CLR_WHITE,
                                                        WHITE_DRAGON_SCALES),
DRGN_ARMR("оранжевая чешуя дракона",      0, SLEEP_RES,   500, 7, CLR_ORANGE,
                                                        ORANGE_DRAGON_SCALES),
DRGN_ARMR("чёрная чешуя дракона",       0, DISINT_RES,  700, 7, CLR_BLACK,
                                                        BLACK_DRAGON_SCALES),
DRGN_ARMR("синяя чешуя дракона",        0, SHOCK_RES,   500, 7, CLR_BLUE,
                                                        BLUE_DRAGON_SCALES),
DRGN_ARMR("зелёная чешуя дракона",       0, POISON_RES,  500, 7, CLR_GREEN,
                                                        GREEN_DRAGON_SCALES),
DRGN_ARMR("жёлтая чешуя дракона",      0, ACID_RES,    500, 7, CLR_YELLOW,
                                                        YELLOW_DRAGON_SCALES),
#undef DRGN_ARMR
/* other suits */
ARMOR("латный доспех", NoDes,
      1, 0, 1,  0, 40, 5, 450, 600,  3, 2,  ARM_SUIT, IRON, HI_METAL,
                                                        PLATE_MAIL),
ARMOR("хрустальный латный доспех", NoDes,
      1, 0, 1,  0, 10, 5, 415, 820,  3, 2,  ARM_SUIT, GLASS, CLR_WHITE,
                                                        CRYSTAL_PLATE_MAIL),
ARMOR("бронзовый латный доспех", NoDes,
      1, 0, 1,  0, 23, 5, 450, 400,  4, 1,  ARM_SUIT, COPPER, HI_COPPER,
                                                        BRONZE_PLATE_MAIL),
ARMOR("доспех из пластин", NoDes,
      1, 0, 1,  0, 57, 5, 400,  80,  4, 1,  ARM_SUIT, IRON, HI_METAL,
                                                        SPLINT_MAIL),
ARMOR("полосатый доспех", NoDes,
      1, 0, 1,  0, 66, 5, 350,  90,  4, 1,  ARM_SUIT, IRON, HI_METAL,
                                                        BANDED_MAIL),
ARMOR("дварфийский мифриловый доспех", NoDes,
      1, 0, 0,  0, 10, 1, 150, 240,  4, 2,  ARM_SUIT, MITHRIL, HI_SILVER,
                                                        DWARVISH_MITHRIL_COAT),
ARMOR("эльфийский мифриловый доспех", NoDes,
      1, 0, 0,  0, 15, 1, 150, 240,  5, 2,  ARM_SUIT, MITHRIL, HI_SILVER,
                                                        ELVEN_MITHRIL_COAT),
ARMOR("кольчуга", NoDes,
      1, 0, 0,  0, 66, 5, 300,  75,  5, 1,  ARM_SUIT, IRON, HI_METAL,
                                                        CHAIN_MAIL),
ARMOR("оркская кольчуга", "грубая кольчуга",
      0, 0, 0,  0, 19, 5, 300,  75,  6, 1,  ARM_SUIT, IRON, CLR_BLACK,
                                                        ORCISH_CHAIN_MAIL),
ARMOR("чешуйчатый доспех", NoDes,
      1, 0, 0,  0, 66, 5, 250,  45,  6, 1,  ARM_SUIT, IRON, HI_METAL,
                                                        SCALE_MAIL),
ARMOR("кожаный доспех с шипами", NoDes,
      1, 0, 0,  0, 66, 3, 200,  15,  7, 1,  ARM_SUIT, LEATHER, HI_LEATHER,
                                                        STUDDED_LEATHER_ARMOR),
ARMOR("кольчатый доспех", NoDes,
      1, 0, 0,  0, 66, 5, 250, 100,  7, 1,  ARM_SUIT, IRON, HI_METAL,
                                                        RING_MAIL),
ARMOR("оркский кольчатый доспех", "грубый кольчатый доспех",
      0, 0, 0,  0, 19, 5, 250,  80,  8, 1,  ARM_SUIT, IRON, CLR_BLACK,
                                                        ORCISH_RING_MAIL),
ARMOR("кожаный доспех", NoDes,
      1, 0, 0,  0, 75, 3, 150,   5,  8, 1,  ARM_SUIT, LEATHER, HI_LEATHER,
                                                        LEATHER_ARMOR),
ARMOR("кожаная куртка", NoDes,
      1, 0, 0,  0, 11, 0,  30,  10,  9, 0,  ARM_SUIT, LEATHER, CLR_BLACK,
                                                        LEATHER_JACKET),

/* shirts */
ARMOR("гавайская рубашка", NoDes,
      1, 0, 0,  0,  8, 0,   5,   3, 10, 0,  ARM_SHIRT, CLOTH, CLR_MAGENTA,
                                                        HAWAIIAN_SHIRT),
ARMOR("футболка", NoDes,
      1, 0, 0,  0,  2, 0,   5,   2, 10, 0,  ARM_SHIRT, CLOTH, CLR_WHITE,
                                                        T_SHIRT),

/* cloaks */
CLOAK("обмотка мумии", NoDes,
      1, 0,          0,  0, 0,  3,  2, 10, 1,  CLOTH, CLR_GRAY,
                                                        MUMMY_WRAPPING),
        /* worn mummy wrapping blocks invisibility */
CLOAK("эльфийский плащ", "выцветшая мантия",
      0, 1,    STEALTH,  8, 0, 10, 60,  9, 1,  CLOTH, CLR_BLACK, ELVEN_CLOAK),
CLOAK("оркский плащ", "грубая мантилья",
      0, 0,          0,  8, 0, 10, 40, 10, 1,  CLOTH, CLR_BLACK,
                                                        ORCISH_CLOAK),
CLOAK("дварфийский плащ", "плащ с капюшоном",
      0, 0,          0,  8, 0, 10, 50, 10, 1,  CLOTH, HI_CLOTH,
                                                        DWARVISH_CLOAK),
CLOAK("прорезиненный плащ", "скользкий плащ",
      0, 0,          0,  8, 0, 10, 50,  9, 2,  CLOTH, HI_CLOTH,
                                                        OILSKIN_CLOAK),
CLOAK("мантия", NoDes,
      1, 1,          0,  6, 0, 15, 50,  8, 2,  CLOTH, CLR_RED, ROBE),
        /* robe was adopted from slash'em, where it's worn as a suit
           rather than as a cloak and there are several variations */
CLOAK("алхимический халат", "фартук",
      0, 1, POISON_RES, 11, 0, 10, 50,  9, 1,  CLOTH, CLR_WHITE,
                                                        ALCHEMY_SMOCK),
CLOAK("кожаный плащ", NoDes,
      1, 0,          0,  8, 0, 15, 40,  9, 1,  LEATHER, CLR_BROWN,
                                                        LEATHER_CLOAK),
/* with shuffled appearances... */
CLOAK("плащ защиты", "рваный плащ",
      0, 1, PROTECTION, 11, 0, 10, 50,  7, 3,  CLOTH, HI_CLOTH,
                                                        CLOAK_OF_PROTECTION),
        /* cloak of protection is now the only item conferring MC 3 */
CLOAK("плащ невидимости", "оперный плащ",
      0, 1,      INVIS, 12, 0, 10, 60,  9, 1,  CLOTH, CLR_BRIGHT_MAGENTA,
                                                        CLOAK_OF_INVISIBILITY),
CLOAK("плащ магической защиты", "узорчатая накидка",
      0, 1,  ANTIMAGIC,  6, 0, 10, 60,  9, 1,  CLOTH, CLR_WHITE,
                                                   CLOAK_OF_MAGIC_RESISTANCE),
        /*  'cope' is not a spelling mistake... leave it be */
CLOAK("плащ смещения", "кусок ткани",
      0, 1,  DISPLACED, 12, 0, 10, 50,  9, 1,  CLOTH, HI_CLOTH,
                                                        CLOAK_OF_DISPLACEMENT),

/* shields */
SHIELD("малый щит", "деревянный щит",
       0, 0, 0,          0,  6, 0,  30,  3, 9, 0,  WOOD, HI_WOOD,
                                                        SMALL_SHIELD),
SHIELD("щит сопротивления истощению", "деревянный щит",
       0, 1, 0,  DRAIN_RES, 12, 0,  30, 50, 9, 0,  WOOD, HI_WOOD,
                                                  SHIELD_OF_DRAIN_RESISTANCE),
SHIELD("щит сопротивления электричеству", "деревянный щит",
       0, 1, 0,  SHOCK_RES, 12, 0,  30, 50, 9, 0,  WOOD, HI_WOOD,
                                                  SHIELD_OF_SHOCK_RESISTANCE),
SHIELD("эльфийский щит", "сине-зелёный щит",
       0, 0, 0,          0,  2, 0,  40,  7, 8, 0,  WOOD, CLR_GREEN,
                                                        ELVEN_SHIELD),
SHIELD("щит урук-хай", "щит с белой рукой",
       0, 0, 0,          0,  2, 0,  50,  7, 9, 0,  IRON, HI_METAL,
                                                        URUK_HAI_SHIELD),
SHIELD("оркский щит", "щит с красным глазом",
       0, 0, 0,          0,  2, 0,  50,  7, 9, 0,  IRON, CLR_RED,
                                                        ORCISH_SHIELD),
SHIELD("большой щит", NoDes,
       1, 0, 1,          0,  4, 0, 100, 10, 8, 0,  IRON, HI_METAL,
                                                        LARGE_SHIELD),
SHIELD("дварфийский круглый щит", "большой круглый щит",
       0, 0, 0,          0,  3, 0, 100, 10, 8, 0,  IRON, HI_METAL,
                                                        DWARVISH_ROUNDSHIELD),
SHIELD("щит отражения", "полированный серебряный щит",
       0, 1, 0, REFLECTING,  7, 0,  50, 50, 8, 0,  SILVER, HI_SILVER,
                                                        SHIELD_OF_REFLECTION),

/* gloves */
/* These have their color but not material shuffled, so the IRON must
 * stay CLR_BROWN (== HI_LEATHER) even though it's normally either
 * HI_METAL or CLR_BLACK.  All have shuffled descriptions.
 */
GLOVES("кожаные перчатки", "старые перчатки",
       0, 0,        0, 15, 1, 10,  8, 9, 0,  LEATHER, HI_LEATHER,
                                                        LEATHER_GLOVES),
GLOVES("рукавицы неуклюжести", "стеганые перчатки",
       0, 1, FUMBLING,  8, 1, 10, 50, 9, 0,  LEATHER, HI_LEATHER,
                                                    GAUNTLETS_OF_FUMBLING),
GLOVES("рукавицы силы", "перчатки для верховой езды",
       0, 1,        0,  8, 1, 30, 50, 9, 0,  IRON, CLR_BROWN,
                                                    GAUNTLETS_OF_POWER),
GLOVES("рукавицы ловкости", "фехтовальные перчатки",
       0, 1,        0,  8, 1, 10, 50, 9, 0,  LEATHER, HI_LEATHER,
                                                    GAUNTLETS_OF_DEXTERITY),

/* boots */
BOOTS("низкие ботинки", "ботинки для ходьбы",
      0, 0,          0, 23, 2, 10,  8, 9, 0, LEATHER, HI_LEATHER, LOW_BOOTS),
BOOTS("железные ботинки", "твёрдые ботинки",
      0, 0,          0,  7, 2, 50, 16, 8, 0, IRON, HI_METAL, IRON_SHOES),
BOOTS("высокие ботинки", "ботинки-бутсы",
      0, 0,          0, 14, 2, 20, 12, 8, 0, LEATHER, HI_LEATHER, HIGH_BOOTS),
/* with shuffled appearances... */
BOOTS("ботинки скорости", "боевые ботинки",
      0, 1,       FAST, 12, 2, 20, 50, 9, 0, LEATHER, HI_LEATHER, SPEED_BOOTS),
BOOTS("ботинки хождения по воде", "тропические ботинки",
      0, 1,   WWALKING, 12, 2, 15, 50, 9, 0, LEATHER, HI_LEATHER,
                                                        WATER_WALKING_BOOTS),
BOOTS("прыгательные ботинки", "походные ботинки",
      0, 1,    JUMPING, 12, 2, 20, 50, 9, 0, LEATHER, HI_LEATHER,
                                                        JUMPING_BOOTS),
BOOTS("эльфийские ботинки", "ботинки для грязи",
      0, 1,    STEALTH, 12, 2, 15,  8, 9, 0, LEATHER, HI_LEATHER,
                                                        ELVEN_BOOTS),
BOOTS("ботинки для пинков", "ботинки с пряжками",
      0, 1,          0, 12, 2, 50,  8, 9, 0, IRON, CLR_BROWN,
                                                        KICKING_BOOTS),
        /* CLR_BROWN for same reason as gauntlets of power */
BOOTS("неуклюжие ботинки", "ботинки для верховой езды",
      0, 1,   FUMBLING, 12, 2, 20, 30, 9, 0, LEATHER, HI_LEATHER,
                                                        FUMBLE_BOOTS),
BOOTS("ботинки левитации", "снежные ботинки",
      0, 1, LEVITATION, 12, 2, 15, 30, 9, 0, LEATHER, HI_LEATHER,
                                                        LEVITATION_BOOTS),
#undef HELM
#undef CLOAK
#undef SHIELD
#undef GLOVES
#undef BOOTS
#undef ARMOR

/* rings ... */
#define RING(name,stone,power,cost,mgc,spec,mohs,metal,color,sn) \
    OBJECT(OBJ(name, stone),                                          \
           BITS(0, 0, spec, 0, mgc, spec, 0, 0, 0,                    \
                HARDGEM(mohs), 0, P_NONE, metal),                     \
           power, RING_CLASS, 1, 0, 3, cost, 0, 0, 0, 0, 15, color,sn)
RING("украшение", "деревянный",
     ADORNED,                  100, 1, 1, 2, WOOD, HI_WOOD, RIN_ADORNMENT),
RING("увеличение силы", "гранитный",
     0,                        150, 1, 1, 7, MINERAL, HI_MINERAL,
                                                            RIN_GAIN_STRENGTH),
RING("увеличение конституции", "опаловый",
     0,                        150, 1, 1, 7, MINERAL, HI_MINERAL,
                                                        RIN_GAIN_CONSTITUTION),
RING("повышение точности", "глиняный",
     0,                        150, 1, 1, 4, MINERAL, CLR_RED,
                                                        RIN_INCREASE_ACCURACY),
RING("повышение урона", "коралловый",
     0,                        150, 1, 1, 4, MINERAL, CLR_ORANGE,
                                                        RIN_INCREASE_DAMAGE),
RING("защита", "чёрный оникс",
     PROTECTION,               100, 1, 1, 7, MINERAL, CLR_BLACK,
                                                        RIN_PROTECTION),
        /* 'PROTECTION' intrinsic enhances MC from worn armor by +1,
           regardless of ring's enchantment; wearing a second ring of
           protection (or even one ring of protection combined with
           cloak of protection) doesn't give a second MC boost */
RING("регенерация", "лунный камень",
     REGENERATION,             200, 1, 0,  6, MINERAL, HI_MINERAL,
                                                        RIN_REGENERATION),
RING("поиск", "тигровый глаз",
     SEARCHING,                200, 1, 0,  6, GEMSTONE, CLR_BROWN,
                                                        RIN_SEARCHING  ),
RING("скрытность", "нефритовый",
     STEALTH,                  100, 1, 0,  6, GEMSTONE, CLR_GREEN,
                                                        RIN_STEALTH),
RING("поддержание способности", "бронзовый",
     FIXED_ABIL,               100, 1, 0,  4, COPPER, HI_COPPER,
                                                        RIN_SUSTAIN_ABILITY),
RING("левитация", "агатовый",
     LEVITATION,               200, 1, 0,  7, GEMSTONE, CLR_RED,
                                                        RIN_LEVITATION),
RING("голод", "топазовый",
     HUNGER,                   100, 1, 0,  8, GEMSTONE, CLR_CYAN,
                                                        RIN_HUNGER),
RING("раздражение монстров", "сапфировый",
     AGGRAVATE_MONSTER,        150, 1, 0,  9, GEMSTONE, CLR_BLUE,
                                                        RIN_AGGRAVATE_MONSTER),
RING("конфликт", "рубиновый",
     CONFLICT,                 300, 1, 0,  9, GEMSTONE, CLR_RED,
                                                        RIN_CONFLICT),
RING("предупреждение", "алмазный",
     WARNING,                  100, 1, 0, 10, GEMSTONE, CLR_WHITE,
                                                        RIN_WARNING),
RING("сопротивление яду", "жемчужный",
     POISON_RES,               150, 1, 0,  4, BONE, CLR_WHITE,
                                                        RIN_POISON_RESISTANCE),
RING("сопротивление огню", "железный",
     FIRE_RES,                 200, 1, 0,  5, IRON, HI_METAL,
                                                        RIN_FIRE_RESISTANCE),
RING("сопротивление холоду", "латунный",
     COLD_RES,                 150, 1, 0,  4, COPPER, HI_COPPER,
                                                        RIN_COLD_RESISTANCE),
RING("сопротивление электричеству", "медный",
     SHOCK_RES,                150, 1, 0,  3, COPPER, HI_COPPER,
                                                        RIN_SHOCK_RESISTANCE),
RING("свобода действий", "витой",
     FREE_ACTION,              200, 1, 0,  6, IRON, HI_METAL,
                                                        RIN_FREE_ACTION),
RING("замедленное пищеварение", "стальной",
     SLOW_DIGESTION,           200, 1, 0,  8, IRON, HI_METAL,
                                                        RIN_SLOW_DIGESTION),
RING("телепортация", "серебряный",
     TELEPORT,                 200, 1, 0,  3, SILVER, HI_SILVER,
                                                        RIN_TELEPORTATION),
RING("контроль телепортации", "золотой",
     TELEPORT_CONTROL,         300, 1, 0,  3, GOLD, HI_GOLD,
                                                        RIN_TELEPORT_CONTROL),
RING("превращение", "слоновая кость",
     POLYMORPH,                300, 1, 0,  4, BONE, CLR_WHITE,
                                                        RIN_POLYMORPH),
RING("контроль превращения", "изумрудный",
     POLYMORPH_CONTROL,        300, 1, 0,  8, GEMSTONE, CLR_BRIGHT_GREEN,
                                                        RIN_POLYMORPH_CONTROL),
RING("невидимость", "проволочный",
     INVIS,                    150, 1, 0,  5, IRON, HI_METAL,
                                                        RIN_INVISIBILITY),
RING("видение невидимого", "обручальный",
     SEE_INVIS,                150, 1, 0,  5, IRON, HI_METAL,
                                                        RIN_SEE_INVISIBLE),
RING("защита от принимающих облик", "блестящий",
     PROT_FROM_SHAPE_CHANGERS, 100, 1, 0,  5, IRON, CLR_BRIGHT_CYAN,
                                               RIN_PROTECTION_FROM_SHAPE_CHAN),
#undef RING

/* amulets ... - THE Amulet comes last because it is special */
#define AMULET(name,desc,power,prob,sn) \
    OBJECT(OBJ(name, desc),                                            \
           BITS(0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, P_NONE, IRON),        \
           power, AMULET_CLASS, prob, 0, 20, 150, 0, 0, 0, 0, 20, HI_METAL, sn)
AMULET("амулет экстрасенсорного восприятия",                "круглый", TELEPAT, 120,
                                                        AMULET_OF_ESP),
MARKER(FIRST_AMULET, AMULET_OF_ESP)
AMULET("амулет спасения",       "сферический", LIFESAVED, 75,
                                                        AMULET_OF_LIFE_SAVING),
AMULET("амулет удушения",          "овальный", STRANGLED, 115,
                                                      AMULET_OF_STRANGULATION),
AMULET("амулет спокойного сна",    "треугольный", SLEEPY, 115,
                                                      AMULET_OF_RESTFUL_SLEEP),
AMULET("амулет против яда",        "пирамидальный", POISON_RES, 115,
                                                        AMULET_VERSUS_POISON),
AMULET("амулет изменения",               "квадратный", 0, 115,
                                                        AMULET_OF_CHANGE),
AMULET("амулет неизменности",          "вогнутый", UNCHANGING, 60,
                                                        AMULET_OF_UNCHANGING),
AMULET("амулет отражения",        "шестиугольный", REFLECTING, 75,
                                                        AMULET_OF_REFLECTION),
AMULET("амулет магического дыхания", "восьмиугольный", MAGICAL_BREATHING, 75,
                                                  AMULET_OF_MAGICAL_BREATHING),
        /* +2 AC and +2 MC; +2 takes naked hero past 'warded' to 'guarded' */
AMULET("амулет защиты",         "перфорированный", PROTECTION, 75,
                                                        AMULET_OF_GUARDING),
        /* cubical: some descriptions are already three dimensional and
           parallelogrammatical (real word!) would be way over the top */
AMULET("амулет полёта",              "кубический", FLYING, 60,
                                                        AMULET_OF_FLYING),
/* fixed descriptions; description duplication is deliberate;
 * fake one must come before real one because selection for
 * description shuffling stops when a non-magic amulet is encountered
 */
OBJECT(OBJ("дешёвая пластиковая имитация Амулета Йендора",
           "Амулет Йендора"),
       BITS(0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, PLASTIC),
       0, AMULET_CLASS, 0, 0, 20, 0, 0, 0, 0, 0, 1, HI_METAL,
                                                FAKE_AMULET_OF_YENDOR),
OBJECT(OBJ("Амулет Йендора", /* note: description == name */
           "Амулет Йендора"),
       BITS(0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, MITHRIL),
       0, AMULET_CLASS, 0, 0, 20, 30000, 0, 0, 0, 0, 20, HI_METAL,
                                                AMULET_OF_YENDOR),
MARKER(LAST_AMULET, AMULET_OF_YENDOR)
#undef AMULET

/* tools ... */
/* tools with weapon characteristics come last */
#define TOOL(name,desc,kn,mrg,mgc,chg,prob,wt,cost,mat,color,sn) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(kn, mrg, chg, 0, mgc, chg, 0, 0, 0, 0, 0, P_NONE, mat), \
           0, TOOL_CLASS, prob, 0, wt, cost, 0, 0, 0, 0, wt, color, sn)
#define CONTAINER(name,desc,kn,mgc,chg,prob,wt,cost,mat,color,sn) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(kn, 0, chg, 1, mgc, chg, 0, 0, 0, 0, 0, P_NONE, mat),   \
           0, TOOL_CLASS, prob, 0, wt, cost, 0, 0, 0, 0, wt, color, sn)
#define EYEWEAR(name,desc,kn,prop,prob,wt,cost,mat,color,sn) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(kn, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, mat),         \
           prop, TOOL_CLASS, prob, 0, wt, cost, 0, 0, 0, 0, wt, color, sn)
#define WEPTOOL(name,desc,kn,mgc,bi,prob,wt,cost,sdam,ldam,hitbon,sub, \
                mat,clr,sn)                                             \
    OBJECT(OBJ(name, desc),                                             \
           BITS(kn, 0, 1, 0, mgc, 1, 0, 0, bi, 0, hitbon, sub, mat),    \
           0, TOOL_CLASS, prob, 0, wt, cost, sdam, ldam, hitbon, 0, wt, \
           clr, sn)
/* containers */
CONTAINER("большой ящик",       NoDes, 1, 0, 0, 40, 350,   8, WOOD, HI_WOOD,
                                                                LARGE_BOX),
CONTAINER("сундук",           NoDes, 1, 0, 0, 35, 600,  16, WOOD, HI_WOOD,
                                                                CHEST),
CONTAINER("ледник",         NoDes, 1, 0, 0,  5, 900,  42, PLASTIC, CLR_WHITE,
                                                                ICE_BOX),
CONTAINER("мешок",           "сумка", 0, 0, 0, 35,  15,   2, CLOTH, HI_CLOTH,
                                                                SACK),
CONTAINER("прорезиненный мешок",   "сумка", 0, 0, 0,  5,  15, 100, CLOTH, HI_CLOTH,
                                                                OILSKIN_SACK),
CONTAINER("мешок вместительности", "сумка", 0, 1, 0, 20,  15, 100, CLOTH, HI_CLOTH,
                                                               BAG_OF_HOLDING),
CONTAINER("мешок трюков",  "сумка", 0, 1, 1, 20,  15, 100, CLOTH, HI_CLOTH,
                                                                BAG_OF_TRICKS),
#undef CONTAINER

/* lock opening tools */
TOOL("отмычка",       "ключ", 0, 0, 0, 0, 80,  3, 10, IRON, HI_METAL,
                                                                SKELETON_KEY),
TOOL("отмычка",           NoDes, 1, 0, 0, 0, 60,  4, 20, IRON, HI_METAL,
                                                                LOCK_PICK),
TOOL("кредитная карточка",         NoDes, 1, 0, 0, 0, 15,  1, 10, PLASTIC, CLR_WHITE,
                                                                CREDIT_CARD),
/* light sources */
TOOL("сальная свеча",   "свеча", 0, 1, 0, 0, 20,  2, 10, WAX, CLR_WHITE,
                                                                TALLOW_CANDLE),
TOOL("восковая свеча",      "свеча", 0, 1, 0, 0,  5,  2, 20, WAX, CLR_WHITE,
                                                                WAX_CANDLE),
TOOL("латунный фонарь",       NoDes, 1, 0, 0, 0, 30, 30, 12, COPPER, CLR_YELLOW,
                                                                BRASS_LANTERN),
TOOL("масляная лампа",          "лампа", 0, 0, 0, 0, 45, 20, 10, COPPER, CLR_YELLOW,
                                                                OIL_LAMP),
TOOL("волшебная лампа",        "лампа", 0, 0, 1, 0, 15, 20, 50, COPPER, CLR_YELLOW,
                                                                MAGIC_LAMP),
/* other tools */
TOOL("дорогая камера",    NoDes, 1, 0, 0, 1, 15, 12,200, PLASTIC, CLR_BLACK,
                                                            EXPENSIVE_CAMERA),
TOOL("зеркало",   "зеркальце", 0, 0, 0, 0, 45, 13, 10, GLASS, HI_SILVER,
                                                                MIRROR),
TOOL("хрустальный шар", "стеклянный шар", 0, 0, 1, 1, 15,150, 60, GLASS, HI_GLASS,
                                                                CRYSTAL_BALL),
/* eyewear - tools which can be worn on the face; (!mrg, !chg, !mgc)
   worn lenses don't confer the Blinded property, blindfolds and towels do;
   wet towel can be used as a weapon but is not a weptool and uses obj->spe
   differently from weapons and weptools */
EYEWEAR("линзы",           NoDes, 1,       0,  5,  3, 80, GLASS, HI_GLASS,
                                                                LENSES),
EYEWEAR("повязка на глаза",        NoDes, 1, BLINDED, 50,  2, 20, CLOTH, CLR_BLACK,
                                                                BLINDFOLD),
EYEWEAR("полотенце",            NoDes, 1, BLINDED, 50,  5, 50, CLOTH, CLR_MAGENTA,
                                                                TOWEL),
#undef EYEWEAR

/* still other tools */
TOOL("седло",              NoDes, 1, 0, 0, 0,  5,200,150, LEATHER, HI_LEATHER,
                                                                SADDLE),
TOOL("поводок",               NoDes, 1, 0, 0, 0, 65, 12, 20, LEATHER, HI_LEATHER,
                                                                LEASH),
TOOL("стетоскоп",         NoDes, 1, 0, 0, 0, 25,  4, 75, IRON, HI_METAL,
                                                                STETHOSCOPE),
TOOL("набор для консервирования",         NoDes, 1, 0, 0, 1, 15,100, 30, IRON, HI_METAL,
                                                                TINNING_KIT),
TOOL("открывалка для банок",          NoDes, 1, 0, 0, 0, 35,  4, 30, IRON, HI_METAL,
                                                                TIN_OPENER),
TOOL("банка смазки",       NoDes, 1, 0, 0, 1, 15, 15, 20, IRON, HI_METAL,
                                                                CAN_OF_GREASE),
TOOL("статуэтка",            NoDes, 1, 0, 1, 0, 25, 50, 80, MINERAL, HI_MINERAL,
                                                                FIGURINE),
        /* monster type specified by obj->corpsenm */
TOOL("волшебный маркер",        NoDes, 1, 0, 1, 1, 15,  2, 50, PLASTIC, CLR_RED,
                                                                MAGIC_MARKER),
/* traps */
TOOL("мина",           NoDes, 1, 0, 0, 0, 0, 200,180, IRON, CLR_RED,
                                                                LAND_MINE),
TOOL("капкан",            NoDes, 1, 0, 0, 0, 0, 200, 60, IRON, HI_METAL,
                                                                BEARTRAP),
/* instruments;
   "If tin whistles are made out of tin, what do they make foghorns out of?" */
TOOL("оловянный свисток",    "свисток", 0, 0, 0, 0,100, 3, 10, METAL, HI_METAL,
                                                                TIN_WHISTLE),
TOOL("волшебный свисток",  "свисток", 0, 0, 1, 0, 30, 3, 10, METAL, HI_METAL,
                                                                MAGIC_WHISTLE),
TOOL("деревянная флейта",     "флейта", 0, 0, 0, 0,  4, 5, 12, WOOD, HI_WOOD,
                                                                WOODEN_FLUTE),
TOOL("волшебная флейта",      "флейта", 0, 0, 1, 1,  2, 5, 36, WOOD, HI_WOOD,
                                                                MAGIC_FLUTE),
TOOL("резной рог",       "рог", 0, 0, 0, 0,  5, 18, 15, BONE, CLR_WHITE,
                                                                TOOLED_HORN),
TOOL("морозный рог",        "рог", 0, 0, 1, 1,  2, 18, 50, BONE, CLR_WHITE,
                                                                FROST_HORN),
TOOL("огненный рог",         "рог", 0, 0, 1, 1,  2, 18, 50, BONE, CLR_WHITE,
                                                                FIRE_HORN),
TOOL("рог изобилия",    "рог", 0, 0, 1, 1,  2, 18, 50, BONE, CLR_WHITE,
                                                            HORN_OF_PLENTY),
        /* horn, but not an instrument */
TOOL("деревянная арфа",       "арфа", 0, 0, 0, 0,  4, 30, 50, WOOD, HI_WOOD,
                                                                WOODEN_HARP),
TOOL("волшебная арфа",        "арфа", 0, 0, 1, 1,  2, 30, 50, WOOD, HI_WOOD,
                                                                MAGIC_HARP),
TOOL("колокольчик",                NoDes, 1, 0, 0, 0,  2, 30, 50, COPPER, HI_COPPER,
                                                                BELL),
TOOL("горн",               NoDes, 1, 0, 0, 0,  4, 10, 15, COPPER, HI_COPPER,
                                                                BUGLE),
TOOL("кожаный барабан",      "барабан", 0, 0, 0, 0,  4, 25, 25, LEATHER, HI_LEATHER,
                                                                LEATHER_DRUM),
TOOL("барабан землетрясения","барабан", 0, 0, 1, 1,  2, 25, 25, LEATHER, HI_LEATHER,
                                                          DRUM_OF_EARTHQUAKE),
/* tools useful as weapons */
WEPTOOL("кирка", NoDes,
        1, 0, 0, 20, 100,  50,  6,  3, WHACK,  P_PICK_AXE, IRON, HI_METAL,
                                                                PICK_AXE),
WEPTOOL("абордажный крюк", NoDes,
        1, 0, 0,  5,  30,  50,  2,  6, WHACK,  P_FLAIL,    IRON, HI_METAL,
                                                             GRAPPLING_HOOK),
WEPTOOL("рог единорога", NoDes,
        1, 1, 1,  0,  20, 100, 12, 12, PIERCE, P_UNICORN_HORN,
                                                           BONE, CLR_WHITE,
                                                                UNICORN_HORN),
        /* 3.4.1: unicorn horn left classified as "magic" */
/* two unique tools;
 * not artifacts, despite the comment which used to be here
 */
OBJECT(OBJ("Канделябр Призыва", "канделябр"),
       BITS(0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, P_NONE, GOLD),
       0, TOOL_CLASS, 0, 0, 10, 5000, 0, 0, 0, 0, 200, HI_GOLD,
                                                   CANDELABRUM_OF_INVOCATION),
OBJECT(OBJ("Колокол Открытия", "серебряный колокол"),
       BITS(0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, P_NONE, SILVER),
       0, TOOL_CLASS, 0, 0, 10, 5000, 0, 0, 0, 0, 50, HI_SILVER,
                                                   BELL_OF_OPENING),
#undef TOOL
#undef WEPTOOL

/* Comestibles ... */
#define FOOD(name, prob, delay, wt, unk, tin, nutrition, color, sn) \
    OBJECT(OBJ(name, NoDes),                                            \
           BITS(1, 1, unk, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, tin), 0,     \
           FOOD_CLASS, prob, delay, wt, nutrition / 20 + 5, 0, 0, 0, 0, \
           nutrition, color, sn)
/* All types of food (except tins & corpses) must have a delay of at least 1.
 * Delay on corpses is computed and is weight dependent.
 * Domestic pets prefer tripe rations above all others.
 * Fortune cookies can be read, using them up without ingesting them.
 * Carrots improve your vision.
 * +0 tins contain monster meat.
 * +1 tins (of spinach) make you stronger (like Popeye).
 * Meatballs/sticks/rings are only created from objects via stone to flesh.
 */
/* meat */
FOOD("рубец",        140,  2, 10, 0, FLESH, 200, CLR_BROWN,
                                                        TRIPE_RATION),
FOOD("труп",                0,  1,  0, 0, FLESH,   0, CLR_BROWN,
                                                        CORPSE),
FOOD("яйцо",                  85,  1,  1, 1, FLESH,  80, CLR_WHITE,
                                                        EGG),
FOOD("фрикаделька",              0,  1,  1, 0, FLESH,   5, CLR_BROWN,
                                                        MEATBALL),
FOOD("мясная палочка",            0,  1,  1, 0, FLESH,   5, CLR_BROWN,
                                                        MEAT_STICK),
/* formerly "huge chunk of meat" */
FOOD("огромная фрикаделька",     0, 20,400, 0, FLESH,2000, CLR_BROWN,
                                                        ENORMOUS_MEATBALL),
/* special case because it's not mergeable */
OBJECT(OBJ("мясное кольцо", NoDes),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FLESH),
       0, FOOD_CLASS, 0, 1, 5, 1, 0, 0, 0, 0, 5, CLR_BROWN, MEAT_RING),
/* pudding 'corpses' will turn into these and combine;
   must be in same order as the pudding monsters */
FOOD("комок серой слизи",     0,  2, 20, 0, FLESH,  20, CLR_GRAY,
                                                       GLOB_OF_GRAY_OOZE),
FOOD("комок коричневого пудинга", 0,  2, 20, 0, FLESH,  20, CLR_BROWN,
                                                       GLOB_OF_BROWN_PUDDING),
FOOD("комок зелёной слизи",   0,  2, 20, 0, FLESH,  20, CLR_GREEN,
                                                       GLOB_OF_GREEN_SLIME),
FOOD("комок чёрного пудинга", 0,  2, 20, 0, FLESH,  20, CLR_BLACK,
                                                       GLOB_OF_BLACK_PUDDING),

/* fruits & veggies */
FOOD("лопасть ламинарии",            0,  1,  1, 0, VEGGY,  30, CLR_GREEN, KELP_FROND),
FOOD("лист эвкалипта",       3,  1,  1, 0, VEGGY,   1, CLR_GREEN,
                                                          EUCALYPTUS_LEAF),
FOOD("яблоко",                15,  1,  2, 0, VEGGY,  50, CLR_RED, APPLE),
FOOD("апельсин",               10,  1,  2, 0, VEGGY,  80, CLR_ORANGE, ORANGE),
FOOD("груша",                 10,  1,  2, 0, VEGGY,  50, CLR_BRIGHT_GREEN,
                                                          PEAR),
FOOD("дыня",                10,  1,  5, 0, VEGGY, 100, CLR_BRIGHT_GREEN,
                                                          MELON),
FOOD("банан",               10,  1,  2, 0, VEGGY,  80, CLR_YELLOW, BANANA),
FOOD("морковь",               15,  1,  2, 0, VEGGY,  50, CLR_ORANGE, CARROT),
FOOD("веточка аконита",    7,  1,  1, 0, VEGGY,  40, CLR_GREEN,
                                                          SPRIG_OF_WOLFSBANE),
FOOD("зубчик чеснока",       7,  1,  1, 0, VEGGY,  40, CLR_WHITE,
                                                          CLOVE_OF_GARLIC),
/* name of slime mold is changed based on player's OPTION=fruit:something
   and bones data might have differently named ones from prior games */
FOOD("слизевой гриб",           75,  1,  5, 0, VEGGY, 250, HI_ORGANIC,
                                                          SLIME_MOLD),

/* people food */
FOOD("комок маточного молочка",   0,  1,  2, 0, VEGGY, 200, CLR_YELLOW,
                                                        LUMP_OF_ROYAL_JELLY),
FOOD("кремовый пирог",            25,  1, 10, 0, VEGGY, 100, CLR_WHITE, CREAM_PIE),
FOOD("шоколадный батончик",            13,  1,  2, 0, VEGGY, 100, CLR_BRIGHT_BLUE,
                                                                CANDY_BAR),
FOOD("печенье с предсказанием",       55,  1,  1, 0, VEGGY,  40, CLR_YELLOW,
                                                              FORTUNE_COOKIE),
FOOD("блин",              25,  2,  2, 0, VEGGY, 200, CLR_YELLOW, PANCAKE),
FOOD("лембас",         20,  2,  5, 0, VEGGY, 800, CLR_WHITE,
                                                                LEMBAS_WAFER),
FOOD("концентрированный паёк",          20,  3, 15, 0, VEGGY, 600, HI_ORGANIC,
                                                                CRAM_RATION),
FOOD("паёк",         380,  5, 20, 0, VEGGY, 800, HI_ORGANIC,
                                                                FOOD_RATION),
FOOD("паёк К",              0,  1, 10, 0, VEGGY, 400, HI_ORGANIC, K_RATION),
FOOD("паёк С",              0,  1, 10, 0, VEGGY, 300, HI_ORGANIC, C_RATION),
/* tins have type specified by obj->spe (+1 for spinach, other implies
   flesh; negative specifies preparation method {homemade,boiled,&c})
   and by obj->corpsenm (type of monster flesh) */
FOOD("банка",                  75,  0, 10, 1, METAL,   0, HI_METAL, TIN),
#undef FOOD

/* potions ... */
#define POTION(name,desc,mgc,power,prob,cost,color,sn) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(0, 1, 0, 0, mgc, 0, 0, 0, 0, 0, 0, P_NONE, GLASS),      \
           power, POTION_CLASS, prob, 0, 20, cost, 0, 0, 0, 0, 10, color, sn)
POTION("повышение способности",           "рубиновый",  1, 0, 40, 300, CLR_RED,
                                                        POT_GAIN_ABILITY),
POTION("восстановление способности",        "розовый",  1, 0, 40, 100, CLR_BRIGHT_MAGENTA,
                                                        POT_RESTORE_ABILITY),
POTION("замешательство",            "оранжевый",  1, CONFUSION, 40, 100, CLR_ORANGE,
                                                        POT_CONFUSION),
POTION("слепота",            "жёлтый",  1, BLINDED, 30, 150, CLR_YELLOW,
                                                        POT_BLINDNESS),
POTION("паралич",           "изумрудный",  1, 0, 40, 300, CLR_BRIGHT_GREEN,
                                                        POT_PARALYSIS),
POTION("скорость",            "тёмно-зелёный",  1, FAST, 40, 200, CLR_GREEN,
                                                        POT_SPEED),
POTION("левитация",             "голубой",  1, LEVITATION, 40, 200, CLR_CYAN,
                                                        POT_LEVITATION),
POTION("галлюцинация",      "небесно-голубой",  1, HALLUC, 30, 100, CLR_CYAN,
                                                        POT_HALLUCINATION),
POTION("невидимость", "ярко-синий",  1, INVIS, 40, 150, CLR_BRIGHT_BLUE,
                                                        POT_INVISIBILITY),
POTION("видение невидимого",       "пурпурный",  1, SEE_INVIS, 40, 50, CLR_MAGENTA,
                                                        POT_SEE_INVISIBLE),
POTION("лечение",          "пурпурно-красный",  1, 0, 115, 20, CLR_MAGENTA,
                                                        POT_HEALING),
POTION("усиленное лечение",          "бледно-коричневый",  1, 0, 45, 100, CLR_RED,
                                                        POT_EXTRA_HEALING),
POTION("получение уровня",            "молочный",  1, 0, 20, 300, CLR_WHITE,
                                                        POT_GAIN_LEVEL),
POTION("просветление",        "завихряющийся",  1, 0, 20, 200, CLR_BROWN,
                                                        POT_ENLIGHTENMENT),
POTION("обнаружение монстров",    "пузырящийся",  1, 0, 40, 150, CLR_WHITE,
                                                        POT_MONSTER_DETECTION),
POTION("обнаружение предметов",      "дымчатый",  1, 0, 40, 150, CLR_GRAY,
                                                        POT_OBJECT_DETECTION),
POTION("получение энергии",          "мутный",  1, 0, 40, 150, CLR_WHITE,
                                                        POT_GAIN_ENERGY),
POTION("сон",       "шипучий",  1, 0, 40, 100, CLR_GRAY,
                                                        POT_SLEEPING),
POTION("полное лечение",          "чёрный",  1, 0, 10, 200, CLR_BLACK,
                                                        POT_FULL_HEALING),
POTION("превращение",            "золотой",  1, 0, 10, 200, CLR_YELLOW,
                                                        POT_POLYMORPH),
POTION("алкоголь",                 "коричневый",  0, 0, 40,  50, CLR_BROWN,
                                                        POT_BOOZE),
POTION("болезнь",              "газированный",  0, 0, 40,  50, CLR_CYAN,
                                                        POT_SICKNESS),
POTION("фруктовый сок",            "тёмный",  0, 0, 40,  50, CLR_BLACK,
                                                        POT_FRUIT_JUICE),
POTION("кислота",                  "белый",  0, 0, 10, 250, CLR_WHITE,
                                                        POT_ACID),
POTION("масло",                   "мутный",  0, 0, 30, 250, CLR_BROWN,
                                                        POT_OIL),
/* fixed description
 */
POTION("вода",                 "прозрачный",  0, 0, 80, 100, CLR_CYAN,
                                                        POT_WATER),
#undef POTION

/* scrolls ... */
#define SCROLL(name,text,mgc,prob,cost,sn) \
    OBJECT(OBJ(name, text),                                           \
           BITS(0, 1, 0, 0, mgc, 0, 0, 0, 0, 0, 0, P_NONE, PAPER),    \
           0, SCROLL_CLASS, prob, 0, 5, cost, 0, 0, 0, 0, 6, \
           HI_PAPER, sn)
SCROLL("зачаровать броню",              "ZELGO MER",  1,  63,  80,
                                                        SCR_ENCHANT_ARMOR),
SCROLL("уничтожить броню",         "JUYED AWK YACC",  1,  45, 100,
                                                        SCR_DESTROY_ARMOR),
SCROLL("замешательство монстра",                 "NR 9",  1,  53, 100,
                                                        SCR_CONFUSE_MONSTER),
SCROLL("отпугивание монстра",   "XIXAXA XOXAXA XUXAXA",  1,  35, 100,
                                                        SCR_SCARE_MONSTER),
SCROLL("снятие проклятия",             "PRATYAVAYAH",  1,  65,  80,
                                                        SCR_REMOVE_CURSE),
SCROLL("зачаровать оружие",         "DAIYEN FOOELS",  1,  80,  60,
                                                        SCR_ENCHANT_WEAPON),
SCROLL("создание монстра",       "LEP GEX VEN ZEA",  1,  45, 200,
                                                        SCR_CREATE_MONSTER),
SCROLL("приручение",                   "PRIRUTSENIE",  1,  15, 200,
                                                        SCR_TAMING),
SCROLL("геноцид",                  "ELBIB YLOH",  1,  15, 300,
                                                        SCR_GENOCIDE),
SCROLL("свет",                 "VERR YED HORRE",  1,  90,  50,
                                                        SCR_LIGHT),
SCROLL("телепортация",        "VENZAR BORGAVVE",  1,  55, 100,
                                                        SCR_TELEPORTATION),
SCROLL("обнаружение золота",                 "THARR",  1,  33, 100,
                                                        SCR_GOLD_DETECTION),
SCROLL("обнаружение еды",               "YUM YUM",  1,  25, 100,
                                                        SCR_FOOD_DETECTION),
SCROLL("опознание",                  "KERNOD WEL",  1, 180,  20,
                                                        SCR_IDENTIFY),
SCROLL("магическое картирование",              "ELAM EBOW",  1,  45, 100,
                                                        SCR_MAGIC_MAPPING),
SCROLL("амнезия",                   "DUAM XNAHT",  1,  35, 200,
                                                        SCR_AMNESIA),
SCROLL("огонь",                  "ANDOVA BEGARIN",  1,  30, 100,
                                                        SCR_FIRE),
SCROLL("земля",                          "KIRJE",  1,  18, 200,
                                                        SCR_EARTH),
SCROLL("наказание",            "VE FORBRYDERNE",  1,  15, 300,
                                                        SCR_PUNISHMENT),
SCROLL("зарядка",                "HACKEM MUCHE",  1,  15, 300,
                                                        SCR_CHARGING),
SCROLL("зловонное облако",             "VELOX NEB",  1,  15, 300,
                                                        SCR_STINKING_CLOUD),
    /* Extra descriptions, shuffled into use at start of new game.
     * Code in win/share/tilemap.c depends on SCR_STINKING_CLOUD preceding
     * these and on how many of them there are.  If a real scroll gets added
     * after stinking cloud or the number of extra descriptions changes,
     * tilemap.c must be modified to match.  Mgc,Prob,Cost are superfluous.
     * SC values must be distinct but are only used by 'nethack --dumpenums'.
     */
#define XTRA_SCROLL_LABEL(text, sn) SCROLL(NoDes, text, 1, 0, 100, sn)
XTRA_SCROLL_LABEL(     "ФУБИ БЛЕТЧ", SC01),
XTRA_SCROLL_LABEL(             "ТЕМОВ", SC02),
XTRA_SCROLL_LABEL(        "ГАРВЕН ДЕХ", SC03),
XTRA_SCROLL_LABEL(           "ПРОЧТИ МЕНЯ", SC04),
XTRA_SCROLL_LABEL(     "ЭТАОИН ШРДЛУ", SC05),
XTRA_SCROLL_LABEL(       "ЛОРЕМ ИПСУМ", SC06),
XTRA_SCROLL_LABEL(             "ФНОРД", SC07), /* Illuminati */
XTRA_SCROLL_LABEL(           "КО БАТЕ", SC08), /* Kurd Lasswitz */
XTRA_SCROLL_LABEL(     "АБРА КА ДАБРА", SC09), /* traditional incantation */
XTRA_SCROLL_LABEL(      "АШПД СОДАЛГ", SC10), /* Portal */
XTRA_SCROLL_LABEL(           "ЗЛОРФИК", SC11), /* Zak McKracken */
XTRA_SCROLL_LABEL(     "ГНИК СИСИ ВЛЕ", SC12), /* Zak McKracken */
XTRA_SCROLL_LABEL(   "ГАПАКС ЛЕГОМЕНОН", SC13),
XTRA_SCROLL_LABEL( "ЭЙРИС САЗУН ИДИСИ", SC14), /* Merseburg Incantations */
XTRA_SCROLL_LABEL(   "ФОЛ ЭНДЕ ВОДАН", SC15), /* Merseburg Incantations */
XTRA_SCROLL_LABEL(             "ГХОТИ", SC16), /* pronounced as 'fish',
                                                * George Bernard Shaw */
XTRA_SCROLL_LABEL("МАПИРО МАХАМА ДИРОМАТ", SC17), /* Wizardry */
XTRA_SCROLL_LABEL( "ВАС КОРП БЕТ МАНИ", SC18), /* Ultima */
XTRA_SCROLL_LABEL(           "XOR OTA", SC19), /* Aarne Haapakoski */
XTRA_SCROLL_LABEL("СТРЦ ПРСТ СКРЖ КРК", SC20), /* Czech and Slovak
                                                * tongue-twister */
#undef XTRA_SCROLL_LABEL
    /* These must come last because they have special fixed descriptions.
     */
#ifdef MAIL_STRUCTURES
SCROLL("почта",          "со штампом",  0,   0,   0, SCR_MAIL),
#endif
SCROLL("чистый лист", "без надписи",  0,  28,  60, SCR_BLANK_PAPER),
#undef SCROLL

/* spellbooks ... */
    /* Expanding beyond 52 spells would require changes in spellcasting
     * or imposition of a limit on number of spells hero can know because
     * they are currently assigned successive letters, a-zA-Z, when learned.
     * [The existing spell sorting capability could conceivably be extended
     * to enable moving spells from beyond Z to within it, bumping others
     * out in the process, allowing more than 52 spells be known but keeping
     * only 52 be castable at any given time.]
     */
#define SPELL(name,desc,sub,prob,delay,level,mgc,dir,color,sn)  \
    OBJECT(OBJ(name, desc),                                             \
           BITS(0, 0, 0, 0, mgc, 0, 0, 0, 0, 0, dir, sub, PAPER),       \
           0, SPBOOK_CLASS, prob, delay, 50, level * 100,               \
           0, 0, 0, level, 20, color, sn)
/* Spellbook description normally refers to book covers (primarily color).
   Parchment and vellum would never be used for such, but rather than
   eliminate those, finagle their definitions to refer to the pages
   rather than the cover.  They are made from animal skin (typically of
   a goat or sheep) and books using them for pages generally need heavy
   covers with straps or clamps to tightly close the book in order to
   keep the pages flat.  (However, a wooden cover might itself be covered
   by a sheet of parchment, making this become less of an exception.  Also,
   changing the internal composition from paper to leather makes eating a
   parchment or vellum spellbook break vegetarian conduct, as it should.) */
#define PAPER LEATHER /* override enum for use in SPELL() expansion */
SPELL("копание",             "пергаментный",
      P_MATTER_SPELL,      20,  6, 5, 1, RAY, HI_LEATHER, SPE_DIG),
MARKER(FIRST_SPELL, SPE_DIG)
/* magic missile ... finger of death must be in this order; see buzz() */
SPELL("магическая стрела",   "веленевый",
      P_ATTACK_SPELL,      45,  2, 2, 1, RAY, HI_LEATHER, SPE_MAGIC_MISSILE),
#undef PAPER /* revert to normal material */
SPELL("огненный шар",        "рваный",
      P_ATTACK_SPELL,      20,  4, 4, 1, RAY, HI_PAPER, SPE_FIREBALL),
SPELL("конус холода",    "с загнутыми уголками",
      P_ATTACK_SPELL,      10,  7, 4, 1, RAY, HI_PAPER, SPE_CONE_OF_COLD),
SPELL("сон",           "пёстрый",
      P_ENCHANTMENT_SPELL, 30,  1, 3, 1, RAY, HI_PAPER, SPE_SLEEP),
SPELL("палец смерти", "в пятнах",
      P_ATTACK_SPELL,       5, 10, 7, 1, RAY, HI_PAPER, SPE_FINGER_OF_DEATH),
SPELL("свет",           "тканевый",
      P_DIVINATION_SPELL,  45,  1, 1, 1, NODIR, HI_CLOTH, SPE_LIGHT),
SPELL("обнаружение монстров", "кожаный",
      P_DIVINATION_SPELL,  43,  1, 1, 1, NODIR, HI_LEATHER,
                                                        SPE_DETECT_MONSTERS),
SPELL("лечение",         "белый",
      P_HEALING_SPELL,     40,  2, 1, 1, IMMEDIATE, CLR_WHITE,
                                                        SPE_HEALING),
SPELL("открывание",           "розовый",
      P_MATTER_SPELL,      25,  1, 1, 1, IMMEDIATE, CLR_BRIGHT_MAGENTA,
                                                        SPE_KNOCK),
SPELL("силовой снаряд",      "красный",
      P_ATTACK_SPELL,      30,  2, 1, 1, IMMEDIATE, CLR_RED,
                                                        SPE_FORCE_BOLT),
SPELL("замешательство монстра", "оранжевый",
      P_ENCHANTMENT_SPELL, 49,  2, 1, 1, IMMEDIATE, CLR_ORANGE,
                                                        SPE_CONFUSE_MONSTER),
SPELL("лечение слепоты",  "жёлтый",
      P_HEALING_SPELL,     25,  2, 2, 1, IMMEDIATE, CLR_YELLOW,
                                                        SPE_CURE_BLINDNESS),
SPELL("истощение жизни",      "бархатный",
      P_ATTACK_SPELL,      10,  2, 2, 1, IMMEDIATE, CLR_MAGENTA,
                                                        SPE_DRAIN_LIFE),
SPELL("замедление монстра",    "светло-зелёный",
      P_ENCHANTMENT_SPELL, 30,  2, 2, 1, IMMEDIATE, CLR_BRIGHT_GREEN,
                                                        SPE_SLOW_MONSTER),
SPELL("волшебный замок",     "тёмно-зелёный",
      P_MATTER_SPELL,      25,  3, 2, 1, IMMEDIATE, CLR_GREEN,
                                                        SPE_WIZARD_LOCK),
SPELL("создание монстра",  "бирюзовый",
      P_CLERIC_SPELL,      35,  3, 2, 1, NODIR, CLR_BRIGHT_CYAN,
                                                        SPE_CREATE_MONSTER),
SPELL("обнаружение еды",     "голубой",
      P_DIVINATION_SPELL,  30,  3, 2, 1, NODIR, CLR_CYAN,
                                                        SPE_DETECT_FOOD),
SPELL("вызов страха",      "светло-синий",
      P_ENCHANTMENT_SPELL, 25,  3, 3, 1, NODIR, CLR_BRIGHT_BLUE,
                                                        SPE_CAUSE_FEAR),
SPELL("ясновидение",    "тёмно-синий",
      P_DIVINATION_SPELL,  15,  3, 3, 1, NODIR, CLR_BLUE,
                                                        SPE_CLAIRVOYANCE),
SPELL("лечение болезни",   "индиго",
      P_HEALING_SPELL,     32,  3, 3, 1, NODIR, CLR_BLUE,
                                                        SPE_CURE_SICKNESS),
SPELL("очарование монстра",   "пурпурный",
      P_ENCHANTMENT_SPELL, 20,  3, 5, 1, IMMEDIATE, CLR_MAGENTA,
                                                        SPE_CHARM_MONSTER),
SPELL("ускорение себя",      "фиолетовый",
      P_ESCAPE_SPELL,      33,  4, 3, 1, NODIR, CLR_MAGENTA,
                                                        SPE_HASTE_SELF),
SPELL("обнаружение невидимого",   "фиолетовый",
      P_DIVINATION_SPELL,  20,  4, 3, 1, NODIR, CLR_MAGENTA,
                                                        SPE_DETECT_UNSEEN),
SPELL("левитация",      "светло-коричневый",
      P_ESCAPE_SPELL,      20,  4, 4, 1, NODIR, CLR_BROWN,
                                                        SPE_LEVITATION),
SPELL("усиленное лечение",   "клетчатый",
      P_HEALING_SPELL,     27,  5, 3, 1, IMMEDIATE, CLR_GREEN,
                                                        SPE_EXTRA_HEALING),
SPELL("восстановление способности", "светло-коричневый",
      P_HEALING_SPELL,     25,  5, 4, 1, NODIR, CLR_BROWN,
                                                        SPE_RESTORE_ABILITY),
SPELL("невидимость",    "тёмно-коричневый",
      P_ESCAPE_SPELL,      20,  5, 4, 1, NODIR, CLR_BROWN,
                                                        SPE_INVISIBILITY),
SPELL("обнаружение сокровищ", "серый",
      P_DIVINATION_SPELL,  20,  5, 4, 1, NODIR, CLR_GRAY,
                                                        SPE_DETECT_TREASURE),
SPELL("снятие проклятия",    "морщинистый",
      P_CLERIC_SPELL,      25,  5, 3, 1, NODIR, HI_PAPER,
                                                        SPE_REMOVE_CURSE),
SPELL("магическое картирование",   "пыльный",
      P_DIVINATION_SPELL,  18,  7, 5, 1, NODIR, HI_PAPER,
                                                        SPE_MAGIC_MAPPING),
SPELL("опознание",        "бронзовый",
      P_DIVINATION_SPELL,  20,  6, 3, 1, NODIR, HI_COPPER,
                                                        SPE_IDENTIFY),
SPELL("изгнание нежити",     "медный",
      P_CLERIC_SPELL,      16,  8, 6, 1, IMMEDIATE, HI_COPPER,
                                                        SPE_TURN_UNDEAD),
SPELL("превращение",       "серебряный",
      P_MATTER_SPELL,      10,  8, 6, 1, IMMEDIATE, HI_SILVER,
                                                        SPE_POLYMORPH),
SPELL("телепортация прочь",   "золотой",
      P_ESCAPE_SPELL,      15,  6, 6, 1, IMMEDIATE, HI_GOLD,
                                                        SPE_TELEPORT_AWAY),
SPELL("создание фамильяра", "сверкающий",
      P_CLERIC_SPELL,      10,  7, 6, 1, NODIR, CLR_WHITE,
                                                        SPE_CREATE_FAMILIAR),
SPELL("отмена",    "блестящий",
      P_MATTER_SPELL,      15,  8, 7, 1, IMMEDIATE, CLR_WHITE,
                                                        SPE_CANCELLATION),
SPELL("защита",      "тусклый",
      P_CLERIC_SPELL,      18,  3, 1, 1, NODIR, HI_PAPER,
                                                        SPE_PROTECTION),
SPELL("прыжок",         "тонкий",
      P_ESCAPE_SPELL,      20,  3, 1, 1, IMMEDIATE, HI_PAPER,
                                                        SPE_JUMPING),
SPELL("камень в плоть",  "толстый",
      P_HEALING_SPELL,     15,  1, 3, 1, IMMEDIATE, HI_PAPER,
                                                        SPE_STONE_TO_FLESH),
SPELL("цепная молния", "в клетку",
      P_ATTACK_SPELL,      25,  4, 2, 1, NODIR, CLR_GRAY,
                                                        SPE_CHAIN_LIGHTNING),

#if 0 /* DEFERRED */
/* from slash'em, create a tame critter which explodes when attacking,
   damaging adjacent creatures--friend or foe--and dying in the process */
SPELL("огненная сфера",    "холщовый",
      P_MATTER_SPELL,      20,  2, 1, 1, NODIR, CLR_BROWN,
                                                        SPE_FLAME_SPHERE),
SPELL("морозная сфера",   "в твёрдой обложке",
      P_MATTER_SPELL,      20,  2, 1, 1, NODIR, CLR_BROWN,
                                                        SPE_FREEZE_SPHERE),
#endif
/* books with fixed descriptions
 */
SPELL("чистый лист", "обычный", P_NONE, 18, 0, 0, 0, 0, HI_PAPER,
                                                        SPE_BLANK_PAPER),
/* LAST_SPELL is used to calculate MAXSPELL, allocation size of spl_book[];
   by including blank paper, which has no actual spell, we ensure that
   even if hero learns every spell, spl_book[] will have at least one
   unused slot at end; an unused slot is needed for use as terminator */
MARKER(LAST_SPELL, SPE_BLANK_PAPER)
/* tribute book added in 3.6 */
OBJECT(OBJ("роман", "в мягкой обложке"),
       BITS(0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, PAPER),
       0, SPBOOK_CLASS, 1, 0, 10, 20, 0, 0, 0, 1, 20, CLR_BRIGHT_BLUE,
                                                        SPE_NOVEL),
/* a special, one of a kind, spellbook */
OBJECT(OBJ("Книга Мёртвых", "папирусный"),
       BITS(0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, P_NONE, PAPER),
       0, SPBOOK_CLASS, 0, 0, 50, 10000, 0, 0, 0, 7, 20, HI_PAPER,
                                                        SPE_BOOK_OF_THE_DEAD),
#undef SPELL

/* wands ... */
#define WAND(name,typ,prob,cost,mgc,dir,metal,color,sn) \
    OBJECT(OBJ(name, typ),                                              \
           BITS(0, 0, 1, 0, mgc, 1, 0, 0, 0, 0, dir, P_NONE, metal),    \
           0, WAND_CLASS, prob, 0, 7, cost, 0, 0, 0, 0, 30, color, sn)
WAND("свет",           "стеклянный", 95, 100, 1, NODIR, GLASS, HI_GLASS,
                                                            WAN_LIGHT),
WAND("обнаружение потайных дверей",
                        "бальсовый", 50, 150, 1, NODIR, WOOD, HI_WOOD,
                                                    WAN_SECRET_DOOR_DETECTION),
WAND("просветление", "хрустальный", 15, 150, 1, NODIR, GLASS, HI_GLASS,
                                                    WAN_ENLIGHTENMENT),
WAND("создание монстра",  "кленовый", 50, 200, 1, NODIR, WOOD, HI_WOOD,
                                                    WAN_CREATE_MONSTER),
WAND("желание",          "сосновый",  5, 500, 1, NODIR, WOOD, HI_WOOD,
                                                    WAN_WISHING),
WAND("стазис",        "из красного дерева", 45, 150, 1, NODIR, WOOD, CLR_RED,
                                                    WAN_STASIS),
WAND("ничего",           "дубовый", 25, 100, 0, IMMEDIATE, WOOD, HI_WOOD,
                                                    WAN_NOTHING),
WAND("удар",        "из чёрного дерева", 30, 150, 1, IMMEDIATE, WOOD, HI_WOOD,
                                                    WAN_STRIKING),
WAND("сделать невидимым", "мраморный", 45, 150, 1, IMMEDIATE, MINERAL, HI_MINERAL,
                                                    WAN_MAKE_INVISIBLE),
WAND("замедление монстра",      "оловянный", 50, 150, 1, IMMEDIATE, METAL, HI_METAL,
                                                    WAN_SLOW_MONSTER),
WAND("ускорение монстра",   "латунный", 50, 150, 1, IMMEDIATE, COPPER, HI_COPPER,
                                                    WAN_SPEED_MONSTER),
WAND("обращение нежити", "медный", 50, 150, 1, IMMEDIATE, COPPER, HI_COPPER,
                                                    WAN_UNDEAD_TURNING),
WAND("превращение",      "серебряный", 45, 200, 1, IMMEDIATE, SILVER, HI_SILVER,
                                                    WAN_POLYMORPH),
WAND("отмена", "платиновый", 45, 200, 1, IMMEDIATE, PLATINUM, CLR_WHITE,
                                                    WAN_CANCELLATION),
WAND("телепортация", "иридиевый", 45, 200, 1, IMMEDIATE, METAL,
                                     CLR_BRIGHT_CYAN, WAN_TELEPORTATION),
WAND("открывание",          "цинковый", 30, 150, 1, IMMEDIATE, METAL, HI_METAL,
                                                    WAN_OPENING),
WAND("запирание",      "алюминиевый", 30, 150, 1, IMMEDIATE, METAL, HI_METAL,
                                                    WAN_LOCKING),
WAND("разведка",       "урановый", 30, 150, 1, IMMEDIATE, METAL, HI_METAL,
                                                    WAN_PROBING),
WAND("копание",          "железный", 40, 150, 1, RAY, IRON, HI_METAL,
                                                    WAN_DIGGING),
/* magic missile ... lightning must be in this order; see buzz() */
WAND("магическая стрела",   "стальной", 50, 150, 1, RAY, IRON, HI_METAL,
                                                    WAN_MAGIC_MISSILE),
WAND("огонь",        "шестиугольный", 40, 175, 1, RAY, IRON, HI_METAL,
                                                    WAN_FIRE),
WAND("холод",            "короткий", 40, 175, 1, RAY, IRON, HI_METAL,
                                                    WAN_COLD),
WAND("сон",           "рунический", 50, 175, 1, RAY, IRON, HI_METAL,
                                                    WAN_SLEEP),
WAND("смерть",            "длинный",  5, 500, 1, RAY, IRON, HI_METAL,
                                                    WAN_DEATH),
WAND("молния",      "изогнутый", 40, 175, 1, RAY, IRON, HI_METAL,
                                                    WAN_LIGHTNING),
/* extra descriptions, shuffled into use at start of new game */
WAND(NoDes,             "развильчатый",  0, 150, 1, 0, WOOD, HI_WOOD, WAN1),
WAND(NoDes,             "с шипами",  0, 150, 1, 0, IRON, HI_METAL, WAN2),
WAND(NoDes,            "украшенный самоцветами",  0, 150, 1, 0, IRON, HI_MINERAL, WAN3),
#undef WAND

/* coins ... - so far, gold is all there is */
#define COIN(name,prob,metal,worth,sn) \
    OBJECT(OBJ(name, NoDes),                                         \
           BITS(1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, metal),    \
           0, COIN_CLASS, prob, 0, 1, worth, 0, 0, 0, 0, 0, HI_GOLD, sn)
COIN("золотая монета", 1000, GOLD, 1, GOLD_PIECE),
#undef COIN

/* gems ... - includes stones and rocks but not boulders */
#define GEM(name,desc,prob,wt,gval,nutr,mohs,glass,color,sn) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(0, 1, 0, 0, 0, 0, 0, 0, 0,                              \
                HARDGEM(mohs), 0, -P_SLING, glass),                     \
           0, GEM_CLASS, prob, 0, wt, gval, 3, 3, 0, 0, nutr, color, sn)
#define ROCK(name,desc,kn,prob,wt,gval,sdam,ldam,mgc,nutr,mohs,glass,colr,sn) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(kn, 1, 0, 0, mgc, 0, 0, 0, 0,                           \
                HARDGEM(mohs), 0, -P_SLING, glass),                     \
           0, GEM_CLASS, prob, 0, wt, gval, sdam, ldam, 0, 0, nutr, colr, sn)
GEM("кристалл дилития", "белый",  2, 1, 4500, 15,  5, GEMSTONE, CLR_WHITE,
                                                        DILITHIUM_CRYSTAL),
MARKER(FIRST_REAL_GEM, DILITHIUM_CRYSTAL)
GEM("алмаз",           "белый",  3, 1, 4000, 15, 10, GEMSTONE, CLR_WHITE,
                                                        DIAMOND),
GEM("рубин",                "красный",  4, 1, 3500, 15,  9, GEMSTONE, CLR_RED,
                                                        RUBY),
GEM("гиацинт",          "оранжевый",  3, 1, 3250, 15,  9, GEMSTONE, CLR_ORANGE,
                                                        JACINTH),
GEM("сапфир",           "синий",  4, 1, 3000, 15,  9, GEMSTONE, CLR_BLUE,
                                                        SAPPHIRE),
GEM("чёрный опал",        "чёрный",  3, 1, 2500, 15,  8, GEMSTONE, CLR_BLACK,
                                                        BLACK_OPAL),
GEM("изумруд",           "зелёный",  5, 1, 2500, 15,  8, GEMSTONE, CLR_GREEN,
                                                        EMERALD),
GEM("бирюза",         "зелёный",  6, 1, 2000, 15,  6, GEMSTONE, CLR_GREEN,
                                                        TURQUOISE),
GEM("цитрин",          "жёлтый",  4, 1, 1500, 15,  6, GEMSTONE, CLR_YELLOW,
                                                        CITRINE),
GEM("аквамарин",        "зелёный",  6, 1, 1500, 15,  8, GEMSTONE, CLR_GREEN,
                                                        AQUAMARINE),
GEM("янтарь",   "желтовато-коричневый",  8, 1, 1000, 15,  2, GEMSTONE, CLR_BROWN,
                                                        AMBER),
GEM("топаз",   "желтовато-коричневый", 10, 1,  900, 15,  8, GEMSTONE, CLR_BROWN,
                                                        TOPAZ),
GEM("гагат",               "чёрный",  6, 1,  850, 15,  7, GEMSTONE, CLR_BLACK,
                                                        JET),
GEM("опал",              "белый", 12, 1,  800, 15,  6, GEMSTONE, CLR_WHITE,
                                                        OPAL),
GEM("хризоберилл",      "жёлтый",  8, 1,  700, 15,  5, GEMSTONE, CLR_YELLOW,
                                                        CHRYSOBERYL),
GEM("гранат",              "красный", 12, 1,  700, 15,  7, GEMSTONE, CLR_RED,
                                                        GARNET),
GEM("аметист",         "фиолетовый", 14, 1,  600, 15,  7, GEMSTONE, CLR_MAGENTA,
                                                        AMETHYST),
GEM("яшма",              "красный", 15, 1,  500, 15,  7, GEMSTONE, CLR_RED,
                                                        JASPER),
GEM("флюорит",         "фиолетовый", 15, 1,  400, 15,  4, GEMSTONE, CLR_MAGENTA,
                                                        FLUORITE),
GEM("обсидиан",          "чёрный",  9, 1,  200, 15,  6, GEMSTONE, CLR_BLACK,
                                                        OBSIDIAN),
GEM("агат",            "оранжевый", 12, 1,  200, 15,  6, GEMSTONE, CLR_ORANGE,
                                                        AGATE),
GEM("нефрит",              "зелёный", 10, 1,  300, 15,  6, GEMSTONE, CLR_GREEN,
                                                        JADE),
MARKER(LAST_REAL_GEM, JADE)
GEM("бесполезный кусок белого стекла", "белый",
    77, 1, 0, 6, 5, GLASS, CLR_WHITE, WORTHLESS_WHITE_GLASS),
MARKER(FIRST_GLASS_GEM, WORTHLESS_WHITE_GLASS)
GEM("бесполезный кусок синего стекла", "синий",
    77, 1, 0, 6, 5, GLASS, CLR_BLUE, WORTHLESS_BLUE_GLASS),
GEM("бесполезный кусок красного стекла", "красный",
    77, 1, 0, 6, 5, GLASS, CLR_RED, WORTHLESS_RED_GLASS),
GEM("бесполезный кусок желтовато-коричневого стекла", "желтовато-коричневый",
    77, 1, 0, 6, 5, GLASS, CLR_BROWN, WORTHLESS_YELLOWBROWN_GLASS),
GEM("бесполезный кусок оранжевого стекла", "оранжевый",
    76, 1, 0, 6, 5, GLASS, CLR_ORANGE, WORTHLESS_ORANGE_GLASS),
GEM("бесполезный кусок жёлтого стекла", "жёлтый",
    77, 1, 0, 6, 5, GLASS, CLR_YELLOW, WORTHLESS_YELLOW_GLASS),
GEM("бесполезный кусок чёрного стекла", "чёрный",
    76, 1, 0, 6, 5, GLASS, CLR_BLACK, WORTHLESS_BLACK_GLASS),
GEM("бесполезный кусок зелёного стекла", "зелёный",
    77, 1, 0, 6, 5, GLASS, CLR_GREEN, WORTHLESS_GREEN_GLASS),
GEM("бесполезный кусок фиолетового стекла", "фиолетовый",
    77, 1, 0, 6, 5, GLASS, CLR_MAGENTA, WORTHLESS_VIOLET_GLASS),
MARKER(LAST_GLASS_GEM, WORTHLESS_VIOLET_GLASS)

/* Placement note: there is a wishable subrange for
 * "gray stones" in the o_ranges[] array in objnam.c
 * that is currently everything between luckstones and flint
 * (inclusive).
 */
ROCK("камень удачи", "серый",  0,  10,  10, 60, 3, 3, 1, 10, 7, MINERAL, CLR_GRAY,
                                                                    LUCKSTONE),
ROCK("магнитный камень", "серый",  0,  10, 500,  1, 3, 3, 1, 10, 6, MINERAL, CLR_GRAY,
                                                                    LOADSTONE),
ROCK("пробный камень", "серый", 0,   8,  10, 45, 3, 3, 1, 10, 6, MINERAL, CLR_GRAY,
                                                                  TOUCHSTONE),
ROCK("кремень", "серый",      0,  10,  10,  1, 6, 6, 0, 10, 7, MINERAL, CLR_GRAY,
                                                                    FLINT),
ROCK("камень", NoDes,         1, 100,  10,  0, 3, 3, 0, 10, 7, MINERAL, CLR_GRAY,
                                                                    ROCK),
#undef GEM
#undef ROCK

/* miscellaneous ... */
/* Note: boulders and rocks are not normally created at random; the
 * probabilities only come into effect when you try to polymorph them.
 * Boulders weigh more than MAX_CARR_CAP; statues use corpsenm to take
 * on a specific type and may act as containers (both affect weight).
 */
OBJECT(OBJ("валун", NoDes),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, P_NONE, MINERAL), 0,
       ROCK_CLASS, 100, 0, 6000, 0, 20, 20, 0, 0, 2000, HI_MINERAL, BOULDER),
OBJECT(OBJ("статуя", NoDes),
       BITS(1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, P_NONE, MINERAL), 0,
       ROCK_CLASS, 900, 0, 2500, 0, 20, 20, 0, 0, 2500, CLR_WHITE, STATUE),

OBJECT(OBJ("тяжёлый железный шар", NoDes),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, WHACK, P_NONE, IRON), 0,
       BALL_CLASS, 1000, 0, 480, 10, 25, 25, 0, 0, 200, HI_METAL,
                                                            HEAVY_IRON_BALL),
        /* +d4 when "very heavy" */
OBJECT(OBJ("железная цепь", NoDes),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, WHACK, P_NONE, IRON), 0,
       CHAIN_CLASS, 1000, 0, 120, 0, 4, 4, 0, 0, 200, HI_METAL, IRON_CHAIN),
        /* +1 both l & s */

/* Venom is normally a transitory missile (spit by various creatures)
 * but can be wished for in wizard mode so could occur in bones data.
 */
OBJECT(OBJ("брызги ослепляющего яда", "брызги яда"),
       BITS(0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, LIQUID), 0,
       VENOM_CLASS, 500, 0, 1, 0, 0, 0, 0, 0, 0, HI_ORGANIC, BLINDING_VENOM),
OBJECT(OBJ("брызги кислотного яда", "брызги яда"),
       BITS(0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, LIQUID), 0,
       VENOM_CLASS, 500, 0, 1, 0, 6, 6, 0, 0, 0, HI_ORGANIC, ACID_VENOM),
        /* +d6 small or large */

#if defined(OBJECTS_DESCR_INIT) || defined(OBJECTS_INIT)
/* fencepost, the deadly Array Terminator -- name [1st arg] *must* be NULL */
OBJECT(OBJ(NoDes, NoDes),
       BITS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, 0), 0,
       ILLOBJ_CLASS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#undef BITS
#endif

#undef OBJ
#undef OBJECT
#undef MARKER
#undef HARDGEM
#undef NoDes

/*objects.c*/
