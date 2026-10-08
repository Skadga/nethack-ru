# NetHack 5.0.0 RU — Состояние перевода

Дата последнего обновления: 2026-10-08
Ветка: main
Последний коммит: 06d16625

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

## Что осталось

### Высокий приоритет

1. mon_gen() — падежи монстров, 36 мест в s_suffix
   (mhitm, mhitu, shk, uhitm, trap, potion, zap, explode, polyself и др.)
   Нужна таблица ~500 имён × 5 падежей. Большая кампания.

2. s_suffix остаток: 63 строки в 22 файлах
   - 36 ждут mon_gen
   - 5 ждут падежей предметов (apply.c:1689, dothrow.c:2124/2296/2677)
   - 9 не s_suffix (комментарии, #if 0, Lua API)
   - ~13 прочее

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
    shk.c:2015-2023.

11. fingers_or_gloves() — нет полиморфизма.

12. strsubst на кириллице не работает (trap.c:2777).

13. noun_case() — линейный поиск. При расширении — хеш.

14. resнички/споры → обычная парадигма (правило plural-only).

15. 9 латинских имён богов — переведены в role.c, но
    оригиналы в artilist.h остались. Проверить согласованность.

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