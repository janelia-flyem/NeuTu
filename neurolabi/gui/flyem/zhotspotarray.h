#ifndef ZHOTSPOTARRAY_H
#define ZHOTSPOTARRAY_H

#include <vector>
#include <string>
#include "flyem/zhotspot.h"
#include "ztextlinecompositer.h"

namespace FlyEm {

class ZHotSpotArray : public std::vector<ZHotSpot*>
{
public:
  ZHotSpotArray();
  ~ZHotSpotArray();

public:
  void append(FlyEm::ZHotSpot *hotSpot);
  void concat(ZHotSpotArray *spotArray);
  std::string toString() const;
  ZTextLineCompositer toLineCompositer() const;

  std::string toJsonString() const;
  bool exportJsonFile(const std::string &filePath);
  bool exportRavelerBookmark(const std::string &filePath,
                             const double *resolution,
                             const int *imageSize);
};

}

#endif // ZHOTSPOTARRAY_H
