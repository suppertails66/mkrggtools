#ifndef PSCMP_H
#define PSCMP_H


#include "util/TStream.h"

namespace Sms {

class PsCmp {
public:
  static void cmpPs(BlackT::TStream& src, BlackT::TStream& dst,
                    int interleaving);
  static void decmpPs(BlackT::TStream& src, BlackT::TStream& dst,
                    int interleaving);
protected:
  static void decmpPsPlane(BlackT::TStream& src, BlackT::TStream& dst);
};


}


#endif
