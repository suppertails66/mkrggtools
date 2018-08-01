#ifndef MKRGGGRPCMP_H
#define MKRGGGRPCMP_H


#include "util/TStream.h"
#include "util/TThingyTable.h"
#include <map>
#include <string>
#include <iostream>

namespace Sms {


class MkrGGGrpCmp {
public:
  static void cmpMkrGG(BlackT::TStream& src, BlackT::TStream& dst);
  static void decmpMkrGG(BlackT::TStream& src, BlackT::TStream& dst);
  
protected:
  
};


}


#endif
