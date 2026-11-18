# NetHack 5.0.0 RU — Состояние перевода

Дата последнего обновления: 2026-11-17
Ветка: main
Последний коммит: f58d93b

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
- nethack_strnicmp (hacklib.c) — всегда компилируется, не под STRNCMPI.
  strncmpi — обёртка для не-Windows. Редиректы strncmpi/strcmpi/strnicmp
  в windconf.h (только WIN32).
- Перед побайтовой правкой — dry-run: проверять счётчик вхождений маркера
  и отступ строки ДО записи; при сомнении — сохранять бэкап .bak в temp.

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
- highc/lowc расширены на CP1251 (а-я↔А-Я, ё↔Ё); strstri/lcase/ucase
  заработали на кириллице (закоммичено f10f3bf)
- strcmpi/strncmpi/strnicmp на Windows → nethack_strnicmp
  (CP1251-aware). Закрыто 509 вызовов + objnam.c:4015/4019/4333
  (закоммичено b12584e)
- helm_case() (objnam.c) + 2 записи в bp_paradigms[] (шлем/шляпа, оба
  рода) — падежи шлема/шляпы через noun_case. Применено в 6 местах:
  mhitu.c:601 (BP_GEN), hack.c:3485 (BP_DAT), do_wear.c:1943/2073,
  mhitu.c:2115, sounds.c:1452 (BP_ACC). Прототип в extern.h.
  Сборка build12: 0 ошибок (без коммита).
- s_suffix полностью закрыт (кроме shkname/группы B, ждёт п.18):
  mhitm.c:613-635 рерайт шаблона (см. п.24). build15: 0 ошибок.
- vtense — КАМПАНИЯ ЗАКРЫТА (39e3927): 6 мест (wield/do/mkobj/makemon/
  mcastu), глаголы по числу. Остаток — locomotion()/stagger() в mondata.c.
- Английские видимые литералы, батч №1: ball.c (5), explode.c (4),
  region.c (4), sounds.c (3) — 16 мест. Сборка Release x64: 0 ошибок.
- Батч №2 (35aeaf1): mkobj.c — alteration_verbs[] (20 рус. глаголов),
  «Вы <глагол> это/эти ..., вы платите за него/них», глоб
  растворяется/сжимается, контейнер легче, «Оно/Они светится/светятся
  ярче/тусклее»; priest.c:130 + wizard.c:444 «подбирает»; steed.c
  «падает в воду»/«соскальзывает в лаву»; invent.c подсказка команды.
- Батч №3 (dc425cc): attrib.c — plusattr/minusattr/attrname/exertext,
  «<Атрибут>: врождённое значение улучшилось/ухудшилось», «на пределе»,
  «чувствуете себя очень сильными», «должно быть, ...»/«видимо, не ...»;
  apply.c — свисток (Deaf-ветка), потрескивает/мерцает, загорается/
  кажется тёплым, «не водолазная лампа», verbalize без itself; pager.c —
  «не показано ни одного ...», «и это сделали вы», «Спросите о '&'»,
  «Какая команда?».
- Батч №4: attrib.c — gainstr/losestr в творительном падеже, шаблоны
  «себя %s!»/«себя менее %s!»; wizard.c — амулет обжигает/тёплый, «с
  ужасом чувствуете...», «замечаете ... сияние», «злобно смеётся»;
  wizcmds.c — принюхивание, «не издаёте запаха», «Таймаут для»,
  «Мигрируют:»; mail.c, bones.c; engrave.c — everb2 (1-е лицо),
  eloc=surface_ru, иней/пыль, «заканчиваете %s»; o_init.c — меню
  открытий; dig.c — surface_ru; end.c, exper.c, hack.c (люк/дыра),
  mplayer.c (реплики), polyself.c, read.c, options.c, cfgfiles.c,
  files.c. Сборка Release x64: 0 ошибок, новых warning'ов нет.
- Батч №5: mkmaze.c — диагностика пузырей («пузырь: x = ... < xmin»),
  nhlua.c — «ТЕСТ:»/«Сброс времени на ход #%ld.», shknam.c — «Комната в
  ...»/«дверь [...]», display.c — «проблемы с режимом стен». Сборка
  Release x64: 0 ошибок.
