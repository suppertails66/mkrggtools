#ifndef SMSCRAM_H
#define SMSCRAM_H


#include "sms/SmsPalette.h"
#include "util/TStream.h"

namespace Sms {


class SmsCram {
public:
  SmsCram();
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;
  
protected:
  SmsPalette tilePalette_;
  SmsPalette spritePalette_;
};


}


#endif
