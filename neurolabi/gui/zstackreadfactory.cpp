#include "zstackreadfactory.h"

ZStackReadFactory::ZStackReadFactory()
{
}

ZStack* ZStackReadFactory::makeStack(ZStack *stack) const
{
  return m_source.readStack(stack);
}
