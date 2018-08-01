
echo "********************************************************************************"
echo "Magic Knight Rayearth (Game Gear) English translation build script"
echo "********************************************************************************"

echo "********************************************************************************"
echo "Setting up environment..."
echo "********************************************************************************"

set -o errexit

PATH=".:$PATH"

BASE_PWD=$PWD
INROM="mkr.gg"
OUTROM="mkr_en.gg"
WLADX="./wla-dx/binaries/wla-z80"
WLALINK="./wla-dx/binaries/wlalink"
#rm -f -r "out"

cp "$INROM" "$OUTROM"

echo "********************************************************************************"
echo "Building project tools..."
echo "********************************************************************************"

make blackt
make libsms
make

if [ ! -f $WLADX ]; then
  
  echo "********************************************************************************"
  echo "Building WLA-DX..."
  echo "********************************************************************************"
  
  cd wla-dx
    cmake -G "Unix Makefiles" .
    make
  cd $BASE_PWD
  
fi

echo "********************************************************************************"
echo "Building and injecting 1bpp font..."
echo "********************************************************************************"

mkdir -p "out/font_1bpp"
mkrgg_1bppcmp "rsrc/font/trans/font_dlg.png" "out/font_1bpp/font.bin" 183 -p "pal/title_tiles.pal"
datpatch "$OUTROM" "$OUTROM" "out/font_1bpp/font.bin" 0x38757

echo "********************************************************************************"
echo "Building script..."
echo "********************************************************************************"

mkdir -p "out/script"
mkdir -p "out/items_long"
mkdir -p "out/splash_screen"
mkrgg_scriptbuild "script/mkr_script_en.txt" "table/mkr.tbl" "out/script/"
mkrgg_scriptbuild "script/mkr_items_long.txt" "table/mkr_tiletext_english.tbl" "out/items_long/"
mkrgg_scriptbuild "script/splash_screen.txt" "table/mkr_tiletext_english.tbl" "out/splash_screen/"

echo "********************************************************************************"
echo "Running tilemapper scripts..."
echo "********************************************************************************"

mkdir -p "out/maps"
mkdir -p "out/precmp"
for file in tilemappers/*; do
  tilemapper_gg $file
done

echo "********************************************************************************"
echo "Generating font graphics..."
echo "********************************************************************************"

#mkrgg_grpundmp "rsrc_raw/title_tiles.png" "out/precmp/title_tiles.bin" 140 "pal/title_tiles.pal"
mkrgg_grpundmp "rsrc/font/trans/font_tiletext.png" "out/precmp/font_tiletext.bin" 137 "pal/title_tiles.pal"
mkrgg_grpundmp "rsrc/menu/trans/lv.png" "out/precmp/menu_lv.bin" 3 "pal/title_tiles.pal"

echo "********************************************************************************"
echo "Patching in-place tilemaps..."
echo "********************************************************************************"

datpatch "$OUTROM" "$OUTROM" "out/maps/title_logo.bin" 0x3434F
datpatch "$OUTROM" "$OUTROM" "out/maps/title_start.bin" 0xC422
datpatch "$OUTROM" "$OUTROM" "out/maps/title_start_blank.bin" 0xC44A
datpatch "$OUTROM" "$OUTROM" "out/maps/map_01.bin" 0x414AA
datpatch "$OUTROM" "$OUTROM" "out/maps/map_02.bin" 0x4148A
datpatch "$OUTROM" "$OUTROM" "out/maps/map_03.bin" 0x414EA
datpatch "$OUTROM" "$OUTROM" "out/maps/map_04.bin" 0x4152A
datpatch "$OUTROM" "$OUTROM" "out/maps/map_05.bin" 0x4150A
datpatch "$OUTROM" "$OUTROM" "out/maps/map_06.bin" 0x4146A
datpatch "$OUTROM" "$OUTROM" "out/maps/bonus_msg_00.bin" 0x1671B
datpatch "$OUTROM" "$OUTROM" "out/maps/bonus_msg_01.bin" 0x16EEC
datpatch "$OUTROM" "$OUTROM" "out/maps/bonus_msg_02.bin" 0x16ADB
datpatch "$OUTROM" "$OUTROM" "out/maps/bonus_msg_03.bin" 0x1680B
datpatch "$OUTROM" "$OUTROM" "out/maps/bonus_msg_04.bin" 0x169EB
datpatch "$OUTROM" "$OUTROM" "out/maps/bonus_msg_05.bin" 0x168FB
# this label isn't used
#datpatch "$OUTROM" "$OUTROM" "out/maps/map_07.bin" 0x414CA

echo "********************************************************************************"
echo "Compressing graphics..."
echo "********************************************************************************"

mkdir -p "out/cmp"
for file in out/precmp/*; do
  mkrgg_grpcmp "$file" "out/cmp/$(basename $file)"
done

echo "********************************************************************************"
echo "Generating final injection scripts..."
echo "********************************************************************************"

mkdir -p "out/injectscript"
cat "rsrc/mkrgg_freespace.txt" "out/script/msg_space.txt" > "out/injectscript/mkrgg_freespace.txt"
cat "rsrc/mkrgg_inject_script.txt" "out/script/msg_index.txt" > "out/injectscript/mkrgg_inject_script.txt"

echo "********************************************************************************"
echo "Injecting data..."
echo "********************************************************************************"

datinjct "out/injectscript/mkrgg_freespace.txt" "out/injectscript/mkrgg_inject_script.txt" "$OUTROM" "$OUTROM"

echo "********************************************************************************"
echo "Patching tilemap-based text..."
echo "********************************************************************************"

mkrgg_tilescriptbuild "$OUTROM" "script/mkr_script_tiletext_en.txt" "table/mkr_tiletext_english.tbl" "$OUTROM"

echo "********************************************************************************"
echo "Patching battle text..."
echo "********************************************************************************"

mkrgg_battlescriptbuild "$OUTROM" "script/mkr_battletext_en.txt" "table/mkr_tiletext_english.tbl" 0x70708 "$OUTROM"

#echo "********************************************************************************"
#echo "Applying VERY TEMPORARY fix for text speed..."
#echo "********************************************************************************"

#datpatch "$OUTROM" "$OUTROM" "rsrc/dlg_speed_0x2646.bin" 0x2443

echo "********************************************************************************"
echo "Applying ASM patches..."
echo "********************************************************************************"

cp "$OUTROM" "asm/mkr.gg"

cd asm
  # apply hacks
  ../$WLADX -o "main.o" "main.s"
  ../$WLALINK -v linkfile mkr_patched.gg
  
  mv -f "mkr_patched.gg" "mkr.gg"
  
  # update region code in header (WLA-DX forces it to 4,
  # for "export SMS", when the .smstag directive is used
  # -- we want 7, for "international GG")
  ../$WLADX -o "main2.o" "main2.s"
  ../$WLALINK -v linkfile2 mkr_patched.gg
cd "$BASE_PWD"

mv -f "asm/mkr_patched.gg" "$OUTROM"
rm "asm/mkr.gg"
rm "asm/main.o"
rm "asm/main2.o"

echo "********************************************************************************"
echo "Success!"
echo "Output ROM: $OUTROM"
echo "********************************************************************************"



