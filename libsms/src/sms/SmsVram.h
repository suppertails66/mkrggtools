#ifndef SMSVRAM_H
#define SMSVRAM_H


#include "util/TByte.h"
#include "util/TStream.h"
#include "sms/SmsPattern.h"
#include "sms/SmsPalette.h"

namespace Sms {


class SmsVram {
public:
  const static int numPatterns = 0x200;
  const static int numBytes = numPatterns * SmsPattern::size;
  const static int numPaletteLines = 2;

  SmsVram();
  
  const SmsPattern& getPattern(int index) const;
  SmsPattern setPattern(int index, const SmsPattern& pattern);
  
  const SmsPalette& getPalette(int index) const;
  SmsPalette setPalette(int index, const SmsPalette& palette);
  const SmsPalette& getTilePalette() const;
  SmsPalette setTilePalette(const SmsPalette& tilePalette__);
  const SmsPalette& getSpritePalette() const;
  SmsPalette setSpritePalette(const SmsPalette& spritePalette__);
  
//  void read(const char* data, int numPatterns,
//            int startPattern = 0);
  
  void read(BlackT::TStream& ifs, int numPatterns,
            int startPattern = 0);
protected:
  
  SmsPattern patterns[numPatterns];
  SmsPalette tilePalette;
  SmsPalette spritePalette;
  
};


}


#endif
