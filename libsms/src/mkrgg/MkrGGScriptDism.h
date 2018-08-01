#ifndef MKRGGSCRIPTDISM_H
#define MKRGGSCRIPTDISM_H


#include "util/TStream.h"
#include "util/TThingyTable.h"
#include <map>
#include <string>
#include <iostream>

namespace Sms {


class MkrGGScriptDism {
public:
  
  MkrGGScriptDism(BlackT::TStream& ifs__,
                  std::ostream& ofs__,
                  BlackT::TThingyTable& thingy__,
                  int address__);
  
  void operator()(bool terminated = false);
  
protected:

  void handleOpcode(int opcode);
  void printRawValue(int value, int numBytes);
  std::string toRawHex(int value);
  
  std::map<int, std::string> posToLabel;
  
  BlackT::TStream* ifs;
  std::ostream* ofs;
  BlackT::TThingyTable* thingy;
  int address;
  
  bool terminatorFound;
  
  int lastNext;
  
};


}


#endif
