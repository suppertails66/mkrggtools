#ifndef SMSPATTERN_H
#define SMSPATTERN_H


#include "util/TStream.h"
#include "util/TTwoDArray.h"
#include "util/TGraphic.h"
#include "sms/SmsPalette.h"
#include <cstdlib>

namespace Sms {


class SmsPattern {
public:
  const static int w = 8;
  const static int h = 8;
  const static int size = 32;
  const static int bitplanesPerRow = 4;

  SmsPattern();
  
  bool operator==(const SmsPattern& other) const;
  
  void read(BlackT::TStream& ifs);
  void write(BlackT::TStream& ofs) const;
  
  void toGraphic(BlackT::TGraphic& dst,
                 const SmsPalette* palette = NULL,
                 int xOffset = 0, int yOffset = 0,
                 bool transparency = false,
                 bool transparencyIsBlit = false,
                 bool ggMode = false) const;
  
  void approximateGraphic(const BlackT::TGraphic& src,
                 SmsPalette& dstPalette,
                 bool* colorsUsed,
                 const bool* colorsAvailable,
                 int xOffset = 0, int yOffset = 0,
                 bool transparency = false,
                 bool colorsLocked = false,
                 bool ggMode = false);
  
  int fromColorGraphic(const BlackT::TGraphic& src,
                        const SmsPalette& line,
                        int xoffset, int yoffset);
  
  int fromColorGraphicGG(const BlackT::TGraphic& src,
                        const SmsPalette& line,
                        int xoffset, int yoffset);
                 
  bool isEmpty() const;
  
  int data(int x, int y);
  int data(int x, int y) const;
  void setData(int x, int y, BlackT::TByte value);
  
  void clear();
  void flipV();
  void flipH();
protected:
  
  BlackT::TTwoDArray<BlackT::TByte> data_;
  
};


}


#endif
