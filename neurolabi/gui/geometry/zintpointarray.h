#ifndef ZINTPOINTARRAY_H
#define ZINTPOINTARRAY_H

#include <vector>
#include "core/zsharedpointer.h"
#include "zintpoint.h"

class ZIntPointArray;

typedef ZSharedPointer<ZIntPointArray> ZIntPointArrayPtr;

class ZIntPointArray : public std::vector<ZIntPoint>
{
public:
  ZIntPointArray();

  void append(const ZIntPoint &pt);
  void append(int x, int y, int z);
  template<typename InputIterator>
  void append(InputIterator first, InputIterator last);

  static ZIntPointArrayPtr MakePointer();
};

template<typename InputIterator>
void ZIntPointArray::append(InputIterator first, InputIterator last)
{
  for(InputIterator iter = first; iter != last; ++iter) {
    append(*iter);
  }
}

#endif // ZINTPOINTARRAY_H
