#include "zflyembookmark.h"
#include <iostream>

#include "zjsonobject.h"
#include "tz_math.h"
#include "zpainter.h"
#include "zstring.h"
#include "zjsonparser.h"
#include "zjsonarray.h"

ZFlyEmBookmark::ZFlyEmBookmark() :
  m_bodyId(0), m_bookmarkType(TYPE_LOCATION), m_isChecked(false),
  m_isCustom(false), m_isInTable(true)
{
  m_type = ZStackObject::TYPE_FLYEM_BOOKMARK;
  m_visualEffect = NeuTube::Display::Sphere::VE_DOT_CENTER;
  setColor(255, 0, 0);
  setRadius(5.0);
  setHittable(false);
  useCosmeticPen(true);
}

void ZFlyEmBookmark::clear()
{
  m_bodyId = 0;
  m_bookmarkType = TYPE_LOCATION;
  m_isChecked = false;
  m_isCustom = false;
  m_userName.clear();
  m_comment.clear();
  m_status.clear();
  m_time.clear();
}

void ZFlyEmBookmark::print() const
{
  std::cout << "Body ID: " << m_bodyId << std::endl;
  std::cout << "Location: " << getLocation().toString() << std::endl;
}

QString ZFlyEmBookmark::getDvidKey() const
{
  return QString("%1_%2_%3").arg(iround(getCenter().x())).
      arg(iround(getCenter().y())).
      arg(iround(getCenter().z()));
}

ZJsonObject ZFlyEmBookmark::toJsonObject(bool ignoringComment) const
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
  if (isCustom()) {
    obj.setEntry("custom", isCustom());
  }

  std::string text;
  switch (m_bookmarkType) {
  case TYPE_FALSE_MERGE:
    obj.setEntry("type", std::string("Split"));
    text = "split";
//    obj.setEntry("text", "split <username=" + m_userName.toStdString() + ">");
    break;
  case TYPE_FALSE_SPLIT:
    obj.setEntry("type", std::string("Merge"));
    text = "merge";
//    obj.setEntry("text", "merge <username=" + m_userName.toStdString() + ">");
    break;
  default:
    text = "other";
//    obj.setEntry("text", "other <username=" + m_userName.toStdString() + ">");
    break;
  }

  if (!m_userName.isEmpty()) {
    text += " <username=" + m_userName.toStdString() + ">";
  }

  if (!text.empty()) {
    obj.setEntry("text", text);
  }

  if (!ignoringComment) {
    if (!m_comment.isEmpty()) {
      obj.setEntry("comment", m_comment.toStdString());
    }
  }

  return obj;
}

void ZFlyEmBookmark::setCustom(bool state)
{
  m_isCustom = state;
  setHittable(state);
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
        painter.setPen(QColor(0, 0, 0));
        painter.drawText(center.getX(), center.getY(), width, height,
                         Qt::AlignLeft, decorationText);
//        painter.restore();
      }
    }
//    }
  }
}

void ZFlyEmBookmark::loadJsonObject(const ZJsonObject &jsonObj)
{
  clear();

  if (jsonObj["location"] != NULL) {
    std::vector<int> coordinates =
        ZJsonParser::integerArray(jsonObj["location"]);

    if (coordinates.size() == 3) {
      setLocation(coordinates[0], coordinates[1], coordinates[2]);
      uint64_t bodyId = ZJsonParser::integerValue(jsonObj["body ID"]);
      setBodyId(bodyId);

      ZString text = ZJsonParser::stringValue(jsonObj["text"]);
      text.toLower();
      text.trim();

      ZString type = ZJsonParser::stringValue(jsonObj["type"]);
      if (!type.empty()) {
        if (type == "Merge") {
          setBookmarkType(ZFlyEmBookmark::TYPE_FALSE_SPLIT);
        } else if (type == "Split") {
          setBookmarkType(ZFlyEmBookmark::TYPE_FALSE_MERGE);
        }
      } else {
        if (text.startsWith("split") || text.startsWith("small split")) {
          setBookmarkType(ZFlyEmBookmark::TYPE_FALSE_MERGE);
        } else if (text.startsWith("merge")) {
          setBookmarkType(ZFlyEmBookmark::TYPE_FALSE_SPLIT);
        } else {
          setBookmarkType(ZFlyEmBookmark::TYPE_LOCATION);
        }
      }

      if (text.contains("<username=")) {
        std::string::size_type pos = text.rfind("<username=") +
          std::string("<username=").size();
        std::string::size_type lastPos = text.find_first_of(">", pos);
        ZString userName = text.substr(pos, lastPos - pos);
        userName.trim();
#ifdef _DEBUG_2
        std::cout << userName << std::endl;
#endif
        setUser(userName.c_str());
      }

      setComment(ZJsonParser::stringValue(jsonObj["comment"]));
      setStatus(ZJsonParser::stringValue(jsonObj["status"]));

      if (jsonObj.hasKey("checked")) {
        setChecked(ZJsonParser::booleanValue(jsonObj["checked"]));
      }

      if (jsonObj.hasKey("custom")) {
        setCustom(ZJsonParser::booleanValue(jsonObj["custom"]));
      }
    }
  }
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZFlyEmBookmark)
