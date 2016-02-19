#ifndef TR1_HEADER_H
#define TR1_HEADER_H

#ifdef __GLIBCXX__
#include <tr1/memory>
using namespace std::tr1;
#else
#include <memory>
using namespace std;
#endif

#endif // TR1_HEADER_H
