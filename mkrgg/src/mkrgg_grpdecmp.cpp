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
    cout << "Magic Knight Rayearth (Game Gear) graphics decompressor"
      << endl;
    cout << "Usage: " << argv[0] << " <infile> <offset> <outfile>"
      << endl;
    return 0;
  }
  
  TIfstream ifs(argv[1], ios_base::binary);
  ifs.seek(TStringConversion::stringToInt(string(argv[2])));
  int start = ifs.tell();
//  TOfstream ofs(argv[3], ios_base::binary);
  TBufStream ofs(0x100000);
  MkrGGGrpCmp::decmpMkrGG(ifs, ofs);
  ofs.save(argv[3]);
  
  std::cout << "Compressed size: " << (ifs.tell() - start) << std::endl;
  
/*  TIfstream ifs(argv[1], ios_base::binary);
  TBufStream buffer(0x100000);
  MkrGGGrpCmp::cmpMkrGG(ifs, buffer);
  buffer.seek(0);
  TOfstream ofs(argv[2], ios_base::binary);
  ofs.write(buffer.data().data(), buffer.size()); */
  
  return 0;
}
