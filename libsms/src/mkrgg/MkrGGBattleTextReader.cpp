#include "mkrgg/MkrGGBattleTextReader.h"
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

const static int bytesPerTextBox = (4 * 18);

MkrGGBattleTextReader::MkrGGBattleTextReader(
                  BlackT::TStream& src__,
                  BlackT::TStream& dst__,
                  const BlackT::TThingyTable& thingy__,
                  int outputAddr__)
  : src(src__),
    dst(dst__),
    thingy(thingy__),
    outputAddr(outputAddr__),
    lineNum(0),
    currentScriptBuffer(scriptBufferCapacity)
//    blockStart(0)
{
  loadThingy(thingy__);
  dst.seek(outputAddr);
}

void MkrGGBattleTextReader::operator()() {
  while (!src.eof()) {
    std::string line;
    src.getLine(line);
    ++lineNum;
    
    if (line.size() <= 0) continue;
    
    // discard lines containing only ASCII spaces and tabs
//    bool onlySpace = true;
//    for (int i = 0; i < line.size(); i++) {
//      if ((line[i] != ' ')
//          && (line[i] != '\t')) {
//        onlySpace = false;
//        break;
//      }
//    }
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
    
    while (!ifs.eof()) {
      outputNextSymbol(ifs);
    }
  }
}
  
void MkrGGBattleTextReader::loadThingy(const BlackT::TThingyTable& thingy__) {
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
  
void MkrGGBattleTextReader::outputNextSymbol(TStream& ifs) {
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
                          "MkrGGBattleTextReader::outputNextSymbol()",
                          "Line "
                            + TStringConversion::intToString(lineNum)
                            + ":\n  Couldn't match symbol at: '"
                            + remainder
                            + "'");
}

void MkrGGBattleTextReader::flushActiveScript() {
  if ((currentScriptBuffer.size() % bytesPerTextBox) != 0) {
    throw TGenericException(T_SRCANDLINE,
                            "MkrGGBattleTextReader::flushActiveScript()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Too many/few characters to fit text box"
                              + " (total: "
                              + TStringConversion::intToString(
                                  currentScriptBuffer.size())
                              + ")");
  }

  messageSets.push_back(currentScriptBuffer);
  currentScriptBuffer = BlackT::TBufStream(scriptBufferCapacity);
}
  
