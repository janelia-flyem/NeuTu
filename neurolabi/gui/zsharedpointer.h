#ifndef ZSHAREDPOINTER_H
#define ZSHAREDPOINTER_H

#include <utility>

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

template <typename T>
class ZUniquePointer : public ztr1::auto_ptr<T> {
public:
  ZUniquePointer(T *p = NULL);
  ZUniquePointer(const ZUniquePointer<T> &p);
};
#endif

#define ZSharedPointer ztr1::shared_ptr
#define Make_Shared ztr1::make_shared
#define Shared_Dynamic_Cast ztr1::dynamic_pointer_cast

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
