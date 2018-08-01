#include "sms/SmsPattern.h"
#include "mkrgg/MkrGGGrpCmp.h"
#include "util/TIfstream.h"
#include "util/TBufStream.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include "util/TStringConversion.h"
#include "util/TOpt.h"
#include <string>
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace Sms;

int patternsPerRow = 16;

int main(int argc, char* argv[]) {
  if (argc < 4) {
    cout << "Magic Knight Rayearth (Game Gear) compressed graphics dumper"
      << endl;
    cout << "Usage: " << argv[0] << " <infile> <outfile> <offset>"
      << endl;
    cout << "Options:" << endl;
    cout << "  p    Specify palette file" << endl;
    return 0;
  }
  
  TIfstream rom(argv[1], ios_base::binary);
  int offset = TStringConversion::stringToInt(string(argv[3]));
//  int numPatterns = TStringConversion::stringToInt(string(argv[4]));

  char* palettename = TOpt::getOpt(argc, argv, "-p");
  SmsPalette pal;
  SmsPalette* palptr = NULL;
  if (palettename != 0) {
    TIfstream palifs(palettename, ios_base::binary);
    pal.readGG(palifs);
    palptr = &pal;
  }

  rom.seek(offset);
  TBufStream ifs(0x100000);
  int start = rom.tell();
  MkrGGGrpCmp::decmpMkrGG(rom, ifs);
  int end = rom.tell();
  ifs.seek(0);
  
  cout << "Compressed size: " << end - start << endl;
  
  int numPatterns = ifs.size() / SmsPattern::size;
//  cerr << numPatterns << endl;
  
  int outW = patternsPerRow * SmsPattern::w;
  int outH = numPatterns / patternsPerRow;
  if ((numPatterns % patternsPerRow)) ++outH;
  outH *= SmsPattern::h;
  
  TGraphic g(outW, outH);
  g.clearTransparent();
  
  for (int i = 0; i < numPatterns; i++) {
    int x = (i % patternsPerRow) * SmsPattern::w;
    int y = (i / patternsPerRow) * SmsPattern::h;
  
    SmsPattern pattern;
    pattern.read(ifs);
    pattern.toGraphic(g, palptr, x, y,
                      false, false, true);
  }
  
  TPngConversion::graphicToRGBAPng(string(argv[2]), g);
  
  return 0;
}
