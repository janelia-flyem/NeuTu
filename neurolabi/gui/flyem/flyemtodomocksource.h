#ifndef FLYEMTODOMOCKSOURCE_H
#define FLYEMTODOMOCKSOURCE_H

#include "flyemtodosource.h"

class FlyEmTodoMockSource : public FlyEmTodoSource
{
public:
  FlyEmTodoMockSource();

  std::vector<ZFlyEmToDoItem> getData(const ZIntCuboid &box) const override;
  ZIntCuboid getRange() const override;
};

#endif // FLYEMTODOMOCKSOURCE_H
