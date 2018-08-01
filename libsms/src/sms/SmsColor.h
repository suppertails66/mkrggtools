#ifndef SMSCOLOR_H
#define SMSCOLOR_H


#include "util/TStream.h"
#include "util/TColor.h"

namespace Sms {


class SmsColor {
public:
  SmsColor();
  
  bool operator==(const SmsColor& other) const;
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;
  
  void readGG(BlackT::TStream& ifs);
  void writeGG(BlackT::TStream& ofs) const;
  
  BlackT::TColor asTrueColor() const;
  void approximateTrueColor(BlackT::TColor color);
  
  BlackT::TColor asTrueColorGG() const;
  void approximateTrueColorGG(BlackT::TColor color);
  
  int r() const;
  void setR(int r__);
  int g() const;
  void setG(int g__);
  int b() const;
  void setB(int b__);
  
protected:
  const static int rMask = 0x03;
  const static int rShift = 0;
  const static int gMask = 0x0C;
  const static int gShift = 2;
  const static int bMask = 0x30;
  const static int bShift = 4;
  const static int trueColorShift = 6;
  const static int trueColorRange = 255;
  const static int smsColorRange = 3;
  
  const static int rMaskGG = 0x000F;
  const static int rShiftGG = 0;
  const static int gMaskGG = 0x00F0;
  const static int gShiftGG = 4;
  const static int bMaskGG = 0x0F00;
  const static int bShiftGG = 8;
  const static int trueColorShiftGG = 12;
  const static int ggColorRange = 15;
  
  int r_;
  int g_;
  int b_;
  
  void approximateTrueColorInternal(int src, int& dst);
  void approximateTrueColorInternalGG(int src, int& dst);
  
};


}


#endif
