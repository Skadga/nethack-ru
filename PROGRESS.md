# NetHack 5.0.0 RU — Состояние перевода

Дата последнего обновления: 2026-10-08
Ветка: main
Последний коммит: b74a0b5

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

## Что осталось

### Высокий приоритет

1. mon_gen() — таблица падежей монстров (do_name.c). Сделано: каркас +
   67 частых имён (фолбэк на номинатив). Осталось наполнять по мере
   новых вызовов; NAMS-монстры — см. п.16.

2. s_suffix — batch A2+B и B-остаток (apply/potion) закрыты
   (~23 места из ~50). Остаток (29):
   - shkname: shk.c:711/817/831/847/850/1027/5898 + zap.c:5554 — 8 мест
     (следующий крупный блок; проверить shkname() на параметр падежа)
   - отложено (E): mhitm.c:622, mhitu.c:600, uhitm.c:1696/1716/1727
     (плоть / объект hmd->saved_oname), trap.c:2770
   - explode.c:499/598 — галлюцинация
   - не-монстры: cmd.c:4478 (#if0), end.c:241 (комм.), mhitm.c:618/619
     (комм.), files.c:3215, mkobj.c:2433, mthrowu.c:428, nhlua.c:881-888
     (Lua API), objnam.c:1853, questpgr.c:389

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

10. shtypes[] — нет падежей (13 типов лавок).
    shk.c:2015-2023. mon.c:3024 — ждёт родительного падежа shtypes[].name.
    shk.c:711/817/831/847/850/1027, zap.c:5554 — ждут падежей
    shtypes[].name + личных имён лавочников.

11. fingers_or_gloves() — нет полиморфизма.

12. strsubst на кириллице не работает (trap.c:2777).

13. noun_case() — линейный поиск. При расширении — хеш.

14. resнички/споры → обычная парадигма (правило plural-only).

15. 9 латинских имён богов — переведены в role.c, но
    оригиналы в artilist.h остались. Проверить согласованность.

16. mon_gen: NAMS-монстры (15 типов, дварф-лорд/леди и т.п.) —
    падежи по роду, отдельная партия.

17. l_monnam: падеж для «на <монстра>» — 3 места в apply.c:
    855, 859 (успешная привязка), 877 (снятие; там ещё англ. «your »).
    l_monnam() даёт им. падеж; для «на …» нужен вин. (mon_gen BP_ACC).

18. shkname() — личные имена лавочников (Izchak и т.п.) латинские,
    падежей нет. Места shk.c:5898 (shk_owns), zap.c:5554,
    shk.c:711/817/831/847/850/1027 — ЗАБЛОКИРОВАНО до решения по
    личным именам. Варианты: (C) таблица личных имён → родительный
    (десятки имён), либо (B) кавычки в им. падеже (компромисс).

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
Оба батча (A2+B и Batch 1) закоммичены, рабочее дерево чистое
(кроме данного PROGRESS.md).