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

};

#endif // ZFLYEMBOOKMARKARRAY_H
