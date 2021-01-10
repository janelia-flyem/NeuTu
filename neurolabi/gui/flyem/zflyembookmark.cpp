#include "zflyembookmark.h"
#include <iostream>
#include <sstream>

#include "common/math.h"
#include "logging/zqslog.h"
#include "neutubeconfig.h"

#include "zjsonobject.h"
#include "zpainter.h"
#include "zstring.h"
#include "zjsonparser.h"
#include "zjsonarray.h"
#include "zjsonfactory.h"

ZFlyEmBookmark::ZFlyEmBookmark()
{
  init();
}

ZFlyEmBookmark::~ZFlyEmBookmark()
{
  ZOUT(LTRACE(), 5) << "Deconstructing " << this << ": bookmark " << ", "
                    << getSource();
#ifdef _DEBUG_2
  std::cout << "Deconstructing " << this << ": bookmark " << ", "
            << getSource() << std::endl;
#endif
}

void ZFlyEmBookmark::init()
{
  m_type = GetType();
  setTarget(ETarget::WIDGET_CANVAS);

  m_bodyId = 0;
  m_bookmarkType = EBookmarkType::LOCATION;
  m_isChecked = false;
  setCustom(false);

  m_isInTable = true;

  addVisualEffect(neutu::display::Sphere::VE_DOT_CENTER |
                  neutu::display::Sphere::VE_OUT_FOCUS_DIM);
//  m_visualEffect = neutu::display::Sphere::VE_DOT_CENTER;
  setColor(255, 0, 0);
  setRadius(5.0);
//  setHittable(false);
  useCosmeticPen(true);
}

void ZFlyEmBookmark::clear()
{
  m_bodyId = 0;
  m_bookmarkType = EBookmarkType::LOCATION;
  m_isChecked = false;
  setCustom(false);
//  m_bookmarkRole = ROLE_USER;
//  m_isCustom = false;
  m_userName.clear();
  m_comment.clear();
  m_status.clear();
  m_time.clear();
  m_tags.clear();
}

void ZFlyEmBookmark::print() const
{
  std::cout << "Body ID: " << m_bodyId << std::endl;
  std::cout << "Location: " << getLocation().toString() << std::endl;
}

ZFlyEmBookmark *ZFlyEmBookmark::clone() const
{
  return new ZFlyEmBookmark(*this);
}

QString ZFlyEmBookmark::getDvidKey() const
{
  return QString("%1_%2_%3").arg(neutu::iround(getCenter().x())).
      arg(neutu::iround(getCenter().y())).
      arg(neutu::iround(getCenter().z()));
}

ZJsonObject ZFlyEmBookmark::toDvidAnnotationJson() const
{
  return ZJsonFactory::MakeAnnotationJson(*this);
}

