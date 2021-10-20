#include "zstackobjecthandle.h"

std::atomic<uint64_t> ZStackObjectHandle::m_currentHandle{0};

ZStackObjectHandle::ZStackObjectHandle() : m_handle(GetNextHandle())
{
}

uint64_t ZStackObjectHandle::GetNextHandle()
{
  return ++m_currentHandle;
}

bool ZStackObjectHandle::isValid() const
{
  return getValue() > 0;
}

uint64_t ZStackObjectHandle::getValue() const
{
  return m_handle;
}

uint64_t ZStackObjectHandle::_getValue_() const
{
  return getValue();
}

void ZStackObjectHandle::_reset_()
{
  m_currentHandle = 0;
}

bool ZStackObjectHandle::operator== (const ZStackObjectHandle &handle) const
{
  return m_handle == handle.m_handle;
}

bool ZStackObjectHandle::operator!= (const ZStackObjectHandle &handle) const
{
  return m_handle != handle.m_handle;
}
