#include "mkrgg/MkrGGScriptReader.h"
#include "util/TBufStream.h"
#include "util/TStringConversion.h"
#include "exception/TGenericException.h"
#include <cctype>
#include <algorithm>
#include <string>
#include <iostream>

using namespace BlackT;

namespace Sms {


const static int scriptBufferCapacity = 0x10000;

MkrGGScriptReader::MkrGGScriptReader(
                  BlackT::TStream& src__,
                  const BlackT::TThingyTable& thingy__,
                  std::string outprefix__)
  : src(src__),
    thingy(thingy__),
    outprefix(outprefix__),
    lineNum(0),
    currentScriptBuffer(scriptBufferCapacity),
    blockStart(0) {
  loadThingy(thingy__);
  spaceOfs.open((outprefix + "msg_space.txt").c_str());
  indexOfs.open((outprefix + "msg_index.txt").c_str());
}

void MkrGGScriptReader::operator()() {
  while (!src.eof()) {
    std::string line;
    src.getLine(line);
    ++lineNum;
    
    if (line.size() <= 0) continue;
    
    // discard lines containing only ASCII spaces and tabs
    bool onlySpace = true;
    for (int i = 0; i < line.size(); i++) {
      if ((line[i] != ' ')
          && (line[i] != '\t')) {
        onlySpace = false;
        break;
      }
    }
//    if (onlySpace) continue;
    
    TBufStream ifs(line.size());
    ifs.write(line.c_str(), line.size());
    ifs.seek(0);
    
    // check for comments
    if ((ifs.size() >= 2)
        && (ifs.peek() == '/')) {
      ifs.get();
      if (ifs.peek() == '/') continue;
      else ifs.unget();
    }
    
    // check for special stuff
    if (ifs.peek() == '#') {
      // directives
      ifs.get();
      processDirective(ifs);
      continue;
    }
    else if (ifs.peek() == ':') {
      // labels
      ifs.get();
      processLabel(ifs);
      continue;
    }
    
    while (!ifs.eof()) {
      outputNextSymbol(ifs);
    }
  }
}
  
void MkrGGScriptReader::loadThingy(const BlackT::TThingyTable& thingy__) {
  thingy = thingy__;

  // To efficiently unmap script values, we order the thingy's strings
  // by length
  thingiesBySize.clear();
  thingiesBySize.resize(thingy.entries.size());
  int pos = 0;
  for (TThingyTable::RawTable::iterator it = thingy.entries.begin();
       it != thingy.entries.end();
       ++it) {
    ThingyValueAndKey t = { it->second, it->first };
    thingiesBySize[pos++] = t;
  }
  std::sort(thingiesBySize.begin(), thingiesBySize.end());
}
  
void MkrGGScriptReader::outputNextSymbol(TStream& ifs) {
  // literal value
  if ((ifs.remaining() >= 5)
      && (ifs.peek() == '<')) {
    int pos = ifs.tell();
    
    ifs.get();
    if (ifs.peek() == '$') {
      ifs.get();
      std::string valuestr = "0x";
      valuestr += ifs.get();
      valuestr += ifs.get();
      
      if (ifs.peek() == '>') {
        ifs.get();
        int value = TStringConversion::stringToInt(valuestr);
        
//        dst.writeu8(value);
        currentScriptBuffer.writeu8(value);

        return;
      }
    }
    
    // not a literal value
    ifs.seek(pos);
  }

  for (int i = 0; i < thingiesBySize.size(); i++) {
    if (checkSymbol(ifs, thingiesBySize[i].value)) {
      int symbolSize;
      if (thingiesBySize[i].key <= 0xFF) symbolSize = 1;
      else if (thingiesBySize[i].key <= 0xFFFF) symbolSize = 2;
      else if (thingiesBySize[i].key <= 0xFFFFFF) symbolSize = 3;
      else symbolSize = 4;
      
//      dst.writeInt(thingiesBySize[i].key, symbolSize,
//        EndiannessTypes::big, SignednessTypes::nosign);
      currentScriptBuffer.writeInt(thingiesBySize[i].key, symbolSize,
        EndiannessTypes::big, SignednessTypes::nosign);
        
      // when terminator reached, flush script to ROM stream
//      if (thingiesBySize[i].key == 0) {
//        flushActiveScript();
//      }
      
      return;
    }
  }
  
  std::string remainder;
  ifs.getLine(remainder);
  
  // if we reached end of file, this is not an error: we're done
  if (ifs.eof()) return;
  
  throw TGenericException(T_SRCANDLINE,
                          "MkrGGScriptReader::outputNextSymbol()",
                          "Line "
                            + TStringConversion::intToString(lineNum)
                            + ":\n  Couldn't match symbol at: '"
                            + remainder
                            + "'");
}
  
void MkrGGScriptReader::flushActiveScript() {
  int outputSize = currentScriptBuffer.size();
  int outputPos = blockStart;
  
  // try to put script in target block
/*  if (outputSize <= blockRemaining) {
    if (currentPointerTablePos != -1) {
      int curBlockPos = dst.tell();
      
      dst.seek(currentPointerTablePos);
      int address;
      if (blockIsInFixedBank) {
        address = UxRomBanking::directToBankedAddressFixed(curBlockPos);
      }
      else {
        address = UxRomBanking::directToBankedAddressMovable(curBlockPos);
      }
      dst.writeu16le(address);
      currentPointerTablePos += 2;
      dst.seek(curBlockPos);
    }
    
    // write data
    dst.write(currentScriptBuffer.data().data(), currentScriptBuffer.size());
    blockRemaining -= outputSize;
  }
  // if it won't fit, try to put it in the free block
  else if (outputSize <= freeBlockRemaining) {
    // temporarily move output position to free block
    int curBlockPos = dst.tell();
    dst.seek(freeBlockPos);
    dst.write(currentScriptBuffer.data().data(), currentScriptBuffer.size());
    // restore current position
    dst.seek(curBlockPos);
    
    if (currentPointerTablePos != -1) {
      int curBlockPos = dst.tell();
      
      dst.seek(currentPointerTablePos);
      int address;
      if (freeBlockIsInFixedBank) {
        address = UxRomBanking::directToBankedAddressFixed(freeBlockPos);
      }
      else {
        address = UxRomBanking::directToBankedAddressMovable(freeBlockPos);
      }
      dst.writeu16le(address);
      currentPointerTablePos += 2;
      dst.seek(curBlockPos);
    }
    
    freeBlockPos += outputSize;
    freeBlockRemaining -= outputSize;
  }
  else {
    throw TGenericException(T_SRCANDLINE,
                            "MkrGGScriptReader::flushActiveScript()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Not enough free space for script");
  } */
  
  // resolve jumps
  for (std::map<int, std::string>::iterator it = pendingJumps.begin();
       it != pendingJumps.end();
       ++it) {
    std::map<std::string, int>::iterator findIt = labels.find(it->second);
    if (findIt == labels.end()) {
      throw TGenericException(T_SRCANDLINE,
                              "MkrGGScriptReader::flushActiveScript()",
                              "Line "
                                + TStringConversion::intToString(lineNum)
                                + ":\n  Reference to undefined label "
                                + it->second
                                + " at "
                                + TStringConversion::intToString(it->first,
                                    TStringConversion::baseHex));
    }
    
//    std::cerr << findIt->first << " " << findIt->second << " " << it->first << std::endl;
    
    unsigned int distance = findIt->second - it->first;
    if (distance > 0xFF) {
      throw TGenericException(T_SRCANDLINE,
                              "MkrGGScriptReader::flushActiveScript()",
                              "Line "
                                + TStringConversion::intToString(lineNum)
                                + ":\n  Jump to label "
                                + it->second
                                + " is too wide ("
                                + TStringConversion::intToString(distance,
                                    TStringConversion::baseHex)
                                + " bytes, max 255)");
    }
    
    currentScriptBuffer.seek(it->first);
    currentScriptBuffer.put(distance);
  }
  
  // output message binary
  std::string blockStartString = TStringConversion::intToString(blockStart,
                 TStringConversion::baseHex);
  currentScriptBuffer.seek(0);
  std::string filename =
    outprefix
    + std::string("msg_")
    + blockStartString
    + ".bin";
  TOfstream ofs(filename.c_str(), std::ios_base::binary);
  ofs.write(currentScriptBuffer.data().data(), currentScriptBuffer.size());
  
  // output index entry
  indexOfs << "#******************************************************************************"
    << std::endl;
  indexOfs << "# Message " << blockStartString
    << std::endl;
  indexOfs << "#******************************************************************************"
    << std::endl;
  indexOfs << "[Message" << blockStartString << "]" << std::endl;
  indexOfs << "source=" << filename << std::endl;
  indexOfs << "origPos=" << blockStartString << std::endl;
  // do not allow overwriting at original location -- force inserter to
  // allocate from free space (since we have specifically freed all the
  // strings we built)
  indexOfs << "origSize=0" << std::endl;
  
  int slotNum = -1;
  if (pointers.size() > 0) {
    // there should always be at least one pointer, and all of them should
    // use the same slot
    
    slotNum = pointers[0].slotNum;
  }
  
  if (slotNum == -1) {
//    throw TGenericException(T_SRCANDLINE,
//                            "MkrGGScriptReader::flushActiveScript()",
//                            "Line "
//                              + TStringConversion::intToString(lineNum)
//                              + ":\n  Undefined slot number");
  }
  else {
    indexOfs << "slot=" << slotNum << std::endl;
    
    indexOfs << "pointers=";
    for (int i = 0; i < pointers.size(); i++) {
      indexOfs << TStringConversion::intToString(pointers[i].address,
                   TStringConversion::baseHex);
      if (i != (pointers.size() - 1)) indexOfs << " ";
    }
    indexOfs << std::endl;
  }
  
  // clear script buffer
  currentScriptBuffer = TBufStream(scriptBufferCapacity);
  // reset position
  blockStart = -1;
  // clear pointers
  pointers.clear();
  // clear labels
  labels.clear();
  pendingJumps.clear();
}
  
bool MkrGGScriptReader::checkSymbol(BlackT::TStream& ifs, std::string& symbol) {
  if (symbol.size() > ifs.remaining()) return false;
  
  int startpos = ifs.tell();
  for (int i = 0; i < symbol.size(); i++) {
    if (symbol[i] != ifs.get()) {
      ifs.seek(startpos);
      return false;
    }
  }
  
  return true;
}
  
void MkrGGScriptReader::processDirective(BlackT::TStream& ifs) {
  skipSpace(ifs);
  
  std::string name = matchName(ifs);
  matchChar(ifs, '(');
  
  for (int i = 0; i < name.size(); i++) {
    name[i] = toupper(name[i]);
  }
  
  if (name.compare("LOADTABLE") == 0) {
    processLoadTable(ifs);
  }
//  else if (name.compare("STARTBLOCK") == 0) {
//    processStartBlock(ifs);
//  }
  else if (name.compare("SETMSGLOC") == 0) {
    processAddFreeBlock(ifs);
  }
  else if (name.compare("MSGPTR") == 0) {
    processAddMsgPtr(ifs);
  }
  else if (name.compare("ENDMSG") == 0) {
    processEndMsg(ifs);
  }
  else if (name.compare("JUMPIF") == 0) {
    processJumpIf(ifs);
  }
  else if (name.compare("JUMPIFNOT") == 0) {
    processJumpIfNot(ifs);
  }
  else {
    throw TGenericException(T_SRCANDLINE,
                            "MkrGGScriptReader::processDirective()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unknown directive: "
                              + name);
  }
  
  matchChar(ifs, ')');
}

void MkrGGScriptReader::processLoadTable(BlackT::TStream& ifs) {
  std::string tableName = matchString(ifs);
  TThingyTable table(tableName);
  loadThingy(table);
}

void MkrGGScriptReader::processAddFreeBlock(BlackT::TStream& ifs) {
  // argument 1: address of block we want to put data in
  int freeBlockPos = matchInt(ifs);
  matchChar(ifs, ',');
  // argument 2: size of destination block
  int freeBlockRemaining = matchInt(ifs);
  
  blockStart = freeBlockPos;
  
  spaceOfs << "[" << TStringConversion::intToString(freeBlockPos,
                        TStringConversion::baseHex) << "]"
    << std::endl;
  
  spaceOfs << "size=" << TStringConversion::intToString(freeBlockRemaining,
                        TStringConversion::baseHex)
    << std::endl << std::endl;
}

void MkrGGScriptReader::processAddMsgPtr(BlackT::TStream& ifs) {
  PointerStruct p;
  // argument 1: address of pointer we want to put data in
  p.address = matchInt(ifs);
  matchChar(ifs, ',');
  // argument 2: slot number through which pointer is accessed
  p.slotNum = matchInt(ifs);
  pointers.push_back(p);
}

void MkrGGScriptReader::processJumpIf(BlackT::TStream& ifs) {
  // argument 1: flag number
  int value = matchInt(ifs);
  matchChar(ifs, ',');
  // argument 2: name of label to jump to
  std::string label = matchName(ifs);
  
  currentScriptBuffer.put(0xFF);
  currentScriptBuffer.put(0x0F);
  currentScriptBuffer.put(value);
  pendingJumps[currentScriptBuffer.tell()] = label;
  currentScriptBuffer.put(0xFF);  // placeholder
}

void MkrGGScriptReader::processJumpIfNot(BlackT::TStream& ifs) {
  // argument 1: flag number
  int value = matchInt(ifs);
  matchChar(ifs, ',');
  // argument 2: name of label to jump to
  std::string label = matchName(ifs);
  
  currentScriptBuffer.put(0xFF);
  currentScriptBuffer.put(0x0E);
  currentScriptBuffer.put(value);
  pendingJumps[currentScriptBuffer.tell()] = label;
  currentScriptBuffer.put(0xFF);  // placeholder
}

void MkrGGScriptReader::processEndMsg(BlackT::TStream& ifs) {
  flushActiveScript();
}

void MkrGGScriptReader::skipSpace(BlackT::TStream& ifs) const {
  ifs.skipSpace();
}

bool MkrGGScriptReader::checkString(BlackT::TStream& ifs) const {
  skipSpace(ifs);
  
  if (!ifs.eof() && (ifs.peek() == '"')) return true;
  return false;
}

bool MkrGGScriptReader::checkInt(BlackT::TStream& ifs) const {
  skipSpace(ifs);
  
  if (!ifs.eof()
      && (isdigit(ifs.peek()) || ifs.peek() == '$')) return true;
  return false;
}

bool MkrGGScriptReader::checkChar(BlackT::TStream& ifs, char c) const {
  skipSpace(ifs);
  
  if (!ifs.eof() && (ifs.peek() == c)) return true;
  return false;
}

std::string MkrGGScriptReader::matchString(BlackT::TStream& ifs) const {
  skipSpace(ifs);
  if (ifs.eof()) {
    throw TGenericException(T_SRCANDLINE,
                            "MkrGGScriptReader::matchString()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected end of line");
  }
  
  if (!checkString(ifs)) {
    throw TGenericException(T_SRCANDLINE,
                            "MkrGGScriptReader::matchString()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected non-string at: "
                              + getRemainingContent(ifs));
  }
  
  ifs.get();
  
  std::string result;
  while (!ifs.eof() && (ifs.peek() != '"')) result += ifs.get();
  
  matchChar(ifs, '"');
  
  return result;
}

int MkrGGScriptReader::matchInt(BlackT::TStream& ifs) const {
  skipSpace(ifs);
  if (ifs.eof()) {
    throw TGenericException(T_SRCANDLINE,
                            "MkrGGScriptReader::matchInt()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected end of line");
  }
  
  if (!checkInt(ifs)) {
    throw TGenericException(T_SRCANDLINE,
                            "MkrGGScriptReader::matchInt()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected non-int at: "
                              + getRemainingContent(ifs));
  }
  
  std::string numstr;
  // get first character (this covers the case of an initial '$' for
  // hexadecimal)
  numstr += ifs.get();
  // handle possible initial "0x"
  if ((numstr[0] == '0') && (tolower(ifs.peek()) == 'x')) numstr += ifs.get();
  
  char next = ifs.peek();
  while (!ifs.eof()
         && (isdigit(next)
          || (tolower(next) == 'a')
          || (tolower(next) == 'b')
          || (tolower(next) == 'c')
          || (tolower(next) == 'd')
          || (tolower(next) == 'e')
          || (tolower(next) == 'f'))) {
    numstr += ifs.get();
    next = ifs.peek();
  }
  
  return TStringConversion::stringToInt(numstr);
}

void MkrGGScriptReader::matchChar(BlackT::TStream& ifs, char c) const {
  skipSpace(ifs);
  if (ifs.eof()) {
    throw TGenericException(T_SRCANDLINE,
                            "MkrGGScriptReader::matchChar()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected end of line");
  }
  
  if (ifs.peek() != c) {
    throw TGenericException(T_SRCANDLINE,
                            "MkrGGScriptReader::matchChar()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Expected '"
                              + c
                              + "', got '"
                              + ifs.get()
                              + "'");
  }
  
  ifs.get();
}
  
std::string MkrGGScriptReader
  ::getRemainingContent(BlackT::TStream& ifs) const {
  std::string content;
  while (!ifs.eof()) content += ifs.get();
  return content;
}

std::string MkrGGScriptReader::matchName(BlackT::TStream& ifs) const {
  skipSpace(ifs);
  if (ifs.eof()) {
    throw TGenericException(T_SRCANDLINE,
                            "MkrGGScriptReader::matchName()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected end of line");
  }
  
  if (!isalpha(ifs.peek())) {
    throw TGenericException(T_SRCANDLINE,
                            "MkrGGScriptReader::matchName()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Couldn't read name at: "
                              + getRemainingContent(ifs));
  }
  
  std::string result;
  result += ifs.get();
  while (!ifs.eof()
         && (isalnum(ifs.peek()) || (ifs.peek() == '_'))) {
    result += ifs.get();
  }
  
  return result;
}
  
void MkrGGScriptReader::processLabel(BlackT::TStream& ifs) {
  skipSpace(ifs);
  
  std::string name = matchName(ifs);
  labels[name] = currentScriptBuffer.tell();
  
//  std::cerr << "adding label: " << name << " " << currentScriptBuffer.tell() << std::endl;
}


}
