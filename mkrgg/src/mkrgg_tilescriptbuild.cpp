#include "mkrgg/MkrGGTilemapReader.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "util/TStringConversion.h"
#include "util/TOpt.h"
#include "util/TThingyTable.h"
#include "exception/TGenericException.h"
#include <string>
#include <iostream>

using namespace std;
using namespace BlackT;
using namespace Sms;

int patternsPerRow = 16;

int main(int argc, char* argv[]) {
  if (argc < 5) {
    cout << "Magic Knight Rayearth (Game Gear) tilemap text script builder"
      << endl;
    cout << "Usage: " << argv[0] << " <inrom> <scriptfile> <table> <outrom>"
      << endl;
    return 0;
  }
  
  TBufStream rom(1);
  rom.open(argv[1]);
  TIfstream ifs(argv[2], ios_base::binary);
  
  TThingyTable table;
  table.readSjis(std::string(argv[3]));
  
  try {
    MkrGGTilemapReader(ifs, rom, table)();
  }
  catch (BlackT::TGenericException& e) {
    std::cerr << "TGenericException ("
      << e.source()
      << "):"
      << endl
      << e.problem()
      << std::endl;
    return 1;
  }
  
  rom.save(argv[4]);
  
  return 0;
}
