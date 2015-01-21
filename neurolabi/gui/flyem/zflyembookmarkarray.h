#ifndef ZFLYEMBOOKMARKARRAY_H
#define ZFLYEMBOOKMARKARRAY_H

#include <QVector>
#include "zflyembookmark.h"

class ZFlyEmCoordinateConverter;

class ZFlyEmBookmarkArray : public QVector<ZFlyEmBookmark>
{
public:
  ZFlyEmBookmarkArray();

  void importJsonFile(const std::string &filePath,
                      const ZFlyEmCoordinateConverter *converter);

  void print() const;
};

#endif // ZFLYEMBOOKMARKARRAY_H
