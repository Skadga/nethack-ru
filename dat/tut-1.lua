
local tut_ctrl_key = nil;
local tut_alt_key = nil;

function tut_key(command)
   local s = nh.eckey(command);
   local m = s:match("^^([A-Z])$"); -- ^X is Ctrl-X
   if (m ~= nil) then
      tut_ctrl_key = m;
      return "Ctrl-" .. m;
   end

   m = s:match("^M%-([A-Z])$"); -- M-X is Alt-X
   if (m ~= nil) then
      tut_alt_key = m;
      return "Alt-" .. m;
   end

   return s;
end

function tut_key_help(x, y)
   if (tut_ctrl_key ~= nil) then
      des.engraving({ coord = { x,y }, type = "engrave", text = "Примечание: вне туториала комбинации с Ctrl отображаются с кареткой, например '^" .. tut_ctrl_key .. "'", degrade = false });
      tut_ctrl_key = nil;
   end
end

des.level_init({ style = "solidfill", fg = " " });
des.level_flags("mazelevel", "noflip",
                "nomongen", "nodeathdrops", "noautosearch");

des.map([[
---------------------------------------------------------------------------
|-.--|.......|......|..S....|.F.......|.............|.......|.............|
|.-..........|......|--|....|.F.....|.|S-------.....|.....................|
||.--|.......|..T......|....|.F.....|.|.......|.....|.......|.............|
||.|.|.......|......|-.|....|.F.....|.|.......|.....|--------.............|
||.|.|.......|......||.|-.-----------.-.......|-S----.....................|
|-+-S---------..---.||........................|...|.......................|
|......|          |.-------------------.......|...|....--S----............|
|......|  ######  |.........|      |..S.......|...|....|.....|............|
|----.-| -+-   #  |.....---.|######+..|.......S...|....|.....|............|
|----+----.----+---.|.--|.|.|#     ------------...|....|.....F............|
|........|.|......|.|...F...|#  ........|.....+...|....|.....|............|
|.P......-S|......|------.---# .........|.....|...|....-------........----|
|..........|......+.|...|.|.S# ..--S-----.....|LLL|..................|..| |
|.W......---......|.|.|.|.|.|# ..|......|.....|LLL|..................|..--|
|....Z.L.S.F......|.|.|.|.---#   |......+.....|...|..................|..|.|
|........|--......|...|.....|####+......|.....|...+..................||...|
---------------------------------------------------------------------------
]]);


des.region(selection.area(01,01, 73, 16), "lit");

des.non_diggable();

des.teleport_region({ region = { 9,3, 9,3 } });

-- TODO:
--  - save (more of) hero state when entering
--  - quit-command should maybe exit the tutorial?

-- turn on some newbie-friendly options
nh.parse_config("OPTIONS=mention_walls");
nh.parse_config("OPTIONS=mention_decor");
nh.parse_config("OPTIONS=lit_corridor");

local movekeys = tut_key("movewest") .. " " ..
   tut_key("movesouth") .. " " ..
   tut_key("movenorth") .. " " ..
   tut_key("moveeast");

local diagmovekeys = tut_key("movesouthwest") .. " " ..
   tut_key("movenortheast") .. " " ..
   tut_key("movesoutheast") .. " " ..
   tut_key("movenorthwest");

des.engraving({ coord = { 9,3 }, type = "engrave", text = "Двигайся с помощью " .. movekeys, degrade = false });
des.engraving({ coord = { 5,2 }, type = "engrave", text = "Двигайся по диагонали с помощью " .. diagmovekeys, degrade = false });

if (u.role == "Knight") then
   des.engraving({ coord = { 12,1 }, type = "engrave", text = "Рыцари могут прыгать с помощью '" .. tut_key("jump") .. "'", degrade = false });
end

--

des.engraving({ coord = { 2,4 }, type = "engrave", text = "Некоторые действия могут потребовать несколько попыток, прежде чем удастся", degrade = false });
des.engraving({ coord = { 2,5 }, type = "engrave", text = "Открой дверь, войдя в неё", degrade = false });
des.door({ coord = { 2,6 }, state = "closed" });

des.engraving({ coord = { 2,7 }, type = "engrave", text = "Закрой дверь с помощью '" .. tut_key("close") .. "'", degrade = false });


--

