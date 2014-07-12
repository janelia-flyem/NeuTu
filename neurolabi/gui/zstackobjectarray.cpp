#include "zstackobjectarray.h"

ZStackObjectArray::ZStackObjectArray()
{
}

ZStackObjectArray::~ZStackObjectArray()
{
  for (iterator iter = begin(); iter != end(); ++iter) {
    delete *iter;
  }

  clear();
}
