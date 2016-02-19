#include "zhotspotarray.h"
#include <algorithm>
#include "ztextlinearray.h"
#include "zstring.h"
#include "zjsonobject.h"
#include "zjsonparser.h"

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
  if (hotSpot != NULL) {
    push_back(hotSpot);
  }
}

ZTextLineCompositer FlyEm::ZHotSpotArray::toLineCompositer() const
{
  ZTextLineCompositer text;
  ZString line;
  line.appendNumber((uint64_t) size());
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

void FlyEm::ZHotSpotArray::print() const
{
  std::cout << toString() << std::endl;
}

void FlyEm::ZHotSpotArray::concat(FlyEm::ZHotSpotArray *spotArray)
{
#ifdef _DEBUG_
  std::cout << spotArray->toString() << std::endl;
#endif
  if (spotArray != NULL) {
    insert(end(), spotArray->begin(), spotArray->end());
    spotArray->clear();
  }
}

bool FlyEm::ZHotSpotArray::exportJsonFile(const std::string &filePath)
{
  ZJsonObject obj;
  ZJsonArray arrayObj;
  for (FlyEm::ZHotSpotArray::const_iterator iter = begin(); iter != end();
       ++iter) {
    ZHotSpot *hotSpot = *iter;
    ZJsonObject spotObj = hotSpot->toJsonObject();
    arrayObj.append(spotObj);
  }

  obj.setEntry("hot_spot", arrayObj);

  return obj.dump(filePath);
}

std::string FlyEm::ZHotSpotArray::toJsonString() const
{
  ZJsonObject obj;
  if (!empty()) {
    ZJsonArray arrayObj;
    for (FlyEm::ZHotSpotArray::const_iterator iter = begin(); iter != end();
         ++iter) {
      ZHotSpot *hotSpot = *iter;
      ZJsonObject spotObj = hotSpot->toJsonObject();
      arrayObj.append(spotObj);
    }

    obj.setEntry("hot_spot", arrayObj);
  } else {
    obj.consumeEntry("hot_spot", json_array());
  }

  return obj.dumpString();
}

bool FlyEm::ZHotSpotArray::exportRavelerBookmark(
    const std::string &filePath,
    const double *resolution,
    const int *imageSize)
{
  ZJsonObject obj;
  ZJsonArray arrayObj;
  for (FlyEm::ZHotSpotArray::const_iterator iter = begin(); iter != end();
       ++iter) {
    ZHotSpot *hotSpot = *iter;
    ZJsonObject spotObj = hotSpot->toRavelerJsonObject(resolution, imageSize);
    arrayObj.append(spotObj);
  }

  obj.setEntry("data", arrayObj);

  return obj.dump(filePath);
}

void FlyEm::ZHotSpotArray::sort()
{
  std::sort(begin(), end(), ZHotSpot::compareConfidence);
}

ZPointArray FlyEm::ZHotSpotArray::toPointArray() const
{
  ZPointArray ptArray;
  for (ZHotSpotArray::const_iterator iter = begin(); iter != end(); ++iter) {
    const ZHotSpot *hotSpot = *iter;
    ptArray.append(hotSpot->toPointArray());
  }

  return ptArray;
}

ZLineSegmentArray FlyEm::ZHotSpotArray::toLineSegmentArray() const
{
  ZLineSegmentArray lineArray;
  for (ZHotSpotArray::const_iterator iter = begin(); iter != end(); ++iter) {
    const ZHotSpot *hotSpot = *iter;
    lineArray.append(hotSpot->toLineSegmentArray());
  }

  return lineArray;
}
