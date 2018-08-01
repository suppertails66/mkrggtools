#ifndef SMSTILEMAP_H
#define SMSTILEMAP_H


#include "util/TArray.h"
#include "util/TTwoDArray.h"
#include "util/TByte.h"
#include "util/TGraphic.h"
#include "sms/SmsTileId.h"

namespace Sms {


class SmsTilemap {
public:
  
  SmsTilemap();
  
  void resize(int w, int h);
  const SmsTileId& getTileId(int x, int y) const;
  void setTileId(int x, int y, const SmsTileId& tileId);
  
  void read(const char* src, int w, int h);
  
  void toColorGraphic(BlackT::TGraphic& dst,
                      const SmsVram& vram,
                      bool ggMode = false
//                      const SmsPalette& pal
                      );
  
  void toGrayscaleGraphic(BlackT::TGraphic& dst,
                      const SmsVram& vram,
                      bool ggMode = false);
  
  BlackT::TTwoDArray<SmsTileId> tileIds;
protected:
};


}


#endif