bool MkrGGBattleTextReader::checkSymbol(BlackT::TStream& ifs, std::string& symbol) {
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
  
void MkrGGBattleTextReader::processDirective(BlackT::TStream& ifs) {
  skipSpace(ifs);
  
  std::string name = matchName(ifs);
  matchChar(ifs, '(');
  
  for (int i = 0; i < name.size(); i++) {
    name[i] = toupper(name[i]);
  }
  
  if (name.compare("LOADTABLE") == 0) {
    processLoadTable(ifs);
  }
//  else if (name.compare("SETADDR") == 0) {
//    processSetAddr(ifs);
//  }
  else if (name.compare("ENDMSGSET") == 0) {
    processEndMsgSet(ifs);
  }
  else if (name.compare("ENDMSGSETGROUP") == 0) {
    processEndMsgSetGroup(ifs);
  }
  else if (name.compare("ADDQUANTITYPTR") == 0) {
    processAddQuantityPtr(ifs);
  }
  else if (name.compare("ADDMESSAGEPTR") == 0) {
    processAddMessagePtr(ifs);
  }
  else {
    throw TGenericException(T_SRCANDLINE,
                            "MkrGGBattleTextReader::processDirective()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unknown directive: "
                              + name);
  }
  
  matchChar(ifs, ')');
}

void MkrGGBattleTextReader::processLoadTable(BlackT::TStream& ifs) {
  std::string tableName = matchString(ifs);
  TThingyTable table(tableName);
  loadThingy(table);
}

/*void MkrGGBattleTextReader::processAddFreeBlock(BlackT::TStream& ifs) {
  // argument 1: address of block we want to put data in
  int freeBlockPos = matchInt(ifs);
  matchChar(ifs, ',');
  // argument 2: size of destination block
  int freeBlockRemaining = matchInt(ifs);
  
  blockStart = freeBlockPos;
}

void MkrGGBattleTextReader::processSetAddr(BlackT::TStream& ifs) {
  // argument 1: address of data
  int addr = matchInt(ifs);
  
  dst.seek(addr);
} */

void MkrGGBattleTextReader::processEndMsgSet(BlackT::TStream& ifs) {
  flushActiveScript();
}

void MkrGGBattleTextReader::processEndMsgSetGroup(BlackT::TStream& ifs) {
  int numMessageSets = messageSets.size();
  int quantityTableAddr = dst.tell();
  int addrTableAddr = quantityTableAddr + (numMessageSets * 1);
  int messagesAddr = addrTableAddr + (numMessageSets * 2);
  
  int putpos = messagesAddr;
  for (unsigned int i = 0; i < messageSets.size(); i++) {
    
    BlackT::TBufStream& stream = messageSets[i];
    int numMessages = stream.size() / bytesPerTextBox;
    int messageStartAddr = putpos + (numMessages * 2);
    
    // write count of messages to quantity table
    dst.seek(quantityTableAddr + (i * 1));
    dst.writeu8(numMessages);
    
    // write pointer to submessage pointer table
    dst.seek(addrTableAddr + (i * 2));
    dst.writeu16le((putpos % 0x4000) + 0x8000);
    
    // write pointer table for submessages
    dst.seek(putpos);
    for (int i = 0; i < numMessages; i++) {
      dst.writeu16le(((messageStartAddr + (i * bytesPerTextBox))
                       % 0x4000) + 0x8000);
    }
    
    // write messages
    stream.seek(0);
    while (!stream.eof()) dst.put(stream.get());
    putpos = dst.tell();
  }
  
  int endpos = dst.tell();
  
  // Update table pointers
  
  for (unsigned int i = 0; i < quantityPointers.size(); i++) {
    dst.seek(quantityPointers[i]);
    dst.writeu16le((quantityTableAddr % 0x4000) + 0x8000);
  }
  
  for (unsigned int i = 0; i < messagePointers.size(); i++) {
    dst.seek(messagePointers[i]);
    dst.writeu16le((addrTableAddr % 0x4000) + 0x8000);
  }
  
  dst.seek(endpos);
  
  // Clear all data in preparation for next set
  messageSets.clear();
  quantityPointers.clear();
  messagePointers.clear();
}

void MkrGGBattleTextReader::processAddQuantityPtr(BlackT::TStream& ifs) {
  // argument 1: address of data
  int addr = matchInt(ifs);
  
//  dst.seek(addr);
  quantityPointers.push_back(addr);
}

void MkrGGBattleTextReader::processAddMessagePtr(BlackT::TStream& ifs) {
  // argument 1: address of data
  int addr = matchInt(ifs);
  
//  dst.seek(addr);
  messagePointers.push_back(addr);
}

void MkrGGBattleTextReader::skipSpace(BlackT::TStream& ifs) const {
  ifs.skipSpace();
}

bool MkrGGBattleTextReader::checkString(BlackT::TStream& ifs) const {
  skipSpace(ifs);
  
  if (!ifs.eof() && (ifs.peek() == '"')) return true;
  return false;
}

bool MkrGGBattleTextReader::checkInt(BlackT::TStream& ifs) const {
  skipSpace(ifs);
  
  if (!ifs.eof()
      && (isdigit(ifs.peek()) || ifs.peek() == '$')) return true;
  return false;
}

bool MkrGGBattleTextReader::checkChar(BlackT::TStream& ifs, char c) const {
  skipSpace(ifs);
  
  if (!ifs.eof() && (ifs.peek() == c)) return true;
  return false;
}

std::string MkrGGBattleTextReader::matchString(BlackT::TStream& ifs) const {
  skipSpace(ifs);
  if (ifs.eof()) {
    throw TGenericException(T_SRCANDLINE,
                            "MkrGGBattleTextReader::matchString()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected end of line");
  }
  
  if (!checkString(ifs)) {
    throw TGenericException(T_SRCANDLINE,
                            "MkrGGBattleTextReader::matchString()",
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

int MkrGGBattleTextReader::matchInt(BlackT::TStream& ifs) const {
  skipSpace(ifs);
  if (ifs.eof()) {
    throw TGenericException(T_SRCANDLINE,
                            "MkrGGBattleTextReader::matchInt()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected end of line");
  }
  
  if (!checkInt(ifs)) {
    throw TGenericException(T_SRCANDLINE,
                            "MkrGGBattleTextReader::matchInt()",
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

void MkrGGBattleTextReader::matchChar(BlackT::TStream& ifs, char c) const {
  skipSpace(ifs);
  if (ifs.eof()) {
    throw TGenericException(T_SRCANDLINE,
                            "MkrGGBattleTextReader::matchChar()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected end of line");
  }
  
  if (ifs.peek() != c) {
    throw TGenericException(T_SRCANDLINE,
                            "MkrGGBattleTextReader::matchChar()",
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
  
std::string MkrGGBattleTextReader
  ::getRemainingContent(BlackT::TStream& ifs) const {
  std::string content;
  while (!ifs.eof()) content += ifs.get();
  return content;
}

std::string MkrGGBattleTextReader::matchName(BlackT::TStream& ifs) const {
  skipSpace(ifs);
  if (ifs.eof()) {
    throw TGenericException(T_SRCANDLINE,
                            "MkrGGBattleTextReader::matchName()",
                            "Line "
                              + TStringConversion::intToString(lineNum)
                              + ":\n  Unexpected end of line");
  }
  
  if (!isalpha(ifs.peek())) {
    throw TGenericException(T_SRCANDLINE,
                            "MkrGGBattleTextReader::matchName()",
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


}