- Батч №6 (on_fire): on_fire() в mondata.c получил параметр `you` и
  возвращает русские формы («уже горит/горите», «кипит/кипите»,
  «плавится/плавитесь», «раскаляется/раскаляетесь», «зажаривается/
  зажариваетесь», «горит/горите»); uhitm.c×3, mon.c — выбор глагола
  для лавы по русским строкам («выкипает/расплавляется/сгорает
  дотла»); extern.h. Сборка Release x64: 0 ошибок. Кампания
  английских литералов закрыта.
- Батч №7 (locomotion/stagger): locoverbs[] в mondata.c переведены
  («парит», «летит», «ползёт», «растекается», «извивается»; для
  stagger — «покачивается/порхает/пошатывается/спотыкается/дрожит/
  пульсирует»); def-аргументы русские (monmove.c «двигается»,
  muse.c «прыгает»); makeplural убран; monverbself() больше не
  принимает русский глагол за множественное число. Сборка Release
  x64: 0 ошибок.
- Батч №8 (0c4f17a): fountain.c — fingers_or_gloves() в именительном
  падеже; trap.c — helm_material_ru() (родовые формы прилагательных по
  hard_helmet), «рубашка» как подлежащее в water_damage(); zap.c —
  item_what() даёт « (шлем)» без «вашей»; apply.c — snuff_candle() без
  английских 's/is/are («Пламя свечи/свечей/подсвечника гаснет.»);
  shk.c — лавки «Имя» с падежом shoptype_case(shoptype_name(...)),
  shk_owns() вместо s_suffix() возвращает «неоплаченный». Сборка
  Release x64: 0 ошибок, новых warning'ов нет.
- Батч №9 (энциклопедия): dat/data.base — переведены статьи богов
  huan*ti, ishtar, issek, izchak, kos, mog; к английским ключам
  добавлены русские синонимы («хуан-ди», «хуан ди», «иштар», «изек»,
  «изак», «кос», «мог»). util/makedefs.c: do_data() сравнивал
  `*line > ' '` при signed char — кириллические ключи попадали в
  текст; теперь `(uchar) *line > ' '`. Сборка Release x64: 0 ошибок,
  dat/data перегенерирован, русские ключи в секции ключей с верными
  смещениями.
- Батч №10 (147c985): apply.c use_leash_core() — падежи монстра через
  mon_gen(BP_ACC/BP_DAT/BP_GEN), убран английский "your ", «Не удаётся
  привязать поводок к …», «… уже на поводке», «Поводок не наденется
  на …», «снимаете поводок с …». l_monnam в apply.c больше не нужен.
  Сборка Release x64: 0 ошибок, новых warning'ов нет.
- Батч №11 (f58d93b): do_name.c mon_paradigms[] +32 записи (67→99).
  Составные имена (27): дварф/кобольд/гном/огр/вампир/эльф + лорд,
  леди, вождь, король, королева, правитель, тиран, знатный —
  склоняются обе части через дефис, «леди» не склоняется. Простые (5):
  жрица, клирик, верховный жрец, верховная жрица, верховный клирик
  (последние три — через пробел). Сборка Release x64: 0 ошибок.
- Батч №12 (11db990): dat/quest.lua — секция common (46 замен):
  angel_cuss, banished, demon_cuss, quest_complete_no_bell,
  quest_portal, quest_portal_again, quest_portal_demand.
- Батч №13 (b126caa): dat/quest.lua — секция Arc (29 ключей).
- Батч №14 (c04650d): dat/quest.lua — секции Bar и Cav (по 29 ключей).
- Батч №15 (ac86fb2): dat/quest.lua — секции Hea и Kni (по 28 ключей).
- Батч №16: dat/quest.lua — секции Mon и Pri (по 28 ключей).
  В текстах квестов работают %p (имя игрока), %c, %r, %R, %s, %S,
  %l, %n, %i, %o/%O, %H, %d, %a, %g, %Z и суффиксы C/c, h/H, i/I,
  j/J (род); непереносимые %na/%ns/%ls/%ra/%ca/%lt/%shood и суффиксы
  a/A, p/P, s/S, t заменены перестройкой фраз.
