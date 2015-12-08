#ifndef ZFLYEMBOOKMARKPTRARRAY_H
#define ZFLYEMBOOKMARKPTRARRAY_H

#include <QVector>
#include <QString>

#include "zflyembookmark.h"

class ZFlyEmBookmarkPtrArray
{
public:
  ZFlyEmBookmarkPtrArray();

  size_t size() const;
  void clear();

  void append(ZFlyEmBookmark *bookmark);

  ZFlyEmBookmark* findFirstBookmark(const QString &key) const;

  ZFlyEmBookmark* & operator [] (size_t index);
  ZFlyEmBookmark* const& operator [] (size_t index) const;

  template <class InputIterator>
  static ZFlyEmBookmark* findFirstBookmark(
      InputIterator first, InputIterator last, const QString &key);

  void remove(const QVector<ZFlyEmBookmark*> &bookmarkArray);

private:
  QList<ZFlyEmBookmark*> m_bookmarkArray;
};

template <class InputIterator>
ZFlyEmBookmark* ZFlyEmBookmarkPtrArray::findFirstBookmark(
    InputIterator first, InputIterator last, const QString &key)
{
  ZFlyEmBookmark* out = NULL;
  for (InputIterator iter = first; iter != last; ++iter) {
    ZFlyEmBookmark *bookmark = *iter;
    if (bookmark->getDvidKey() == key) {
      out = bookmark;
      break;
    }
  }

  return out;
}


#endif // ZFLYEMBOOKMARKPTRARRAY_H
