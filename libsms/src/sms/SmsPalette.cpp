#include "sms/SmsPalette.h"
#include "exception/TGenericException.h"
#include "util/TStringConversion.h"

using namespace BlackT;

namespace Sms {


SmsPalette::SmsPalette() {
//  for (int i = 0; i < numColors; i++) colorsSet_[i] = false;
}
  
void SmsPalette::read(BlackT::TStream& ifs) {
  for (int i = 0; i < numColors; i++) colors_[i].read(ifs);
}

void SmsPalette::write(BlackT::TStream& ofs) const {
  for (int i = 0; i < numColors; i++) colors_[i].write(ofs);
}
  
void SmsPalette::readGG(BlackT::TStream& ifs) {
  for (int i = 0; i < numColors; i++) colors_[i].readGG(ifs);
}

void SmsPalette::writeGG(BlackT::TStream& ofs) const {
  for (int i = 0; i < numColors; i++) colors_[i].writeGG(ofs);
}

SmsColor SmsPalette::getColor(int index) const {
  if (index >= numColors) {
    throw TGenericException(T_SRCANDLINE,
                            "SmsPalette::getColor()",
                            "Out-of-range index: "
                              + TStringConversion::intToString(index));
  }
  
  return colors_[index];
}

void SmsPalette::setColor(int index, SmsColor color) {
  if (index >= numColors) {
    throw TGenericException(T_SRCANDLINE,
                            "SmsPalette::setColor()",
                            "Out-of-range index: "
                              + TStringConversion::intToString(index));
  }
  
  colors_[index] = color;
//  colorsSet_[index] = true;
}
  
/*bool SmsPalette::colorSet(int index) const {
  return colorsSet_[index];
} */

int SmsPalette::indexOfColor(const BlackT::TColor& color) const {
  SmsColor test;
  test.approximateTrueColor(color);
  for (int i = 0; i < numColors; i++) {
    if (colors_[i] == test) return i;
  }
  
  return -1;
}

int SmsPalette::indexOfColorGG(const BlackT::TColor& color) const {
  SmsColor test;
  test.approximateTrueColorGG(color);
  for (int i = 0; i < numColors; i++) {
    if (colors_[i] == test) return i;
  }
  
  return -1;
}


}
