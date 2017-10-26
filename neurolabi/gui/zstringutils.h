#ifndef ZSTRINGUTILS_H
#define ZSTRINGUTILS_H

#include <QString>

bool naturalSortLessThan(const QString& s1, const QString& s2);

class QStringNaturalCompare
{
public:
  inline bool operator()(const QString& s1, const QString& s2) const
  {
    return naturalSortLessThan(s1, s2);
  }
};

class QStringKeyNaturalLess
{
public:

  template<typename L, typename R>
  bool operator()(const L& l, const R& r) const
  {
    return naturalSortLessThan(l.first, r.first);
  }
};

#endif // ZSTRINGUTILS_H
