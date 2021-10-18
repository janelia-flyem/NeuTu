#ifndef ZFLYEMBOOKMARKARRAY_H
#define ZFLYEMBOOKMARKARRAY_H

#include <QVector>
#include <QString>
#include "zflyembookmark.h"

class ZFlyEmCoordinateConverter;
class ZPunctum;
class ZJsonArray;

class ZFlyEmBookmarkArray : public QVector<ZFlyEmBookmark>
{
public:
  ZFlyEmBookmarkArray();

  void importJsonFile(const std::string &filePath,
                      const ZFlyEmCoordinateConverter *converter = nullptr);

  void print() const;

  ZFlyEmBookmarkArray getBookmarkArray(ZFlyEmBookmark::EBookmarkType type);
  ZFlyEmBookmarkArray getBookmarkArray(uint64_t bodyId);

  ZJsonArray toAnnotationJson() const;
  QVector<ZPunctum*> toPunctumArray(bool isVisible) const;

  ZFlyEmBookmark* findFirstBookmark(const QString &key) const;

  template <class InputIterator>
  static ZFlyEmBookmark* findFirstBookmark(
      InputIterator first, InputIterator last, const QString &key);

private:
  void loadJsonArray(
      const ZJsonArray &json, const ZFlyEmCoordinateConverter *converter);
  void loadJsonObject(
      const ZJsonObject &json, const ZFlyEmCoordinateConverter *converter);
  void appendJson(
      const ZJsonObject &obj, const ZFlyEmCoordinateConverter *converter);
};

template <class InputIterator>
ZFlyEmBookmark* ZFlyEmBookmarkArray::findFirstBookmark(
    InputIterator first, InputIterator last, const QString &key)
{
  ZFlyEmBookmark* out = NULL;
  for (ZFlyEmBookmarkArray::const_iterator iter = first; iter != last; ++iter) {
    const ZFlyEmBookmark &bookmark = *iter;
    if (bookmark.getDvidKey() == key) {
      out = const_cast<ZFlyEmBookmark*>(&bookmark);
      break;
    }
  }

  return out;
}
#endif // ZFLYEMBOOKMARKARRAY_H
