#ifndef FLYEMTODOSOURCE_H
#define FLYEMTODOSOURCE_H

#include <vector>

class ZIntCuboid;
class ZFlyEmToDoItem;
class ZIntPoint;

class FlyEmTodoSource
{
public:
  FlyEmTodoSource();

  virtual std::vector<ZFlyEmToDoItem> getData(const ZIntCuboid &box) const = 0;
  virtual ZIntCuboid getRange() const = 0;
  virtual ZIntPoint getBlockSize() const;

};

#endif // FLYEMTODOSOURCE_H
