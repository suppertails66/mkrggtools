#include "mkrgg/MkrGGScriptDism.h"
#include "util/TStringConversion.h"
#include <iostream>

using namespace BlackT;

namespace Sms {

MkrGGScriptDism::MkrGGScriptDism(BlackT::TStream& ifs__,
                                 std::ostream& ofs__,
                                 BlackT::TThingyTable& thingy__,
                                 int address__)
  : ifs(&ifs__),
    ofs(&ofs__),
    thingy(&thingy__),
    address(address__),
    terminatorFound(false),
    lastNext(0) { }

void MkrGGScriptDism::operator()(bool terminated) {
  while (ifs->remaining() > 0) {
    if (terminated && terminatorFound) {
      // don't stop processing until all referenced labels are resolved and
      // the next end command has been reached
      if ((posToLabel.size() == 0)) break;
      else terminatorFound = false;
    }
    
    // Check if a label needs to be inserted here
    std::map<int, std::string>::const_iterator findIt
      = posToLabel.find(address + ifs->tell());
    if (findIt != posToLabel.end()) {
      *ofs << ":" << findIt->second << std::endl;
      posToLabel.erase(findIt);
    }
  
    int next = (unsigned char)ifs->get();
    
    if (next == 0xFF) {
      next <<= 8;
      next |= (unsigned char)ifs->get();
      
      // special sequences
      if (thingy->hasEntry(next)) {
        handleOpcode(next & 0xFF);
        continue;
      }
      // diacritical characters (handled as regular characters in table)
      else {
        next <<= 8;
        next |= (unsigned char)ifs->get();
      }
    }
    
    if (thingy->hasEntry(next)) {
      *ofs << thingy->getEntry(next);
    }
    else {
      *ofs << "?";
    }
    
  }
}

void MkrGGScriptDism::handleOpcode(int opcode) {
  /* 
  00          = end of script?
                (subscenes don't have an explicit terminator -- see opcode 12)
  01          = linebreak
  02 XX       = pause XX frames
  03          = wait for button
  04 XX YY ZZ = set text color (set decompression bytes at C024/C025 to XX/YY)
                if ZZ nonzero, set C023 to it (position??)
  05          = add dakuten to next character
  06          = add handakuten to next character
  07          = set C1E9 to 01 (enable auto-dialogue?)
  08          = clear C1E9 (disable auto-dialogue?),
                then set next character delay to 04
  09          = clear box
  0A XX       = advance XX spaces
  0B XX       = set flag XX (set (C300 + XX) to 01)
  0C XX       = clear flag XX (set (C300 + XX) to 00)
  0D XXXX     = call (XXXX), then set next character delay counter to 01
  0E XX YY    = branch if flag not set
                fetch byte from (C300 + XX)
                jump forward YY bytes (from address of YY) if read value is zero
                otherwise, advance to next script instruction
  0F XX YY    = branch if flag set (see 0E; this is exactly the inverse)
  10 XXXX YY  = write YY to (XXXX)
  11 XXXX     = print character at (XXXX)
  12          = triggers "next action"
                this is screen transitions in cutscenes, yes/no prompt logic in
                shops, etc.
                cutscenes are set up to end after the last one of these
  13 XX       = play sound? (write to C605)
  */
  
  int rawOp = opcode | 0xFF00;

  switch (opcode) {
  // single-byte ops
  case 0x00:
    {
      *ofs << thingy->getEntry(rawOp);
      terminatorFound = true;
    }
    break;
  case 0x01:
  case 0x03:
  // diacriticals are handled separately
//  case 0x05:
//  case 0x06:
  case 0x07:
  case 0x08:
  case 0x09:
    {
      *ofs << thingy->getEntry(rawOp);
    }
    break;
  case 0x12:
    {
      *ofs << thingy->getEntry(rawOp);
      // TODO: remove this
//      *ofs << thingy->getEntry(rawOp)
//        << " <addr:" << TStringConversion::intToString(address + lastNext,
//                      TStringConversion::baseHex)
//        << ", size:" << TStringConversion::intToString(ifs->tell() - lastNext,
//                      TStringConversion::baseHex)  << ">";
      lastNext = ifs->tell();
    }
    break;
  // multi-byte ops
  case 0x02:  // delay
    {
//      int value = ifs->readu8();
//      printRawValue(value, 1);
      *ofs << thingy->getEntry(rawOp);
      printRawValue(ifs->readu8(), 1);
    }
    break;
  case 0x04:  // text color
    {
      *ofs << thingy->getEntry(rawOp);
      printRawValue(ifs->readu8(), 1);
      printRawValue(ifs->readu8(), 1);
      printRawValue(ifs->readu8(), 1);
    }
    break;
  case 0x0A:  // spaces
    {
      *ofs << thingy->getEntry(rawOp);
      printRawValue(ifs->readu8(), 1);
    }
    break;
  case 0x0B:  // set flag
    {
      *ofs << thingy->getEntry(rawOp);
      printRawValue(ifs->readu8(), 1);
    }
    break;
  case 0x0C:  // clear flag
    {
      *ofs << thingy->getEntry(rawOp);
      printRawValue(ifs->readu8(), 1);
    }
    break;
  case 0x0D:  // call
    {
      *ofs << thingy->getEntry(rawOp);
      printRawValue(ifs->readu16le(), 2);
    }
    break;
  case 0x0E:  // ifNot
  case 0x0F:  // if
    {
//      *ofs << thingy->getEntry(rawOp);
      int flagNum = ifs->readu8();
      int branchLen = ifs->readu8() - 1;
      
      int branchPos = address + ifs->tell() + branchLen;
      std::string labelName = "loc_" + toRawHex(branchPos);
      posToLabel[branchPos] = labelName;
      
      if (opcode == 0x0E) {
        *ofs << "#JUMPIFNOT(" << flagNum << ", " << labelName << ")"
          << std::endl;
      }
      else {
        *ofs << "#JUMPIF(" << flagNum << ", " << labelName << ")"
          << std::endl;
      }
    }
    break;
  case 0x10:  // setMem
    {
      *ofs << thingy->getEntry(rawOp);
      printRawValue(ifs->readu16le(), 2);
      printRawValue(ifs->readu8(), 1);
    }
    break;
  case 0x11:  // printPtr
    {
      *ofs << thingy->getEntry(rawOp);
      printRawValue(ifs->readu16le(), 2);
    }
    break;
  case 0x13:  // playSound
    {
      *ofs << thingy->getEntry(rawOp);
      printRawValue(ifs->readu8(), 1);
    }
    break;
  default:
    std::cerr << "Unknown opcode " << std::hex << opcode << std::endl;
    break;
  }
  
  //*********************************************
  // Add line breaks after appropriate codes
  //*********************************************
  
  switch (opcode) {
  // double line break
  case 0x00:
  case 0x03:
  case 0x09:
  case 0x12:
    {
      *ofs << std::endl << std::endl;
    }
    break;
  // single line break
  case 0x01:
  case 0x05:
  case 0x08:
    {
      *ofs << std::endl;
    }
    break;
  default:
    break;
  }
}

void MkrGGScriptDism::printRawValue(int value, int numBytes) {
  for (int i = 0; i < numBytes; i++) {
    int subval = (value & 0xFF);
    value >>= 8;
    
    std::string str = toRawHex(subval);
    while (str.size() < 2) str = std::string("0") + str;
    *ofs << "<$" << str << ">";
  }
}

std::string MkrGGScriptDism::toRawHex(int value) {
  return TStringConversion::intToString(value, TStringConversion::baseHex)
            .substr(2, std::string::npos);
}


}
