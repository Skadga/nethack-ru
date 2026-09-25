
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("mazelevel", "noflip",
                "nomongen", "nodeathdrops", "noautosearch");

des.map([[
--------------
|............|
|............|
|............|
|............|
|............|
|............|
--------------
]]);


des.region(selection.area(01,01, 73, 16), "lit");

des.stair({ dir = "up", coord = { 2,2 } });

des.engraving({ coord = { 1,1 }, type = "burn", text = "Используй '" .. nh.eckey("up") .. "', чтобы подняться по лестнице", degrade = false });


des.trap({ type = "magic portal", coord = { 11,5 }, seen = true });

des.non_diggable();
