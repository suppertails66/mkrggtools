#include "sms/SmsPattern.h"
#include "mkrgg/MkrGGGrpCmp.h"
#include "sms/OneBppCmp.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
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
  if (argc < 3) {
    cout << "Magic Knight Rayearth (Game Gear) 1bpp graphics compressor"
      << endl;
    cout << "Usage: " << argv[0] << " <infile> <outfile> <numpatterns>"
      << endl;
    cout << "Options:" << endl;
    cout << "  p    Specify palette file" << endl;
    return 0;
  }
  
//  TIfstream ifs(argv[1], ios_base::binary);
//  int numPatterns = TStringConversion::stringToInt(string(argv[4]));
  int numPatterns = TStringConversion::stringToInt(string(argv[3]));

  SmsPalette* palptr = NULL;
  char* palettename = TOpt::getOpt(argc, argv, "-p");
  SmsPalette pal;
  if (palettename != 0) {
    TIfstream palifs(palettename, ios_base::binary);
    pal.readGG(palifs);
    palptr = &pal;
  }
  
//  int outW = patternsPerRow * SmsPattern::w;
//  int outH = numPatterns / patternsPerRow;
//  if ((numPatterns % patternsPerRow)) ++outH;
//  outH *= SmsPattern::h;
  
  TGraphic g;
  TPngConversion::RGBAPngToGraphic(string(argv[1]), g);
  
  TOfstream ofs(argv[2], ios_base::binary);
  for (int i = 0; i < numPatterns; i++) {
    int x = (i % patternsPerRow) * SmsPattern::w;
    int y = (i / patternsPerRow) * SmsPattern::h;
  
    SmsPattern pattern;
    for (int k = 0; k < SmsPattern::h; k++) {
      for (int j = 0; j < SmsPattern::w; j++) {
        SmsColor color;
        color.approximateTrueColorGG(g.getPixel(x + j, y + k));
        if (!(color == pal.getColor(0))) {
          pattern.setData(j, k, 0x1);
        }
      }
    }
    OneBppCmp::cmpOneBpp(pattern, ofs);
  }
  
  
  return 0;
}
