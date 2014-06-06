#ifndef ZSHAREDPOINTER_H
#define ZSHAREDPOINTER_H

#ifdef __GLIBCXX__
#include <tr1/memory>
typedef std::tr1::shared_ptr<T> ZSharedPoint<T>;
#else
#include <memory>
typedef std::shared_ptr<T> ZSharedPoint<T>;
#endif

#endif // ZSHAREDPOINTER_H
