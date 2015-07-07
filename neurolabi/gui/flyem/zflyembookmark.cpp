#include "zflyembookmark.h"
#include <iostream>

#include "zjsonobject.h"

ZFlyEmBookmark::ZFlyEmBookmark() :
  m_bodyId(-1), m_type(TYPE_LOCATION), m_isChecked(false)
{
}

void ZFlyEmBookmark::print() const
{
  std::cout << "Body ID: " << m_bodyId << std::endl;
  std::cout << "Location: " << m_location.toString() << std::endl;
}

QString ZFlyEmBookmark::getDvidKey() const
{
  return QString("%1_%2_%3").arg(m_location.getX()).arg(m_location.getY()).
      arg(m_location.getZ());
}

ZJsonObject ZFlyEmBookmark::toJsonObject() const
{
  ZJsonObject obj;
  obj.setEntry("body ID", m_bodyId);
  if (!m_status.isEmpty()) {
    obj.setEntry("status", m_status.toStdString());
  }
  int location[3];
  location[0] = m_location.getX();
  location[1] = m_location.getY();
  location[2] = m_location.getZ();
  obj.setEntry("location", location, 3);

  obj.setEntry("checked", m_isChecked);

  switch (m_type) {
  case TYPE_FALSE_MERGE:
    obj.setEntry("type", "false merge");
    break;
  case TYPE_FALSE_SPLIT:
    obj.setEntry("type", "false split");
    break;
  default:
    break;
  }

  return obj;
}
