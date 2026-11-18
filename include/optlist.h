/* NetHack 5.0	optlist.h */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef OPTLIST_H
#define OPTLIST_H

/*
 *  NOTE:  If you add (or delete) an option, please review:
 *             doc/options.txt
 *
 *         It contains how-to info and outlines some required/suggested
 *         updates that should accompany your change.
 */

#define BACKWARD_COMPAT

extern int optfn_boolean(int, int, boolean, char *, char *);

enum OptType { BoolOpt, CompOpt, OthrOpt };
enum Y_N { No, Yes };
enum Off_On { Off, On };
/* Advanced options are only shown in the full, traditional options menu */
enum OptSection {
    OptS_General, OptS_Behavior, OptS_Map, OptS_Status, OptS_Advanced
};
enum menu_terminology_preference {
    Term_False, Term_Off, Term_Disabled, Term_Excluded, num_terms
};

struct allopt_t {
    const char *name;
    enum OptSection section;
    int minmatch;
    int expectedbuf;
    int idx;
    enum optset_restrictions setwhere;
    enum OptType opttyp;
    enum Y_N negateok;
    enum Y_N valok;
    enum Y_N dupeok;
    enum Y_N pfx;
    enum menu_terminology_preference termpref;
    boolean opt_in_out, *addr;
    int (*optfn)(int, int, boolean, char *, char *);
    const char *alias;
    const char *descr;
    const char *prefixgw;
    boolean initval, has_handler, dupdetected, disregarded;
};

#endif /* OPTLIST_H */

#if defined(NHOPT_PROTO) || defined(NHOPT_ENUM) || defined(NHOPT_PARSE)
/* clang-format off */
/* *INDENT-OFF* */

#define NoAlias ((const char *) 0)

#if defined(NHOPT_PROTO)
#define NHOPTB(a, sec, b, c, s, i, n, v, d, al, bp, termp, desc) /*empty*/
#define NHOPTC(a, sec, b, c, s, n, v, d, h, al, z)               \
static int optfn_##a(int, int, boolean, char *, char *);
#define NHOPTP(a, sec, b, c, s, n, v, d, h, al, z)               \
static int pfxfn_##a(int, int, boolean, char *, char *);
#define NHOPTO(m, sec, a, b, c, s, n, v, d, al, z)               \
static int optfn_##a(int, int, boolean, char *, char *);

#elif defined(NHOPT_ENUM)
#define NHOPTB(a, sec, b, c, s, i, n, v, d, al, bp, termp, desc) opt_##a,
#define NHOPTC(a, sec, b, c, s, n, v, d, h, al, z)   opt_##a,
#define NHOPTP(a, sec, b, c, s, n, v, d, h, al, z)   pfx_##a,
#define NHOPTO(m, sec, a, b, c, s, n, v, d, al, z)   opt_##a,

#elif defined(NHOPT_PARSE)
#define NHOPTB(a, sec, b, c, s, i, n, v, d, al, bp, termp, desc)             \
    { #a, OptS_##sec, 0, b, opt_##a, s, BoolOpt, n, v, d, No, termp, c,  \
      bp, &optfn_boolean, al, desc, (const char *) 0, i, 0, 0 , 0 },
