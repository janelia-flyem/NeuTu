#include "zstackobjecthandle.h"

std::atomic<uint64_t> ZStackObjectHandle::m_currentHandle{0};

ZStackObjectHandle::ZStackObjectHandle()
{
  m_handle = GetNextHandle();
}

/*
ZStackObjectHandle::ZStackObjectHandle(ZStackObjectHandle &&rhs)
{
  *this = std::move(rhs);
}
*/

uint64_t ZStackObjectHandle::GetNextHandle()
{
  return ++m_currentHandle;
}

bool ZStackObjectHandle::isValid() const
{
  return m_handle > 0;
}

/*
ZStackObjectHandle& ZStackObjectHandle::operator=(ZStackObjectHandle&& rhs)
{
  m_handle = rhs.m_handle;
  return *this;
}
*/

bool ZStackObjectHandle::operator== (const ZStackObjectHandle &handle) const
{
  return m_handle == handle.m_handle;
}

bool ZStackObjectHandle::operator!= (const ZStackObjectHandle &handle) const
{
  return m_handle != handle.m_handle;
}