- Батч №17 (2903239): dat/quest.lua — секция Ran.
- Батч №18 (89b64dc): dat/quest.lua — секция Rog.
- Батч №19 (515941b): dat/quest.lua — секция Sam (29 ключей).
- Батч №20 (782c858): dat/quest.lua — секция Tou (29 ключей).
- Батч №21 (8f0fb14): dat/quest.lua — секция Val (28 ключей).
- Батч №22 (10106af): dat/quest.lua — секция Wiz (29 ключей).
  quest.lua переведён полностью: common + все 13 ролей. Во всех
  секциях учтены %p, %c, %r, %R, %s, %S, %l, %n, %i, %o/%O, %H, %d,
  %Z и суффиксы C/c, h/H, i/I, j/J; непереносимые %x, %a, %gP, %oh,
  %nh, %oC, %oiself, %ns, %ls, %ra, %ca, %ds и суффиксы a/A, p/P,
  s/S, t заменены перестройкой фраз. Сборка Release x64: 0 ошибок,
  nhdat500 содержит русские строки; Android синхронизирован (fc /b).
- Батч №23 (fc31c55): cmd.c (doextlist — «вымерли»), options.c
  (perminv_modes + сокращения в strsubst), steal.c (worn_item_removal —
  русские суффиксы статуса), timeout.c (stoned/vomiting/sickness/slime —
  русские strsubst/strstri), polyself.c (« из чешуи дракона»),
  pager.c (add_quoted_engraving, look_engrs — «могила/надпись»).
  Сборка 0 ошибок, 19 прежних warning'ов; Android синхронизирован.
- Батч №24 (86f4963): pager.c — описания клетки (look/farlook).
  lookat: «[видно: …]», «внутренность <монстр, род.>» (mon_gen),
  «тёмная часть комнаты», алтарь. add_cmap_descr: убран мёртвый блок
  "water"/article, «ловушка». «внутренность монстра»/«неразведано»,
  «вы», «неисследовано»/«суша», «вместе с валуном», «может быть
  многое», ice_descr по is_ice() и «лёд», «заблокированная лестница
  вниз», «Дополнительная информация о "…"?», «Что именно? (введите
  слово)», what_is_a_location в дательном, списки /M и /T, «, скрыто
  %s», suptext1/2, docontact. Сборка 0 ошибок, 19 прежних warning'ов;
  Android синхронизирован.
- Батч №25 (7bbd21a): xcalled — «под названием»; end.c — «по имени»;
  topten — русская фраза смерти без английской сводки, «, while» →
  «, %s», перевод сводок и локаций; stairs.c — «лестница вверх/вниз»,
  «ответвление: …», «ветка: … (…)». Удалены мёртвые английские
  strsubst в dungeon.c, role.c, mkobj.c, botl.c. Сборка 0 ошибок,
  19 прежних warning'ов; Android синхронизирован.
- Батч №26 (82bdbc6): options.c — paranoia[].explain (13), menutype и
  msgwind, objsymvals, default_menu_cmd_info, меню disclosure
  (disclosure_labels[] для показа, disclosure_names[] оставлен
  английским из-за проверок v/g), заголовки меню, msgtype_names,
  show_menu_controls, dotogglepickup («ВКЛ/ВЫКЛ»). Сборка 0 ошибок,
  19 прежних warning'ов; Android синхронизирован.
- Батч №27 (e6370f5): shk.c — суммы через currency() («зоркмид»),
  убраны «gold piece%s»/«zorkmid%s»; worn.c — «другой <броня>» через
  Snprintf вместо пустого strsubst; dungeon.c — «(ресурс)!», «из»,
  «(файл)!». Сборка 0 ошибок, 19 прежних warning'ов; Android
  синхронизирован.
- Батч №28 (b767ae8): botl.c — состояния в статусной строке и
  conditions[] («Кам, Слиз, Удуш, …»), hutxt[] и aligntxt[] приведены
  к hu_stat[]/align_str() (подсветка снова совпадает), служебная
  строка «ничего из перечисленного (оборотень)», «порог hilite_status
  … вне диапазона», «процентное значение». Сборка 0 ошибок,
  19 прежних warning'ов; Android синхронизирован.
