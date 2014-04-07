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
  void append(ZHotSpot *hotSpot);
  void concat(ZHotSpotArray *spotArray);
  std::string toString() const;
  ZTextLineCompositer toLineCompositer() const;

  bool exportJsonFile(const std::string &filePath);
};

}

#endif // ZHOTSPOTARRAY_H
