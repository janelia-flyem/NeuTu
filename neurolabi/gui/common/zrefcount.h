#ifndef ZREFCOUNT_H
#define ZREFCOUNT_H

#include <atomic>

class ZRefCount
{
public:
  ZRefCount();
  ZRefCount(const ZRefCount &ref);
  virtual ~ZRefCount();

  unsigned int getRefCount() const;

private:
  static std::atomic<unsigned int> m_refCount;
};

#endif // ZREFCOUNT_H
