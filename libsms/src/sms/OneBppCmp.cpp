#include "sms/OneBppCmp.h"
#include "util/TBufStream.h"
#include <vector>
#include <iostream>

using namespace BlackT;

namespace Sms {


void OneBppCmp::cmpOneBpp(const SmsPattern& pattern, BlackT::TStream& dst) {
  for (int j = 0; j < SmsPattern::h; j++) {
    int mask = 0x80;
    unsigned char next = 0;
    for (int i = 0; i < SmsPattern::w; i++) {
      if (pattern.data(i, j) != 0) next |= mask;
      mask >>= 1;
    }
    dst.put(next);
  }
}

void OneBppCmp::decmpOneBpp(BlackT::TStream& src, SmsPattern& pattern,
                            int index) {
  for (int j = 0; j < SmsPattern::h; j++) {
    unsigned char next = src.get();
    int mask = 0x80;
    for (int i = 0; i < SmsPattern::w; i++) {
      if ((next & mask) != 0) pattern.setData(i, j, index);
      mask >>= 1;
    }
  }
}


}
