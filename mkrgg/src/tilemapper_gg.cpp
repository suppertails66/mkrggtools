#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TIniFile.h"
#include "util/TStringConversion.h"
#include "util/TFreeSpace.h"
#include "util/TFileManip.h"
#include "util/TArray.h"
#include "util/TByte.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include "sms/SmsTilemap.h"
#include "sms/SmsPattern.h"
#include "sms/SmsPalette.h"
#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstring>

using namespace std;
using namespace BlackT;
using namespace Sms;

// stupid and inefficient, but good enough for what we're doing
typedef vector<int> Blacklist;

int readIntString(const string& src, int* pos) {
  string numstr;
  while (*pos < src.size()) {
    // accept "x" for hex
    if (!isdigit(src[*pos]) && !(src[*pos] == 'x')) break;
    else {
      numstr += src[(*pos)++];
    }
  }
  
  if (*pos < src.size()) ++(*pos);
  
  return TStringConversion::stringToInt(numstr);
}

void readBlacklist(Blacklist& blacklist, const string& src) {
  int pos = 0;
  
  while ((pos < src.size())) {
    int next = readIntString(src, &pos);
    
    // check if this is a range
    if ((pos < src.size()) && (src[(pos - 1)] == '-')) {
      int next2 = readIntString(src, &pos);
      for (unsigned int i = next; i <= next2; i++) {
        blacklist.push_back(i);
      }
    }
    else {
      blacklist.push_back(next);
    }
  }
}

bool isBlacklisted(Blacklist blacklist, int value) {
  for (unsigned int i = 0; i < blacklist.size(); i++) {
    if (blacklist[i] == value) return true;
  }
  
  return false;
}

int processTile(const TGraphic& srcG, int x, int y,
                const SmsVram& vram,
                SmsTileId* dstId,
                vector<SmsPattern>& rawTiles,
                vector<SmsPattern>& rawTilesFlipX,
                vector<SmsPattern>& rawTilesFlipY,
                vector<SmsPattern>& rawTilesFlipXY,
                int forcePaletteLine = -1,
                bool disableFlipping = false) {
  
  // Get target graphic
//  TGraphic tileG(SmsPattern::w, SmsPattern::h);
//  tileG.copy(srcG,
//             TRect(0, 0, 0, 0),
//             TRect(x, y, 0, 0));
  
  int paletteNum = 0;
  SmsPattern pattern;
  // If palette forcing is on, use the specified palette
  if (forcePaletteLine != -1) {
    paletteNum = forcePaletteLine;
    int result = pattern.fromColorGraphicGG(
                             srcG, vram.getPalette(forcePaletteLine),
                             x, y);
    if (result != 0) paletteNum = SmsVram::numPaletteLines;
  }
  else {
    // Find a palette that matches this part of the image.
    // If none matches, we can't proceed.
    for ( ; paletteNum < SmsVram::numPaletteLines; paletteNum++) {
      int result = pattern.fromColorGraphicGG(
                               srcG, vram.getPalette(paletteNum),
                               x, y);
      if (result == 0) break;
    }
  }
  
  if (paletteNum >= SmsVram::numPaletteLines) return -1;
  
  dstId->palette = paletteNum;
  
  // Determine if target graphic matches any existing tile.
  // If so, we don't need to add a new tile.
  // We must also account for possible horizontal/vertical flipping.
  bool foundMatch = false;
  for (int i = 0; i < rawTiles.size(); i++) {
    if (pattern == rawTiles[i]) {
      dstId->pattern = i;
      
      foundMatch = true;
      break;
    }
    else if (!disableFlipping
              && (pattern == rawTilesFlipX[i])) {
      
      dstId->pattern = i;
      dstId->hflip = true;
      
      foundMatch = true;
      break;
    }
    else if (!disableFlipping
              && (pattern == rawTilesFlipY[i])) {
      
      dstId->pattern = i;
      dstId->vflip = true;
      
      foundMatch = true;
      break;
    }
    else if (!disableFlipping
              && (pattern == rawTilesFlipXY[i])) {
      
      dstId->pattern = i;
      dstId->hflip = true;
      dstId->vflip = true;
      
      foundMatch = true;
      break;
    }
  }
  
  // if we found a match, we're done
  if (foundMatch) {
//    cout << dstId->pattern << endl;
    return 0;
  }
  
  // otherwise, add a new tile
  
  rawTiles.push_back(pattern);
  
  SmsPattern flipcopy = pattern;
  flipcopy.flipH();
  rawTilesFlipX.push_back(flipcopy);
  
  pattern.flipV();
  rawTilesFlipY.push_back(pattern);
  
  flipcopy.flipV();
  rawTilesFlipXY.push_back(flipcopy);
  
  dstId->pattern = rawTiles.size() - 1;
  
  return 0;
}

