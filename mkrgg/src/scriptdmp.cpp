#include "sms/SmsPattern.h"
#include "util/TIfstream.h"
#include "util/TBufStream.h"
#include "util/TGraphic.h"
#include "util/TPngConversion.h"
#include "util/TThingyTable.h"
#include "util/TStringConversion.h"
#include "mkrgg/MkrGGScriptDism.h"
#include <map>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;
using namespace BlackT;
using namespace Sms;

std::map<int, int> scriptptrToIndex;
std::map<int, int> indexToScriptptr;
std::map<int, int> indexToLength;
std::map<int, int> scriptptrToLength;
int curscrindex;

std::string toRawHex(int value) {
  return TStringConversion::intToString(value, TStringConversion::baseHex)
            .substr(2, std::string::npos);
}

void addPointer(std::ostream& ofs, int address, int slot) {
  ofs << endl
      << "#MSGPTR($" << toRawHex(address) << ", " << slot << ")"
      << endl
      << endl;
}

void clearFreeBlocks(std::ostream& ofs) {
  ofs << "#CLEARFREEBLOCKS()"
    << endl;
}

void addFreeBlock(std::ostream& ofs, int address, int len) {
  ofs << "#SETMSGLOC($" << toRawHex(address) << ", " << len << ")"
    << endl
    << endl;
}

void addFixedBlock(std::ostream& ofs, int address, int len) {
  ofs << "#ADDFIXEDBLOCK($" << toRawHex(address) << ", " << len << ")"
    << endl;
}

void dismPos(TStream& ifs, std::ostream& ofs,
             std::ostream& spaceOfs,
             TThingyTable& thingy,
             int pos, int len) {
  ifs.seek(pos);
  TBufStream stream(len);
  stream.writeFrom(ifs, len);
  stream.seek(0);
  
//  ofs << "#STARTMSG()" << endl;
//  ofs << "#MSGBANK(" << (pos / 0x4000) << ")" << endl
//     << endl;
  addFreeBlock(ofs, pos, len);
  ofs << "// $" << toRawHex(pos) << " (" << len << " bytes)" << endl
     << endl;
//  addFreeBlock(spaceOfs, pos, len);
  MkrGGScriptDism(stream, ofs, thingy, pos)();
  ofs << endl << endl << "#ENDMSG()" << endl << endl;
}

void dismPosTerminated(TStream& ifs, std::ostream& ofs,
             std::ostream& spaceOfs,
             TThingyTable& thingy,
             int pos) {
  ifs.seek(pos);
  
//  ofs << "#MSGBANK(" << (pos / 0x4000) << ")" << endl
//     << endl;
  
  MkrGGScriptDism(ifs, ofs, thingy, pos)(true);
  
  int len = ifs.tell() - pos;
  ofs << "// ^---- $" << toRawHex(pos) << " (" << len << " bytes)" << endl
     << endl;
//  addFreeBlock(spaceOfs, pos, len);
  addFreeBlock(ofs, pos, len);
  ofs << endl << endl << "#ENDMSG()" << endl << endl;
}

