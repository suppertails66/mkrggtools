#include "sms/StCmp.h"
#include "util/TBufStream.h"
#include <vector>
#include <iostream>

using namespace BlackT;

namespace Sms {


/*void StCmp::cmpSt(BlackT::TStream& src, BlackT::TStream& dst) {
  TBufStream srcbuf(0x100000);
  srcbuf.writeFrom(src, src.remaining());
  srcbuf.seek(0);
  
  TBufStream dstbuf(0x100000);
  
  int resultSz = gfxcomp_phantasystar
    ::compress((uint8_t*)srcbuf.data().data(), srcbuf.remaining(),
               (uint8_t*)dstbuf.data().data(), dstbuf.remaining());
  dstbuf.setEndPos(resultSz);
  dstbuf.seek(0);
  dst.write(dstbuf.data().data(), dstbuf.size());
} */

void StCmp::decmpSt(BlackT::TStream& src, BlackT::TStream& dst) {
  while (true) {
    unsigned char cmd = src.get();
    
    for (int mask = 0x01; mask <= 0x80; mask <<= 1) {
      // bit set = literal
      if ((mask & cmd) != 0) {
        dst.put(src.get());
      }
      // bit unset = lookback
      else {
        int value = src.readu16le();
        // parameter of 0 = done
        if (value == 0) return;
        
        int offset = (value & 0x0FFF) - 0x1000;
        int len = ((value & 0xF000) >> 12) + 3;
        
        int pos = dst.tell();
        for (int i = 0; i < len; i++) {
          dst.seek(pos + offset);
          char next = dst.get();
          dst.seek(pos);
          dst.put(next);
          ++pos;
        }
      }
    }
    
  }
}


}
