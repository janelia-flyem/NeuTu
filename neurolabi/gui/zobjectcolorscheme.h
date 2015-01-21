#ifndef ZOBJECTCOLORSCHEME_H
#define ZOBJECTCOLORSCHEME_H

#include "zcolorscheme.h"

class ZObjectColorScheme : public ZColorScheme
{
public:
  ZObjectColorScheme();
  ~ZObjectColorScheme() {}

  virtual void setColorScheme(EColorScheme scheme);
};

#endif // ZOBJECTCOLORSCHEME_H
