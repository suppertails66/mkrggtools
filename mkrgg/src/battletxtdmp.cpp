#include "util/TThingyTable.h"
#include "util/TStringConversion.h"
#include "util/TIfstream.h"
#include "util/TOfstream.h"
#include "util/TBufStream.h"
#include "exception/TGenericException.h"
#include <string>
#include <fstream>
#include <iostream>

using namespace std;
using namespace BlackT;

// Width/height of a text box in tiles
const static int boxWidth = 18;
const static int boxHeight = 4;

const static int numBattleMessageSets = 0x12;
//const static int battleMessageIndexBank = 0x1C;
const static int battleMessageBank = 0x1C;

const static int preBattleMessageIndexAddr = 0x70708;
const static int postBattleMessageIndexAddr = 0x7073E;

void dumpBattleTextBox(TStream& ifs, std::ostream& ofs,
                    TThingyTable& thingy) {
  int addr = ifs.tell();
  ofs << "// " << TStringConversion::intToString(addr,
                    TStringConversion::baseHex)
       << endl;
//  ofs << "#SETADDR(" << TStringConversion::intToString(addr,
//                    TStringConversion::baseHex)
//       << ")"
//       << endl;
  
  ifs.seek(addr);
  
  for (int j = 0; j < boxHeight; j++) {
    ofs << "//";
    
    for (int i = 0; i < boxWidth; i++) {
      int next = ifs.readu8();
      if (!thingy.hasEntry(next)) {
        ofs << "?";
        cerr << "error" << endl;
      }
      else {
        ofs << thingy.getEntry(next);
      }
    }
    
    ofs << endl;
  }
    
  ofs << endl << endl;
}

void dumpBattleMessageSet(TStream& ifs, std::ostream& ofs,
                          TThingyTable& thingy,
                          int numMessages) {
  int baseAddr = ifs.tell();
  
  for (int i = 0; i < numMessages; i++) {
    ifs.seek(baseAddr + (i * 2));
    int msgPtr = ifs.readu16le();
    int msgAddr = (msgPtr - 0x8000) + (0x4000 * battleMessageBank);
    ifs.seek(msgAddr);
    dumpBattleTextBox(ifs, ofs, thingy);
  }
  
  ofs << "#ENDMSGSET()" << endl << endl;
}

void dumpBattleMessageSets(TStream& ifs, std::ostream& ofs,
                          TThingyTable& thingy,
                          int numSets) {
  int baseAddr = ifs.tell();
  int setPtrBaseAddr = baseAddr + numSets;
  
  for (int i = 0; i < numSets; i++) {
    ofs << "//**************************" << endl;
    ofs << "// Message set " << i << endl;
    ofs << "//**************************" << endl << endl;
  
    ifs.seek(baseAddr + i);
    int numMessages = ifs.readu8();
    ifs.seek(setPtrBaseAddr + (i * 2));
    int msgSetPtr = ifs.readu16le();
    
    int msgSetAddr = (msgSetPtr - 0x8000) + (0x4000 * battleMessageBank);
    
//    for (int j = 0; j < numMessages; j++) {
      ifs.seek(msgSetAddr);
      dumpBattleMessageSet(ifs, ofs, thingy, numMessages);
//    }
    
  }
  
  ofs << endl << endl;
  ofs << "#ENDMSGSETGROUP()" << endl << endl;
  ofs << endl << endl;
}

int main(int argc, char* argv[]) {

  if (argc < 4) {
    cout << "Magic Knight Rayearth (Game Gear) battle text dumper" << endl;
    cout << "Usage: " << argv[0] << " <thingy> <infile> <outfile>" << endl;
    cout << "The Thingy table must be in SJIS (or compatible) format."
      << endl;
    
    return 0;
  }
  
  TThingyTable thingy;
  thingy.readSjis(string(argv[1]));
  TIfstream ifs(argv[2], ios_base::binary);
  std::ofstream ofs(argv[3], ios_base::binary);
  
  //**********************************
  // shop
  //**********************************
//  ofs << "//**********************************" << endl;
//  ofs << "// shop" << endl;
//  ofs << "//**********************************" << endl;
//  ofs << endl;
  
  // buy
//  ifs.seek(0xA2E1);
//  dumpTileText(ifs, ofs, thingy,
//               18, 6);

  ofs << "//*******************************************************" << endl;
  ofs << "// Pre-battle messages" << endl;
  ofs << "//*******************************************************" << endl << endl;
  ifs.seek(preBattleMessageIndexAddr);
  dumpBattleMessageSets(ifs, ofs, thingy, numBattleMessageSets);

  ofs << "//*******************************************************" << endl;
  ofs << "// Post-battle messages" << endl;
  ofs << "//*******************************************************" << endl << endl;
  ifs.seek(postBattleMessageIndexAddr);
  dumpBattleMessageSets(ifs, ofs, thingy, numBattleMessageSets);
  
  return 0; 
}
