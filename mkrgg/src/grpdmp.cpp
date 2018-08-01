#include "sms/SmsPattern.h"
#include "util/TIfstream.h"
#include "util/TBufStream.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace Sms;

int patternsPerRow = 16;

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "Game Gear raw graphics dumper" << endl;
    cout << "Usage: " << argv[0] << " <infile> <outfile>" << endl;
    return 0;
  }
  
  TIfstream ifs(argv[1], ios_base::binary);
  int numPatterns = ifs.size() / SmsPattern::size;
  
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
    pattern.toGraphic(g, NULL, x, y,
                      false, false, true);
  }
  
  TPngConversion::graphicToRGBAPng(string(argv[2]), g);
  
  return 0;
}
