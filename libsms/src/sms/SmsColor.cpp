#include "sms/SmsColor.h"
#include <iostream>

using namespace BlackT;

namespace Sms {


SmsColor::SmsColor()
  : r_(0),
    g_(0),
    b_(0) { }

bool SmsColor::operator==(const SmsColor& other) const {
  if ((r_ == other.r_) && (g_ == other.g_) && (b_ == other.b_)) return true;
  return false;
}
  
void SmsColor::read(BlackT::TStream& ifs) {
  TByte next = ifs.get();
  r_ = (next & rMask) >> rShift;
  g_ = (next & gMask) >> gShift;
  b_ = (next & bMask) >> bShift;
}

void SmsColor::write(BlackT::TStream& ofs) const {
  TByte next = 0;
  next |= ((r_) << rShift);
  next |= ((g_) << gShift);
  next |= ((b_) << bShift);
  ofs.put(next);
}
  
void SmsColor::readGG(BlackT::TStream& ifs) {
  int next = ifs.readu16le();
  r_ = (next & rMaskGG) >> rShiftGG;
  g_ = (next & gMaskGG) >> gShiftGG;
  b_ = (next & bMaskGG) >> bShiftGG;
}

void SmsColor::writeGG(BlackT::TStream& ofs) const {
  int next = 0;
  next |= ((r_) << rShiftGG);
  next |= ((g_) << gShiftGG);
  next |= ((b_) << bShiftGG);
  ofs.writeu16le(next);
}
  
void SmsColor::approximateTrueColor(BlackT::TColor color) {
  approximateTrueColorInternal(color.r(), r_);
  approximateTrueColorInternal(color.g(), g_);
  approximateTrueColorInternal(color.b(), b_);
}
  
BlackT::TColor SmsColor::asTrueColor() const {
  int trueR = ((double)r_ / (double)smsColorRange)
                * (double)trueColorRange;
  int trueG = ((double)g_ / (double)smsColorRange)
                * (double)trueColorRange;
  int trueB = ((double)b_ / (double)smsColorRange)
                * (double)trueColorRange;

  return TColor(trueR, trueG, trueB, TColor::fullAlphaOpacity);
}
  
void SmsColor::approximateTrueColorGG(BlackT::TColor color) {
  approximateTrueColorInternalGG(color.r(), r_);
  approximateTrueColorInternalGG(color.g(), g_);
  approximateTrueColorInternalGG(color.b(), b_);
}
  
BlackT::TColor SmsColor::asTrueColorGG() const {
  int trueR = ((double)r_ / (double)ggColorRange)
                * (double)trueColorRange;
  int trueG = ((double)g_ / (double)ggColorRange)
                * (double)trueColorRange;
  int trueB = ((double)b_ / (double)ggColorRange)
                * (double)trueColorRange;

  return TColor(trueR, trueG, trueB, TColor::fullAlphaOpacity);
}

void SmsColor::approximateTrueColorInternal(int src, int& dst) {
  double sourceLevel = (double)src / (double)trueColorRange;
  
  // get the SMS color levels above and below the source color, as fractions
  // of 1
  double lower
    = (int)(src / ((double)trueColorRange / (double)smsColorRange))
                    / (double)smsColorRange;
  double upper = lower + ((double)1.0 / (double)smsColorRange);
  
//  std::cout << sourceLevel << " " << lower << " " << upper << std::endl;
  
  // the output color level is whichever the source is closer to
  if ((sourceLevel - lower) > (upper - sourceLevel)) {
    // closer to upper
    dst = (upper * smsColorRange);
  }
  else {
    // closer to lower
    dst = (lower * smsColorRange);
  }
}

void SmsColor::approximateTrueColorInternalGG(int src, int& dst) {
  double sourceLevel = (double)src / (double)trueColorRange;
  
  // get the GG color levels above and below the source color, as fractions
  // of 1
  double lower
    = (int)(src / ((double)trueColorRange / (double)ggColorRange))
                    / (double)ggColorRange;
  double upper = lower + ((double)1.0 / (double)ggColorRange);
  
//  std::cout << sourceLevel << " " << lower << " " << upper << std::endl;
  
  // the output color level is whichever the source is closer to
  if ((sourceLevel - lower) > (upper - sourceLevel)) {
    // closer to upper
    dst = (upper * ggColorRange);
  }
  else {
    // closer to lower
    dst = (lower * ggColorRange);
  }
}

int SmsColor::r() const {
  return r_;
}

void SmsColor::setR(int r__) {
  r_ = r__;
}

int SmsColor::g() const {
  return g_;
}

void SmsColor::setG(int g__) {
  g_ = g__;
}

int SmsColor::b() const {
  return b_;
}

void SmsColor::setB(int b__) {
  b_ = b__;
}


}