- Батч №29 (551d8aa): cmd.c — исправлено «размер обла%ss» →
  «размер %s области»; параноидальные подтверждения по-русски
  («[да|нет|выход]», «[да|н|в] (н)», ввод без учёта регистра,
  принимаются да/нет/выход и yes/no/quit); подсказка '&' для
  префикса reqmenu снова срабатывает. invent.c — заголовки #adjust
  («Собираем:/Перемещаем:/Разделяем:/Объединяем:/Меняем:»), метка
  «Кольцо». Сборка 0 ошибок, 19 прежних warning'ов; Android
  синхронизирован.
- Батч №30 (7c9404c): mon.c — «Окаменевающий … растекается по/
  оседает на/корчится на» (surface_ru), «Пусть … изжарится/
  заржавеет/сгниёт с миром», угорь «ныряет под воду», обычные
  «прячется под», выход из проглотившего «выбираешься наружу/
  вырываешься наружу» («из брюха чудовища»), удалена l_oldname,
  отладочный вопрос «(вид монстра)?», livelog «(2-й раз)»,
  «Больше не защищён от окаменения, ». eat.c — «будущий изящный
  шедевр»/«будущий арт». Сборка 0 ошибок, 19 прежних warning'ов;
  Android синхронизирован.
- Батч №31 (554a142): end.c «Содержимое %s:»; mail.c «Голос
  гремит по пещерам (%s):», «Системная команда (%s)»; sit.c
  «пошевелиться, чтобы сесть»; options.c отключение предупреждений
  и ошибка о будущей версии; apply.c «тереть о камень»; sounds.c
  три сообщения о файле/синтаксисе SOUND; topten.c «Не найдено …
  записей для …», имена через запятую, русский Usage. Сборка
  0 ошибок, 19 прежних warning'ов; Android синхронизирован.
- Батч №32 (fce974b): ошибки конфигурации — cfgfiles.c (24
  сообщения), coloratt.c, earlyarg.c, glyphs.c, files.c; все
  config_error_add переведены на русский. Сборка 0 ошибок,
  19 прежних warning'ов; Android синхронизирован.
- Батч №33 (4077772): options.c — 102 литерала сообщений разбора
  параметров (неизвестные/неоднозначные параметры, недопустимые
  значения, symset, hilite_status, palette, привязки клавиш, MSGTYPE,
  windowcolors); ключевые слова конфигурации оставлены латиницей.
  Сборка 0 ошибок, 19 прежних warning'ов; Android синхронизирован.
- Батч №34 (d303d28): 16 файлов, 40 мест — из Tobjnam/Yobjnam2/otense
  убраны английские глаголы (are/have/glow/shine/slip/quiver/drop/
  land/hit/vanish/crumble/trigger/slam/resist/fail/go/burn/flicker/
  die/seem/look/radiate/issue/confuse/damage/assault/zap/explode/
  implode/tumble/heat/get/suddenly roll). Где фраза уже содержала
  русский глагол — The(xname())/Yname2(), где нет — фраза
  перестроена. hcolor() получает русские формы, NH_AMBER/NH_BLACK
  заменены. read.c p_glow3() разделён для слепого и зрячего. Сборка
  0 ошибок, 19 прежних warning'ов; Android синхронизирован.
- Батч №35 (679ccd3): files.c — 82 пары служебных сообщений (уровни,
  bones, (рас)упаковка, таблица состояний файла сохранения, блокировка
  fcntl, таблица рекордов, восстановление, пути и конфигурация,
  tribute); «%s is %s %s» → «%s: %s». read.c — «Округа становится
  [не]дружелюбнее». Удалены мёртвые английские аргументы. Сборка
  0 ошибок, 19 прежних warning'ов; Android синхронизирован.
- Батч №36 (565a8ca): 11 файлов — cfgfiles.c (доступ и файлы
  конфигурации, SYSCF_FILE), earlyarg.c (-u, -version), end.c (паника
  и отчёт об ошибке), explode.c (русская таблица типов взрывов,
  флаг str_gen для родительного «взрыва <монстр>», причины смерти без
  падежей, killer.name «взрыва %s»), mail.c, symbols.c, sounds.c,
  options.c (подсветка статуса), topten.c, version.c, windows.c.
  Сборка 0 ошибок, 19 прежних warning'ов; Android синхронизирован.
