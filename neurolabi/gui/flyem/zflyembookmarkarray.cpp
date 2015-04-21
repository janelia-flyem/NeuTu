#include "zflyembookmarkarray.h"
#include <iostream>
#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonparser.h"
#include "zstring.h"
#include "flyem/zflyemcoordinateconverter.h"
#include "flyem/zflyemdatainfo.h"
#include "tz_math.h"

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
    if ((text.startsWith("split") || text.startsWith("small split"))
        && bookmarkObj["location"] != NULL) {
      int bodyId = ZJsonParser::integerValue(bookmarkObj["body ID"]);
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
