#include "sms/SmsVram.h"
#include "util/TStringConversion.h"
#include "exception/TGenericException.h"
#include <string>

using namespace BlackT;

namespace Sms {


SmsVram::SmsVram() { }

const SmsPattern& SmsVram::getPattern(int index) const {
  if (index >= numPatterns) {
    throw TGenericException(T_SRCANDLINE,
                            "SmsVram::getPattern(int)",
                            (std::string("Out-of-range index: ")
                              + TStringConversion::intToString(index))
                              .c_str());
  }
  
  return patterns[index];
}

SmsPattern SmsVram::setPattern(int index, const SmsPattern& pattern) {
  if (index >= numPatterns) {
    throw TGenericException(T_SRCANDLINE,
                            "SmsVram::setPattern(int)",
                            (std::string("Out-of-range index: ")
                              + TStringConversion::intToString(index))
                              .c_str());
  }
  
  return patterns[index] = pattern;
}

const SmsPalette& SmsVram::getPalette(int index) const {
  if (index == 0) return getTilePalette();
  else return getSpritePalette();
}

SmsPalette SmsVram::setPalette(int index, const SmsPalette& palette) {
  if (index == 0) return setTilePalette(palette);
  else return setSpritePalette(palette);
}

const SmsPalette& SmsVram::getTilePalette() const {
  return tilePalette;
}

SmsPalette SmsVram::setTilePalette(const SmsPalette& tilePalette__) {
  tilePalette = tilePalette__;
  return tilePalette;
}

const SmsPalette& SmsVram::getSpritePalette() const {
  return spritePalette;
}

SmsPalette SmsVram::setSpritePalette(const SmsPalette& spritePalette__) {
  spritePalette = spritePalette__;
  return spritePalette;
}
  
//void SmsVram::read(const char* data, int numPatterns,
//                  int startPattern) {
//  for (int i = 0; i < numPatterns; i++) {
//    patterns[startPattern + i].read(data + (i * SmsPattern::size));
//  }
//}

void SmsVram::read(BlackT::TStream& ifs, int numPatterns,
                   int startPattern) {
  for (int i = 0; i < numPatterns; i++) {
    patterns[startPattern + i].read(ifs);
  }
}


}
