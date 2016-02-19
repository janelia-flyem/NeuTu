#include "zintpairmap.h"

#include <iostream>

using namespace std;

void ZIntPairMap::incPairCount(int x, int y)
{
  std::pair<int, int> key(x, y);
  if (this->count(key) == 0) {
    (*this)[key] = 1;
  } else {
    (*this)[key]++;
  }
}

std::pair<int, int> ZIntPairMap::getPeak() const
{
  std::pair<int, int> peak(0, 0);
  int maxCount = -1;
  for (const_iterator iter = begin(); iter != end(); ++iter) {
    if (maxCount < iter->second) {
      maxCount = iter->second;
      peak = iter->first;
    }
  }

  return peak;
}

void ZIntPairMap::add(const ZIntPairMap &submap)
{
  for (ZIntPairMap::const_iterator iter = submap.begin();
       iter != submap.end(); ++iter) {
    const std::pair<int, int> &key = iter->first;
    (*this)[key] += iter->second;
  }
}

void ZIntPairMap::print() const
{
  if (empty()) {
    std::cout << "Empty map." << std::endl;
  }
  for (const_iterator iter = begin(); iter != end(); ++iter) {
    cout << (iter->first).first << " " << (iter->first).second
         << ": " << iter->second << endl;
  }
}

void ZIntPairMap::printSummary() const
{
  cout << "Int pair map: " << size() << " entries" << endl;
}