- Батч №37 (2292ee0): dat/data.base — 204 английских ключа-названия
  монстров заменены на русские по таблице pmnames (pager.c ищет по
  pmnames[NEUTRAL] в нижнем регистре, поэтому раньше статьи монстров
  были недоступны). Тексты статей пока английские. Совпадения ключей:
  обитатель пещер (4), эльфийский монарх (3), любвеобильный демон (3),
  медуза (2). Сборка 0 ошибок, 19 прежних warning'ов; makedefs дал
  211 кириллических ключей; Android синхронизирован.
- Батч №38 (ee80d57): apply.c (no_elbow_room, nothing_else_happens),
  attrib.c (because_of → « (источник: %s)», «зелье или заклинание»,
  «надетое снаряжение»), role.c (метки пролога имя/класс/раса/пол/
  мировоззр.: и значения « — выбор», « — не указано», « — случайно»),
  wizcmds.c («Вы%s %s.», «всё ещё», «превращаетесь в слизь/камень»,
  «тошните»). Сборка 0 ошибок, 19 прежних warning'ов; Android
  синхронизирован.
- Батч №39 (a463fbe): polyself.c — mbodypart() спецслучаи переведены
  (лапа, лапый, задняя лапа, коготь/когтистый, хобот, кожа, щупальце,
  роговица, лучистый/луч, голова, загадочная часть); у собак/кошек/
  грызунов/совомедведя ARM/LEG берут animal_parts. insight.c —
  исправлено перевёрнутое условие «обычно» в сообщении о правше/левше.
  Сборка 0 ошибок, 19 прежних warning'ов; Android синхронизирован.
- Батч №40 (7a2ba7e): botl.c — terrain_descr[] и c_Wall переведены
  (Камень, Стена, Решётка, Дерево, Вода, Ров, Лава, Лава-стена,
  Прутья, Проём, Коридор, Комната, Лестница, Стремянка, Фонтан, Трон,
  Раковина, Могила, Алтарь, Лёд, Мост, Воздух, Облако, Пол, Земля,
  Откр./Закр. дверь, Болото, Под водой, Море, Вода-стена). Видно при
  опции terrainstatus. Сборка 0 ошибок, 19 прежних warning'ов; Android
  синхронизирован.
- Батч №41 (a2915f6): wizcmds.c — промпты и заголовки wizard-команд
  (#wizkill, #wizloadlua/#wizloaddes, #wizfliplevel, #wizlevelchange,
  #wizhurtle, #wizpanic, #wizfuzzer, #wizmap + флаги уровня, #wizintrinsic,
  #wizshowmigrating, статистика памяти, #wizdispmacros, #wizmondiff,
  #wizcustom); убраны английские adjs в #wizkill (нужно согласование по
  роду); мёртвое сравнение "unreachable" заменено на strcmp с
  «недоступно/некопаемо». report.c — «Собираю дополнительные сведения
  для отчёта:» в обеих ветках. options.c — закомментированный текст.
  Сборка 0 ошибок, 19 прежних warning'ов; Android синхронизирован.
- Батч №42 (d3074ca): dat/data.base — переведены 13 статей
  (aclys/aklys/thonged club, agate, alchemy, алеакс, altar/offer/
  sacrific, amaterasu omikami, amber, amnesia, amulet of yendor ×2,
  angel, angry god, anhur). botl.c — hilite_status снова принимает
  английские ключи: имена состояний (condition_cfg_names[] в том же
  порядке, что conditions[]), satiated/hungry/weak/fainting/fainted/
  starved (hutxt_en[]) и chaotic/neutral/lawful (aligntxt_en[]);
  русские названия остались. Сборка 0 ошибок, 19 прежних warning'ов;
  Android синхронизирован.
