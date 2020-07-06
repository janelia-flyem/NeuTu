#ifndef FLYEMTODODVIDSOURCE_H
#define FLYEMTODODVIDSOURCE_H

#include "flyemtodosource.h"
#include "dvid/zdvidwriter.h"

class FlyEmTodoDvidSource : public FlyEmTodoSource
{
public:
  FlyEmTodoDvidSource();
  virtual ~FlyEmTodoDvidSource();

  void setDvidTarget(const ZDvidTarget &target);

  std::vector<ZFlyEmToDoItem> getData(const ZIntCuboid &box) const override;
  ZIntCuboid getRange() const override;

  void saveItem(const ZFlyEmToDoItem &item) override;
  void removeItem(const ZIntPoint &pos) override;

private:
  ZDvidWriter m_writer;
};

#endif // FLYEMTODODVIDSOURCE_H
