#ifndef ZPUNCTUMCOLORSCHEME_H
#define ZPUNCTUMCOLORSCHEME_H

#include "zcolorscheme.h"

class ZPunctumColorScheme : public ZColorScheme
{
public:
  ZPunctumColorScheme();

  void setColorScheme(EColorScheme scheme);

private:
  void buildPunctumTypeColorTable();
};

#endif // ZPUNCTUMCOLORSCHEME_H
