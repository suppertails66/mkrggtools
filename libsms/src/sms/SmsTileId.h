#ifndef SMSTILEID_H
#define SMSTILEID_H


#include "util/TByte.h"
#include "util/TGraphic.h"
#include "sms/SmsVram.h"
#include "sms/SmsPalette.h"

namespace Sms {


class SmsTileId {
public:
  const static int size = 2;
  
  SmsTileId();
  
  void read(const char* data);
  void write(char* data) const;
  
  void toColorGraphic(BlackT::TGraphic& dst,
                      const SmsVram& vram,
//                      const SmsPalette& pal,
                      int x = 0, int y = 0,
                      bool ggMode = false) const;
  
  void toGrayscaleGraphic(BlackT::TGraphic& dst,
                      const SmsVram& vram,
                      int x = 0, int y = 0,
                      bool ggMode = false) const;
  
  bool priority;
  int palette;
  bool vflip;
  bool hflip;
  int pattern;
  
protected:
  
};


}


#endif
