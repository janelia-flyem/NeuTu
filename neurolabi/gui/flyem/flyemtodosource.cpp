#include "flyemtodosource.h"

#include "geometry/zintpoint.h"

FlyEmTodoSource::FlyEmTodoSource()
{
}

ZIntPoint FlyEmTodoSource::getBlockSize() const
{
  return ZIntPoint(32, 32, 32);
}

void FlyEmTodoSource::removeItem(int x, int y, int z)
{
  removeItem(ZIntPoint(x, y, z));
}
