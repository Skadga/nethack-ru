-- NetHack endgame fire.lua	$NHDT-Date: 1700398454 2023/11/19 12:54:14 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.4 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992,1993 by Izchak Miller, David Cohrs,
--                      and Timo Hakulinen
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "shortsighted", "hot", "fumaroles")
-- The player lands, upon arrival, in the
-- lower-right.  The location of the
-- portal to the next level is randomly chosen.
-- This map has no visible outer boundary, and
-- is mostly open area, with lava lakes and bunches of fire traps.
-- It fills the entire mappable area.
des.map([[
LL.............LL..............L...LL.........LL.................LL...........L
LL....LLLLLLLL............L...L.............LL....LLL.......................LL.
L....LL...................L......................LLLL................LL........
.....L.............LLLL...LL....LL...............LLLLL.............LLL.........
.L.LLLL..............LL....L.....LLL..............LLLL..............LLLL......L
LL..........LLLL...LLLL...LLL....LLL......L........LLLL....LL........LLL......L
LL........LLLLLLL...LL.....L......L......LL.........LL......LL........LL...L...
L.........LL..LLL..LL......LL......LLLL..L.........LL......LLL............LL...
......L..LL....LLLLL.................LLLLLLL.......L......LL............LLLLLL.
......L..L.....LL.LLLL.......L............L........LLLLL.LL......LL.........LL.
......LL........L...LL......LL.............LLL.....L...LLL.......LLL.........L.
.L.....LLLLLL........L.......LLL.............L....LL...L.LLL......LLLLLLL......
LL..........LLLL............LL.L.............L....L...LL.........LLL..LLL......
.L...........................LLLLL...........LL...L...L........LLLL..LLLLLL...L
.L.....LLLL.............LL....LL.......LLL...LL.......L..LLL....LLLLLLL.......L
.........LLL.........LLLLLLLLLLL......LLLLL...L...........LL...LL...LL.........
...........LL.......LL.........LL.......LLL....L..LLL....LL.........LL.........
............LLLLLLLLL...........LL....LLL.......LLLLL.....LL........LL.........
.LL...............L.............LLLLLL............LL...LLLL.........LL.......L.
LL.....L..........................LL....................LL..................LLL
L.....LLL......................LLLLL.........L.........LLLLLLLL..............LL
]]);
des.teleport_region({ region = {71,16,71,16} })
des.levregion({ region = {0,0,78,19}, exclude = {67,13,78,19}, type="portal", name="water" })

des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
des.trap("fire")
--  An assortment of fire-appropriate nasties
des.monster(" расный ƒракон")
des.monster("балрог")
des.monster({ id = "ќгненный Ёлементаль", peaceful = 0 })
des.monster({ id = "ќгненный Ёлементаль", peaceful = 0 })
des.monster("ќгненный ¬ихрь")
des.monster("јдска€ √онча€")
--
des.monster("ќгненный √игант")
des.monster("шипастый дь€вол")
des.monster("јдска€ √онча€")
des.monster("камень голем")
des.monster("€ма демон")
des.monster({ id = "ќгненный Ёлементаль", peaceful = 0 })
--
des.monster({ id = "ќгненный Ёлементаль", peaceful = 0 })
des.monster("јдска€ √онча€")
des.monster({ id = "ќгненный Ёлементаль", peaceful = 0 })
des.monster({ id = "ќгненный Ёлементаль", peaceful = 0 })
des.monster("скорпион")
des.monster("ќгненный √игант")
--
des.monster("јдска€ √онча€")
des.monster("ѕылевой ¬ихрь")
des.monster("ќгненный ¬ихрь")
des.monster({ id = "ќгненный Ёлементаль", peaceful = 0 })
des.monster({ id = "ќгненный Ёлементаль", peaceful = 0 })
des.monster({ id = "ќгненный Ёлементаль", peaceful = 0 })
des.monster("јдска€ √онча€")
des.monster({ id = "ќгненный Ёлементаль", peaceful = 0 })
des.monster("камень голем")
des.monster("€ма гадюка")
des.monster("€ма гадюка")
des.monster("ќгненный ¬ихрь")
--
des.monster({ id = "ќгненный Ёлементаль", peaceful = 0 })
des.monster({ id = "ќгненный Ёлементаль", peaceful = 0 })
des.monster("ќгненный √игант")
des.monster({ id = "ќгненный Ёлементаль", peaceful = 0 })
des.monster("ќгненный ¬ихрь")
des.monster("ќгненный ¬ихрь")
des.monster("€ма демон")
des.monster({ id = "ќгненный Ёлементаль", peaceful = 0 })
des.monster("€ма гадюка")
--
des.monster({ id = "саламандра", peaceful = 0 })
des.monster({ id = "саламандра", peaceful = 0 })
des.monster("минотавр")
des.monster({ id = "саламандра", peaceful = 0 })
des.monster("ѕаровой ¬ихрь")
des.monster({ id = "саламандра", peaceful = 0 })
des.monster({ id = "саламандра", peaceful = 0 })
--
des.monster("ќгненный √игант")
des.monster("шипастый дь€вол")
des.monster({ id = "ќгненный Ёлементаль", peaceful = 0 })
des.monster("ќгненный ¬ихрь")
des.monster({ id = "ќгненный Ёлементаль", peaceful = 0 })
des.monster({ id = "ќгненный Ёлементаль", peaceful = 0 })
des.monster("јдска€ √онча€")
des.monster("ќгненный √игант")
des.monster("€ма демон")
des.monster({ id = "ќгненный Ёлементаль", peaceful = 0 })
des.monster({ id = "ќгненный Ёлементаль", peaceful = 0 })
--
des.monster("шипастый дь€вол")
des.monster({ id = "саламандра", peaceful = 0 })
des.monster("ѕаровой ¬ихрь")
des.monster({ id = "саламандра", peaceful = 0 })
des.monster({ id = "саламандра", peaceful = 0 })

des.object("валун")
des.object("валун")
des.object("валун")
des.object("валун")
des.object("валун")

