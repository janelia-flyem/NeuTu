#ifndef FLYEMTODODVIDSOURCE_H
#define FLYEMTODODVIDSOURCE_H

#include "flyemtodosource.h"
#include "dvid/zdvidwriter.h"

class FlyEmTodoDvidSource : public FlyEmTodoSource
{
public:
  FlyEmTodoDvidSource();
  ~FlyEmTodoDvidSource() override;

  void setDvidTarget(const ZDvidTarget &target);

  std::vector<ZFlyEmToDoItem> getData(const ZIntCuboid &box) const override;
  ZIntCuboid getRange() const override;

  void saveItem(const ZFlyEmToDoItem &item) override;
  void removeItem(const ZIntPoint &pos) override;
//  void updateItem(ZFlyEmToDoItem *item) const override;
  ZFlyEmToDoItem getItem(const ZIntPoint &pos) const override;
  void moveItem(const ZIntPoint &/*from*/, const ZIntPoint &/*to*/) override
  {/*todo*/}

private:
  ZDvidWriter m_writer;
};

#endif // FLYEMTODODVIDSOURCE_H
