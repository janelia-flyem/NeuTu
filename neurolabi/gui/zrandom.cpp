#include "zrandom.h"

#include <QThreadStorage>

#if defined(_WIN32) || defined(_WIN64)

#include <intrin.h>
#define rdtsc  __rdtsc

#else

//  For everything else
static unsigned long long rdtsc()
{
  unsigned int lo, hi;
  __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
  return (static_cast<unsigned long long>(hi) << 32) | lo;
}

#endif

ZRandom::ZRandom()
{
  m_eng.seed(rdtsc());
  //LOG(ERROR) << "a new zrandom!";
}

//ZRandom& ZRandom::instance()
//{
//  static tbb::enumerable_thread_specific<ZRandom> globalZRandom;
//  return globalZRandom.local();
//}

ZRandom& ZRandom::getInstance()
{
  // should be thread local,
  // use qt or use boost thread_specific_ptr or wait for c++11 thread_local keyword
  static QThreadStorage<ZRandom> globalZRandom;
  return globalZRandom.localData();
}
