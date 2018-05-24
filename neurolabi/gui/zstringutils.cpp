#include "zstringutils.h"

#include <QCollator>

namespace {

const QCollator& myCollator()
{
  static QCollator collator;
  collator.setNumericMode(true);
  return collator;
}

} // namespace

bool naturalSortLessThan(const QString& s1, const QString& s2)
{
  return myCollator().compare(s1, s2) < 0;
}
