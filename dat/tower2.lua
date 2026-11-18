-- NetHack tower tower2.lua	$NHDT-Date: 1652196037 2022/05/10 15:20:37 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.2 $
--	Copyright (c) 1989 by Jean-Christophe Collet
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "solidify")
des.map({ halign = "half-left", valign = "center", map = [[
  --- --- ---  
  |.| |.| |.|  
---S---S---S---
|.S.........S.|
---.------+----
  |......|..|  
--------.------
|.S......+..S.|
---S---S---S---
  |.| |.| |.|  
  --- --- ---  
]] });
-- Random places are the 10 niches
local place = { {03,01},{07,01},{11,01},{01,03},{13,03},
	   {01,07},{13,07},{03,09},{07,09},{11,09} }
shuffle(place)

des.ladder("up", 11,05)
des.ladder("down", 03,07)
des.door("locked",10,04)
des.door("locked",09,07)
des.monster("&",place[10])
des.monster("&",place[1])
des.monster("ўенок јдской √ончей",place[2])
des.monster("ўенок јдской √ончей",place[3])
des.monster("«имний ¬олк",place[4])
des.object({ id = "сундук", coord = place[5],
             contents = function()
                des.object("амулет спасени€")
             end
});
des.object({ id = "сундук", coord = place[6],
             contents = function()
                des.object("амулет удушени€")
             end
});
des.object("ботинки хождени€ по воде",place[7])
des.object("хрустальный латный доспех",place[8])

local spbooks = {
   "книга заклинаний: невидимость",
   "книга заклинаний: конус холода",
   "книга заклинаний: создание фамиль€ра",
   "книга заклинаний: €сновидение",
   "книга заклинаний: очарование монстра",
   "книга заклинаний: камень в плоть",
   "книга заклинаний: превращение"
}
shuffle(spbooks);
des.object(spbooks[1],place[9])

-- Walls in the tower are non diggable
des.non_diggable(selection.area(00,00,14,10))

