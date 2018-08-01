#include "sms/SmsTileId.h"
#include "util/ByteConversion.h"
#include "util/TStringConversion.h"

using namespace BlackT;

namespace Sms {


SmsTileId::SmsTileId()
  : priority(false),
    palette(0),
    vflip(false),
    hflip(false),
    pattern(0) { }

void SmsTileId::read(const char* data) {
  int raw = ByteConversion::fromBytes(data, 2,
    EndiannessTypes::little, SignednessTypes::nosign);
  priority = (raw & 0x1000) != 0;
  palette = (raw & 0x0800) >> 11;
  vflip = (raw & 0x0400) != 0;
  hflip = (raw & 0x0200) != 0;
  pattern = (raw & 0x01FF);
}

void SmsTileId::write(char* data) const {
  int raw = 0;
  if (priority) raw |= 0x1000;
  raw |= (palette << 11);
  if (vflip) raw |= 0x0400;
  if (hflip) raw |= 0x0200;
  raw |= pattern;
  
  ByteConversion::toBytes(raw, data, 2,
    EndiannessTypes::little, SignednessTypes::nosign);
}
  
void SmsTileId::toColorGraphic(TGraphic& dst,
                    const SmsVram& vram,
//                    const MdPalette& pal,
                    int x, int y,
                    bool ggMode) const {
  const SmsPattern& pat = vram.getPattern(pattern);
  
  TGraphic temp(SmsPattern::w, SmsPattern::h);
//  pat.toColorGraphic(temp, vram.getPalette(palette), 0, 0);
  pat.toGraphic(temp, &(vram.getPalette(palette)),
                0, 0,
                false,
                false,
                ggMode);
  
  if (vflip) temp.flipVertical();
  if (hflip) temp.flipHorizontal();
  
  dst.copy(temp,
           TRect(x, y, 0, 0),
           TRect(0, 0, 0, 0));
}
  
void SmsTileId::toGrayscaleGraphic(TGraphic& dst,
                    const SmsVram& vram,
                    int x, int y,
                    bool ggMode) const {
  const SmsPattern& pat = vram.getPattern(pattern);
  
  TGraphic temp(SmsPattern::w, SmsPattern::h);
  temp.clearTransparent();
//  pat.toGrayscaleGraphic(temp, 0, 0);
  pat.toGraphic(temp, NULL,
                0, 0,
                false,
                false,
                ggMode);
  
  if (vflip) temp.flipVertical();
  if (hflip) temp.flipHorizontal();
  
  dst.copy(temp,
           TRect(x, y, 0, 0),
           TRect(0, 0, 0, 0));
}


}