- Батч №43 (2975f9a): dat/data.base — переведены тела 54 статей
  (ant, anu, архон, axolotl, bee, костяной дьявол, бугбер,
  хроматический дракон, citrine, коатль, койот, разочарователь,
  смещающийся зверь, собака, элементаль, эттин, Очи Верхнего мира,
  fruit, великан, gunyoki, шлемы телепатии/противоположного
  мировоззрения/осторожности, гезру, рогатый дьявол, humanoid,
  ледяной дьявол, инкуб/суккуб, жуиблекс, katana, ки-рин, koto,
  light, longbow of Diana, лорд Сато, lurker, magicbane, mail daemon,
  марилит, master key of thievery, Меркурий, мимик, свежеватель
  разума, mitre of holiness, нага, налфешни, неандерталец, ninja-to,
  Orb of Detection, Оркус, осаку, совомедведь, ямный дьявол,
  platinum yendorian express card). Сборка 0 ошибок, 19 прежних
  warning'ов; Android синхронизирован.

## Что осталось

### Высокий приоритет

1. mon_gen() — таблица падежей монстров (do_name.c). Сделано: каркас +
   67 частых имён (фолбэк на номинатив). Осталось наполнять по мере
   новых вызовов; NAMS-монстры — см. п.16.

2. s_suffix — КАМПАНИЯ ЗАКРЫТА. Остатки в п.9/10/12/16/18/20/22
   и не-монстры (files.c:3215, questpgr.c:389).

