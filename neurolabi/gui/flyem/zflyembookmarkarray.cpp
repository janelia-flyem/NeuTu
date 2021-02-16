#include "zflyembookmarkarray.h"

#include <iostream>

#include "common/math.h"
#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonparser.h"
#include "zjsonfactory.h"
#include "zstring.h"
#include "flyem/zflyemcoordinateconverter.h"
#include "flyem/zflyemdatainfo.h"
#include "zpunctum.h"

ZFlyEmBookmarkArray::ZFlyEmBookmarkArray()
{
}

namespace {

void update_location(
    ZFlyEmBookmark *bookmark, const ZFlyEmCoordinateConverter *converter) {
  if (converter) {
    bookmark->setLocation(
          converter->convert(bookmark->getLocation(),
                             ZFlyEmCoordinateConverter::RAVELER_SPACE,
                             ZFlyEmCoordinateConverter::IMAGE_SPACE));
  }
}

}

void ZFlyEmBookmarkArray::appendJson(
    const ZJsonObject &obj, const ZFlyEmCoordinateConverter *converter)
{
  ZFlyEmBookmark bookmark;
  if (bookmark.loadJsonObject(obj)) {
    update_location(&bookmark, converter);
    append(bookmark);
  }
}

void ZFlyEmBookmarkArray::loadJsonArray(
    const ZJsonArray &json, const ZFlyEmCoordinateConverter *converter)
{
  clear();
  for (size_t i = 0; i < json.size(); ++i) {
    ZJsonObject bookmarkObj(json.value(i));

    appendJson(bookmarkObj, converter);
  }
}

void ZFlyEmBookmarkArray::loadJsonObject(
    const ZJsonObject &json, const ZFlyEmCoordinateConverter *converter)
{
  clear();
  json.forEachValue([&](ZJsonValue value) {
    ZJsonObject bookmarkObj(value);

    appendJson(bookmarkObj, converter);
  });
}

void ZFlyEmBookmarkArray::importJsonFile(
    const std::string &filePath, const ZFlyEmCoordinateConverter *converter)
{
  clear();
  ZJsonValue json;
  json.load(filePath);
  if (json.isObject()) {
    ZJsonObject jsonObj(json);
    if (jsonObj.hasKey("data")) {
      loadJsonArray(ZJsonArray(jsonObj.value("data")), converter);
    } else {
      loadJsonObject(jsonObj, converter);
    }
  } else if (json.isArray()) {
    loadJsonArray(ZJsonArray(json), converter);
  }
}

#if 0
void ZFlyEmBookmarkArray::importJsonFile(
    const std::string &filePath, const ZFlyEmCoordinateConverter *converter)
{
  clear();

  ZJsonObject obj;
  obj.load(filePath);

  /*
  ZFlyEmCoordinateConverter converter;
  ZFlyEmDataInfo dataInfo(FlyEm::DATA_FIB25_7C);
  converter.configure(dataInfo);
*/
  if (obj.hasKey("data")) {
    ZJsonArray bookmarkArrayObj(obj["data"], ZJsonValue::SET_INCREASE_REF_COUNT);
    for (size_t i = 0; i < bookmarkArrayObj.size(); ++i) {
      ZJsonObject bookmarkObj(
            bookmarkArrayObj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);

      ZFlyEmBookmark bookmark;
      if (bookmark.loadJsonObject(bookmarkObj)) {
        update_location(&bookmark, converter);
        append(bookmark);
      }

      /*
      ZString text = ZJsonParser::stringValue(bookmarkObj["text"]);
      text.toLower();
      if (bookmarkObj["location"] != NULL) {
        ZJsonValue idJson = bookmarkObj.value("body ID");
        int64_t bodyId = 0;
        if (idJson.isInteger()) {
          bodyId = ZJsonParser::integerValue(idJson.getData());
        } else if (idJson.isString()) {
          bodyId = ZString::FirstInteger(ZJsonParser::stringValue(idJson.getData()));
        }

        if (bodyId > 0) {
          std::vector<int64_t> coordinates =
              ZJsonParser::integerArray(bookmarkObj["location"]);

          if (coordinates.size() == 3) {
            ZFlyEmBookmark bookmark;
            double x = coordinates[0];
            double y = coordinates[1];
            double z = coordinates[2];
            if (converter) {
              converter->convert(
                    &x, &y, &z, ZFlyEmCoordinateConverter::RAVELER_SPACE,
                    ZFlyEmCoordinateConverter::IMAGE_SPACE);
            }
            bookmark.setLocation(neutu::iround(x), neutu::iround(y), neutu::iround(z));
            bookmark.setBodyId(bodyId);
            if (text.startsWith("split") || text.startsWith("small split")) {
              bookmark.setBookmarkType(ZFlyEmBookmark::EBookmarkType::FALSE_MERGE);
            } else if (text.startsWith("merge")) {
              bookmark.setBookmarkType(ZFlyEmBookmark::EBookmarkType::FALSE_SPLIT);
            } else {
              bookmark.setBookmarkType(ZFlyEmBookmark::EBookmarkType::LOCATION);
            }
            append(bookmark);
          }
        }
      }
      */
    }
  }

}
#endif

