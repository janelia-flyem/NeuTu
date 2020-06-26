#include "flyemtodosource.h"

#include "geometry/zintpoint.h"

FlyEmTodoSource::FlyEmTodoSource()
{
}

ZIntPoint FlyEmTodoSource::getBlockSize() const
{
  return ZIntPoint(32, 32, 32);
}