2a. Английские видимые литералы (87 в 34 файлах) — КАМПАНИЯ ЗАКРЫТА
   (батчи №1-6): ball.c, explode.c, region.c, sounds.c, mkobj.c,
   priest.c, wizard.c, steed.c, invent.c, apply.c, attrib.c, pager.c,
   wizcmds.c, mail.c, bones.c, engrave.c, o_init.c, dig.c, end.c,
   exper.c, hack.c, mplayer.c, polyself.c, read.c, options.c,
   cfgfiles.c, files.c, mkmaze.c, nhlua.c, shknam.c, display.c,
   mondata.c/uhitm.c/mon.c (on_fire).
   Вне охвата: мёртвый код shk.c (#if 0) и makemon.c (HPUX-only).
   Полный список — temp\eng2.txt.

### Средний приоритет

3. questpgr.c — конверсии %G/%V (падежи для текстов квестов).
   questpgr.c:389 — s_suffix на convert_arg.
   Тексты: dat/quest.lua переведён ПОЛНОСТЬЮ (батчи №12-22).

4. vtense — plural-only (волосы, перья, соки, реснички, споры),
   английские глаголы (feel, tingle, sting). Отдельные места.
   ЗАКРЫТО (bd286ae): do.c:2467 (feel, heal_legs) и sit.c:173
   (tingle, throne_sit_effect) — русские формы по числу, vtense убран.
   ЗАКРЫТО (aa647c8): apply.c×5 (1352 flicker, 1353 die, 1357 are,
   1367 are, 1447 ignite) + « more» — use_candelabrum/use_candle.
   ЗАКРЫТО (f8fe530): polyself.c:1236 (pierce, рога) и 1297 (fall,
   очки) — break_armor; подлежащие переведены, ветка otyp == LENSES.
   ЗАКРЫТО (vtense-кампания, 6 мест):
   wield.c:780 (пусты/пуста), do.c:103 (толкает/толкаете),
   mkobj.c:2877-2885 (Зелья/Зелье/Еда + вываливаются/вываливается),
   makemon.c:1492 (внезапно появляется), mcastu.c:676-699
   (змеи/насекомые, появляется/появляются).
   Остаток: глаголы движения в mondata.c locomotion()/stagger() —
   ЗАКРЫТО: locoverbs[] переведены (парит/летит/ползёт/растекается/
   извивается + формы для stagger), def-аргументы в monmove.c и muse.c
   русские, makeplural убран (monmove.c:1637, mhitm.c:1397); в
   do_name.c monverbself() русские глаголы больше не считаются
   множественным числом (проверка старшего бита).

5. Энциклопедия dat/data — английские описания богов и монстров.
   ЧАСТИЧНО (батч №9): переведены 6 статей (Хуан-ди, Иштар, Изек,
   Изак, Кос, Мог) + русские ключи-синонимы рядом с английскими.
   ВАЖНО: util/makedefs.c do_data() использовал `*line > ' '` при
   signed char, из-за чего кириллические ключи считались текстом;
   исправлено на `(uchar) *line > ' '`.
   Батч №37: 204 ключа монстров переведены (доступ к статьям
   восстановлен). Батчи №42-43: переведены тела 67 статей (короткие
   описания монстров, шлемов, артефактов и богов). Осталось:
   английские ТЕКСТЫ остальных статей (в т.ч. 204 монстров),
   wildcard-ключи (orc*, demon*, ~orc ??m* и т.п.) и объединение
   совпавших ключей (обитатель пещер, эльфийский монарх,
   любвеобильный демон, медуза).

### Низкий приоритет

6. name (86 латинских) — квест-артефакты на английском
   (The Orb of Detection, Excalibur, The Longbow of Diana).

7. wish-парсер: конфликт лук (оружие) / лук-овощ.

8. масс-существительные (кровь, рябь, шерсть) — без plural-форм.

9. gloves_simple_name/boots_simple_name — нет параметра падежа,
   14 вызовов. helm_simple_name — падеж СДЕЛАН через helm_case()
   (objnam.c) + bp_paradigms (шлем/шляпа), применено 6 мест.
   Осталось разобрать 3 места: trap.c:1634/1693 (water_damage →
   erode_obj), trap.c:145 (materialnm + burn_dmg), zap.c:5745
   (item_what, wizard-only).

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
    - Мёртвые LAT-шаблоны strsubst: батч №23 закрыл cmd.c:654,
      options.c:3125/3127, steal.c:309/312/313/317, timeout.c:146/212/228/
      337/415, polyself.c:644, pager.c:2214-2218 (батч №23),
      botl.c:478, dungeon.c:3788-3791, mkobj.c:1657/3439, o_init.c:680,
      role.c:1606, stairs.c:232, topten.c:1050 (батч №25).
      Осталось: cmd.c:2609, files.c:1818/1825 (не текст),
      insight.c:1622 (wizard-only), objnam.c:3349/3354/4565/4569/4573
      (wish-парсер), options.c:5985, trap.c:2775.
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

16. mon_gen: NAMS-монстры — СДЕЛАНО (батч №11, f58d93b): 32 записи
    (27 составных дварф/кобольд/гном/огр/вампир/эльф + ранг; 5 простых
    жрица/клирик/верховный жрец/верховная жрица/верховный клирик).
    Остаток: прочие NAMS-имена без парадигм (из 389 уникальных строк
    include/monsters.h покрыто ~99) — фолбэк на номинатив.

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

19. mthrowu.c:431 — ЗАКРЫТО (build16, без коммита): ветка невидимой цели
    переведена раздельно под род подлежащего —
    `pline("%s!", flesh ? "Его плоть опалена" : "Оно обожжено")`;
    убран лживый комментарий «s_suffix returns a modifiable buffer» (428).
    Скан окрестности: 103 слова длиной ≥7, других поломанных нет
    («зоркмиды»=zorkmids, «метапаттерны»=metapatterns — корректны).

20. debug-строки: mkobj.c:2433 (и другие debugpline*) — английские,
    ОТОЧЕНИЯ, отложены.

21. mon_nam() как прямое дополнение даёт ИМ. падеж: mthrowu.c:429 при
    flesh=FALSE (noncorporeal/amorphous) — «Серебро обжигает кобольд»
    вместо «кобольда». Существующий баг, нужен mon_gen(BP_ACC) —
    отдельная кампания.

22. objnam.c:1853 — блок: (1) расширить mon_gen уникальными именами
    (медуза, оракул, влад, кроис, ...), (2) перестроить порядок
    «possessive+adj+mnam+труп» → «adj+труп+имя-в-род.».

23. B2 родо-согласование:
    - hack.c:3485 и mhitu.c:601 — НЕ B2: под hard_helmet() → всегда «шлем»
      (м.р.), fem-ветки нет.
    - СДЕЛАНО (без коммита): do_wear.c:3243 — ветка hard_helmet(otmp)
      («Ваш шлем…» / «Ваша шляпа…»); do_wear.c:1970 — убрано «свой»
      («снимать %s», общий шаблон, род не нужен).
    - ОТЛОЖЕНО: zap.c:5765 — wizard-only enl_msg, «вашей %.40s» — род/число
      не согласуются.
    - ОТЛОЖЕНО (pre-existing, не наш регресс): do_wear.c:3234 (suit,
      возможно «Ваш чешуя») и do_wear.c:3240 (shirt → «Ваш рубашка»).

24. mhitm.c:613-635 — ЗАКРЫТО: рерайт шаблона. Инфинитив («схватить/
    проглотить/прилипнуть»), «Ваша попытка …» / «Попытка <род> …»,
    объект mon_gen(BP_ACC)/«вас», хвост mon_gen(BP_GEN). Шаблон
    "%s %s %s!" (без %.99s). s_suffix убран. build15: 0 ошибок.

25. mbodypart четвероногих (polyself.c:3186/3190) — 'paw'/'rear paw'
    НЕ переведены. Обнажается в trap.c:2770. Отдельная задача.

26. apply.c:1447 — ЗАКРЫТО (aa647c8): опечатка «НОвые» исправлена
    («Новая свеча» / «Новые свечи»), вместе с vtense-группой apply.c.

27. Tobjnam/otense: 69 английских глаголов в 21 файле. Карта — TBD.
    Кампания. Сюда же « more» (apply.c:1441) как отдельный тип.

28. Связки (are/have/go) — 12 мест (detect.c:1226, do.c:870, light.c:289,
    pickup.c:2592/2810/2997/3669, worn.c:427/985, wield.c:1078,
    apply.c:1511, engrave.c:830). Шаблоны переписываются без связки.

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
## 2026-11-11 — баг алтаря расшифрован, титулы, синк с Android

- Баг алтаря (краш NetHackW 12.10, «игра требует перезапуска»): причина
  найдена по дампу NetHackW.exe.16388.dmp — AV 0xC0000005 READ @ 0x2000
  в strnlen(0x2000, 3) из формата "%.3s" (save.c:1007
  store_plname_in_file / bones.c:581). В билде d3c1acd (06.10) таблица
  genders[] в role.c содержала 6 значений на строку при 7 полях
  struct Gender → сдвиг: filecode получал ROLE_FEMALE=0x2000 вместо
  указателя. Героиня (flags.female=1) → AV при СОХРАНЕНИИ игры
  («перезапуск» = WER-диалог Windows). Исправлено коммитом 06d1662
  (08.10); текущий билд чист, новых NetHack-дампов нет, сейвы пишутся.
  «Трупы не принимаются» отдельным крашем не подтверждено.
- 949e192 — sounds: noun_is_plural (волосы дыбом, morgue_mon_sound).
- 4f2d7fc — титулы: «Над переводом работал: Skadga» +
  https://boosty.to/skadga — splash (mhsplash.c), About-диалог (mhmain.c;
  NetHackW.rc: IDD_ABOUTBOX 75→93, IDC_ABOUT_COPYRIGHT 40→56 DLU),
  tty-баннер (wintty.c). Сборка 11.11 — 0 ошибок.
