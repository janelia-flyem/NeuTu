#include "zstackdvidgrayscalefactory.h"
#include "dvid/zdvidreader.h"

ZStackDvidGrayscaleFactory::ZStackDvidGrayscaleFactory()
{
}

ZStack* ZStackDvidGrayscaleFactory::makeStack(ZStack *stack) const
{
  ZDvidReader reader;
  if (reader.open(m_dvidTarget)) {
    ZStack *tmpStack = reader.readGrayScale(m_range);
    if (stack != NULL) {
      stack->swapData(tmpStack);
    } else {
      stack = tmpStack;
    }
  } else {
    return NULL;
  }

  return stack;
}