int main(int argc, char* argv[]) {
  
  
  // Input:
  // * output filename for graphics
  //   (tilemaps assumed from input names)
  // * raw graphic(s)
  // * target offset in VRAM of tilemapped data
  // * optional output prefix
  // * palette
  //   (don't think we need this on a per-file basis?)
  
  if (argc < 2) {
    cout << "Game Gear tilemap generator" << endl;
    cout << "Usage: " << argv[0] << " <scriptfile>" << endl;
    
    return 0;
  }
  
  TIniFile script = TIniFile(string(argv[1]));
  
  if (!script.hasSection("Properties")) {
    cerr << "Error: Script has no 'Properties' section" << endl;
    return 1;
  }
  
  string paletteName, destName;
  int loadAddr = 0;
  int minTiles = 0;
  int maxTiles = -1;
  Blacklist blacklist;
  
  // Mandatory fields
  
  if (!script.hasKey("Properties", "palette")) {
    cerr << "Error: Properties.palette is undefined" << endl;
    return 1;
  }
  paletteName = script.valueOfKey("Properties", "palette");
  
  if (!script.hasKey("Properties", "dest")) {
    cerr << "Error: Properties.dest is undefined" << endl;
    return 1;
  }
  destName = script.valueOfKey("Properties", "dest");
  
  // Optional fields
  
  if (script.hasKey("Properties", "loadAddr")) {
    loadAddr = TStringConversion::stringToInt(script.valueOfKey(
      "Properties", "loadAddr"));
  }
  
  if (script.hasKey("Properties", "minTiles")) {
    minTiles = TStringConversion::stringToInt(script.valueOfKey(
      "Properties", "minTiles"));
  }
  
  if (script.hasKey("Properties", "maxTiles")) {
    maxTiles = TStringConversion::stringToInt(script.valueOfKey(
      "Properties", "maxTiles"));
  }
  
  if (script.hasKey("Properties", "blacklist")) {
    string blacklistStr = script.valueOfKey("Properties", "blacklist");
    readBlacklist(blacklist, blacklistStr);
  }
  
//  for (int i = 0; i < blacklist.size(); i++) {
//    cout << blacklist[i] << endl;
//  }
  
  // get palette
//  SmsPalette palette;
  SmsVram vram;
  {
    TIfstream ifs(paletteName.c_str(), ios_base::binary);
    SmsPalette pal;
    pal.readGG(ifs);
    vram.setTilePalette(pal);
    
    if (ifs.remaining() > 0) {
      pal.readGG(ifs);
      vram.setSpritePalette(pal);
    }
  }
  
  // 1. go through all source images and analyze for matching tiles
  // 2. create per-image tilemap corresponding to raw tile indices
  // 3. map raw tile indices to actual tile positions (accounting for
  //    blacklist, etc.)
  // 4. generate final tilemaps by mapping raw indices to final positions
  
//  vector<MdTilemap> rawTilemaps;
  map<string, SmsTilemap> rawTilemaps;
  vector<SmsPattern> rawTiles;
  vector<SmsPattern> rawTilesFlipX;
  vector<SmsPattern> rawTilesFlipY;
  vector<SmsPattern> rawTilesFlipXY;
  
  for (SectionKeysMap::const_iterator it = script.cbegin();
       it != script.cend();
       ++it) {
    // iterate over all sections beginning with "Tilemap"
    string cmpstr = "Tilemap";
    if (it->first.substr(0, cmpstr.size()).compare(cmpstr) != 0) continue;
    string sectionName = it->first;
    
    string sourceStr, copyPriorityMapStr;
    int blanketPriority = 0;
    bool hasCopyPriorityMap = false;
    int forcePaletteLine = -1;
    bool noflip = false;
    
    // mandatory fields
    
    if (!script.hasKey(sectionName, "source")) {
      cerr << "Error: " << sectionName << ".source is undefined" << endl;
      return 1;
    }
    sourceStr = script.valueOfKey(sectionName, "source");
    
    // optional fields
    
    if (script.hasKey(sectionName, "copyPriorityMap")) {
      copyPriorityMapStr = script.valueOfKey(sectionName, "copyPriorityMap");
      hasCopyPriorityMap = true;
      
      // read copy map
      // ...
    }
    
    if (script.hasKey(sectionName, "priority")) {
      blanketPriority = TStringConversion::stringToInt(
        script.valueOfKey(sectionName, "priority"));
    }
    
    if (script.hasKey(sectionName, "palette")) {
      forcePaletteLine = TStringConversion::stringToInt(
        script.valueOfKey(sectionName, "palette"));
    }
    
    if (script.hasKey(sectionName, "noflip")) {
      noflip = (TStringConversion::stringToInt(
        script.valueOfKey(sectionName, "noflip")) != 0);
    }
    
    // get source graphic
    TGraphic srcG;
    TPngConversion::RGBAPngToGraphic(sourceStr, srcG);
    
    // infer tilemap dimensions from source size
    int tileW = srcG.w() / SmsPattern::w;
    int tileH = srcG.h() / SmsPattern::h;
    
    SmsTilemap tilemap;
    tilemap.resize(tileW, tileH);
    
//    cout << tileW << " " << tileH << endl;

    for (int j = 0; j < tilemap.tileIds.h(); j++) {
      for (int i = 0; i < tilemap.tileIds.w(); i++) {
        SmsTileId& tileId = tilemap.tileIds.data(i, j);
        tileId.priority = blanketPriority;
        tileId.hflip = false;
        tileId.vflip = false;
        // copy from priority map if needed
        // ...
      
        int result = processTile(srcG, i * SmsPattern::w, j * SmsPattern::h,
                                 vram,
                                 &tileId,
                                 rawTiles, rawTilesFlipX, rawTilesFlipY,
                                 rawTilesFlipXY,
                                 forcePaletteLine,
                                 noflip);
        
        if (result != 0) {
          cerr << "Error in " << sectionName
            << ": failed processing tile (" << i << ", " << j << ")" << endl;
          return 2;
        }
      }
    }
    
    rawTilemaps[sectionName] = tilemap;
  }
  
//  cout << rawTiles.size() << endl;

  // Produce the final arrangement of tiles
  
  map<int, SmsPattern> outputTiles;
  map<int, int> rawToOutputMap;
  int outputTileNum = 0;
  for (int i = 0; i < rawTiles.size(); i++) {
    // Skip blacklisted content
    while (isBlacklisted(blacklist, outputTileNum)) {
      outputTiles[outputTileNum] = SmsPattern();
      ++outputTileNum;
    }
    
    outputTiles[outputTileNum] = rawTiles[i];
    rawToOutputMap[i] = outputTileNum;
    ++outputTileNum;
  }
  
  // Give an error if tile limit exceeded
  if (outputTiles.size() > maxTiles) {
    cerr << "Error: Tile limit exceeded (limit is "
      << maxTiles << "; generated "
      << outputTiles.size() << ")" << endl;
    return -3;
  }
  
  // Write tile data
  {
    TOfstream ofs(destName.c_str(), ios_base::binary);
//    TBufStream buffer(SmsPattern::size);
    for (map<int, SmsPattern>::const_iterator it = outputTiles.cbegin();
         it != outputTiles.cend();
         ++it) {
      it->second.write(ofs);
//      ofs.write(buffer.data().data(), SmsPattern::size);
    }
    
    // pad with extra tiles to meet minimum length
//    memset((char*)buffer.data().data(), 0, SmsPattern::size);
    int padTiles = minTiles - outputTiles.size();
    SmsPattern pattern;
    for (int i = 0; i < padTiles; i++) {
//      ofs.write(buffer.data().data(), SmsPattern::size);
      pattern.write(ofs);
    }
  }
  
  // Update tilemaps and write to destinations
  
  for (SectionKeysMap::const_iterator it = script.cbegin();
       it != script.cend();
       ++it) {
    // iterate over all sections beginning with "Tilemap"
    string cmpstr = "Tilemap";
    if (it->first.substr(0, cmpstr.size()).compare(cmpstr) != 0) continue;
    string sectionName = it->first;
    
    if (!script.hasKey(sectionName, "dest")) {
      cerr << "Error: " << sectionName << ".dest is undefined" << endl;
      return 1;
    }
    string destStr = script.valueOfKey(sectionName, "dest");
    
    SmsTilemap& tilemap = rawTilemaps[sectionName];
    
    TOfstream ofs(destStr.c_str(), ios_base::binary);
    
    for (int j = 0; j < tilemap.tileIds.h(); j++) {
      for (int i = 0; i < tilemap.tileIds.w(); i++) {
        SmsTileId& id = tilemap.tileIds.data(i, j);
        id.pattern = rawToOutputMap[id.pattern] + loadAddr;
        char buffer[SmsTileId::size];
        id.write(buffer);
        ofs.write(buffer, SmsTileId::size);
      }
    }
    
  }
  
  return 0;
}
