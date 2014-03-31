#include "zhotspotarray.h"
#include "ztextlinearray.h"
#include "zstring.h"

FlyEm::ZHotSpotArray::ZHotSpotArray()
{
}

FlyEm::ZHotSpotArray::~ZHotSpotArray()
{
  for (iterator iter = begin(); iter != end(); ++iter) {
    delete *iter;
  }
}

void FlyEm::ZHotSpotArray::append(ZHotSpot *hotSpot)
{
  push_back(hotSpot);
}

ZTextLineCompositer FlyEm::ZHotSpotArray::toLineCompositer() const
{
  ZTextLineCompositer text;
  ZString line;
  line.appendNumber(size());
  line += " hot spots:";
  text.appendLine(line);

  for (const_iterator iter = begin(); iter != end(); ++iter) {
    text.appendLine((*iter)->toLineCompositer(), 1);
  }

  return text;
}

std::string FlyEm::ZHotSpotArray::toString() const
{
  return toLineCompositer().toString(2);
}
