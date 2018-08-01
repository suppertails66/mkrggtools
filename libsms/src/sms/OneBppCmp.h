#ifndef ONEBPPCMP_H
#define ONEBPPCMP_H


#include "util/TStream.h"
#include "sms/SmsPattern.h"

namespace Sms {

class OneBppCmp {
public:
  static void cmpOneBpp(const SmsPattern& pattern, BlackT::TStream& dst);
  static void decmpOneBpp(BlackT::TStream& src, SmsPattern& pattern,
                          int index = 0x1);
protected:
};


}


#endif