void ZFlyEmBookmark::loadDvidAnnotation(const ZJsonObject &jsonObj)
{
  clear();


  if (!jsonObj.hasKey("Pos") || !jsonObj.hasKey("Kind")) {
    return;
  }

  if (ZJsonParser::stringValue(jsonObj["Kind"]) == std::string("Note")) {
    std::vector<int64_t> coordinates =
        ZJsonParser::integerArray(jsonObj["Pos"]);

    if (jsonObj.hasKey("Tags")) {
      ZJsonArray tagJson(jsonObj.value("Tags"));
      for (size_t i = 0; i < tagJson.size(); ++i) {
        addTag(ZJsonParser::stringValue(tagJson.at(i)));
      }
    }

    if (coordinates.size() == 3) {
      setLocation(coordinates[0], coordinates[1], coordinates[2]);
      ZJsonObject propJson(jsonObj.value("Prop"));

      m_propertyJson = propJson;

      if (!propJson.isEmpty()) {
        uint64_t bodyId = 0;
        if (ZJsonParser::IsInteger(propJson["body ID"])) {
          bodyId = ZJsonParser::integerValue(propJson["body ID"]);
        } else {
          bodyId = ZString(ZJsonParser::stringValue(propJson["body ID"])).
              firstUint64();
        }
        setBodyId(bodyId);

        ZString text = ZJsonParser::stringValue(propJson["text"]);
        text.toLower();
        text.trim();

        ZString type = ZJsonParser::stringValue(propJson["type"]);
        if (!type.empty()) {
          if (type == "Merge") {
            setBookmarkType(ZFlyEmBookmark::EBookmarkType::FALSE_SPLIT);
          } else if (type == "Split") {
            setBookmarkType(ZFlyEmBookmark::EBookmarkType::FALSE_MERGE);
          }
        } else {
          if (text.startsWith("split") || text.startsWith("small split")) {
            setBookmarkType(ZFlyEmBookmark::EBookmarkType::FALSE_MERGE);
          } else if (text.startsWith("merge")) {
            setBookmarkType(ZFlyEmBookmark::EBookmarkType::FALSE_SPLIT);
          } else {
            setBookmarkType(ZFlyEmBookmark::EBookmarkType::LOCATION);
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
          setUser(userName);
        }

        setComment(ZJsonParser::stringValue(propJson["comment"]));
        setStatus(ZJsonParser::stringValue(propJson["status"]));
        setUser(ZJsonParser::stringValue(propJson["user"]));

        if (propJson.hasKey("checked")) {
          if (ZJsonParser::IsBoolean(propJson["checked"])) {
            setChecked(ZJsonParser::booleanValue(propJson["checked"]));
          } else {
            std::string checked = ZJsonParser::stringValue(propJson["checked"]);
            setChecked(checked == "1");
          }
        }

        if (propJson.hasKey("custom")) {
          if (ZJsonParser::IsBoolean(propJson["custom"])) {
            setCustom(ZJsonParser::booleanValue(propJson["custom"]));
          } else {
            std::string custom = ZJsonParser::stringValue(propJson["custom"]);
            setCustom(custom == "1");
          }
        }
      }
    }
  }
}

QString ZFlyEmBookmark::getTypeString() const
{
  QString text;
  switch (m_bookmarkType) {
  case EBookmarkType::FALSE_MERGE:
    text = "Split";
    break;
  case EBookmarkType::FALSE_SPLIT:
    text = "Merge";
    break;
  default:
    text = "Other";
    break;
  }

  return text;
}

ZJsonObject ZFlyEmBookmark::toJsonObject(bool ignoringComment) const
{
  ZJsonObject obj;
  obj.setEntry("body ID", m_bodyId);
  if (!m_status.isEmpty()) {
    obj.setEntry("status", m_status.toStdString());
  }
  int location[3];
  location[0] = neutu::iround(getCenter().x());
  location[1] = neutu::iround(getCenter().y());
  location[2] = neutu::iround(getCenter().z());


  /*
  location[1] = m_location.getY();
  location[2] = m_location.getZ();
  */
  obj.setEntry("location", location, 3);
  obj.setEntry("checked", m_isChecked);
  if (isCustom()) {
    obj.setEntry("custom", isCustom());
  }

  obj.setEntry("type", getTypeString().toStdString());

  std::string text = getTypeString().toLower().toStdString();

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
  /*
  if (state) {
    m_role = ROLE_USER;
  } else {
    m_role = ROLE_ASSIGNED;
  }
  */
  m_isCustom = state;
  if (state == true) {
    setHitProtocal(ZStackObject::EHitProtocol::HIT_DATA_POS);
  } else {
    setHitProtocal(ZStackObject::EHitProtocol::HIT_NONE);
  }
}

void ZFlyEmBookmark::display(
    ZPainter &painter, int slice, EDisplayStyle option,
    neutu::EAxis sliceAxis) const
{
  ZStackBall::display(painter, slice, option, sliceAxis);

  if (isVisible()) {
    if (isSliceVisible(painter.getZ(slice), sliceAxis)) {
      QString decorationText;
      if (isCustom()) {
        decorationText = "u";
      }
      if (!decorationText.isEmpty()) {
//        painter.save();
        ZIntPoint center = getLocation();
        center.shiftSliceAxis(sliceAxis);
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



bool ZFlyEmBookmark::loadJsonObject(const ZJsonObject &jsonObj)
{
  bool loaded = false;

  clear();

#if 1
  if (jsonObj["location"] != NULL) {
    std::vector<int64_t> coordinates =
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
          setBookmarkType(ZFlyEmBookmark::EBookmarkType::FALSE_SPLIT);
        } else if (type == "Split") {
          setBookmarkType(ZFlyEmBookmark::EBookmarkType::FALSE_MERGE);
        }
      } else {
        if (text.startsWith("split") || text.startsWith("small split")) {
          setBookmarkType(ZFlyEmBookmark::EBookmarkType::FALSE_MERGE);
        } else if (text.startsWith("merge")) {
          setBookmarkType(ZFlyEmBookmark::EBookmarkType::FALSE_SPLIT);
        } else {
          setBookmarkType(ZFlyEmBookmark::EBookmarkType::LOCATION);
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
        setUser(userName);
      }

      setComment(ZJsonParser::stringValue(jsonObj["comment"]));
      setStatus(ZJsonParser::stringValue(jsonObj["status"]));

      if (jsonObj.hasKey("checked")) {
        setChecked(ZJsonParser::booleanValue(jsonObj["checked"]));
      }

      if (jsonObj.hasKey("custom")) {
        setCustom(ZJsonParser::booleanValue(jsonObj["custom"]));
      }

      loaded = true;
    }
  }
#endif

  return loaded;
}

std::string ZFlyEmBookmark::toLogString() const
{
  std::ostringstream stream;
  if (isCustom()) {
    stream << "User ";
  } else {
    stream << "Assigned ";
  }
  stream << "bookmark @" << getCenter().toString() << " with ";
  stream << "ID: " << getBodyId();
  return stream.str();
}

void ZFlyEmBookmark::addTag(const char *tag)
{
  addTag(QString(tag));
}

void ZFlyEmBookmark::addTag(const std::string &tag)
{
  addTag(tag.c_str());
}

void ZFlyEmBookmark::addTag(const QString &tag)
{
  m_tags.append(tag);
}

void ZFlyEmBookmark::addUserTag()
{
  addTag("user:" + getUserName());
}

void ZFlyEmBookmark::setLocation(const ZIntPoint &pt)
{
  setLocation(pt.getX(), pt.getY(), pt.getZ());
}

/*
ZFlyEmBookmark* ZFlyEmBookmark::clone() const
{
  ZFlyEmBookmark *bookmark = new ZFlyEmBookmark;
  *bookmark = *this;

  return bookmark;
}
*/

//ZSTACKOBJECT_DEFINE_CLASS_NAME(ZFlyEmBookmark)
