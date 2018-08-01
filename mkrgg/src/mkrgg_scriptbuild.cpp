#include "mkrgg/MkrGGScriptReader.h"
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
  if (argc < 4) {
    cout << "Magic Knight Rayearth (Game Gear) script builder"
      << endl;
    cout << "Usage: " << argv[0] << " <scriptfile> <table> <outprefix>"
      << endl;
    return 0;
  }
  
  TIfstream ifs(argv[1], ios_base::binary);
  
  TThingyTable table;
  table.readSjis(std::string(argv[2]));
  
  std::string outprefix(argv[3]);
  
  try {
    MkrGGScriptReader(ifs, table, outprefix)();
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
  
  return 0;
}
