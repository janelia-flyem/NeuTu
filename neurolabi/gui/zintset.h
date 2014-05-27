#ifndef ZINTSET_H
#define ZINTSET_H

#include <set>
#include <vector>

class ZIntSet : public std::set<int>
{
public:
  ZIntSet();
  ZIntSet(const std::set<int> &v);

  ZIntSet& operator = (const std::set<int> &v);
  void intersect(const std::set<int> &v);
  void intersect(const std::vector<int> &v);

  void print() const;
};

#endif // ZINTSET_H
