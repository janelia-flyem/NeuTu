#ifndef ZFLYEMBOOKMARKARRAY_H
#define ZFLYEMBOOKMARKARRAY_H

#include <QVector>
#include <QString>
#include "zflyembookmark.h"

class ZFlyEmCoordinateConverter;
class ZPunctum;

class ZFlyEmBookmarkArray : public QVector<ZFlyEmBookmark>
{
public:
  ZFlyEmBookmarkArray();

  void importJsonFile(const std::string &filePath,
                      const ZFlyEmCoordinateConverter *converter);

  void print() const;

  ZFlyEmBookmarkArray getBookmarkArray(ZFlyEmBookmark::EBookmarkType type);
  ZFlyEmBookmarkArray getBookmarkArray(uint64_t bodyId);

  QVector<ZPunctum*> toPunctumArray(bool isVisible) const;

  ZFlyEmBookmark* findFirstBookmark(const QString &key) const;

  template <class InputIterator>
  static ZFlyEmBookmark* findFirstBookmark(
      InputIterator first, InputIterator last, const QString &key);
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
