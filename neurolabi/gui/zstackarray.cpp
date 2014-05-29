#include "zstackarray.h"
#include "zstack.hxx"

ZStackArray::ZStackArray()
{
}

ZStackArray::ZStackArray(const std::vector<ZStack *> &stackArray)
{
  insert(end(), stackArray.begin(), stackArray.end());
}

ZStackArray::~ZStackArray()
{
  for (iterator iter = begin(); iter != end(); ++iter) {
    delete *iter;
  }
}

void ZStackArray::paste(ZStack *stack, int valueIgnored) const
{
  if (stack != NULL) {
    for (const_iterator iter = begin(); iter != end(); ++iter) {
      const ZStack *src = *iter;
      src->paste(stack, valueIgnored);
    }
  }
}
