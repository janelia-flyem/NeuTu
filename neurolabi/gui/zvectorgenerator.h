#ifndef ZVECTORGENERATOR_H
#define ZVECTORGENERATOR_H

#include <vector>

template<typename T>
class ZVectorGenerator
{
public:
  ZVectorGenerator<T>& operator<< (const T& v) {
    m_array.push_back(v);
    return *this;
  }

  operator std::vector<T>() const {
    return m_array;
  }

private:
  std::vector<T> m_array;
};

#endif // ZVECTORGENERATOR_H
