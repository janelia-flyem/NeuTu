#include "zflyembookmark.h"
#include <iostream>

#include "zjsonobject.h"
#include "tz_math.h"

ZFlyEmBookmark::ZFlyEmBookmark() :
  m_bodyId(-1), m_bookmarkType(TYPE_LOCATION), m_isChecked(false)
{
  m_type = ZStackObject::TYPE_FLYEM_BOOKMARK;
  m_visualEffect = NeuTube::Display::Sphere::VE_DOT_CENTER;
}

void ZFlyEmBookmark::print() const
{
  std::cout << "Body ID: " << m_bodyId << std::endl;
  std::cout << "Location: " << getLocation().toString() << std::endl;
}

QString ZFlyEmBookmark::getDvidKey() const
{
  return QString("%1_%2_%3").arg(iround(getCenter().x())).
      arg(iround(getCenter().x())).
      arg(iround(getCenter().x()));
}

ZJsonObject ZFlyEmBookmark::toJsonObject() const
{
  ZJsonObject obj;
  obj.setEntry("body ID", m_bodyId);
  if (!m_status.isEmpty()) {
    obj.setEntry("status", m_status.toStdString());
  }
  int location[3];
  location[0] = iround(getCenter().x());
  location[1] = iround(getCenter().y());
  location[2] = iround(getCenter().z());


  /*
  location[1] = m_location.getY();
  location[2] = m_location.getZ();
  */
  obj.setEntry("location", location, 3);

  obj.setEntry("checked", m_isChecked);

  switch (m_bookmarkType) {
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
