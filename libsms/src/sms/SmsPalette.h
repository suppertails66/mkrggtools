#ifndef SMSPALETTE_H
#define SMSPALETTE_H


#include "util/TStream.h"
#include "sms/SmsColor.h"

namespace Sms {


class SmsPalette {
public:
  const static int numColors = 16;
  const static int spriteTransparencyIndex = 0;

  SmsPalette();
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;
  void readGG(BlackT::TStream& ifs);
  void writeGG(BlackT::TStream& ofs) const;
  
  SmsColor getColor(int index) const;
  void setColor(int index, SmsColor color);
  
  int indexOfColor(const BlackT::TColor& color) const;
  int indexOfColorGG(const BlackT::TColor& color) const;
  
//  bool colorSet(int index) const;
protected:
  SmsColor colors_[numColors];
//  bool colorsSet_[numColors];
  
};


}


#endif
