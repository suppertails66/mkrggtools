#include "mkrgg/MkrGGGrpCmp.h"
#include "sms/PsCmp.h"
#include "sms/StCmp.h"
#include "util/TStringConversion.h"
#include <iostream>

using namespace BlackT;

namespace Sms {


void MkrGGGrpCmp::cmpMkrGG(BlackT::TStream& src, BlackT::TStream& dst) {
  // only PS compression is supported
  dst.put(0);
  PsCmp::cmpPs(src, dst, 4);
}

void MkrGGGrpCmp::decmpMkrGG(BlackT::TStream& src, BlackT::TStream& dst) {
  unsigned char next = src.get();
  if (next == 0) {
    PsCmp::decmpPs(src, dst, 4);
  }
  else {
    StCmp::decmpSt(src, dst);
  }
}


}
