#include "sms/SmsCram.h"

using namespace BlackT;

namespace Sms {


SmsCram::SmsCram() { }
  
void SmsCram::read(BlackT::TStream& ifs) {
  tilePalette_.read(ifs);
  spritePalette_.read(ifs);
}

void SmsCram::write(BlackT::TStream& ofs) const {
  tilePalette_.write(ofs);
  spritePalette_.write(ofs);
}


}
