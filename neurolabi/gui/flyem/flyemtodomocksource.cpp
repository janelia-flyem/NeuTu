#include "flyemtodomocksource.h"

#include <exception>

#include "zflyemtodoitem.h"

FlyEmTodoMockSource::FlyEmTodoMockSource()
{
  ZIntPoint pos(16, 16, 16);
  m_data[pos] = ZFlyEmToDoItem(pos);
  m_data[pos].setDefaultHit();
}

ZIntPoint FlyEmTodoMockSource::getBlockSize() const
{
  return ZIntPoint(32, 32, 32);
}

std::vector<ZFlyEmToDoItem> FlyEmTodoMockSource::getData(const ZIntCuboid &box) const
{
  std::vector<ZFlyEmToDoItem> data;

  for (const auto &d : m_data) {
    if (box.contains(d.first)) {
      data.push_back(d.second);
    }
  }
  /*
  data[0].setPosition(box.getCenter());
  data[0].setKind(ZDvidAnnotation::EKind::KIND_NOTE);
  */

  return data;
}

ZIntCuboid FlyEmTodoMockSource::getRange() const
{
  return ZIntCuboid(0, 0, 0, 511, 511, 511);
}

void FlyEmTodoMockSource::saveItem(const ZFlyEmToDoItem &item)
{
  m_data[item.getPosition()] = item;
}

void FlyEmTodoMockSource::removeItem(const ZIntPoint &pos)
{
  if (m_data.erase(pos) == 0) {
    throw std::runtime_error("No todo removed at " + pos.toString());
  }
}

void FlyEmTodoMockSource::moveItem(const ZIntPoint &from, const ZIntPoint &to)
{
  if (from != to) {
    if (m_data.count(from) > 0) {
      ZFlyEmToDoItem item = m_data[from];
      m_data.erase(from);
      item.setPosition(to);
      saveItem(item);
    }
  }
}

void FlyEmTodoMockSource::updatePartner(ZFlyEmToDoItem *item) const
{
  if (item) {
    item->addPartner(item->getPosition() + ZIntPoint(10, 20, 30));
  }
}

ZFlyEmToDoItem FlyEmTodoMockSource::getItem(const ZIntPoint &pos) const
{
  if (m_data.count(pos) > 0) {
    return m_data.at(pos);
  }
  return ZFlyEmToDoItem();
}