void dismRoomScript(TStream& ifs, std::ostream& ofs,
             std::ostream& spaceOfs,
             TThingyTable& thingy,
             int pos, int len) {
/*    format of pointed-to struct (see 8AC9):
    * 1b banknum
      if nonzero:
      * 2b ?
      * 2b ?
    * 1b banknum
    * 2b ?
    * 1b banknum
      if nonzero:
      * 2b ?
      * 2b ?
    * 2b ?
    * 2b ?
    * 1b ?
      if zero:
        * 2b script pointer
      else if 01:
        * 2b script pointer
        * ?
      else:
        * 2b script pointer
        * ? */
  
  ifs.seek(pos);
  
//  std::cerr << std::hex << pos << std::endl;
  
  // 0
  int temp = ifs.readu8();
  if (temp != 0) ifs.seekoff(4);
  // 1
  ifs.seekoff(3);
  // 2
  temp = ifs.readu8();
  if (temp != 0) ifs.seekoff(4);
  // 3
  ifs.seekoff(4);
  // 4
  temp = ifs.readu8();
  
  addPointer(ofs, ifs.tell(), 1);
  int scriptptr = ifs.readu16le();
  
//  std::cerr << std::hex << ifs.tell() << std::endl;
  scriptptrToIndex[scriptptr] = curscrindex;
  indexToScriptptr[curscrindex] = scriptptr;
  
  // always?
  scriptptr += 0x4000;
  
//  std::cerr << std::hex << scriptptr << std::endl;
  
  dismPos(ifs, ofs, spaceOfs, thingy, scriptptr, len);
  
/*  TBufStream stream(len);
  ifs.seek(scriptptr);
  stream.writeFrom(ifs, len);
  stream.seek(0);
  
  addFreeBlock(spaceOfs, scriptptr, len);
  MkrGGScriptDism(stream, ofs, thingy, scriptptr)();
  ofs << endl << endl << "#ENDMSG()" << endl; */
}