des.engraving({ coord = { 4,5 }, type = "engrave", text = "Ты можешь покинуть туториал через магический портал.", degrade = false });
des.trap({ type = "magic portal", coord = { 4,4 }, seen = true });

--

des.engraving({ coord = { 5,9 }, type = "engrave", text = "Эта дверь заперта. Вышиби её с помощью '" .. tut_key("kick") .. "'", degrade = false });
des.door({ coord = { 5,10 }, state = "locked" });

-- by default, kick is the first command that can be a ctrl-key combo
tut_key_help(6, 8);


des.engraving({ coord = { 5,12 }, type = "engrave", text = "Осмотри карту с помощью '" .. tut_key("glance") .. "', нажми ESC, когда закончишь", degrade = false });

--

des.engraving({ coord = { 10,13 }, type = "engrave", text = "Используй '" .. tut_key("search") .. "' для поиска тайных дверей", degrade = false });

des.engraving({ coord = { 10,15 }, type = "engrave", text = "Не та тайная дверь", degrade = false });

--

des.engraving({ coord = { 10,10 }, type = "engrave", text = "За этой дверью тёмный коридор", degrade = false });
des.door({ coord = { 10,9 }, state = percent(50) and "locked" or "closed" });
des.region(selection.match("#"), "unlit");
des.region(selection.match(" "), "unlit");
des.door({ coord = { 15,10 }, state = percent(50) and "locked" or "closed" });

--

des.engraving({ coord = { 15,11 }, type = "engrave", text = "Рядом с тобой четыре ловушки! Поищи их.", degrade = false });
local locs = { {14,11}, {14,12}, {15,12}, {16,12}, {16,11} };
shuffle(locs);
for i = 1, 4 do
   des.trap({ type = percent(50) and "sleep gas" or "board",
              coord = locs[i], victim = false });
end

des.engraving({ coord = { 15,15 }, type = "engrave", text = "Некоторые ловушки можно обезвредить с помощью '" .. tut_key("untrap") .. "'", degrade = false });
des.trap({ coord = { 15,16 }, type = "web", spider_on_web = false });

--

des.door({ coord = { 18,13 }, state = "closed" });

des.engraving({ coord = { 19,13 }, type = "engrave", text = "Подбирай предметы с помощью '" .. tut_key("pickup") .. "'", degrade = false });

local armor = (u.role == "Monk") and "кожаные перчатки" or "кожаная куртка";

des.object({ id = armor, spe = 0, buc = "cursed", coord = { 19,14} });

des.engraving({ coord = { 19,15 }, type = "engrave", text = "Надевай броню с помощью '" .. tut_key("wear") .. "'", degrade = false });

des.object({ id = "кинжал", spe = 0, buc = "not-cursed", coord = { 21,15} });

des.engraving({ coord = { 21,14 }, type = "engrave", text = "Вооружайся с помощью '" .. tut_key("wield") .. "'", degrade = false });


des.engraving({ coord = { 22,13 }, type = "engrave", text = "Атакуй монстров, врезаясь в них.", degrade = false });

des.monster({ id = "лишайник", coord = { 23,15 }, waiting = true, countbirth = false });

--

des.engraving({ coord = { 24,16 }, type = "engrave", text = "Теперь ты знаешь самые основы. Ты можешь покинуть туториал через магический портал.", degrade = false });

des.engraving({ coord = { 26,16 }, type = "engrave", text = "Войди в этот портал, чтобы покинуть туториал", degrade = false });
des.trap({ type = "magic portal", coord = { 27,16 }, seen = true });

--

des.engraving({ coord = { 25,13 }, type = "engrave", text = "Толкай валуны, врезаясь в них", degrade = false });
des.object({ id = "валун", coord = {25,12} });

--

des.engraving({ coord = { 27,9 }, type = "engrave", text = "Снимай броню с помощью '" .. tut_key("takeoff") .. "'", degrade = false });

--

des.object({ class = "?", id = "снятие проклятия", buc = "blessed", coord = {23,11} })
des.engraving({ coord = { 22,11 }, type = "engrave", text = "Некоторые предметы имеют перемешанные описания, разные в каждой игре", degrade = false });
des.engraving({ coord = { 23,11 }, type = "engrave", text = "Подними этот свиток, прочитай его с помощью '" .. tut_key("read") .. "', и попробуй снять броню снова", degrade = false });

--

