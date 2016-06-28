#include "zflyembookmarkptrarray.h"

ZFlyEmBookmarkPtrArray::ZFlyEmBookmarkPtrArray()
{
}

size_t ZFlyEmBookmarkPtrArray::size() const
{
  return m_bookmarkArray.size();
}

void ZFlyEmBookmarkPtrArray::clear()
{
  m_bookmarkArray.clear();
}

void ZFlyEmBookmarkPtrArray::append(ZFlyEmBookmark *bookmark)
{
  m_bookmarkArray.append(bookmark);
}

ZFlyEmBookmark* ZFlyEmBookmarkPtrArray::findFirstBookmark(const QString &key) const
{
  return findFirstBookmark(m_bookmarkArray.begin(), m_bookmarkArray.end(), key);
}

ZFlyEmBookmark* & ZFlyEmBookmarkPtrArray::operator [] (size_t index)
{
  return m_bookmarkArray[index];
}

ZFlyEmBookmark* const& ZFlyEmBookmarkPtrArray::operator [] (size_t index) const
{
  return m_bookmarkArray[index];
}

int ZFlyEmBookmarkPtrArray::remove(
    const QVector<ZFlyEmBookmark *> &bookmarkArray)
{
  int count = 0;
  for (QVector<ZFlyEmBookmark*>::const_iterator iter = bookmarkArray.begin();
       iter != bookmarkArray.end(); ++iter) {
    count += m_bookmarkArray.removeOne(*iter);
  }

  return count;
}

bool ZFlyEmBookmarkPtrArray::remove(ZFlyEmBookmark *bookmark)
{
  return m_bookmarkArray.removeOne(bookmark);
}

int ZFlyEmBookmarkPtrArray::findFirstIndex(ZFlyEmBookmark *bookmark) const
{
  return m_bookmarkArray.indexOf(bookmark);
}