int main(int argc, char* argv[]) {
  if (argc < 4) {
    cout << "Magic Knight Rayearth (Game Gear) dialogue dumper" << endl;
    cout << "Usage: " << argv[0] << " <infile> <outfile> <table>" << endl;
    return 0;
  }
  
  {
    std::ofstream ofs(argv[2], ios_base::trunc);
  }
  
  TIfstream ifs(argv[1], ios_base::binary);
  std::ofstream ofs(argv[2]);
//  std::ofstream spaceOfs(".scrtemp");
  std::ofstream spaceOfs("mkrgg_space.txt");
  TThingyTable thingy;
  thingy.readSjis(string(argv[3]));
  
//  addFixedBlock(spaceOfs, 0x2FA0, 0x1060);
//  addFreeBlock(spaceOfs, 0x7BF0, 0x400);
//  addFreeBlock(spaceOfs, 0xBA40, 0x5C0);
//  addFreeBlock(spaceOfs, 0xEC00, 0x1400);
  
//  clearFreeBlocks(ofs);
//  addFreeBlock(ofs, 0xDE98, 0x1FF);
//  ofs << endl;
  
  ofs << "//***********************************************************" << endl
      << "// Cutscenes" << endl
      << "//***********************************************************" << endl;
  
  ofs << endl
      << "//********************************" << endl
      << "// Intro 1" << endl
      << "//********************************" << endl;
//  addPointer(ofs, 0xD881, 1);
//  dismPos(ifs, ofs, spaceOfs, thingy, 0xDE98, 0x1FF);

  addPointer(ofs, 0xD881, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xDE98, 0xBB);
  addPointer(ofs, 0xD9A9, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xDF53, 0x76);
  addPointer(ofs, 0xDA0B, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xDFC9, 0x57);
  addPointer(ofs, 0xDA63, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xE020, 0x77);
  
  ofs << endl
      << "//********************************" << endl
      << "// Intro 2" << endl
      << "//********************************" << endl;
  addPointer(ofs, 0xDAE7, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xE097, 0x41);
  
  ofs << endl
      << "//********************************" << endl
      << "// Intro 3" << endl
      << "//********************************" << endl;
//  addPointer(ofs, 0xDB29, 1);
//  dismPos(ifs, ofs, spaceOfs, thingy, 0xE0D8, 0x145);

  addPointer(ofs, 0xDB29, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xE0D8, 0x1B);
  addPointer(ofs, 0xDC9C, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xE0F3, 0x6B);
  addPointer(ofs, 0xDCFB, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xE15E, 0xBF);
  
  ofs << endl
      << "//********************************" << endl
      << "// Ending" << endl
      << "//********************************" << endl;
//  addPointer(ofs, 0xE25F, 1);
//  dismPos(ifs, ofs, spaceOfs, thingy, 0xE596, 0x664);

  addPointer(ofs, 0xE25F, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xE596, 0xD2);
  addPointer(ofs, 0xE33C, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xE668, 0xEB);
  addPointer(ofs, 0xE34E, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xE753, 0xDD);
  addPointer(ofs, 0xE360, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xE830, 0x80);
  addPointer(ofs, 0xE3A3, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xE8B0, 0x172);
  addPointer(ofs, 0xE3F2, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xEA22, 0x1D8);
  
//  ofs << "//***********************************************************" << endl
//      << "// ?" << endl
//      << "//***********************************************************" << endl;
//  for (int i = 0; i < 5; i++) {
//    ofs << endl
//        << "//********************************" << endl
//        << "// Scene " << i << endl
//        << "//********************************" << endl;
//    
/*    int lentable[] = { 0x350, 0x50E, 0x16B, 0x116, 0xF4 };
    
    ifs.seek(0xC6AD + (i * 4) + 2);
    addPointer(ofs, ifs.tell(), 1);
    int addr = ifs.readu16le() + 0x8000;
    int len = lentable[i];
//    if (i != 6) {
//      ifs.seekoff(2);
//      int addr2 = ifs.readu16le() + 0x8000;
//      len = addr2 - addr;
//    }
    
    dismPos(ifs, ofs, spaceOfs, thingy, addr, len);
  } */
  
  ofs << "//***********************************************************" << endl
      << "// Cutscenes" << endl
      << "//***********************************************************" << endl;
    
  ofs << endl
      << "//********************************" << endl
      << "// Cutscene 0" << endl
      << "//********************************" << endl;
  addPointer(ofs, 0xC6AD + (0 * 4) + 2, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xCC6C, 0x83);
    addPointer(ofs, 0xC7A8 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xCCEF, 0x8F);
    addPointer(ofs, 0xC815 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xCD7E, 0x8B);
    addPointer(ofs, 0xC891 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xCE09, 0x8C);
    addPointer(ofs, 0xC8F5 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xCE95, 0x9D);
    addPointer(ofs, 0xC880 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xCF32, 0x8A);
    
  ofs << endl
      << "//********************************" << endl
      << "// Cutscene 1" << endl
      << "//********************************" << endl;
  addPointer(ofs, 0xC6AD + (1 * 4) + 2, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xCFBC, 0x1A9);
    addPointer(ofs, 0xCAA1 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xD165, 0x13F);
    addPointer(ofs, 0xCAE4 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xD2A4, 0x163);
    addPointer(ofs, 0xCB2C + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xD407, 0x6B);
    addPointer(ofs, 0xCA5B + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xD472, 0x58);
    
  ofs << endl
      << "//********************************" << endl
      << "// Cutscene 2" << endl
      << "//********************************" << endl;
  addPointer(ofs, 0xC6AD + (2 * 4) + 2, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xD4CA, 0xA4);
    addPointer(ofs, 0xCBB6 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xD56E, 0xC7);
    
  ofs << endl
      << "//********************************" << endl
      << "// Cutscene 3" << endl
      << "//********************************" << endl;
  addPointer(ofs, 0xC6AD + (3 * 4) + 2, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xD635, 0x116);
    
  ofs << endl
      << "//********************************" << endl
      << "// Cutscene 4" << endl
      << "//********************************" << endl;
  addPointer(ofs, 0xC6AD + (4 * 4) + 2, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xD74B, 0xF4);
  
  
  ofs << "//***********************************************************" << endl
      << "// Rooms" << endl
      << "//***********************************************************" << endl;

  ofs << endl
      << "//********************************" << endl
      << "// Room 0" << endl
      << "//********************************" << endl;
  addPointer(ofs, 0x8C9B, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xA498, 0xE7);
//    addPointer(ofs, 0x0 + 1, 1);  // local jump
//    dismPos(ifs, ofs, spaceOfs, thingy, 0xA4C8, 0x8F);
//    addPointer(ofs, 0x0 + 1, 1);  // local jump
//    dismPos(ifs, ofs, spaceOfs, thingy, 0xA557, 0x28);
    addPointer(ofs, 0x90FB + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA57F, 0x55);
    addPointer(ofs, 0x910F + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA5D4, 0x21);
    addPointer(ofs, 0x914D + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA5F5, 0x32);
    addPointer(ofs, 0x9178 + (0 * 2), 1); // remaining entries in table
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA627, 0x3C);
    addPointer(ofs, 0x9178 + (1 * 2), 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA663, 0x3E);
    addPointer(ofs, 0x9178 + (2 * 2), 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA6A1, 0x3E);
    addPointer(ofs, 0x9178 + (3 * 2), 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA6DF, 0x3C);
    addPointer(ofs, 0x9178 + (4 * 2), 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA71B, 0x3A);
    addPointer(ofs, 0x9178 + (5 * 2), 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA755, 0x3E);
    addPointer(ofs, 0x9178 + (6 * 2), 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA793, 0x3E);
    addPointer(ofs, 0x9178 + (7 * 2), 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA7D1, 0x3E);
    addPointer(ofs, 0x9178 + (8 * 2), 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA80F, 0x3C);
    addPointer(ofs, 0x9178 + (9 * 2), 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA84B, 0x3A);
    addPointer(ofs, 0x9178 + (10 * 2), 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA885, 0x3E);
    addPointer(ofs, 0x9178 + (11 * 2), 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA8C3, 0x3E);
    addPointer(ofs, 0x9178 + (12 * 2), 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA901, 0x3E);
    addPointer(ofs, 0x9178 + (13 * 2), 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA93F, 0x42);
    addPointer(ofs, 0x9178 + (14 * 2), 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA981, 0x40);
    addPointer(ofs, 0x9178 + (15 * 2), 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA9C1, 0x3C);

  ofs << endl
      << "//********************************" << endl
      << "// Room 1" << endl
      << "//********************************" << endl;
  addPointer(ofs, 0x8CAB, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xA169, 0x23);
    addPointer(ofs, 0x954B + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA18C, 0x19);

  ofs << endl
      << "//********************************" << endl
      << "// Room 2" << endl
      << "//********************************" << endl;
  addPointer(ofs, 0x8CC2, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xA9FD, 0x54);
    addPointer(ofs, 0x8F6A + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xAA51, 0xAA);
    addPointer(ofs, 0x8FA8 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xAAFB, 0x9A);
    addPointer(ofs, 0x8FBC + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xAB95, 0x17);

  ofs << endl
      << "//********************************" << endl
      << "// Room 3" << endl
      << "//********************************" << endl;
  addPointer(ofs, 0x8CD6, 1);
  // the other three inns all share this same data
  addPointer(ofs, 0x8E6D, 1);
  addPointer(ofs, 0x8EAE, 1);
  addPointer(ofs, 0x8ED9, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xA013, 0x44);
    addPointer(ofs, 0x91EA + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA057, 0x2D);
    addPointer(ofs, 0x9202 + 1, 1);
    addPointer(ofs, 0x95B1 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA084, 0xE);
    addPointer(ofs, 0x92C5 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA092, 0x14);
    addPointer(ofs, 0x92D7 + 1, 1);
    addPointer(ofs, 0x9487 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA0A6, 0x1F);
    addPointer(ofs, 0x9315 + 1, 1);
    addPointer(ofs, 0x94C5 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA0C5, 0x2C);
    addPointer(ofs, 0x9389 + 1, 1);
    addPointer(ofs, 0x9539 + 1, 1);
    addPointer(ofs, 0x9726 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA0F1, 0x29);
    addPointer(ofs, 0x939B + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA11A, 0x1C);
    addPointer(ofs, 0x93B6 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA136, 0x33);

  ofs << endl
      << "//********************************" << endl
      << "// Room 4" << endl
      << "//********************************" << endl;
  addPointer(ofs, 0x8CED, 1);
  // other item shops reuse this data
  addPointer(ofs, 0x8E98, 1);
  addPointer(ofs, 0x8EF0, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xA299, 0x26);
    addPointer(ofs, 0x97E9 + 1, 1);
    addPointer(ofs, 0x98E3 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA2BF, 0x22);
    // ... tilemap ...
    addPointer(ofs, 0x98F5 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA371, 0x1C);
    addPointer(ofs, 0x9909 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA38D, 0x22);
    addPointer(ofs, 0x9924 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA3AF, 0x26);
    addPointer(ofs, 0x99B5 + 1, 1);
    addPointer(ofs, 0x9ACC + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA3D5, 0x22);
    addPointer(ofs, 0x9ADE + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA487, 0x11);

  ofs << endl
      << "//********************************" << endl
      << "// Room 6" << endl
      << "//********************************" << endl;
  addPointer(ofs, 0x8D17, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xABFE, 0x32);
    addPointer(ofs, 0x9CC3 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xAC30, 0x28);
    addPointer(ofs, 0x9BB7 + 1, 1);
    addPointer(ofs, 0x9CB1 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xAC58, 0x1E);
    addPointer(ofs, 0x9CEA + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xAC76, 0x3F);
    addPointer(ofs, 0x9CD5 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xACB5, 0x2B);

  ofs << endl
      << "//********************************" << endl
      << "// Room 7" << endl
      << "//********************************" << endl;
  addPointer(ofs, 0x8D2D, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xAD70, 0x4A);
    addPointer(ofs, 0x901C + (0 * 2), 1);   // from table
    dismPos(ifs, ofs, spaceOfs, thingy, 0xADBA, 0x4E);
    addPointer(ofs, 0x901C + (1 * 2), 1);   // from table
    dismPos(ifs, ofs, spaceOfs, thingy, 0xAE08, 0x38);
    addPointer(ofs, 0x901C + (2 * 2), 1);   // from table
    dismPos(ifs, ofs, spaceOfs, thingy, 0xAE40, 0x6E);
    addPointer(ofs, 0x901C + (3 * 2), 1);   // from table
    dismPos(ifs, ofs, spaceOfs, thingy, 0xAEAE, 0x30);

  ofs << endl
      << "//********************************" << endl
      << "// Room 10" << endl
      << "//********************************" << endl;
  addPointer(ofs, 0x8D69, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xAF44, 0x23);
    addPointer(ofs, 0x9067 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xAF67, 0x40);
    addPointer(ofs, 0x905D + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xAFA7, 0x12);

  ofs << endl
      << "//********************************" << endl
      << "// Room 24" << endl
      << "//********************************" << endl;
  addPointer(ofs, 0x8E84, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xB273, 0x10B);
    addPointer(ofs, 0x90B5 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xB37E, 0xA1);
    addPointer(ofs, 0x90AB + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xB41F, 0x75);
    addPointer(ofs, 0x90A1 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xB494, 0x29);
    addPointer(ofs, 0x9097 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xB4BD, 0x40);

  ofs << endl
      << "//********************************" << endl
      << "// Room 30" << endl
      << "//********************************" << endl;
  addPointer(ofs, 0x8F06, 1);
  dismPos(ifs, ofs, spaceOfs, thingy, 0xA1A5, 0x5F);
    addPointer(ofs, 0x9674 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA204, 0x1F);
    addPointer(ofs, 0x96B2 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA223, 0x2F);
    addPointer(ofs, 0x9738 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA252, 0x19);
    addPointer(ofs, 0x9753 + 1, 1);
    dismPos(ifs, ofs, spaceOfs, thingy, 0xA26B, 0x2E);
      
  for (int i = 0; i < 0x1F; i++) {
    // many rooms can be done automatically, but not these
    if (   (i == 0)    // clef
        || (i == 1)    // inn rest message
        || (i == 2)    // ?
        || (i == 3)    // inn 1
        || (i == 4)    // item shop
//        || (i == 5)    // ?
        || (i == 6)    // fruit shop
        || (i == 7)    // ?
        || (i == 10)   // ?
//        || (i == 22)   // ferrio hint guy
        || (i == 23)   // inn 2 (reuses inn 1's data)
        || (i == 24)   // presea
        || (i == 25)   // item shop 2
        || (i == 26)   // inn 3 (reuses inn 1's data)
//        || (i == 27)   // ?
        || (i == 28)   // inn 4 (reuses inn 1's data)
        || (i == 29)   // item shop 3
        || (i == 30)   // inn ???
       ) continue;
  
    ofs << endl
        << "//********************************" << endl
        << "// Room " << i << endl
        << "//********************************" << endl;
    
//    int lentable[] = { 0x110, 0x100, 0, 0, 0, 0, 0, 0,
//                       0, 0, 0, 0, 0, 0, 0, 0,
//                       0, 0, 0, 0, 0, 0, 0, 0,
//                       0, 0, 0, 0, 0, 0, 0 };

    int lentable[] = { 1381, 60, 431, 342, 511, 82, 370, 366,
                       26, 76, 117, 33, 70, 36, 50, 36,
                       60, 88, 62, 59, 39, 44, 121, 342,
                       650, 511, 342, 150, 342, 511, 244 };    
    ifs.seek(0x8C4B + (i * 2));
//    addPointer(ofs, ifs.tell(), 1);
    int addr = ifs.readu16le() + 0x4000;
    int len = lentable[i];
//    if (i != 6) {
//      ifs.seekoff(2);
//      int addr2 = ifs.readu16le() + 0x8000;
//      len = addr2 - addr;
//    }
    
    curscrindex = i;
    dismRoomScript(ifs, ofs, spaceOfs, thingy, addr, len);
  }
  for (std::map<int, int>::iterator it = scriptptrToIndex.begin();
       it != scriptptrToIndex.end();
       ++it) {
    ++it;
    if (it == scriptptrToIndex.end()) {
      --it;
      continue;
    }
    
    int pos = it->first;
    --it;
    
    int len = pos - (it->first);
//    indexToLength[it->second] = len;
    scriptptrToLength[it->first] = len;
  }
