#include "zintset.h"
#include <algorithm>
#include <iostream>

ZIntSet::ZIntSet()
{
}

ZIntSet::ZIntSet(const std::set<int> &v)
{
  *this = v;
}

ZIntSet& ZIntSet::operator =(const std::set<int> &v)
{
  std::set<int>::operator =(v);

  return *this;
}

void ZIntSet::intersect(const std::set<int> &v)
{
  std::set<int> result;
  std::set_intersection(begin(), end(), v.begin(), v.end(),
                        std::inserter(result, result.begin()));

  *this = result;
}

void ZIntSet::intersect(const std::vector<int> &v)
{
  std::set<int> result;
  std::set_intersection(begin(), end(), v.begin(), v.end(),
                        std::inserter(result, result.begin()));

  *this = result;
}

void ZIntSet::print() const
{
  if (empty()) {
    std::cout << "Empty set." << std::endl;
  } else {
    std::cout << "Set - " << size() << " element(s): " << std::endl;
    for (std::set<int>::const_iterator iter = begin(); iter != end(); ++iter) {
      std::cout << "  " << *iter << std::endl;
    }
  }
}
