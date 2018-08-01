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
#include "util/TOpt.h"
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

const static int HalfMode = 0;
const static int FullMode = 1;

int main(int argc, char* argv[]) {
  
  if (argc < 9) {
    cout << "Game Gear tilemap renderer" << endl;
    cout << "Usage: " << argv[0] << " <rom> <offset> <mode> <w> <h>"
      << " <tiledata> <loadtilenum> <outfile>" << endl;
    cout << "Options: " << endl;
    cout << "  m     Set mode (full, half)" << endl;
    cout << "  p     Set palette (default: grayscale)" << endl;
    cout << "  h     Set high byte for half-tilemap mode (default: 00)"
      << endl;
    
    return 0;
  }
  
  TIfstream rom(argv[1], ios_base::binary);
  rom.seek(TStringConversion::stringToInt(string(argv[2])));
  
  int mode;
  if (strcmp(argv[3], "half") == 0) {
    mode = HalfMode;
  }
  else if (strcmp(argv[3], "full") == 0) {
    mode = FullMode;
  }
  else {
    cerr << "Unknown mode: " << argv[3] << endl;
    return 1;
  }
  
  int w = TStringConversion::stringToInt(string(argv[4]));
  int h = TStringConversion::stringToInt(string(argv[5]));
  
  int loadtilenum = TStringConversion::stringToInt(string(argv[7]));
  
  SmsVram vram;
  {
    TIfstream ifs(argv[6], ios_base::binary);
    while (ifs.remaining() > 0) {
      SmsPattern pattern;
      pattern.read(ifs);
      vram.setPattern(loadtilenum++, pattern);
    }
  }
  
  unsigned char highbyte = 0x00;
  if (TOpt::getOpt(argc, argv, "-h") != NULL) {
    int b;
    TOpt::readNumericOpt(argc, argv, "-h", &b);
    highbyte = b;
  }
                             
  SmsPalette* palP = NULL;
  SmsPalette pal;
  if (TOpt::getOpt(argc, argv, "-p") != NULL) {
    TIfstream ifs(TOpt::getOpt(argc, argv, "-p"), ios_base::binary);
    pal.readGG(ifs);
    palP = &pal;
  }
  
  int numTiles = w * h;
  int fullSize = numTiles * SmsTileId::size;
  TBufStream buffer(fullSize);
  for (int i = 0; i < numTiles; i++) {
    buffer.put(rom.get());
//    buffer.seekoff(-1);
//    cerr << hex << (int)((unsigned char)buffer.get()) << endl;
    
    if (mode == HalfMode) {
      buffer.put(highbyte);
    }
    else {
      buffer.put(rom.get());
    }
  }
  
  SmsTilemap tilemap;
  tilemap.read((const char*)buffer.data().data(), w, h);
  
  TGraphic g;
  if (palP == NULL) {
    tilemap.toGrayscaleGraphic(g, vram, true);
  }
  else {
    vram.setTilePalette(pal);
    vram.setSpritePalette(pal);
    tilemap.toColorGraphic(g, vram, true);
  }
  
//  std::cerr << argv[7] << std::endl;
  TPngConversion::graphicToRGBAPng(string(argv[8]), g);
  
  return 0;
}