- nethack-500-2 (Android): незакоммиченные shoptype-правки (do_name.c
  shoptype_paradigms/case/name, mon.c logdeadmon, shk.c 2 места,
  extern.h) совпадают с нашим 808b140 — переноса не требуется.
  Переводы вести параллельно в обеих копиях.

## 2026-11-13 — граффити «#######  #??#?»

- Симптом: «Здесь есть граффити на полу. Вы читаете: "#######  #??#?
  ### ##? ####?# # ###?#?### ##?."».
- Причина: в include/config.h был включён MAKEDEFS_FILTER_NONASCII.
  makedefs при генерации dat/rumors (read_rumors_file, dofilter=TRUE)
  заменял каждый байт >126 (CP1251) на '#'. Игра затем wipeout_text
  (engrave.c) менял часть '#' на '?'. В dat/rumors лежало «####### ##
  ######...» — буквы уничтожались ещё при сборке, не при выводе.
- Исправление: MAKEDEFS_FILTER_NONASCII закомментирован в config.h
  (ru-build), пересборка Build Release x64: makedefs заново создал
  dat/rumors, oracles, engrave, epitaph, bogusmon, data; nhdat500
  перепакован. Проверка: dat/rumors декодируется в русский текст,
  строка «Повязка на глазах...» найдена в nhdat500.
- data.base остаётся английским (hi=0) — отдельная задача локализации.
