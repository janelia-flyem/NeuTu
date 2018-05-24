#include "zobjsitem.h"
#include <iostream>

ZObjsItem::ZObjsItem(const QList<QVariant> &data, void *pData, ZObjsItem *parent)
{
  m_parentItem = parent;
  m_itemData = data;
  m_actualObj = pData;
  m_checkState = Qt::Checked;

#ifdef _DEBUG_2
  std::cout << "Item created: " << this << std::endl;
#endif
}

ZObjsItem::~ZObjsItem()
{
  qDeleteAll(m_childItems);
#ifdef _DEBUG_2
  std::cout << "Item deleted: " << this << std::endl;
#endif
}

void ZObjsItem::appendChild(ZObjsItem *item)
{
  m_childItems.append(item);
}

ZObjsItem *ZObjsItem::child(int row)
{
  return m_childItems.value(row);
}

int ZObjsItem::childCount() const
{
  return m_childItems.count();
}

int ZObjsItem::columnCount() const
{
  return m_itemData.count();
}

QVariant ZObjsItem::data(int column) const
{
  return m_itemData.value(column);
}

ZObjsItem *ZObjsItem::parent()
{
  return m_parentItem;
}

int ZObjsItem::row() const
{
  if (m_parentItem)
    return m_parentItem->m_childItems.indexOf(const_cast<ZObjsItem*>(this));

  return 0;
}
