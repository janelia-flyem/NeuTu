#ifndef ZHOTSPOTARRAY_H
#define ZHOTSPOTARRAY_H

#include <vector>
#include <string>
#include "flyem/zhotspot.h"
#include "ztextlinecompositer.h"
#include "zpointarray.h"
#include "zlinesegmentarray.h"

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
  void print() const;
  ZTextLineCompositer toLineCompositer() const;

  std::string toJsonString() const;
  bool exportJsonFile(const std::string &filePath);
  bool exportRavelerBookmark(const std::string &filePath,
                             const double *resolution,
                             const int *imageSize);

  ZPointArray toPointArray() const;
  ZLineSegmentArray toLineSegmentArray() const;

  /*!
   * \brief Sort the hot spots by in the descending order of confidence.
   */
  void sort();
};

}

#endif // ZHOTSPOTARRAY_H
