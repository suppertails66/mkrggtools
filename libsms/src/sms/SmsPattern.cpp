#include "sms/SmsPattern.h"
#include "smsexp/SmsNotEnoughSpaceException.h"
#include <iostream>

using namespace BlackT;

namespace Sms {


SmsPattern::SmsPattern()
  : data_(w, h) {
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      data_.data(i, j) = SmsPalette::spriteTransparencyIndex;
    }
  }
}

bool SmsPattern::operator==(const SmsPattern& other) const {
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      if (data_.data(i, j) != other.data_.data(i, j)) return false;
    }
  }
  
  return true;
}
  
void SmsPattern::read(BlackT::TStream& ifs) {
  clear();
  
  for (int j = 0; j < h; j++) {
    int dstmask = 0x01;
    for (int k = 0; k < bitplanesPerRow; k++) {
      TByte next = ifs.get();
      
      int mask = 0x80;
      for (int i = 0; i < w; i++) {
        if (next & mask) data_.data(i, j) |= dstmask;
        mask >>= 1;
      }
      
      dstmask <<= 1;
    }
  }
}

void SmsPattern::write(BlackT::TStream& ofs) const {
  for (int j = 0; j < h; j++) {
    int srcmask = 0x01;
    for (int k = 0; k < bitplanesPerRow; k++) {
      TByte next = 0;
      
      int mask = 0x80;
      for (int i = 0; i < w; i++) {
        if ((data_.data(i, j) & srcmask) != 0) next |= mask;
        mask >>= 1;
      }
      
      ofs.put(next);
      srcmask <<= 1;
    }
  }
}

void SmsPattern::toGraphic(BlackT::TGraphic& dst,
               const SmsPalette* palette,
               int xOffset, int yOffset,
               bool transparency,
               bool transparencyIsBlit,
               bool ggMode) const {
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      int x = xOffset + i;
      int y = yOffset + j;
      
      if (x < 0) continue;
      if (y < 0) continue;
      if (x >= dst.w()) continue;
      if (y >= dst.h()) continue;
      
      int rawValue = data_.data(i, j);
      
      TColor outputColor;
      if (palette != NULL) {
        // color
        if (ggMode)
          outputColor = palette->getColor(data_.data(i, j)).asTrueColorGG();
        else
          outputColor = palette->getColor(data_.data(i, j)).asTrueColor();
      }
      else {
        // grayscale
        int level = rawValue;
        level |= (level << 4);
        outputColor = TColor(level, level, level, TColor::fullAlphaOpacity);
      }
      
      if (transparency && (rawValue == 0))
        outputColor.setA(TColor::fullAlphaTransparency);
      
      if (transparencyIsBlit && (rawValue == 0)) {
      
      }
      else {
        dst.setPixel(x, y, outputColor);
      }
    }
  }
}

void SmsPattern::approximateGraphic(const BlackT::TGraphic& src,
               SmsPalette& dstPalette,
               bool* colorsUsed,
               const bool* colorsAvailable,
               int xOffset, int yOffset,
               bool transparency,
               bool colorsLocked,
               bool ggMode) {
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      int x = xOffset + i;
      int y = yOffset + j;
      
      // ignore pixels outside image dimensions
      if ((x < 0) || (x >= src.w()) || (y < 0) || (y >= src.h())) continue;
      
      TColor rawColor = src.getPixel(x, y);
      // handle transparency
      if (transparency && (rawColor.a() == TColor::fullAlphaTransparency)) {
        data_.data(i, j) = SmsPalette::spriteTransparencyIndex;
        continue;
      }
      
      SmsColor color;
      if (ggMode)
        color.approximateTrueColorGG(rawColor);
      else
        color.approximateTrueColor(rawColor);
      
      int index = -1;
      
      // check if any existing color matches target
      for (int i = 0; i < SmsPalette::numColors; i++) {
        if (!colorsAvailable[i]) continue;
        if ((colorsUsed[i]) && (dstPalette.getColor(i) == color)) {
          index = i;
          break;
        }
      }
      
      // match found
      if (index != -1) {
        data_.data(i, j) = index;
        continue;
      }
      
      // match not found
      if (colorsLocked) {
        throw SmsNotEnoughSpaceException(T_SRCANDLINE,
                                         "SmsPattern::approximateGraphic()",
                                         "Graphic uses colors not in palette");
      }
      
      // add new color
      
//      std::cout << color.r() << " " << color.g() << " " << color.b() << std::endl;
      for (int i = 0; i < SmsPalette::numColors; i++) {
        if (!colorsAvailable[i]) continue;
        if (!colorsUsed[i]) {
          colorsUsed[i] = true;
          index = i;
          dstPalette.setColor(index, color);
          break;
        }
      }
      
      if (index == -1) {
        throw SmsNotEnoughSpaceException(T_SRCANDLINE,
                                         "SmsPattern::approximateGraphic()",
                                         "Source data uses too many colors");
      }
      
      data_.data(i, j) = index;
    }
  }
}
  
int SmsPattern::fromColorGraphic(const BlackT::TGraphic& src,
               const SmsPalette& line,
               int xoffset, int yoffset) {
  
  // note that this simply chooses _a_ color that matches the
  // one from the source (if one exists in the palette).
  // if the same color is present multiple times, you may get
  // a different index from the original, which could cause problems
  // with dynamic palettes
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      int x = xoffset + i;
      int y = yoffset + j;
    
      TColor color = src.getPixel(x, y);
      int raw = line.indexOfColor(color);
      if (raw < 0) return raw;
      data_.data(i, j) = raw;
    }
  }
  
  return 0;
}
  
int SmsPattern::fromColorGraphicGG(const BlackT::TGraphic& src,
               const SmsPalette& line,
               int xoffset, int yoffset) {
  
  // note that this simply chooses _a_ color that matches the
  // one from the source (if one exists in the palette).
  // if the same color is present multiple times, you may get
  // a different index from the original, which could cause problems
  // with dynamic palettes
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      int x = xoffset + i;
      int y = yoffset + j;
    
      TColor color = src.getPixel(x, y);
      int raw = line.indexOfColorGG(color);
      if (raw < 0) return raw;
      data_.data(i, j) = raw;
    }
  }
  
  return 0;
}

bool SmsPattern::isEmpty() const {
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      if (data_.data(i, j) != 0) return false;
    }
  }
  
  return true;
}
  
int SmsPattern::data(int x, int y) {
  return data_.data(x, y);
}
  
int SmsPattern::data(int x, int y) const {
  return data_.data(x, y);
}

void SmsPattern::setData(int x, int y, BlackT::TByte value) {
  data_.data(x, y) = value;
}
  
void SmsPattern::clear() {
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      data_.data(i, j) = 0;
    }
  }
}

void SmsPattern::flipV() {
  data_.flipV();
}

void SmsPattern::flipH() {
  data_.flipH();
}


}
