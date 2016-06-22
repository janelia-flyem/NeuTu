#ifndef ZSWCCOLORSCHEME_H
#define ZSWCCOLORSCHEME_H


#include "zcolorscheme.h"

class ZSwcColorScheme : public ZColorScheme
{
public:
  ZSwcColorScheme();
  ~ZSwcColorScheme() {}

//  enum EColorScheme {
//    VAA3D_TYPE_COLOR, BIOCYTIN_TYPE_COLOR, JIN_TYPE_COLOR, UNIQUE_COLOR
//  };

  virtual void setColorScheme(EColorScheme scheme);

private:
  void buildVaa3dColorTable();
  void buildBiocytinColorTable();
  void buildUniqueColorTable();
  void buildJinTypeColorTable();
  void buildGmuTypeColorTable();
};

#endif // ZSWCCOLORSCHEME_H
