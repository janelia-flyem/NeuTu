#include "zobjectcolorscheme.h"

ZObjectColorScheme::ZObjectColorScheme()
{
}


void ZObjectColorScheme::setColorScheme(EColorScheme scheme)
{
  switch (scheme) {
  case RANDOM_COLOR:
    buildRandomColorTable(32);
    break;
  default:
    break;
  }
}
