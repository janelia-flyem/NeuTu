#ifndef FLYEMTODOSOURCE_H
#define FLYEMTODOSOURCE_H

#include <vector>

class ZIntCuboid;
class ZFlyEmToDoItem;

class FlyEmTodoSource
{
public:
  FlyEmTodoSource();

  virtual std::vector<ZFlyEmToDoItem> getData(const ZIntCuboid &box) const = 0;

};

#endif // FLYEMTODOSOURCE_H
