#include "zrefcount.h"

std::atomic<unsigned int> ZRefCount::m_refCount{0};

ZRefCount::ZRefCount()
{
  ++m_refCount;
}

ZRefCount::ZRefCount(const ZRefCount &/*ref*/)
{
  ++m_refCount;
}

ZRefCount::~ZRefCount()
{
  --m_refCount;
}

unsigned int ZRefCount::getRefCount() const
{
  return m_refCount;
}
