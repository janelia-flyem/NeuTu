#ifndef ZSTACKOBJECTARRAY_H
#define ZSTACKOBJECTARRAY_H

#include <vector>
#include "zstackobject.h"

class ZStackObjectArray : public std::vector<ZStackObject*>
{
public:
  ZStackObjectArray();
  ~ZStackObjectArray();

public:
  void deleteAll();
};

#endif // ZSTACKOBJECTARRAY_H
