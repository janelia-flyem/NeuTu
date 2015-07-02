#include "zflyembookmark.h"
#include <iostream>

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