des.engraving({ coord = { 19,10 }, type = "engrave", text = "Ещё один магический портал - ещё один способ покинуть этот туториал", degrade = false });
des.trap({ type = "magic portal", coord = { 19,11 }, seen = true });

--

-- rock fall
des.object({ coord = {14, 5}, id = "камень", quantity = math.random(50,99) });
des.object({ coord = {15, 5}, id = "камень", quantity = math.random(10,30) });
des.object({ coord = {14, 4}, id = "камень", quantity = math.random(10,30) });
des.object({ coord = {15, 6}, id = "камень", quantity = math.random(30,60) });
des.object({ coord = {14, 6}, id = "камень", quantity = math.random(30,60) });
des.object({ coord = {14, 6}, id = "валун" });

des.door({ coord = { 20,3 }, state = percent(50) and "open" or "closed" });

des.engraving({ coord = { 21,3 }, type = "engrave", text = "Избегай перегрузки - она замедляет тебя", degrade = false });
des.engraving({ coord = { 22,3 }, type = "engrave", text = "Бросай предметы с помощью '" .. tut_key("drop") .. "'", degrade = false });
des.engraving({ coord = { 22,4 }, type = "engrave", text = "Ты можешь бросить часть стопки, набрав перед буквой слота число", degrade = false });

--

des.monster({ id = "жёлтая плесень", coord = { 26,2 }, waiting = true, countbirth = false });

des.engraving({ coord = { 25,5 }, type = "engrave", text = "Метай предметы с помощью '" .. tut_key("throw") .. "'", degrade = false });

des.trap({ type = "magic portal", coord = { 21,1 }, seen = true });

--

des.monster({ id = "волк", coord = { 29,2 }, peaceful = 0, waiting = true, countbirth = false });

des.engraving({ coord = { 37,4 }, type = "engrave", text = "Метательные снаряды, например камни, летят лучше при стрельбе из подходящего лука", degrade = false });

des.object({ coord = { 37,3 }, id = "праща", buc = "not-cursed", spe = 9 });
des.engraving({ coord = { 37,3 }, type = "engrave", text = "Возьми пращу в руки", degrade = false });
des.engraving({ coord = { 36,1 }, type = "engrave", text = "Используй '" .. tut_key("fire") .. "', чтобы стрелять из взятого в руки лука", degrade = false });

des.engraving({ coord = { 35,4 }, type = "engrave", text = "Выстрел запускает снаряды из твоего колчана; используй '" .. tut_key("quiver") .. "', чтобы положить в него снаряды", degrade = false });

des.engraving({ coord = { 33,4 }, type = "engrave", text = "Ты можешь подождать ход с помощью '" .. tut_key("wait") .. "'", degrade = false });


--

des.door({ coord = { 38,6 }, state = "closed" });

des.engraving({ coord = { 39,6 }, type = "engrave", text = "Ты обыскиваешь контейнеры с помощью '" .. tut_key("loot") .. "'", degrade = false });

des.object({ coord = { 41,6 }, id = "большой ящик", broken = true, trapped = false,
             contents = function(obj)
                des.object({ id = "обнаружение потайных дверей", class = "/", spe = 30 }); end
});
des.engraving({ coord = { 42,6 }, type = "engrave", text = "Контейнеры также можно опорожнить с помощью '" .. tut_key("tip") .. "'", degrade = false });

des.engraving({ coord = { 45,6 }, type = "engrave", text = "Магические жезлы используются с помощью '" .. tut_key("zap") .. "'", degrade = false });

--

des.door({ coord = { 35,9 }, state = "nodoor" });
des.engraving({ coord = { 34,9 }, type = "engrave", text = "Ты можешь бежать, нажав перед клавишей движения '" .. tut_key("run") .. "'", degrade = false });

--

des.door({ coord = { 33,16 }, state = "nodoor" });
des.engraving({ coord = { 35,15 }, type = "engrave", text = "Путешествуй по уровню с помощью '" .. tut_key("travel") .. "'", degrade = false });

--

des.trap({ type = "magic portal", coord = { 27,14 }, seen = true });

--

des.engraving({ coord = { 48,1 }, type = "burn", text = "Используй '" .. tut_key("eat") .. "', чтобы съесть съедобное", degrade = false });

