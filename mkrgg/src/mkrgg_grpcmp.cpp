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
    cout << "Magic Knight Rayearth (Game Gear) graphics compressor"
      << endl;
    cout << "Usage: " << argv[0] << " <infile> <outfile>"
      << endl;
    return 0;
  }
  
  TIfstream ifs(argv[1], ios_base::binary);
  TOfstream ofs(argv[2], ios_base::binary);
  MkrGGGrpCmp::cmpMkrGG(ifs, ofs);
  
/*  TIfstream ifs(argv[1], ios_base::binary);
  TBufStream buffer(0x100000);
  MkrGGGrpCmp::cmpMkrGG(ifs, buffer);
  buffer.seek(0);
  TOfstream ofs(argv[2], ios_base::binary);
  ofs.write(buffer.data().data(), buffer.size()); */
  
  return 0;
}
