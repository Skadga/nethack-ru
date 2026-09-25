-- NetHack endgame earth.lua	$NHDT-Date: 1652196025 2022/05/10 15:20:25 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992,1993 by Izchak Miller, David Cohrs,
--                      and Timo Hakulinen
-- NetHack may be freely redistributed.  See license for details.
--
--
-- These are the ENDGAME levels: earth, air, fire, water, and astral.
-- The top-most level, the Astral Level, has 3 temples and shrines.
-- Players are supposed to sacrifice the Amulet of Yendor on the appropriate
-- shrine.

des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "shortsighted")

des.message("Well done, mortal!")
des.message("But now thou must face the final Test...")
des.message("Prove thyself worthy or perish!")

-- The player lands, upon arrival, in the
-- lower-right cavern.  The location of the
-- portal to the next level is randomly chosen.
-- This map has no visible outer boundary, and
-- is mostly diggable "rock".
des.map([[
                                                                            
  ...                                                                       
 ....                ..                                                     
 .....             ...                                      ..              
  ....              ....                                     ...            
   ....              ...                ....                 ...      .     
    ..                ..              .......                 .      ..     
                                      ..  ...                        .      
              .                      ..    .                         ...    
             ..  ..                  .     ..                         .     
            ..   ...                        .                               
            ...   ...                                                       
              .. ...                                 ..                     
               ....                                 ..                      
                          ..                                       ...      
                         ..                                       .....     
  ...                                                              ...      
 ....                                                                       
   ..                                                                       
                                                                            
]]);

des.replace_terrain({ region={0,0, 75,19}, fromterrain=" ", toterrain=".", lit=0, chance=5 })

--  Since there are no stairs, this forces the hero's initial placement
des.teleport_region({region = {69,16,69,16} })
des.levregion({ region = {0,0,75,19}, exclude = {65,13,75,19}, type="portal", name="air" })
--  Some helpful monsters.  Making sure a
--  pick axe and at least one wand of digging
--  are available.
des.monster("Elvenking", 67,16)
des.monster("минотавр", 67,14)
--  An assortment of earth-appropriate nasties
--  in each cavern.
des.monster({ id = "«емл€ной Ёлементаль", x = 52, y = 13, peaceful = 0 })
des.monster({ id = "«емл€ной Ёлементаль", x = 53, y = 13, peaceful = 0 })
des.monster("камень тролль", 53,12)
des.monster(" аменный √игант", 54,12)
--
des.monster("€ма гадюка", 70,05)
des.monster("шипастый дь€вол", 69,06)
des.monster(" аменный √игант", 69,08)
des.monster("камень голем", 71,08)
des.monster("€ма демон", 70,09)
des.monster({ id = "«емл€ной Ёлементаль", x = 70, y = 08, peaceful = 0 })
--
des.monster({ id = "«емл€ной Ёлементаль", x = 60, y = 03, peaceful = 0 })
des.monster(" аменный √игант", 61,04)
des.monster({ id = "«емл€ной Ёлементаль", x = 62, y = 04, peaceful = 0 })
des.monster({ id = "«емл€ной Ёлементаль", x = 61, y = 05, peaceful = 0 })
des.monster("скорпион", 62,05)
des.monster(" аменный Ѕурав", 63,05)
--
des.monster("умбер громада", 40,05)
des.monster("ѕылевой ¬ихрь", 42,05)
des.monster("камень тролль", 38,06)
des.monster({ id = "«емл€ной Ёлементаль", x = 39, y = 06, peaceful = 0 })
des.monster({ id = "«емл€ной Ёлементаль", x = 41, y = 06, peaceful = 0 })
des.monster({ id = "«емл€ной Ёлементаль", x = 38, y = 07, peaceful = 0 })
des.monster(" аменный √игант", 39,07)
des.monster({ id = "«емл€ной Ёлементаль", x = 43, y = 07, peaceful = 0 })
des.monster("камень голем", 37,08)
des.monster("€ма гадюка", 43,08)
des.monster("€ма гадюка", 43,09)
des.monster("камень тролль", 44,10)
--
des.monster({ id = "«емл€ной Ёлементаль", x = 02, y = 01, peaceful = 0 })
des.monster({ id = "«емл€ной Ёлементаль", x = 03, y = 01, peaceful = 0 })
des.monster("камень голем", 01,02)
des.monster({ id = "«емл€ной Ёлементаль", x = 02, y = 02, peaceful = 0 })
des.monster("камень тролль", 04,03)
des.monster("камень тролль", 03,03)
des.monster("€ма демон", 03,04)
des.monster({ id = "«емл€ной Ёлементаль", x = 04, y = 05, peaceful = 0 })
des.monster("€ма гадюка", 05,06)
--
des.monster({ id = "«емл€ной Ёлементаль", x = 21, y = 02, peaceful = 0 })
des.monster({ id = "«емл€ной Ёлементаль", x = 21, y = 03, peaceful = 0 })
des.monster("минотавр", 21,04)
des.monster({ id = "«емл€ной Ёлементаль", x = 21, y = 05, peaceful = 0 })
des.monster("камень тролль", 22,05)
des.monster({ id = "«емл€ной Ёлементаль", x = 22, y = 06, peaceful = 0 })
des.monster({ id = "«емл€ной Ёлементаль", x = 23, y = 06, peaceful = 0 })
--
des.monster("€ма гадюка", 14,08)
des.monster("шипастый дь€вол", 14,09)
des.monster({ id = "«емл€ной Ёлементаль", x = 13, y = 10, peaceful = 0 })
des.monster("камень тролль", 12,11)
des.monster({ id = "«емл€ной Ёлементаль", x = 14, y = 12, peaceful = 0 })
des.monster({ id = "«емл€ной Ёлементаль", x = 15, y = 13, peaceful = 0 })
des.monster(" аменный √игант", 17,13)
des.monster("камень голем", 18,13)
des.monster("€ма демон", 18,12)
des.monster({ id = "«емл€ной Ёлементаль", x = 18, y = 11, peaceful = 0 })
des.monster({ id = "«емл€ной Ёлементаль", x = 18, y = 10, peaceful = 0 })
--
des.monster("шипастый дь€вол", 02,16)
des.monster({ id = "«емл€ной Ёлементаль", x = 03, y = 16, peaceful = 0 })
des.monster("камень тролль", 02,17)
des.monster({ id = "«емл€ной Ёлементаль", x = 04, y = 17, peaceful = 0 })
des.monster({ id = "«емл€ной Ёлементаль", x = 04, y = 18, peaceful = 0 })

des.object("валун")

