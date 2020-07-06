#ifndef FLYEMTODOMOCKSOURCE_H
#define FLYEMTODOMOCKSOURCE_H

#include <unordered_map>

#include "geometry/zintpoint.h"
#include "flyemtodosource.h"

class FlyEmTodoMockSource : public FlyEmTodoSource
{
public:
  FlyEmTodoMockSource();

  std::vector<ZFlyEmToDoItem> getData(const ZIntCuboid &box) const override;
  ZIntCuboid getRange() const override;

  void saveItem(const ZFlyEmToDoItem &item) override;
  void removeItem(const ZIntPoint &pos) override;

private:
  std::unordered_map<ZIntPoint, ZFlyEmToDoItem> m_data;
};

#endif // FLYEMTODOMOCKSOURCE_H
