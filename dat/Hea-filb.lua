-- NetHack Healer Hea-filb.lua	$NHDT-Date: 1652196003 2022/05/10 15:20:03 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.2 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991, 1993 by M. Stephenson, P. Winner
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = "P" });

des.level_flags("mazelevel", "noflip");

des.level_init({ style="mines", fg=".", bg="P", smoothed=false, joined=true, lit=1, walled=false })

--
des.stair("up")
des.stair("down")
--
des.object()
des.object()
des.object()
des.object()
des.object()
des.object()
des.object()
des.object()
des.object()
des.object()
des.object()
--
des.monster("Бешеная Крыса")
des.monster("Бешеная Крыса")
des.monster({ class = "r", peaceful=0 })
des.monster({ class = "r", peaceful=0 })
des.monster("гигантский угорь")
des.monster("гигантский угорь")
des.monster("гигантский угорь")
des.monster("гигантский угорь")
des.monster("гигантский угорь")
des.monster("электрический угорь")
des.monster("электрический угорь")
des.monster({ class = "D", peaceful=0 })
des.monster({ class = "D", peaceful=0 })
des.monster({ class = "D", peaceful=0 })
des.monster({ class = "D", peaceful=0 })
des.monster({ class = "S", peaceful=0 })
des.monster({ class = "S", peaceful=0 })
des.monster({ class = "S", peaceful=0 })
--
des.trap()
des.trap()
des.trap()
des.trap()
