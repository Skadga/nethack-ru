# NetHack 5.0.0 RU — Состояние перевода

Дата последнего обновления: 2026-10-15
Ветка: main
Последний коммит: f10f3bf

## Сборка

- Репозиторий: C:\Users\user\Desktop\nethack-500\NetHack-5.0.0
- Исходники: CP1251, CRLF. Файлы .lua — CP1251 (117), UTF-8 (14).
- Сборка: MSBuild Release|x64, sys\windows\vs\NetHack.sln
- Пакет: binary\Release\x64\nethack-500-win-x64.zip
- Эталон: C:\Users\user\Desktop\nethack-500-src.tgz

ВАЖНО:
- /t:Rebuild и /t:Clean ЗАПРЕЩЕНЫ — FetchPrereq удаляет lib\pdcursesmod и lib\lua.
- Инструмент edit НЕ использовать на .c/.h/.lua — портит CP1251.
  Только побайтово через Python.
- git checkout только после коммита.
- После правки struct — grep всех инициализаторов + ЗАПУСК игры,
  не только сборка. (Баг his_up стоил двух дней.)
- После правки игровых строк — grep всех инициализаторов + сборка
  /t:Build. Запуск игры проверяет человек, когда удобно.
- Побайтовый скрипт: сначала все проверки (CRLF, FFFD, размер),
  ПОТОМ запись в файл. Если assert после записи — файл уже испорчен.

## Что сделано

