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
  case CONV_RANDOM_COLOR:
    buildConvRandomColorTable(65536);
    break;
  default:
    break;
  }
}
