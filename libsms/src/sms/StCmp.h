#ifndef STCMP_H
#define STCMP_H


#include "util/TStream.h"

namespace Sms {

class StCmp {
public:
//  static void cmpSt(BlackT::TStream& src, BlackT::TStream& dst);
  static void decmpSt(BlackT::TStream& src, BlackT::TStream& dst);
protected:
};


}


#endif