- dat/*.lua: ~700 правок (montype, id, des.monster, des.object, u.role)
- src/sp_lev.c: русские префиксы в class_prefixes[]
- s_suffix → mhis/mhis_up: ~40 правок (uhitm, weapon, zap, eat, dog,
  hack, worm, timeout, apply, insight, mon, muse, shk, read, trap, do)
- obj_case() + arti_paradigms[] + obj_paradigms[]: падежи предметов
- god_gen() + god_gen_tab[37]: падежи богов (pray, minion, priest)
- noun_case(): падежи произвольных имён
- body_part_case() + bp_paradigms[141]: падежи частей тела
- ru_makeplural: 151 пара
- globals: genders[] his_up — критический баг исправлен
- mon.c: 12 правок сообщений (mon_give_prop×6, mon_givit, mpickgold,
  mpickstuff, vamprises, wake_msg, meatobj:1644) — CP1251, diff чистый
- mon_gen() (do_name.c) + mon_paradigms[67]: падежи монстров, ключ —
  номинатив mon_pmname(), фолбэк на номинатив. Прототип в extern.h.
  Заменены s_suffix: uhitm:1280, mhitm:629, mhitu:1716, zap:420,
  potion:1662, pray:798
- s_suffix batch A2+B (закоммичено 7e38932): mon_gen + рус. заменители
  (mpoisons_subj рус. муж. сущ., poisoned — род по первому слову).
  Файлы: mhitu, uhitm, explode, attrib. Итого ~21 место из ~50.
- Batch 1 (закоммичено 6e273b8): apply, detect, lock, mhitm, mthrowu,
  polyself, shk, timeout, trap — 17 мест s_suffix.
- s_suffix B-остаток (закоммичено b74a0b5): apply.c:851 («хвост <род>»),
  potion.c:1712 («седло <род> светится/намокает», cxname+otense).
- s_suffix плоть + const (закоммичено 7a53987): uhitm
  hmon_hitmon_msg_silver/_lightobj — «плоть <род>» через mon_gen(BP_GEN),
  флаг flesh, fem-шаблон для невидимой ветки; noncorporeal/amorphous не
  тронута; uhitm.c:6334 → const char *hands, *hands_ins (C4090).
- s_suffix плоть в mthrowu.c:428 (работа в процессе): то же через
  mon_gen(mtmp, BP_GEN), flesh считается по mtmp->data (420-421).
- s_suffix — КАМПАНИЯ ЗАКРЫТА (7 коммитов: fccb005, 7e38932, 6e273b8,
  b74a0b5, 7a53987, 3150033, 7d9a948). Закрыто ~35 мест из ~50.
  Остаток — только отложенные/не-монстры.
- bp_paradigms: перчатки/рукавицы/ботинки (plural-only) + noun_case в
  fingers_or_gloves (закоммичено 1c1644f)

## Что осталось

### Высокий приоритет

1. mon_gen() — таблица падежей монстров (do_name.c). Сделано: каркас +
   67 частых имён (фолбэк на номинатив). Осталось наполнять по мере
   новых вызовов; NAMS-монстры — см. п.16.

2. s_suffix — КАМПАНИЯ ЗАКРЫТА. Остатки в п.9/10/12/16/18/20/22
   и не-монстры (files.c:3215, questpgr.c:389).

### Средний приоритет

3. questpgr.c — конверсии %G/%V (падежи для текстов квестов).
   questpgr.c:389 — s_suffix на convert_arg.

4. vtense — plural-only (волосы, перья, соки, реснички, споры),
   английские глаголы (feel, tingle, sting). Отдельные места.

5. Энциклопедия dat/data — английские описания богов и монстров
   (Huan Ti, Issek, Kos, Mog, Ishtar, tribute).

### Низкий приоритет

6. name (86 латинских) — квест-артефакты на английском
   (The Orb of Detection, Excalibur, The Longbow of Diana).

7. wish-парсер: конфликт лук (оружие) / лук-овощ.

8. масс-существительные (кровь, рябь, шерсть) — без plural-форм.

9. gloves_simple_name/boots_simple_name — нет параметра падежа,
   14 вызовов.

10. shtypes[] — падежи ЧАСТИЧНО ЗАКРЫТЫ: есть shoptype_case() +
    shoptype_name() в do_name.c, таблица shoptype_paradigms[12] (ключ =
    annotation, для «универсальный магазин» = name). Закрыто 3 из 9:
    mon.c:3024 (BP_GEN + перестройка «владелец/владелица» → тип),
    shk.c:821 (BP_ACC), shk.c:2020 (BP_PREP). Остальные 6 — группа B,
    ждут п.18: shk.c:712/817/831/847/851/1028 (там s_suffix(shkname)).

11. fingers_or_gloves() — падежи перчаток/рукавиц/ботинок СДЕЛАНЫ
    (1c1644f): 3 plural-only записи в bp_paradigms[] (перчатки,
    рукавицы, ботинки) + noun_case в do_wear.c. Осталось:
    fountain.c:568/727: fingers_or_gloves(BP_INS) — текст в
    именительном, разобрать отдельно.

12. strsubst/кириллица — центральный фикс highc/lowc СДЕЛАН
    (без коммита; сборка build10: 0 ошибок, 0 warning по hacklib.c):
    hacklib.c highc/lowc/lcase/ucase расширены на CP1251
    (а-я 0xE0-0xFF ↔ А-Я 0xC0-0xDF, ё 0xB8 ↔ Ё 0xA8).
    lcase/ucase теперь через lowc/highc; strstri (hacklib.c:754)
    использует lowc → складывает регистр кириллицы.
    Сам strsubst — байтовый strstr, на кириллице работает (точные совпадения).
    ОТЛОЖЕНО:
    - Windows: strcmpi/strncmpi = MSVC _stricmp (config1.h:141, windconf.h:170),
      не знает кириллицу. Решение: отключить STRNCMPI для Win32 и использовать
      hacklib. Отдельная задача.
    - Мёртвые LAT-шаблоны strsubst (~40 мест): botl.c:478, cmd.c:654/2609,
      dungeon.c:3788-3791, files.c:1818/1825, insight.c:1622,
      mkobj.c:1657/3439, o_init.c:680, objnam.c:3349/3354/4565/4569/4573,
      options.c:3125/3127/5985, pager.c:2214-2218, polyself.c:644,
      role.c:1606, stairs.c:232, steal.c:309/312/313/317,
      timeout.c:146/212/228/337/415, topten.c:1050, trap.c:2775.
      Шаблон латиницей — на русском тексте no-op. Кампания.
    - strsubst CYR (9 мест, работают): botl.c:465, do_wear.c:632,
      dungeon.c:2575, getpos.c:153, invent.c:5436/5438, muse.c:279,
      steed.c:867, zap.c:4578 — НЕ трогать.
    - strstri prefilter (TABSIZ=0x20, `& 0x1F`): а/А коллизируют (0xC0/0xE0),
      но Ё/ё (0xA8/0xB8) попадают в разные бакеты → strstri может пропустить
      Ё/ё-совпадение. Мелочь, пока не трогать.

13. noun_case() — линейный поиск. При расширении — хеш.

14. resнички/споры → обычная парадигма (правило plural-only).

15. 9 латинских имён богов — переведены в role.c, но
    оригиналы в artilist.h остались. Проверить согласованность.

16. mon_gen: NAMS-монстры (15 типов, дварф-лорд/леди и т.п.) —
    падежи по роду, отдельная партия. Затрагивает и mon_gen_nom():
    rndmonnam() может вернуть «дварф-леди» и т.п. → нет в таблице →
    фолбэк на номинатив (explode.c:499/598).

17. l_monnam: падеж для «на <монстра>» — 3 места в apply.c:
    855, 859 (успешная привязка), 877 (снятие; там ещё англ. «your »).
    l_monnam() даёт им. падеж; для «на …» нужен вин. (mon_gen BP_ACC).

18. shkname() — личные имена лавочников (Izchak и т.п.) латинские,
    падежей нет. Места shk.c:5898 (shk_owns), zap.c:5554,
    shk.c:711/817/831/847/850/1027 — ЗАБЛОКИРОВАНО до решения по
    личным именам. shkname-места (shk.c:712, 817, 831, 847, 851, 1028,
    zap.c:5554) — ждут личных имён монстров (п.22 объектной кампании
    тоже туда). Варианты: (C) таблица личных имён → родительный
    (десятки имён), либо (B) кавычки в им. падеже (компромисс).

19. mthrowu.c:431 — «облянойкова» — сломанная строка, видимая игроку
    (ветка невидимой цели: `pline("%s облянойкова!", ...)`).

20. debug-строки: mkobj.c:2433 (и другие debugpline*) — английские,
    ОТОЧЕНИЯ, отложены.

21. mon_nam() как прямое дополнение даёт ИМ. падеж: mthrowu.c:429 при
    flesh=FALSE (noncorporeal/amorphous) — «Серебро обжигает кобольд»
    вместо «кобольда». Существующий баг, нужен mon_gen(BP_ACC) —
    отдельная кампания.

22. objnam.c:1853 — блок: (1) расширить mon_gen уникальными именами
    (медуза, оракул, влад, кроис, ...), (2) перестроить порядок
    «possessive+adj+mnam+труп» → «adj+труп+имя-в-род.».

## Как продолжить

1. Открой этот файл, прочитай разделы «Сделано» и «Осталось».
2. Запусти ассистента с контекстом: "NetHack RU, продолжаем.
   Состояние в PROGRESS.md. Начни с пункта X."
3. Следуй правилам «СБОРКА» выше — они критичны.

## Полезные ссылки

- Оригинал: C:\Users\user\Desktop\nethack-500-src.tgz
- Распакованный оригинал: C:\Users\user\AppData\Local\Temp\opencode\nhsrc\
- PDB (символы): binary\Release\x64\NetHackW.pdb
- paniclog: C:\Users\user\NetHack\5.0\paniclog
- procdump для дампов: Temp\opencode\dumps\

## Журнал коммитов s_suffix

- fccb005 — mon_gen: падежи монстров (67 имён), 7 правок s_suffix
- 7e38932 — s_suffix: Batch A2+B (attrib, explode, mhitu, uhitm)
- 6e273b8 — s_suffix: Batch 1 (apply, detect, lock, mhitm, mthrowu,
  polyself, shk, timeout, trap)
- b74a0b5 — s_suffix: B-остаток (apply.c:851, potion.c:1712)
- 685f5f3 — PROGRESS.md: журнал коммитов s_suffix
- 8bf804a — PROGRESS.md: apply/potion закрыты
- 974cd40 — PROGRESS.md: shkname-блок отложен (п.10+18)
- 7a53987 — uhitm: плоть %s (mon_gen), const для body_part_case
- c1655fa — PROGRESS.md: uhitm плоть+const закрыты, остаток s_suffix
- 3150033 — mthrowu: плоть %s; PROGRESS: objnam отложен (п.22)
- 7d9a948 — explode/mon_gen: mon_gen_nom (lookup по строке), «взрыв %s»
- 808b140 — shtypes: shoptype_case/name, группа A (3 места)
- 1c1644f — bp_paradigms: перчатки/рукавицы/ботинки (plural-only),
  noun_case в fingers_or_gloves
Все батчи s_suffix закоммичены, рабочее дерево чистое (кроме PROGRESS.md).