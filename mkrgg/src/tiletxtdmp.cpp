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

void dumpTileText(TIfstream& ifs, std::ostream& ofs,
                  TThingyTable& thingy,
//                  int addr,
                  int w, int h) {
  int addr = ifs.tell();
  ofs << "// " << TStringConversion::intToString(addr,
                    TStringConversion::baseHex)
       << endl;
  ofs << "#SETADDR(" << TStringConversion::intToString(addr,
                    TStringConversion::baseHex)
       << ")"
       << endl;
  
  ifs.seek(addr);
  
  
  for (int j = 0; j < h; j++) {
    ofs << "//";
    for (int i = 0; i < w; i++) {
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

int main(int argc, char* argv[]) {
/*  if (argc < 4) {
    cout << "Binary -> text converter, via Thingy table" << endl;
    cout << "Usage: " << argv[0] << " <thingy> <rom> <outfile>"
      << endl;
    cout << "The Thingy table must be in SJIS (or compatible) format."
      << endl;
    cout << "Note that only one-byte encoding sequences are supported."
      << endl;
    
    return 0;
  }
  
  TThingyTable thingy;
  thingy.readSjis(string(argv[1]));
  NesRom rom = NesRom(string(argv[2]));
//  int offset = TStringConversion::stringToInt(argv[3]);
  TBufStream ifs(rom.size());
  ifs.write((char*)rom.directRead(0), rom.size());
//  std::ifstream ifs(argv[2], ios_base::binary);
  std::ofstream ofs(argv[3], ios_base::binary);
  
  ifs.seek(pointerTableStart);
  int bankNum
    = UxRomBanking::directToBankNumMovable(pointerTableStart);
  
  for (int i = 0; i < numPointerTableEntries; i++) {
    int pointer = ifs.readu16le();
    int nextPos = ifs.tell();
    int physicalPointer
      = UxRomBanking::bankedToDirectAddressMovable(bankNum, pointer);
    
    ifs.seek(physicalPointer);
    printScript(ifs, ofs, thingy);
    
    ifs.seek(nextPos);
  }
  
//  while (ifs.good()) {
//    int next = (unsigned char)ifs.get();
//    if (thingy.hasEntry(next)) {
//      ofs << thingy.getEntry(next);
//      
//      if (next == 0x00) ofs << endl;
//    }
//  } 
  
  return 0; */

  if (argc < 4) {
    cout << "Magic Knight Rayearth (Game Gear) tile-based text dumper" << endl;
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
  ofs << "//**********************************" << endl;
  ofs << "// shop" << endl;
  ofs << "//**********************************" << endl;
  ofs << endl;
  
  // buy
  ifs.seek(0xA2E1);
  dumpTileText(ifs, ofs, thingy,
               18, 6);
  dumpTileText(ifs, ofs, thingy,
               18, 2);
  
  // sell
  ifs.seek(0xA3F7);
  dumpTileText(ifs, ofs, thingy,
               18, 6);
  dumpTileText(ifs, ofs, thingy,
               18, 2);
  
  // ?
  ifs.seek(0xACE0);
  dumpTileText(ifs, ofs, thingy,
               18, 6);
  dumpTileText(ifs, ofs, thingy,
               18, 2);
  
  //**********************************
  // title screen, save/load
  //**********************************
  ofs << "//**********************************" << endl;
  ofs << "// title screen, save/load" << endl;
  ofs << "//**********************************" << endl;
  ofs << endl;
  
  ifs.seek(0xC472);
  dumpTileText(ifs, ofs, thingy,
               20, 8);
  ifs.seek(0xC512);
  dumpTileText(ifs, ofs, thingy,
               5, 2);
  dumpTileText(ifs, ofs, thingy,
               5, 2);
  dumpTileText(ifs, ofs, thingy,
               5, 2);
  dumpTileText(ifs, ofs, thingy,
               20, 8);
  dumpTileText(ifs, ofs, thingy,
               4, 2);
  dumpTileText(ifs, ofs, thingy,
               4, 2);
  
  ofs << "//**********************************" << endl;
  ofs << "// items" << endl;
  ofs << "//**********************************" << endl;
  ofs << endl;
  
  ifs.seek(0x38D0F);
  for (int i = 0; i < 33; i++) {
    dumpTileText(ifs, ofs, thingy,
                 8, 2);
  }
  
  ofs << "//**********************************" << endl;
  ofs << "// Item descriptions" << endl;
  ofs << "//**********************************" << endl;
  ofs << endl;
  
  ifs.seek(0x38F5F);
  for (int i = 0; i < 32; i++) {
    dumpTileText(ifs, ofs, thingy,
                 0x12, 8);
  }
  
  ofs << "//**********************************" << endl;
  ofs << "// Main menu" << endl;
  ofs << "//**********************************" << endl;
  ofs << endl;
  
  ifs.seek(0x667A2);
  dumpTileText(ifs, ofs, thingy,
               16, 1);
  dumpTileText(ifs, ofs, thingy,
               14, 5);
  
  ifs.seek(0x6685E);
  dumpTileText(ifs, ofs, thingy,
               18, 4);
  dumpTileText(ifs, ofs, thingy,
               18, 1);
  
  ifs.seek(0x668BC);
  dumpTileText(ifs, ofs, thingy,
               9, 2);
  dumpTileText(ifs, ofs, thingy,
               9, 2);
  dumpTileText(ifs, ofs, thingy,
               4, 1);
  dumpTileText(ifs, ofs, thingy,
               16, 1);
  
  ifs.seek(0x668FA);
  dumpTileText(ifs, ofs, thingy,
               6, 2);
  dumpTileText(ifs, ofs, thingy,
               6, 2);
  dumpTileText(ifs, ofs, thingy,
               6, 2);
  dumpTileText(ifs, ofs, thingy,
               14, 4);
  dumpTileText(ifs, ofs, thingy,
               13, 2);
  dumpTileText(ifs, ofs, thingy,
               14, 1);
  
  ifs.seek(0x66988);
  dumpTileText(ifs, ofs, thingy,
               18, 2);
  dumpTileText(ifs, ofs, thingy,
               18, 2);
  dumpTileText(ifs, ofs, thingy,
               18, 2);
  dumpTileText(ifs, ofs, thingy,
               18, 2);
  dumpTileText(ifs, ofs, thingy,
               18, 2);
  
  ifs.seek(0x66A3C);
  dumpTileText(ifs, ofs, thingy,
               18, 1);
  dumpTileText(ifs, ofs, thingy,
               18, 1);
  dumpTileText(ifs, ofs, thingy,
               6, 1);
  dumpTileText(ifs, ofs, thingy,
               18, 2);
  dumpTileText(ifs, ofs, thingy,
               10, 1);
  
  ofs << "//**********************************" << endl;
  ofs << "// Enemy names." << endl;
  ofs << "// Names are present in two versions: one for two-line messages,"
    << endl
    << "// where the diacriticals have to be combined with the top border of"
    << endl
    << "// the text box, and the other for \"normal\" usage." << endl;
  ofs << "//**********************************" << endl;
  ofs << endl;
  
  ifs.seek(0x6DE2F);
  for (int i = 0; i < 52; i++) {
    dumpTileText(ifs, ofs, thingy,
                 8, 2);
  }
  
  ofs << "//**********************************" << endl;
  ofs << "// Battle messages" << endl;
  ofs << "//**********************************" << endl;
  ofs << endl;
  
  dumpTileText(ifs, ofs, thingy,
               8, 2);
               
  ifs.seek(0x6E199);
  for (int i = 0; i < 6; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  dumpTileText(ifs, ofs, thingy,
               18, 6);
  dumpTileText(ifs, ofs, thingy,
               18, 4);
  dumpTileText(ifs, ofs, thingy,
               18, 4);
  dumpTileText(ifs, ofs, thingy,
               18, 4);
  dumpTileText(ifs, ofs, thingy,
               18, 6);
  dumpTileText(ifs, ofs, thingy,
               18, 4);
  dumpTileText(ifs, ofs, thingy,
               18, 4);
  dumpTileText(ifs, ofs, thingy,
               18, 4);
  
  ofs << "//**********************************" << endl;
  ofs << "// Battle dialogue" << endl;
  ofs << "//**********************************" << endl;
  ofs << endl;
  
  ifs.seek(0x70780);
  for (int i = 0; i < 6; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seek(0x70934);
  for (int i = 0; i < 2; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seek(0x709CA);
  for (int i = 0; i < 3; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seek(0x70AA4);
  for (int i = 0; i < 1; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(6);
  for (int i = 0; i < 3; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(2);
  for (int i = 0; i < 1; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(4);
  for (int i = 0; i < 2; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(6);
  for (int i = 0; i < 3; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(6);
  for (int i = 0; i < 3; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(6);
  for (int i = 0; i < 3; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(4);
  for (int i = 0; i < 2; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(6);
  for (int i = 0; i < 3; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(2);
  for (int i = 0; i < 1; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(6);
  for (int i = 0; i < 3; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(8);
  for (int i = 0; i < 4; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(8);
  for (int i = 0; i < 4; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(10);
  for (int i = 0; i < 5; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(8);
  for (int i = 0; i < 4; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(8);
  for (int i = 0; i < 4; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(6);
  for (int i = 0; i < 3; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  // no table?
  for (int i = 0; i < 1; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(14);
  for (int i = 0; i < 7; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(6);
  for (int i = 0; i < 3; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(10);
  for (int i = 0; i < 5; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(6);
  for (int i = 0; i < 3; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(12);
  for (int i = 0; i < 6; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(12);
  for (int i = 0; i < 6; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(4);
  for (int i = 0; i < 2; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(8);
  for (int i = 0; i < 4; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(8);
  for (int i = 0; i < 4; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(18);
  for (int i = 0; i < 9; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ofs << "//**********************************" << endl;
  ofs << "// More battle messages" << endl;
  ofs << "//**********************************" << endl;
  ofs << endl;
  
  ifs.seek(0x7BA56);
  for (int i = 0; i < 11; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  dumpTileText(ifs, ofs, thingy,
               18, 2);
  dumpTileText(ifs, ofs, thingy,
               18, 2);
  
  ofs << "//**********************************" << endl;
  ofs << "// Still more battle messages" << endl;
  ofs << "//**********************************" << endl;
  ofs << endl;
  
  ifs.seek(0x7DE81);
  dumpTileText(ifs, ofs, thingy,
               18, 5);
  dumpTileText(ifs, ofs, thingy,
               18, 4);
  dumpTileText(ifs, ofs, thingy,
               18, 4);
               
  ifs.seekoff(6);
  for (int i = 0; i < 18; i++) {
    dumpTileText(ifs, ofs, thingy,
                 9, 2);
  }
  
  dumpTileText(ifs, ofs, thingy,
               4, 1);
  dumpTileText(ifs, ofs, thingy,
               18, 2);
  dumpTileText(ifs, ofs, thingy,
               9, 4);
  
  ifs.seek(0x7E131);
  for (int i = 0; i < 12; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ifs.seekoff(22);
  for (int i = 0; i < 9; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  dumpTileText(ifs, ofs, thingy,
               18, 2);
  
  ifs.seekoff(6);
  for (int i = 0; i < 4; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 2);
  }
  dumpTileText(ifs, ofs, thingy,
               18, 3);
               
  ifs.seekoff(34);
  for (int i = 0; i < 17; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  dumpTileText(ifs, ofs, thingy,
               18, 3);
  dumpTileText(ifs, ofs, thingy,
               18, 4);
  dumpTileText(ifs, ofs, thingy,
               18, 4);
               
  ifs.seekoff(30);
  for (int i = 0; i < 12; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 2);
  }
               
  ifs.seekoff(6);
  dumpTileText(ifs, ofs, thingy,
               3, 1);
  dumpTileText(ifs, ofs, thingy,
               3, 1);
  dumpTileText(ifs, ofs, thingy,
               3, 1);
  
  dumpTileText(ifs, ofs, thingy,
               18, 4);
               
  ifs.seekoff(6);
  for (int i = 0; i < 3; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
               
  ifs.seekoff(6);
  for (int i = 0; i < 4; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 4);
  }
  
  ofs << "//**********************************" << endl;
  ofs << "// Items (during battle?)" << endl;
  ofs << "// (as with enemies, repeated with and without border diacriticals)"
    << endl;
  ofs << "//**********************************" << endl;
  ofs << endl;
               
  ifs.seek(0x7F280);
  for (int i = 0; i < 64; i++) {
    dumpTileText(ifs, ofs, thingy,
                 8, 2);
  }
  
  dumpTileText(ifs, ofs, thingy,
               18, 2);
  
  dumpTileText(ifs, ofs, thingy,
               18, 2);
  dumpTileText(ifs, ofs, thingy,
               18, 3);
  dumpTileText(ifs, ofs, thingy,
               18, 2);
  dumpTileText(ifs, ofs, thingy,
               18, 2);
  
  ofs << "//**********************************" << endl;
  ofs << "// Even more battle messages" << endl;
  ofs << "//**********************************" << endl;
  ofs << endl;
  
  dumpTileText(ifs, ofs, thingy,
               18, 4);
  dumpTileText(ifs, ofs, thingy,
               18, 2);
  
  ifs.seekoff(6);
  for (int i = 0; i < 3; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 2);
  }
               
  ifs.seekoff(6);
  for (int i = 0; i < 4; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 2);
  }
               
  ifs.seekoff(8);
  for (int i = 0; i < 8; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 2);
  }
  
  dumpTileText(ifs, ofs, thingy,
               18, 4);
  dumpTileText(ifs, ofs, thingy,
               18, 2);
             
  ifs.seekoff(6);
  for (int i = 0; i < 4; i++) {
    dumpTileText(ifs, ofs, thingy,
                 18, 2);
  }
  
 /* while (ifs.good()) {
    int next = (unsigned char)ifs.get();
    
    if (thingy.hasEntry(next)) {
    
      ofs << thingy.getEntry(next);
    }
      
    if ((ifs.tellg() % 0x40) == 0) {
      ofs << std::endl;
      ofs << "// $" << std::hex << ifs.tellg() << std::endl;
    }
  } */
  
/*  for (int i = 0; i < 521; i++) {
    string left
      = TStringConversion::intToString(i, TStringConversion::baseHex);
    left = left.substr(2, string::npos);
    cout << left << "=？" << std::endl;
  } */
  
  return 0; 
}
