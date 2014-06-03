#ifndef ZINTPAIRMAP_H
#define ZINTPAIRMAP_H

#include <utility>
#include <map>

class ZIntPairMap : public std::map<std::pair<int, int>, int>
{
public:
  void incPairCount(int x, int y);

  void add(const ZIntPairMap &submap);
  std::pair<int, int> getPeak() const;

  void print() const;
  void printSummary() const;
};

#endif // ZINTPAIRMAP_H
