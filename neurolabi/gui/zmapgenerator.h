#ifndef ZMAPGENERATOR_H
#define ZMAPGENERATOR_H

#include <map>

template <typename T1, typename T2>
class ZMapGenerator
{
public:
  ZMapGenerator<T1, T2>& operator<< (std::pair<const T1, const T2> entry) {
    m_map[entry.first] = entry.second;

    return *this;
  }

  operator std::map<T1, T2>() const {
    return m_map;
  }

private:
  std::map<T1, T2> m_map;
};

#endif // ZMAPGENERATOR_H
