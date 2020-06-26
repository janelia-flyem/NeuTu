#include "flyemtodomocksource.h"

#include "zflyemtodoitem.h"

FlyEmTodoMockSource::FlyEmTodoMockSource()
{

}

std::vector<ZFlyEmToDoItem> FlyEmTodoMockSource::getData(const ZIntCuboid &box) const
{
  std::vector<ZFlyEmToDoItem> data(1);
  data[0].setPosition(box.getCenter());
  data[0].setKind(ZDvidAnnotation::EKind::KIND_NOTE);

  return data;
}

ZIntCuboid FlyEmTodoMockSource::getRange() const
{
  return ZIntCuboid(0, 0, 0, 511, 511, 511);
}