des.object({ coord = { 50,3 }, id = "яблоко", buc = "not-cursed"  });
des.object({ coord = { 50,3 }, id = "шоколадный батончик", buc = "not-cursed"  });

des.object({ coord = { 50,3 }, id = "труп", montype = "лишайник", buc = "not-cursed" });

--

des.door({ coord = { 46,11 }, state = "closed" });

des.engraving({ coord = { 43,11 }, type = "burn", text = "Используй '" .. tut_key("twoweapon") .. "', чтобы использовать два оружия сразу", degrade = false });
des.object({ coord = { 43,13 }, id = "нож", buc = "uncursed" });
des.object({ coord = { 43,14 }, id = "кинжал", buc = "blessed" });

des.engraving({ coord = { 43,16 }, type = "burn", text = "Быстро меняй оружие с помощью '" .. tut_key("swap") .. "'", degrade = false });

des.door({ coord = { 40,15 }, state = "random" });

--

des.object({ coord = { 48,7 }, id = "ring of левитация", buc = "not-cursed" });

des.engraving({ coord = { 48,10 }, type = "burn", text = "Надевай украшения с помощью '" .. tut_key("puton") .. "'", degrade = false });

des.engraving({ coord = { 48,16 }, type = "burn", text = "Снимай украшения с помощью '" .. tut_key("remove") .. "'", degrade = false });

des.door({ coord = { 50,16 }, state = "closed" });


--

des.engraving({ coord = { 58,9 }, type = "burn", text = "Используй '" .. tut_key("down") .. "', чтобы спуститься по лестнице", degrade = false });
des.stair({ dir = "down", coord = { 58,10 } });

--

-- one more ctrl-key help, if needed
tut_key_help(64, 4);

des.engraving({ coord = { 65,3 }, type = "burn", text = "В РАЗРАБОТКЕ", degrade = false });

des.trap({ type = "magic portal", coord = { 66,2 }, seen = true });

--

-- squeezing through small gaps

des.engraving({ coord = { 69,12 }, type = "burn", text = "Не можешь протиснуться?  Ты несёшь слишком много.", degrade = false });

-- try to squeeze over boulders, find a trap door

des.object({ id = "валун", coord = {71,16} });
des.object({ id = "валун", coord = {72,16} });
des.object({ id = "валун", coord = {73,16} });
des.trap({ type = "trap door", coord = { 73,15 } });

--

des.engraving({ coord = { 60,2 }, type = "engrave", text = "Колдовство", degrade = false });
if (u.uenmax < 5) then
   -- TODO: make sure hero has enough Pw to cast the spell (5 pw) instead?
   -- TODO: ensure the first cast of this spell succeeds?
   des.engraving({ coord = { 59,2 }, type = "engrave", text = "К сожалению, у тебя недостаточно энергии, чтобы творить заклинания.", degrade = false });
end
des.engraving({ coord = { 57,2 }, type = "engrave", text = "Подними книгу заклинаний с помощью '" .. tut_key("pickup") .. "'", degrade = false });
des.object({ coord = { 57,2 }, id = "spellbook of свет", buc = "blessed" });
des.engraving({ coord = { 55,2 }, type = "engrave", text = "Прочитай книгу заклинаний с помощью '" .. tut_key("read") .. "'", degrade = false });
des.engraving({ coord = { 53,2 }, type = "engrave", text = "Используй '" .. tut_key("cast") .. "', чтобы сотворить заклинание", degrade = false });
des.region(selection.area(53,01, 59, 3), "unlit");

--

des.engraving({ coord = { 72,2 }, type = "engrave", text = "Ты \"отпиваешь\" зелья с помощью '" .. tut_key("quaff") .. "'", degrade = false });
des.object({ coord = { 72,2 }, id = "potion of обнаружение предметов", buc = "blessed" });


----------------

-- entering and leaving tutorial _branch_ now handled by core
-- // nh.callback("cmd_before", "tutorial_cmd_before");
-- // nh.callback("level_enter", "tutorial_enter");
-- // nh.callback("level_leave", "tutorial_leave");
-- // nh.callback("end_turn", "tutorial_turn");

----------------

-- temporary stuff here
-- des.trap({ type = "magic portal", coord = { 9,5 }, seen = true });
-- des.trap({ type = "magic portal", coord = { 9,1 }, seen = true });
-- des.object({ id = "кожаная куртка", spe = 0, coord = { 9,2} });