#define NHOPTC(a, sec, b, c, s, n, v, d, h, al, z) \
    { #a, OptS_##sec, 0, b, opt_##a, s, CompOpt, n, v, d, No, 0, c,  \
      (boolean *) 0, &optfn_##a, al, z, (const char *) 0, Off, h, 0, 0 },
#define NHOPTP(a, sec, b, c, s, n, v, d, h, al, z) \
    { #a, OptS_##sec, 0, b, pfx_##a, s, CompOpt, n, v, d, Yes, 0, c, \
      (boolean *) 0, &pfxfn_##a, al, z, #a, Off, h, 0, 0 },
#define NHOPTO(m, sec, a, b, c, s, n, v, d, al, z) \
    { m, OptS_##sec, 0, b, opt_##a, s, OthrOpt, n, v, d, No, 0, c,   \
      (boolean *) 0, &optfn_##a, al, z, (const char *) 0, On, On, 0, 0 },

/* this is not reliable because TILES_IN_GLYPHMAP might be defined
 * in a multi-interface binary but not apply to the current interface */
#ifdef TILES_IN_GLYPHMAP
#define tiled_map_Def On
#define ascii_map_Def Off
#else
#define ascii_map_Def On
#define tiled_map_Def Off
#endif
#endif

/* B:nm, sec, ln, opt_*, setwhere?, on?, negat?, val?, dup?, hndlr? Alias,
            bool_p, term */
/* C:nm, sec, ln, opt_*, setwhere?, negateok?, valok?, dupok?, hndlr? Alias,
            desc */
/* P:pfx, sec, ln, opt_*, setwhere?, negateok?, valok?, dupok?, hndlr? Alias,
            desc*/
    /*
     * Most of the options are in alphabetical order; a few are forced
     * to the top of list so that doset() will list them first and
     * all_options_str() will gather them first to write to the top of
     * a new RC file by #saveoptions.
     *
     * windowtype comes first because its value can affect how wc_ and
     * wc2_ options are processed; playmode (for players who can't or
     * don't know how to specify a command line) and name (ditto, more
     * or less) come next; then role, race, gender, align.  Those will
     * be at the top of the file for #saveoptions constructed RC file.
     */
    NHOPTC(windowtype, Advanced, WINTYPELEN, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "используемая оконная система (указывается первой)")
    NHOPTC(playmode, Advanced, 8, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "обычная игра, исследовательский режим или отладочный режим")
    NHOPTC(name, Advanced, PL_NSIZ, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "имя вашего персонажа (напр., name:Merlin-W)")
    NHOPTC(role, Advanced, PL_CSIZ, opt_in, set_gameview,
                Yes, Yes, Yes, No, "character",
                "ваша стартовая роль (напр., Barbarian, Valkyrie)")
    NHOPTC(race, Advanced, PL_CSIZ, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias,
                "ваша стартовая раса (напр., Human, Elf)")
    NHOPTC(gender, Advanced, 8, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias,
                "ваш стартовый пол (мужской или женский)")
    NHOPTC(alignment, Advanced, 8, opt_in, set_gameview,
                Yes, Yes, Yes, No, "align",
                "ваша стартовая позиция (lawful, neutral или chaotic)")
    /* end of special ordering; remainder of entries are in alphabetical order
     */
    NHOPTB(accessiblemsg, Advanced, 0, opt_out, set_in_game,
           Off, Yes, No, No, NoAlias, &a11y.accessiblemsg, Term_False,
           "добавлять сведения о месте в сообщения")
    NHOPTB(acoustics, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.acoustics, Term_False,
           "слышит ли ваш персонаж что-нибудь")
 /* NHOPTC(align) -- moved to top */
    NHOPTC(align_message, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, No, Yes, NoAlias, "выравнивание окна сообщений")
    NHOPTC(align_status, Advanced, 20, opt_in, set_gameview,
                No, Yes, No, Yes, NoAlias, "выравнивание окна статуса")
#ifdef WIN32
    NHOPTC(altkeyhandling, Advanced, 20, opt_in, set_in_game,
                No, Yes, No, Yes, "altkeyhandler", "альтернативная обработка клавиш")
#else
    NHOPTC(altkeyhandling, Advanced, 20, opt_in, set_in_config,
                No, Yes, No, Yes, "altkeyhandler", "(неприменимо)")
#endif
#ifdef ALTMETA
    NHOPTB(altmeta, Advanced, 0, opt_out, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.altmeta, Term_False,
           "считать \"ESC c\" как M-c (Meta+c, 8-й бит)")
#else
    NHOPTB(altmeta, Advanced, 0, opt_out, set_in_config,
           Off, Yes, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(armorstatus, Advanced, 0, opt_in, set_in_game,
                Off, Yes, No, No, NoAlias, &flags.armorstatus, Term_False,
                "показывать надетую броню в поле состояния")
    NHOPTB(ascii_map, Advanced, 0, opt_in, set_in_game,
                ascii_map_Def, Yes, No, No, NoAlias, &iflags.wc_ascii_map,
                Term_False, "показывать карту текстом")
    NHOPTO("autocompletions", Advanced, o_autocomplete, BUFSZ, opt_in,
                set_in_game, No, Yes, No, NoAlias, "редактировать автодополнения")
    NHOPTB(autodescribe, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.autodescribe, Term_False,
           "описывать местность под курсором")
    NHOPTB(autodig, Behavior, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.autodig, Term_False,
           "копать при движении с инструментом для копания")
    NHOPTB(autoopen, Behavior, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.autoopen, Term_False,
           "при шаге в дверь пытаться её открыть")
    NHOPTB(autopickup, Behavior, 0, opt_out, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.pickup, Term_False,
           "автоматически подбирать предметы")
    NHOPTO("autopickup exceptions", Behavior, o_autopickup_exceptions, BUFSZ,
                opt_in, set_in_game,
                No, Yes, No, NoAlias, "редактировать исключения автоподбора")
    NHOPTB(autoquiver, Behavior, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.autoquiver, Term_False,
           "заполнять пустой колчан при стрельбе")
    NHOPTC(autounlock, Behavior, 80, opt_out, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "действие при встрече с запертой дверью или сундуком")
    NHOPTB(bgcolors, Map, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.bgcolors, Term_Off,
           "использовать фон для подсветки карты")
    NHOPTO("bind keys", Advanced, o_bind_keys, BUFSZ, opt_in, set_in_game,
                No, Yes, No, NoAlias, "редактировать привязки клавиш")
#if defined(MICRO) && !defined(AMIGA)
    NHOPTB(BIOS, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.BIOS, Term_False,
           "использовать вызовы IBM ROM BIOS")
#else
    NHOPTB(BIOS, Advanced, 0, opt_in, set_in_config,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(blind, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, "permablind", &u.uroleplay.blind, Term_False,
           "ваш персонаж постоянно слеп")
    NHOPTB(bones, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &flags.bones, Term_False,
           "разрешать загрузку файлов костей")
#ifdef BACKWARD_COMPAT
    NHOPTC(boulder, Advanced, 1, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "устарело (используйте S_boulder в файле символов)")
#endif
    NHOPTC(catname, Advanced, PL_PSIZ, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "имя вашего стартового питомца, если это котёнок")
#ifdef INSURANCE
    NHOPTB(checkpoint, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.ins_chkpt, Term_False,
           "сохранять состояние игры при смене уровня")
#else
    NHOPTB(checkpoint, Advanced, 0, opt_out, set_in_config,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(cmdassist, Behavior, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.cmdassist, Term_False,
           "подсказывать при ошибке ввода направления")
    NHOPTB(color, Map, 0, opt_in, set_in_game,
           On, Yes, No, No, "colour", &iflags.wc_color, Term_False,
           "использовать цвет на карте")
    NHOPTB(confirm, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.confirm, Term_False,
           "спрашивать перед ударом мирных монстров")
#ifdef CRASHREPORT
    NHOPTC(crash_email, Advanced, PL_NSIZ, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "адрес электронной почты для отчётов")
    NHOPTC(crash_name, Advanced, PL_NSIZ, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "ваше имя для отчётов")
    NHOPTC(crash_urlmax, Advanced, PL_NSIZ, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "длина самого длинного URL, который мы можем создать")
#endif
#ifdef CURSES_GRAPHICS
    NHOPTC(cursesgraphics, Advanced, 70, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "загрузить символы отображения curses в набор символов")
#endif
    NHOPTB(customcolors, Map, 0, opt_out, set_in_game,
           On, Yes, No, No, "customcolours", &iflags.customcolors,
           Term_False, "использовать свои цвета на карте")
    NHOPTB(customsymbols, Map, 0, opt_out, set_in_game,
           On, Yes, No, No, "customsymbols", &iflags.customsymbols,
           Term_False, "использовать свои символы utf8 на карте")
    NHOPTB(dark_room, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.dark_room, Term_False,
           "показывать пол вне поля зрения иначе")
    NHOPTB(deaf, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, "permadeaf", &u.uroleplay.deaf, Term_False,
           "ваш персонаж постоянно глух")
#ifdef BACKWARD_COMPAT
    NHOPTC(DECgraphics, Advanced, 70, opt_in, set_in_config,
                Yes, Yes, No, No, NoAlias,
                "загрузить символы отображения DECGraphics в набор символов")
#endif
    NHOPTB(debug_hunger, Advanced, 0, opt_in, set_wiznofuz,
           Off, Yes, No, No, NoAlias, &iflags.debug_hunger, Term_False,
           "без голода")
    NHOPTB(debug_mongen, Advanced, 0, opt_in, set_wiznofuz,
           Off, Yes, No, No, NoAlias, &iflags.debug_mongen, Term_False,
           "без случайного появления монстров")
    NHOPTB(debug_overwrite_stairs, Advanced, 0, opt_in, set_wiznofuz,
                Off, Yes, No, No, NoAlias, &iflags.debug_overwrite_stairs,
           Term_False, "генерация уровня может затирать лестницы")
    NHOPTC(disclose, Advanced, sizeof flags.end_disclose * 2,
                opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "виды информации, раскрываемые в конце игры")
    NHOPTC(dogname, Advanced, PL_PSIZ, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "имя вашего стартового питомца, если это маленькая собака")
    NHOPTB(dropped_nopick, Behavior, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.nopick_dropped, Term_False,
           "не подбирать выброшенные предметы")
    NHOPTC(dungeon, Advanced, MAXDCHARS + 1,opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "список символов для отрисовки карты подземелья")
    NHOPTC(effects, Advanced, MAXECHARS + 1, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "список символов для отрисовки спецэффектов")
    NHOPTB(eight_bit_tty, Advanced, 0, opt_in, set_in_game,
                Off, Yes, No, No, NoAlias, &iflags.wc_eight_bit_input,
           Term_False, "передавать 8-битные символы прямо в терминал")
    NHOPTB(extmenu, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.extmenu, Term_False,
           "использовать меню для расширенных команд")
    NHOPTB(female, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, "male", &flags.female, Term_False,
           "устарело; используйте gender:female")
    NHOPTB(fireassist, Behavior, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.fireassist, Term_False,
           "команда стрельбы пытается помочь")
    NHOPTB(fixinv, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.invlet_constant, Term_False,
           "предметы в сумке сохраняют свои буквы")
    NHOPTC(font_map, Advanced, 40, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "шрифт для окна карты")
    NHOPTC(font_menu, Advanced, 40, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "шрифт для меню")
    NHOPTC(font_message, Advanced, 40, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias,
                "шрифт для окна сообщений")
    NHOPTC(font_size_map, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "размер шрифта карты")
    NHOPTC(font_size_menu, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "размер шрифта меню")
    NHOPTC(font_size_message, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "размер шрифта сообщений")
    NHOPTC(font_size_status, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "размер шрифта статуса")
    NHOPTC(font_size_text, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "размер шрифта текста")
    NHOPTC(font_status, Advanced, 40, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "шрифт для окна статуса")
    NHOPTC(font_text, Advanced, 40, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "шрифт для текстовых окон")
    NHOPTB(force_invmenu, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.force_invmenu, Term_False,
           "команды выбора предмета показывают меню")
    NHOPTC(fruit, General, PL_FSIZ, opt_in, set_in_game,
                No, Yes, No, No, NoAlias, "название фрукта, который вам нравится есть")
    NHOPTB(fullscreen, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.wc2_fullscreen, Term_False,
           "переключать полноэкранный режим")
 /* NHOPTC(gender) -- moved to top */
    NHOPTC(glyph, Advanced, 40, opt_in, set_in_game,
                No, Yes, Yes, No, NoAlias,
                "задать представление глифа как значения Unicode и цвета")
    NHOPTB(goldX, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.goldX, Term_False,
           "считать золото неопознанным или не проклятым")
    NHOPTB(guicolor, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.wc2_guicolor, Term_False,
           "использовать цвет в интерфейсе")
    NHOPTB(help, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.help, Term_False,
           "показывать всю доступную информацию в команде whatis")
    NHOPTB(herecmd_menu, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.herecmd_menu, Term_False,
           "показывать доступные здесь команды")
#if defined(MACOS9)
    NHOPTC(hicolor, Advanced, 15, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "то же, что и palette, только в обратном порядке")
#endif
    NHOPTB(hilite_pet, Map, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.wc_hilite_pet, Term_False,
           "подсвечивать питомцев")
    NHOPTB(hilite_pile, Map, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.hilite_pile, Term_False,
           "подсвечивать кучи предметов")
#ifdef STATUS_HILITES
    NHOPTC(hilite_status, Advanced, 13, opt_out, set_in_game,
                Yes, Yes, Yes, No, NoAlias,
                "правило подсветки статуса (может повторяться)")
#else
    NHOPTC(hilite_status, Advanced, 13, opt_out, set_in_config,
                Yes, Yes, Yes, No, NoAlias, "(недоступно)")
#endif
    NHOPTB(hitpointbar, Status, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.wc2_hitpointbar, Term_False,
           "показывать цветную полосу здоровья")
    NHOPTC(horsename, Advanced, PL_PSIZ, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "имя вашего стартового питомца, если это пони")
#ifdef BACKWARD_COMPAT
    NHOPTC(IBMgraphics, Advanced, 70, opt_in, set_in_config,
                Yes, Yes, No, No, NoAlias,
                "загрузить символы отображения IBMGraphics в набор символов")
#endif
    NHOPTB(idlecheckpoint, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.idlecheckpoint, Term_Off,
           "обновлять файл контрольных точек при простое 10 секунд")
#ifndef MACOS9
    NHOPTB(ignintr, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.ignintr, Term_False,
           "игнорировать сигналы прерывания")
#else
    NHOPTB(ignintr, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(implicit_uncursed, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.implicit_uncursed, Term_False,
           "не писать \"не проклято\" в сумке")
#if 0   /* obsolete - pre-OSX Mac */
    NHOPTB(large_font, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.obsolete,
           (char *)0)
#endif
    NHOPTB(legacy, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &flags.legacy, Term_False,
           "показывать вступительное сообщение")
    NHOPTB(lit_corridor, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.lit_corridor, Term_False,
           "показывать тёмные коридоры освещёнными в поле зрения")
    NHOPTB(lootabc, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.lootabc, Term_False,
           "использовать a/b/c вместо o/i/c при обыске")
    NHOPTB(mail, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.biff, Term_False,
           "включить почтового демона")
    NHOPTC(map_mode, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, No, No, NoAlias, "режим отображения карты в Windows")
    NHOPTB(mention_decor, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.mention_decor, Term_False,
           "сообщать при проходе по интересным местам")
    NHOPTB(mention_map, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &a11y.glyph_updates, Term_False,
           "сообщать об изменениях интересных мест")
    NHOPTB(mention_walls, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.mention_walls, Term_False,
           "сообщать при шаге в стену")
    NHOPTC(menu_deselect_all, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "снять выбор со всех пунктов меню")
    NHOPTC(menu_deselect_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "снять выбор со всех пунктов этой страницы меню")
    NHOPTC(menu_first_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "перейти на первую страницу меню")
    NHOPTC(menu_headings, Advanced, 4, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias, "стиль отображения заголовков меню")
    NHOPTC(menu_invert_all, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "инвертировать выбор всех пунктов меню")
    NHOPTC(menu_invert_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "инвертировать выбор на этой странице меню")
    NHOPTC(menu_last_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "перейти на последнюю страницу меню")
    NHOPTC(menu_next_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "перейти к следующей странице меню")
    NHOPTC(menu_objsyms, Advanced, 12, opt_in, set_in_game,
           Yes, Yes, No, Yes, "use_menu_glyphs",
           "показывать символы объектов в меню")
#ifdef TTY_GRAPHICS
    NHOPTB(menu_overlay, Advanced, 0, opt_in, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.menu_overlay, Term_False,
           "меню перекрывают экран и выровнены вправо")
#else
    NHOPTB(menu_overlay, Advanced, 0, opt_in, set_in_config,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTC(menu_previous_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "перейти к предыдущей странице меню")
    NHOPTC(menu_search, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "поиск пункта меню")
    NHOPTC(menu_select_all, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "выбрать все пункты меню")
    NHOPTC(menu_select_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "выбрать все пункты этой страницы меню")
    NHOPTC(menu_shift_left, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "прокрутить текущую страницу меню влево")
    NHOPTC(menu_shift_right, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "прокрутить текущую страницу меню вправо")
    NHOPTB(menu_tab_sep, Advanced, 0, opt_in, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.menu_tab_sep, Term_False,
           "форматирование меню")
    NHOPTB(menucolors, Advanced, 0, opt_in, set_in_game,
           Off, Yes, Yes, No, NoAlias, &iflags.use_menu_color, Term_False,
           "использовать цвета в меню")
    NHOPTO("menu colors", Status, o_menu_colors, BUFSZ, opt_in, set_in_game,
                No, Yes, No, NoAlias, "изменять цвета в меню")
    NHOPTC(menuinvertmode, Advanced, 5, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "экспериментальное поведение инверсии меню")
    NHOPTC(menustyle, Advanced, MENUTYPELEN, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "интерфейс выбора объектов")
    NHOPTO("message types", Advanced, o_message_types, BUFSZ,
                opt_in, set_in_game,
                No, Yes, No, NoAlias, "редактировать типы сообщений")
    NHOPTB(mon_movement, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &a11y.mon_movement, Term_False,
           "сообщение, когда герой видит движение монстра")
    NHOPTB(monpolycontrol, Advanced, 0, opt_in, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.mon_polycontrol, Term_False,
           "управлять превращениями монстров")
    NHOPTB(montelecontrol, Advanced, 0, opt_in, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.mon_telecontrol, Term_False,
           "управлять телепортацией монстров")
    NHOPTC(monsters, Advanced, MAXMCLASSES, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "список символов для монстров")
    NHOPTC(mouse_support, Advanced, 0, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "игра получает сведения о щелчках мыши")
#if PREV_MSGS /* tty or curses */
    NHOPTC(msg_window, Advanced, 1, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "управление поведением «просмотра предыдущих сообщений» (^P)")
#else
    NHOPTC(msg_window, Advanced, 1, opt_in, set_in_config,
                Yes, Yes, No, Yes, NoAlias, "(неприменимо)")
#endif
    NHOPTC(msghistory, Advanced, 5, opt_in, set_gameview,
                Yes, Yes, No, No, NoAlias,
                "число сохраняемых сообщений верхней строки")
 /* NHOPTC(name) -- moved to top */
#ifdef NEWS
    NHOPTB(news, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.news, Term_False,
           "показывать новости при старте игры")
#else
    NHOPTB(news, Advanced, 0, opt_in, set_in_config,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(nudist, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &u.uroleplay.nudist, Term_False,
           "начинать персонажа без брони")
    NHOPTB(null, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.null, Term_False,
           "разрешать передачу нулей в терминал")
    NHOPTC(number_pad, General, 1, opt_in, set_in_game,
                No, Yes, No, Yes, NoAlias,
                "использовать цифровую клавиатуру для движения")
    NHOPTC(objects, Advanced, MAXOCLASSES, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "список символов для объектов")
    NHOPTC(packorder, Advanced, MAXOCLASSES, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "порядок предметов в вашем рюкзаке")
#ifdef CHANGE_COLOR
#ifndef MACOS9     /* not old Mac OS9 */
    NHOPTC(palette, Advanced, 15, opt_in, set_gameview,
                No, Yes, Yes, No, "hicolor",
                "палитра (настройка цвета RGB в палитре (color/R-G-B)")
#else
    NHOPTC(palette, Advanced, 15, opt_in, set_in_game,
                No, Yes, Yes, No, "hicolor",
                "палитра (00c/880/-fff — синий/жёлтый/инверсный белый)")
#endif
#endif
    /* prior to paranoid_confirmation, 'prayconfirm' was a distinct option */
    NHOPTC(paranoid_confirmation, Advanced, 28, opt_in, set_in_game,
                Yes, Yes, Yes, Yes, "prayconfirm",
                "дополнительные запросы в определённых ситуациях")
    NHOPTB(pauper, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &u.uroleplay.pauper, Term_False,
           "начинать персонажа без предметов")
    NHOPTB(perm_invent, Advanced, 0, opt_in, set_in_game,
                Off, Yes, No, No, NoAlias, &iflags.perm_invent, Term_Off,
                "показывать постоянное окно сумки")
    NHOPTC(perminv_mode, Advanced, 20, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "что показывать в постоянном окне инвентаря")
    NHOPTC(petattr, Advanced, 88, opt_in, set_in_game, /* tty/curses only */
                No, Yes, No, Yes, NoAlias, "атрибуты для подсветки питомцев")
    /* pettype is ignored for some roles */
    NHOPTC(pettype, Advanced, 4, opt_in, set_gameview,
                Yes, Yes, No, No, "pet", "ваш предпочитаемый стартовый питомец")
    NHOPTC(pickup_burden, Advanced, 20, opt_in, set_in_game,
                No, Yes, No, Yes, NoAlias,
                "максимальная нагрузка при подборе до запроса")
    NHOPTB(pickup_stolen, Behavior, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.pickup_stolen, Term_False,
           "подбирать украденные предметы")
    NHOPTB(pickup_thrown, Behavior, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.pickup_thrown, Term_False,
           "подбирать брошенные предметы")
    NHOPTC(pickup_types, Behavior, MAXOCLASSES, opt_in, set_in_game,
                No, Yes, No, Yes, NoAlias,
                "типы предметов, подбираемых автоматически")
    NHOPTC(pile_limit, Advanced, 24, opt_in, set_in_game,
                Yes, Yes, No, No, NoAlias,
                "порог для «здесь много предметов»")
    NHOPTC(player_selection, Advanced, 12, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "выбор персонажа через диалог или запросы")
 /* NHOPTC(playmode) -- moved to top */
    NHOPTB(popup_dialog, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.wc_popup_dialog, Term_False,
           (char *)0)
    NHOPTB(preload_tiles, Advanced, 0, opt_out, set_in_config, /* MSDOS only */
           On, Yes, No, No, NoAlias, &iflags.wc_preload_tiles, Term_False,
           (char *)0)
    NHOPTB(price_quotes, General, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.pricequotes, Term_False,
           "показывать известные цены неопознанных предметов")
    NHOPTB(pushweapon, Behavior, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.pushweapon, Term_False,
           "прежнее оружие идёт во вторичный слот")
    NHOPTB(query_menu, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.query_menu, Term_False,
           "использовать меню для вопросов да/нет")
    NHOPTB(quick_farsight, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.quick_farsight, Term_False,
           "пропускать просмотр карты при принудительном показе")
 /* NHOPTC(race) -- moved to top */
#ifdef MICRO
    NHOPTB(rawio, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.rawio, Term_False,
           "разрешать прямой ввод-вывод")
#else
    NHOPTB(rawio, Advanced, 0, opt_in, set_in_config,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(reroll, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &u.uroleplay.reroll, Term_False,
           "разрешать перебор стартовой сумки и предметов")
    NHOPTB(rest_on_space, Advanced, 0, opt_in, set_in_game, Off,
           Yes, No, No, NoAlias, &flags.rest_on_space, Term_False,
           "пробел привязан к команде отдыха")
    NHOPTC(roguesymset, Advanced, 70, opt_in, set_in_game,
                No, Yes, No, Yes, NoAlias,
                "загрузить набор символов отображения rogue из файла символов")
 /* NHOPTC(role) -- moved to top */
    NHOPTC(runmode, Advanced, sizeof "teleport", opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "частота отображения при «движении» или «путешествии»")
    NHOPTB(safe_pet, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.safe_dog, Term_False,
           "не давать бить питомцев")
    NHOPTB(safe_wait, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.safe_wait, Term_False,
           "не давать ждать рядом с врагами")
    NHOPTB(sanity_check, Advanced, 0, opt_in, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.sanity_check, Term_False,
           "выполнять проверки целостности данных")
    NHOPTC(scores, Advanced, 32, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "части таблицы очков, которые вы хотите видеть")
    NHOPTC(scroll_amount, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, No, No, NoAlias,
                "величина прокрутки карты при достижении scroll_margin")
    NHOPTC(scroll_margin, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, No, No, NoAlias,
                "прокручивать карту на этом расстоянии от края")
    NHOPTB(selectsaved, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &iflags.wc2_selectsaved, Term_False,
           (char *)0)
    NHOPTB(showdamage, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.showdamage, Term_False,
           "показывать получаемый урон в строке сообщений")
    NHOPTB(showexp, Status, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.showexp, Term_False,
           "показывать опыт в строке состояния")
    NHOPTB(showrace, Map, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.showrace, Term_False,
           "показывать персонажа по расе, а не по роли")
#ifdef SCORE_ON_BOTL
    NHOPTB(showscore, Status, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.showscore, Term_False,
           "показывать текущий счёт в строке состояния")
#else
    NHOPTB(showscore, Status, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(showvers, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.showvers, Term_False,
           "показывать версию в строке состояния")
    NHOPTB(silent, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.silent, Term_False,
           "не использовать звонок терминала")
    NHOPTB(softkeyboard, Advanced, 0, opt_in, set_in_config,
                Off, Yes, No, No, NoAlias, &iflags.wc2_softkeyboard,
           Term_False, (char *)0)
    NHOPTC(sortdiscoveries, Advanced, 0, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "предпочтительный порядок показа исследованных объектов")
    NHOPTC(sortloot, Advanced, 4, opt_in, set_in_game,
                No, Yes, No, Yes, NoAlias,
                "сортировать списки выбора предметов по описанию")
    NHOPTB(sortpack, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.sortpack, Term_False,
           "группировать предметы сумки по типу")
    NHOPTC(sortvanquished, Advanced, 0, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "предпочтительный порядок показа поверженных монстров")
    NHOPTC(soundlib, Advanced, WINTYPELEN, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "используемый интерфейс soundlib (если есть)")
#ifdef SND_LIB_INTEGRATED
    NHOPTB(sounds, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.sounds, Term_Off,
           "использовать встроенные звуки")
#else
    NHOPTB(sounds, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.sounds, Term_Off,
           "использовать звуки")
#endif
    NHOPTB(sparkle, Map, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.sparkle, Term_False,
           "показывать искры при сопротивлении магии")
    NHOPTB(spot_monsters, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &a11y.mon_notices, Term_False,
           "сообщение, когда герой замечает монстра")
    NHOPTB(splash_screen, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &iflags.wc_splash_screen, Term_False,
           (char *)0)
    NHOPTB(standout, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.standout, Term_False,
           "использовать выделение для --more--")
    NHOPTB(status_updates, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &iflags.status_updates, Term_False,
           "разрешать обновление строк состояния")
    NHOPTO("status condition fields", Status, o_status_cond, BUFSZ,
                opt_in, set_in_game,
                No, Yes, No, NoAlias, "изменять подсветку состояний")
#ifdef STATUS_HILITES
    NHOPTC(statushilites, Advanced, 20, opt_in, set_in_game,
                Yes, Yes, Yes, No, NoAlias,
                "0=без подсветки статуса, N=показывать подсветку N ходов")
    NHOPTO("status highlight rules", Status, o_status_hilites, BUFSZ,
                opt_in, set_in_game,
                No, Yes, No, NoAlias, "изменять подсветку строки состояния")
#else
    NHOPTC(statushilites, Advanced, 20, opt_in, set_in_config,
                Yes, Yes, Yes, No, NoAlias, "управление подсветкой")
#endif
    NHOPTC(statuslines, Status, 20, opt_in, set_in_game,
                No, Yes, No, No, NoAlias, "2 или 3 строки для отображения статуса")
#ifdef WIN32CON
    NHOPTC(subkeyvalue, Advanced, 7, opt_in, set_in_config,
                No, Yes, Yes, No, NoAlias, "заменить значение нажатия клавиши")
#endif
    NHOPTC(suppress_alert, Advanced, 8, opt_in, set_in_game,
                No, Yes, Yes, No, NoAlias,
                "подавлять предупреждения о функциях конкретной версии")
    NHOPTC(symset, Map, 70, opt_in, set_in_game,
                No, Yes, No, Yes, NoAlias,
                "загрузить набор символов отображения из файла символов")
    NHOPTC(term_cols, Advanced, 6, opt_in, set_in_config,
                No, Yes, No, No, "termcolumns", "число колонок")
    NHOPTC(term_rows, Advanced, 6, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "число строк")
    NHOPTB(terrainstatus, Advanced, 0, opt_in, set_in_game,
                Off, Yes, No, No, NoAlias, &flags.terrainstatus, Term_False,
                "показывать место героя в поле состояния")
    NHOPTC(tile_file, Advanced, 70, opt_in, set_gameview,
                No, Yes, No, No, NoAlias, "имя файла плиток")
    NHOPTC(tile_height, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, No, No, NoAlias, "высота плиток")
    NHOPTC(tile_width, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, No, No, NoAlias, "ширина плиток")
    NHOPTB(tiled_map, Advanced, 0, opt_in, set_in_game,
                tiled_map_Def, Yes, No, No, NoAlias, &iflags.wc_tiled_map,
           Term_False, (char *)0)
    NHOPTB(time, Status, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.time, Term_False,
           "показывать ходы в строке состояния")
#ifdef TIMED_DELAY
    NHOPTB(timed_delay, Map, 0, opt_out, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.nap, Term_False,
           "делать паузы для эффектов отображения")
#else
    NHOPTB(timed_delay, Map, 0, opt_in, set_in_config,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(tips, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.tips, Term_False,
           "показывать полезные советы во время игры")
    NHOPTB(tombstone, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.tombstone, Term_False,
           "показывать надгробие при смерти персонажа")
    NHOPTB(toptenwin, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.toptenwin, Term_False,
           "показывать таблицу рекордов в окне")
    NHOPTC(traps, Advanced, MAXTCHARS + 1, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "список символов для отрисовки ловушек")
    NHOPTB(travel, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.travelcmd, Term_False,
           "включать переход по щелчку мыши")
#ifdef DEBUG
    NHOPTB(travel_debug, Advanced, 0, opt_out, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.trav_debug, Term_False,
           (char *)0)
#else
    NHOPTB(travel_debug, Advanced, 0, opt_out, set_wizonly,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(tutorial, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &flags.tutorial, Term_False,
           "спрашивать, нужен ли учебник")
    NHOPTB(use_darkgray, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &iflags.wc2_darkgray, Term_False,
           "использовать чёрный вместо синего")
    NHOPTB(use_inverse, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.wc_inverse, Term_False,
           "показывать обнаруженных монстров инверсией")
    NHOPTB(use_truecolor, Advanced, 0, opt_in, set_in_config,
                Off, Yes, No, No, "use_truecolour",
           &iflags.use_truecolor, Term_False,
           (char *)0)
    NHOPTC(vary_msgcount, Advanced, 20, opt_in, set_gameview,
                No, Yes, No, No, NoAlias, "показывать больше старых сообщений за раз")
    NHOPTB(verbose, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.verbose, Term_False,
           (char *)0)
    NHOPTC(versinfo, Advanced, 80, opt_out, set_in_game,
           No, Yes, No, Yes, NoAlias, "дополнительная информация для «showvers»")
#ifdef MSDOS
    NHOPTC(video, Advanced, 20, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "способ обновления видео")
#endif
#ifdef VIDEOSHADES
    NHOPTC(videocolors, Advanced, 40, opt_in, set_gameview,
                No, Yes, No, No, "videocolours",
                "сопоставления цветов для внутренних экранных процедур")
    NHOPTC(videoshades, Advanced, 32, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "серые оттенки для чёрного/серого/белого")
#endif
#ifdef MSDOS
    NHOPTC(video_width, Advanced, 10, opt_in, set_gameview,
                No, Yes, No, No, NoAlias, "ширина видео")
    NHOPTC(video_height, Advanced, 10, opt_in, set_gameview,
                No, Yes, No, No, NoAlias, "высота видео")
#endif
#ifdef SND_SPEECH
    NHOPTB(voices, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.voices, Term_Off,
           (char *)0)
#else
    NHOPTB(voices, Advanced, 0, opt_in, set_gameview,
           Off, Yes, No, No, NoAlias, &iflags.voices, Term_Excluded,
           (char *)0)
#endif
#ifdef TTY_TILES_ESCCODES
    NHOPTB(vt_tiledata, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.vt_tiledata, Term_False,
           "выводить особые escape-коды")
#else
    NHOPTB(vt_tiledata, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
#ifdef TTY_SOUND_ESCCODES
    NHOPTB(vt_sounddata, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.vt_sounddata, Term_False,
           "выводить звуковые данные в escape-кодах")
#else
    NHOPTB(vt_sounddata, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTC(warnings, Advanced, 10, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "символы для предупреждений")
    NHOPTB(weaponstatus, Advanced, 0, opt_in, set_in_game,
                Off, Yes, No, No, NoAlias, &flags.weaponstatus, Term_False,
                "показывать оружие в руке в поле состояния")
    NHOPTC(whatis_coord, Advanced, 1, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "показывать координаты при автоописании позиции курсора")
    NHOPTC(whatis_filter, Advanced, 1, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "фильтровать координаты при наведении на следующее или предыдущее")
    NHOPTB(whatis_menu, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.getloc_usemenu, Term_False,
           "показывать меню при выборе места на карте")
    NHOPTB(whatis_moveskip, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.getloc_moveskip, Term_False,
           "пропускать тот же символ при выборе места")
    NHOPTC(windowborders, Advanced, 9, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias, "0 (выкл), 1 (вкл), 2 (авто)")
#ifdef WINCHAIN
    NHOPTC(windowchain, Advanced, WINTYPELEN, opt_in, set_in_sysconf,
                No, Yes, No, No, NoAlias, "используемый процессор окон")
#endif
    NHOPTC(windowcolors, Advanced, 80, opt_in, set_gameview,
                No, Yes, Yes, No, NoAlias,
                "цвета текста и фона окон")
 /* NHOPTC(windowtype) -- moved to top */
    NHOPTB(wizmgender, Advanced, 0, opt_in, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.wizmgender, Term_False,
           (char *)0)
    NHOPTB(wizweight, Advanced, 0, opt_in, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.wizweight, Term_False,
           (char *)0)
    NHOPTB(wraptext, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.wc2_wraptext, Term_False,
           (char *)0)

    /*
     * Prefix-based Options
     */

    NHOPTP(cond_, Advanced, 0, opt_in, set_hidden,
                Yes, No, Yes, Yes, NoAlias, "префикс для опций cond_")
    NHOPTP(font, Advanced, 0, opt_in, set_hidden,
                Yes, Yes, Yes, No, NoAlias, "префикс для опций шрифта")
#if defined(MICRO) && !defined(AMIGA)
    /* included for compatibility with old NetHack.cnf files */
    NHOPTP(IBM_, Advanced, 0, opt_in, set_hidden,
                No, No, Yes, No, NoAlias, "префикс для старых опций IBM_")
#endif /* MICRO */

#undef NoAlias
#undef NHOPTB
#undef NHOPTC
#undef NHOPTP
#undef NHOPTO

/* *INDENT-ON* */
/* clang-format on */
#endif /* NHOPT_PROTO || NHOPT_ENUM || NHOPT_PARSE */

/*optlist.h*/
