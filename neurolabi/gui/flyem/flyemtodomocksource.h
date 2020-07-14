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
  void updatePartner(ZFlyEmToDoItem *item) const override;
  ZFlyEmToDoItem getItem(const ZIntPoint &pos) const override;
  void moveItem(const ZIntPoint &from, const ZIntPoint &to);

private:
  std::unordered_map<ZIntPoint, ZFlyEmToDoItem> m_data;
};

#endif // FLYEMTODOMOCKSOURCE_H