void ZFlyEmBookmarkArray::print() const
{
  std::cout << size() << " bookmarks:" << std::endl;
  foreach (ZFlyEmBookmark bookmark, *this) {
    bookmark.print();
  }
}

ZFlyEmBookmarkArray ZFlyEmBookmarkArray::getBookmarkArray(uint64_t bodyId)
{
  ZFlyEmBookmarkArray bookmarkArray;
  for (ZFlyEmBookmarkArray::const_iterator iter = begin(); iter != end();
       ++iter) {
    const ZFlyEmBookmark &bookmark = *iter;
    if (bookmark.getBodyId() == bodyId) {
      bookmarkArray.append(bookmark);
    }
  }


  return bookmarkArray;
}

ZFlyEmBookmarkArray ZFlyEmBookmarkArray::getBookmarkArray(ZFlyEmBookmark::EBookmarkType type)
{
  ZFlyEmBookmarkArray bookmarkArray;
  for (ZFlyEmBookmarkArray::const_iterator iter = begin(); iter != end();
       ++iter) {
    const ZFlyEmBookmark &bookmark = *iter;
    if (bookmark.getBookmarkType() == type) {
      bookmarkArray.append(bookmark);
    }
  }

  return bookmarkArray;
}

ZJsonArray ZFlyEmBookmarkArray::toAnnotationJson() const
{
  ZJsonArray array;
  for (const ZFlyEmBookmark &bookmark : *this) {
    array.append(ZJsonFactory::MakeAnnotationJson(bookmark));
  }

  return array;
}

QVector<ZPunctum*> ZFlyEmBookmarkArray::toPunctumArray(bool isVisible) const
{
  QVector<ZPunctum*> punctumArray;
  for (ZFlyEmBookmarkArray::const_iterator iter = begin();
       iter != end(); ++iter) {
    const ZFlyEmBookmark &bookmark = *iter;
    ZPunctum *circle = new ZPunctum;
    circle->setRole(ZStackObjectRole::ROLE_TMP_BOOKMARK);
    circle->set(bookmark.getLocation(), 5);

//      ZStackBall *circle = new ZStackBall;
//      circle->set(bookmark.getLocation(), 5);
    circle->setColor(255, 0, 0);
    circle->setVisible(isVisible);
    circle->setHitProtocal(ZStackObject::EHitProtocol::HIT_NONE);
    punctumArray.push_back(circle);
  }

  return punctumArray;
}

ZFlyEmBookmark* ZFlyEmBookmarkArray::findFirstBookmark(const QString &key) const
{
  return findFirstBookmark(begin(), end(), key);
#if 0
  ZFlyEmBookmark* out = NULL;
  for (ZFlyEmBookmarkArray::const_iterator iter = begin();
       iter != end(); ++iter) {
    const ZFlyEmBookmark &bookmark = *iter;
    if (bookmark.getDvidKey() == key) {
      out = const_cast<ZFlyEmBookmark*>(&bookmark);
      break;
    }
  }

  return out;
#endif
}
