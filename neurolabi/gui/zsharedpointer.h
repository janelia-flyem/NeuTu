#ifndef ZSHAREDPOINTER_H
#define ZSHAREDPOINTER_H

#ifdef __GLIBCXX__
#include <tr1/memory>
namespace ztr1 = std::tr1;
#else
#include <memory>
namespace ztr1 = std;
#endif

#if 0
template <typename T>
class ZSharedPointer : public ztr1::shared_ptr<T> {
public:
  ZSharedPointer(T *p = NULL);
  ZSharedPointer(const ZSharedPointer<T> &p);
};
#endif

#define ZSharedPointer ztr1::shared_ptr

template<typename T>
struct array_deleter
{
  void operator()(T const * p)
  {
    delete[] p;
  }
};

//#include "zsharedpointer.cpp"


#endif // ZSHAREDPOINTER_H