/*  std::cout << " { ";
  for (int i = 0; i < 0x1F; i++) {
    int scriptptr = indexToScriptptr[i];
    int len = 0;
    if (scriptptrToLength.find(scriptptr) != scriptptrToLength.end()) {
      len = scriptptrToLength[scriptptr];
    }
    cout << len << ", ";
  }
//  for (std::map<int, int>::iterator it = indexToLength.begin();
//       it != indexToLength.end();
//       ++it) {
//    cout << it->second << ", ";
//  }
  
  std::cout << " }" << std::endl; */
  
  ofs << "//***********************************************************" << endl
      << "// ?" << endl
      << "//***********************************************************" << endl;
  for (int i = 0; i < 4; i++) {
    ofs << endl
        << "//********************************" << endl
        << "// Scene " << i << endl
        << "//********************************" << endl;
    
    int lentable[] = { 0x40, 0x2E, 0x3A, 0xAC };
    
    ifs.seek(0x83FA + (i * 2));
    addPointer(ofs, ifs.tell(), 1);
    int addr = ifs.readu16le() + 0x4000;
    int len = lentable[i];
//    if (i != 6) {
//      ifs.seekoff(2);
//      int addr2 = ifs.readu16le() + 0x8000;
//      len = addr2 - addr;
//    }
    
    dismPos(ifs, ofs, spaceOfs, thingy, addr, len);
  }
  
  ofs << "//***********************************************************" << endl
      << "// Object scripts" << endl
      << "//***********************************************************" << endl;
  for (int i = 0; i < 0x2C; i++) {
    ofs << endl
        << "//********************************" << endl
        << "// Script " << i << endl
        << "//********************************" << endl;
    
//    int lentable[] = { 0x40, 0x2E, 0x3A, 0xAC };
    
    ifs.seek(0x601B + (i * 4) + 2);
    addPointer(ofs, ifs.tell(), 1);
    int addr = ifs.readu16le() + 0x0000;
//    int len = lentable[i];
//    int len = -1;

//    if (i != 6) {
//      ifs.seekoff(2);
//      int addr2 = ifs.readu16le() + 0x8000;
//      len = addr2 - addr;
//    }
    
    dismPosTerminated(ifs, ofs, spaceOfs, thingy, addr);
  }
  
  ofs << "//***********************************************************" << endl
      << "// NPC scripts" << endl
      << "//***********************************************************" << endl;
  for (int i = 0; i < 0x27; i++) {
    ofs << endl
        << "//********************************" << endl
        << "// NPC " << i << " script" << endl
        << "//********************************" << endl;
    
//    int lentable[] = { 0x40, 0x2E, 0x3A, 0xAC };

    if (((i >= 11) && (i <= 16))
        || ((i >= 23) && (i <= 24))
        || ((i >= 27) && (i <= 29))
        || ((i >= 33) && (i <= 37))) continue;
    
    ifs.seek(0x5331 + (i * 2));
    int npcaddr = ifs.readu16le() + 0x0000;
    ifs.seek(npcaddr + 7);
    
    int addr = ifs.readu16le() + 0x0000;
    ifs.seekoff(-2);
    if (addr == 0) continue;
    
    addPointer(ofs, ifs.tell(), 1);
    dismPosTerminated(ifs, ofs, spaceOfs, thingy, addr);
  }
  
  ofs << "//***********************************************************" << endl
      << "// Treasure chest scripts" << endl
      << "//***********************************************************" << endl;
  for (int i = 0; i < 0xD; i++) {
    ofs << endl
        << "//********************************" << endl
        << "// Chest " << i << " script" << endl
        << "//********************************" << endl;

//    if (((i >= 11) && (i <= 16))
//        || ((i >= 23) && (i <= 24))
//        || ((i >= 27) && (i <= 29))
//        || ((i >= 33) && (i <= 37))) continue;
    
    ifs.seek(0x52E3 + (i * 6) + 2);
    int addr = ifs.readu16le() + 0x0000;
    ifs.seekoff(-2);
    if (addr == 0) continue;
    
    addPointer(ofs, ifs.tell(), 1);
    dismPosTerminated(ifs, ofs, spaceOfs, thingy, addr);
  }
  
  ofs << "//***********************************************************" << endl
      << "// Treasure chest failure scripts" << endl
      << "//***********************************************************" << endl;
  for (int i = 0; i < 0xD; i++) {
    ofs << endl
        << "//********************************" << endl
        << "// Chest " << i << " failure script" << endl
        << "//********************************" << endl;

//    if (((i >= 11) && (i <= 16))
//        || ((i >= 23) && (i <= 24))
//        || ((i >= 27) && (i <= 29))
//        || ((i >= 33) && (i <= 37))) continue;
    
    ifs.seek(0x52E3 + (i * 6) + 4);
    int addr = ifs.readu16le() + 0x0000;
    ifs.seekoff(-2);
    if (addr == 0) continue;
    
    addPointer(ofs, ifs.tell(), 1);
    dismPosTerminated(ifs, ofs, spaceOfs, thingy, addr);
  }
  
  ofs << endl
      << "//********************************" << endl
      << "// Items full script" << endl
      << "//********************************" << endl;
  addPointer(ofs, 0x52CC, 1);
  dismPosTerminated(ifs, ofs, spaceOfs, thingy, 0x78AE);
  
  
  
  
  
  
  // reformat script so that free space declarations are at start
/*  spaceOfs << endl;
  ofs.close();
  spaceOfs.close();
  {
    TBufStream scrifs(1);
    scrifs.open(argv[2]);
    TBufStream spaceifs(1);
    spaceifs.open(".scrtemp");
    
    std::ofstream ofs(argv[2]);
    ofs.write(spaceifs.data().data(), spaceifs.size());
    ofs.write(scrifs.data().data(), scrifs.size());
  } */
  
  return 0;
}
