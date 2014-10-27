#ifndef ZFLYEMBOOKMARKARRAY_H
#define ZFLYEMBOOKMARKARRAY_H

#include <QVector>
#include "zflyembookmark.h"

class ZFlyEmBookmarkArray : public QVector<ZFlyEmBookmark>
{
public:
  ZFlyEmBookmarkArray();

  void importJsonFile(const std::string &filePath);

  void print() const;
};

#endif // ZFLYEMBOOKMARKARRAY_H
