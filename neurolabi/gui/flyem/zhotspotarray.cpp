#include "zhotspotarray.h"
#include <algorithm>
#include "ztextlinearray.h"
#include "zstring.h"
#include "zjsonobject.h"
#include "zjsonparser.h"

flyem::ZHotSpotArray::ZHotSpotArray()
{
}

flyem::ZHotSpotArray::~ZHotSpotArray()
{
  for (iterator iter = begin(); iter != end(); ++iter) {
    delete *iter;
  }
}

void flyem::ZHotSpotArray::append(ZHotSpot *hotSpot)
{
  if (hotSpot != NULL) {
    push_back(hotSpot);
  }
}

ZTextLineCompositer flyem::ZHotSpotArray::toLineCompositer() const
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

std::string flyem::ZHotSpotArray::toString() const
{
  return toLineCompositer().toString(2);
}

void flyem::ZHotSpotArray::print() const
{
  std::cout << toString() << std::endl;
}

void flyem::ZHotSpotArray::concat(flyem::ZHotSpotArray *spotArray)
{
#ifdef _DEBUG_
  std::cout << spotArray->toString() << std::endl;
#endif
  if (spotArray != NULL) {
    insert(end(), spotArray->begin(), spotArray->end());
    spotArray->clear();
  }
}

bool flyem::ZHotSpotArray::exportJsonFile(const std::string &filePath)
{
  ZJsonObject obj;
  ZJsonArray arrayObj;
  for (flyem::ZHotSpotArray::const_iterator iter = begin(); iter != end();
       ++iter) {
    ZHotSpot *hotSpot = *iter;
    ZJsonObject spotObj = hotSpot->toJsonObject();
    arrayObj.append(spotObj);
  }

  obj.setEntry("hot_spot", arrayObj);

  return obj.dump(filePath);
}

std::string flyem::ZHotSpotArray::toJsonString() const
{
  ZJsonObject obj;
  if (!empty()) {
    ZJsonArray arrayObj;
    for (flyem::ZHotSpotArray::const_iterator iter = begin(); iter != end();
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

bool flyem::ZHotSpotArray::exportRavelerBookmark(
    const std::string &filePath,
    const double *resolution,
    const int *imageSize)
{
  ZJsonObject obj;
  ZJsonArray arrayObj;
  for (flyem::ZHotSpotArray::const_iterator iter = begin(); iter != end();
       ++iter) {
    ZHotSpot *hotSpot = *iter;
    ZJsonObject spotObj = hotSpot->toRavelerJsonObject(resolution, imageSize);
    arrayObj.append(spotObj);
  }

  obj.setEntry("data", arrayObj);

  return obj.dump(filePath);
}

void flyem::ZHotSpotArray::sort()
{
  std::sort(begin(), end(), ZHotSpot::compareConfidence);
}

ZPointArray flyem::ZHotSpotArray::toPointArray() const
{
  ZPointArray ptArray;
  for (ZHotSpotArray::const_iterator iter = begin(); iter != end(); ++iter) {
    const ZHotSpot *hotSpot = *iter;
    ptArray.append(hotSpot->toPointArray());
  }

  return ptArray;
}

ZLineSegmentArray flyem::ZHotSpotArray::toLineSegmentArray() const
{
  ZLineSegmentArray lineArray;
  for (ZHotSpotArray::const_iterator iter = begin(); iter != end(); ++iter) {
    const ZHotSpot *hotSpot = *iter;
    lineArray.append(hotSpot->toLineSegmentArray());
  }

  return lineArray;
}
