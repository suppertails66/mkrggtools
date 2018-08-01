#include "sms/SmsTilemap.h"

using namespace BlackT;

namespace Sms {


SmsTilemap::SmsTilemap() { }
  
void SmsTilemap::resize(int w, int h) {
  tileIds.resize(w, h);
}

const SmsTileId& SmsTilemap::getTileId(int x, int y) const {
  return tileIds.data(x, y);
}

void SmsTilemap::setTileId(int x, int y, const SmsTileId& tileId) {
  tileIds.data(x, y) = tileId;
}

void SmsTilemap::read(const char* src, int w, int h) {
  resize(w, h);
  
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      tileIds.data(i, j).read(src);
      src += SmsTileId::size;
    }
  }
}
  
void SmsTilemap::toColorGraphic(BlackT::TGraphic& dst,
                    const SmsVram& vram,
                    bool ggMode
//                    const SmsPalette& pal
                    ) {
  dst.resize(tileIds.w() * SmsPattern::w,
             tileIds.h() * SmsPattern::h);
  dst.clearTransparent();
  
  for (int j = 0; j < tileIds.h(); j++) {
    for (int i = 0; i < tileIds.w(); i++) {
      tileIds.data(i, j).toColorGraphic(
        dst, vram, (i * SmsPattern::w), (j * SmsPattern::h), ggMode);
    }
  }
}
  
void SmsTilemap::toGrayscaleGraphic(BlackT::TGraphic& dst,
                    const SmsVram& vram,
                    bool ggMode) {
  dst.resize(tileIds.w() * SmsPattern::w,
             tileIds.h() * SmsPattern::h);
  dst.clearTransparent();
  
  for (int j = 0; j < tileIds.h(); j++) {
    for (int i = 0; i < tileIds.w(); i++) {
      tileIds.data(i, j).toGrayscaleGraphic(
        dst, vram, (i * SmsPattern::w), (j * SmsPattern::h), ggMode);
    }
  }
}


}
