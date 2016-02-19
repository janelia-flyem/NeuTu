#include "zflyembookmarkarray.h"
#include <iostream>
#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonparser.h"
#include "zstring.h"
#include "flyem/zflyemcoordinateconverter.h"
#include "flyem/zflyemdatainfo.h"
#include "tz_math.h"
#include "zpunctum.h"

ZFlyEmBookmarkArray::ZFlyEmBookmarkArray()
{
}

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
  ZJsonArray bookmarkArrayObj(obj["data"], false);
  for (size_t i = 0; i < bookmarkArrayObj.size(); ++i) {
    ZJsonObject bookmarkObj(bookmarkArrayObj.at(i), false);
    ZString text = ZJsonParser::stringValue(bookmarkObj["text"]);
    text.toLower();
    if (bookmarkObj["location"] != NULL) {
      ZJsonValue idJson = bookmarkObj.value("body ID");
      int64_t bodyId = 0;
      if (idJson.isInteger()) {
        bodyId = ZJsonParser::integerValue(idJson.getData());
      } else if (idJson.isString()) {
        bodyId = ZString::firstInteger(ZJsonParser::stringValue(idJson.getData()));
      }

      if (bodyId > 0) {
        std::vector<int> coordinates =
            ZJsonParser::integerArray(bookmarkObj["location"]);

        if (coordinates.size() == 3) {
          ZFlyEmBookmark bookmark;
          double x = coordinates[0];
          double y = coordinates[1];
          double z = coordinates[2];
          if (converter != NULL) {
            converter->convert(
                  &x, &y, &z, ZFlyEmCoordinateConverter::RAVELER_SPACE,
                  ZFlyEmCoordinateConverter::IMAGE_SPACE);
          }
          bookmark.setLocation(iround(x), iround(y), iround(z));
          bookmark.setBodyId(bodyId);
          if (text.startsWith("split") || text.startsWith("small split")) {
            bookmark.setBookmarkType(ZFlyEmBookmark::TYPE_FALSE_MERGE);
          } else if (text.startsWith("merge")) {
            bookmark.setBookmarkType(ZFlyEmBookmark::TYPE_FALSE_SPLIT);
          } else {
            bookmark.setBookmarkType(ZFlyEmBookmark::TYPE_LOCATION);
          }
          append(bookmark);
        }
      }
    }
  }
}

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
    circle->setHittable(false);
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
