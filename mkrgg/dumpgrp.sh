
mkdir -p rsrc_raw/tiles
./mkrgg_grpdecmp mkr.gg 0x3465F rsrc_raw/tiles/title.bin
./mkrgg_grpdecmp mkr.gg 0x410AD rsrc_raw/tiles/map_labels.bin
./mkrgg_grpdecmp mkr.gg 0x16CBB rsrc_raw/tiles/bonus_text.bin

mkdir -p rsrc/title/orig
./tilemapdmp_gg mkr.gg 0x3434F full 20 18 rsrc_raw/tiles/title.bin 0x100 rsrc/title/orig/title_logo.png -p pal/title_tiles.pal
./tilemapdmp_gg mkr.gg 0xC422 full 20 1 rsrc_raw/tiles/title.bin 0x100 rsrc/title/orig/title_start.png -p pal/title_tiles.pal
./tilemapdmp_gg mkr.gg 0xC44A full 20 1 rsrc_raw/tiles/title.bin 0x100 rsrc/title/orig/title_start_blank.png -p pal/title_tiles.pal

mkdir -p rsrc/map/orig
# -h 0x01
#./tilemapdmp_gg mkr.gg 0x4154A full 8 2 rsrc_raw/tiles/map_labels.bin 0xC0 rsrc/map/orig/map_00.png -p pal/map.pal
./tilemapdmp_gg mkr.gg 0x414AA full 8 2 rsrc_raw/tiles/map_labels.bin 0xC0 rsrc/map/orig/map_01.png -p pal/map.pal
./tilemapdmp_gg mkr.gg 0x4148A full 8 2 rsrc_raw/tiles/map_labels.bin 0xC0 rsrc/map/orig/map_02.png -p pal/map.pal
./tilemapdmp_gg mkr.gg 0x414EA full 8 2 rsrc_raw/tiles/map_labels.bin 0xC0 rsrc/map/orig/map_03.png -p pal/map.pal
./tilemapdmp_gg mkr.gg 0x4152A full 8 2 rsrc_raw/tiles/map_labels.bin 0xC0 rsrc/map/orig/map_04.png -p pal/map.pal
./tilemapdmp_gg mkr.gg 0x4150A full 8 2 rsrc_raw/tiles/map_labels.bin 0xC0 rsrc/map/orig/map_05.png -p pal/map.pal
./tilemapdmp_gg mkr.gg 0x4146A full 8 2 rsrc_raw/tiles/map_labels.bin 0xC0 rsrc/map/orig/map_06.png -p pal/map.pal
./tilemapdmp_gg mkr.gg 0x414CA full 8 2 rsrc_raw/tiles/map_labels.bin 0xC0 rsrc/map/orig/map_07.png -p pal/map.pal

mkdir -p rsrc/bonus/orig
./tilemapdmp_gg mkr.gg 0x1671B full 0x14 0x06 rsrc_raw/tiles/bonus_text.bin 0x189 rsrc/bonus/orig/msg_00.png -p pal/bonus_tiles.pal
./tilemapdmp_gg mkr.gg 0x16EEC full 0x14 0x06 rsrc_raw/tiles/bonus_text.bin 0x189 rsrc/bonus/orig/msg_01.png -p pal/bonus_tiles.pal
./tilemapdmp_gg mkr.gg 0x16ADB full 0x14 0x06 rsrc_raw/tiles/bonus_text.bin 0x189 rsrc/bonus/orig/msg_02.png -p pal/bonus_tiles.pal
./tilemapdmp_gg mkr.gg 0x1680B full 0x14 0x06 rsrc_raw/tiles/bonus_text.bin 0x189 rsrc/bonus/orig/msg_03.png -p pal/bonus_tiles.pal
./tilemapdmp_gg mkr.gg 0x169EB full 0x14 0x06 rsrc_raw/tiles/bonus_text.bin 0x189 rsrc/bonus/orig/msg_04.png -p pal/bonus_tiles.pal
./tilemapdmp_gg mkr.gg 0x168FB full 0x14 0x06 rsrc_raw/tiles/bonus_text.bin 0x189 rsrc/bonus/orig/msg_05.png -p pal/bonus_tiles.pal


