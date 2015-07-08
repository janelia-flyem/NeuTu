#include "zflyembookmark.h"
#include <iostream>

#include "zjsonobject.h"
#include "tz_math.h"
#include "zpainter.h"

ZFlyEmBookmark::ZFlyEmBookmark() :
  m_bodyId(0), m_bookmarkType(TYPE_LOCATION), m_isChecked(false),
  m_isCustom(false)
{
  m_type = ZStackObject::TYPE_FLYEM_BOOKMARK;
  m_visualEffect = NeuTube::Display::Sphere::VE_DOT_CENTER;
  setColor(255, 0, 0);
  useCosmeticPen(true);
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

void ZFlyEmBookmark::setCustom(bool state)
{
  m_isCustom = state;
}

void ZFlyEmBookmark::display(
    ZPainter &painter, int slice, EDisplayStyle option) const
{
  ZStackBall::display(painter, slice, option);

  if (isVisible()) {
    if (isSliceVisible(painter.getZ(slice))) {
      QString decorationText;
      if (m_isCustom) {
        decorationText = "u";
      }
      if (!decorationText.isEmpty()) {
//        painter.save();
        ZIntPoint center = getLocation();
        int width = decorationText.size() * 50;
        int height = 50;
        painter.drawText(center.getX(), center.getY(), width, height,
                         Qt::AlignLeft, decorationText);
//        painter.restore();
      }
    }
//    }
  }
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZFlyEmBookmark)
