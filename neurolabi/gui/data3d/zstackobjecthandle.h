#ifndef ZSTACKOBJECTHANDLE_H
#define ZSTACKOBJECTHANDLE_H

#include <atomic>

class ZStackObjectHandle
{
public:
  ZStackObjectHandle();
  ZStackObjectHandle(ZStackObjectHandle &&rhs) = delete;
  ZStackObjectHandle(const ZStackObjectHandle&) = delete;

  bool operator == (const ZStackObjectHandle &handle) const;
  bool operator != (const ZStackObjectHandle &handle) const;

  bool isValid() const;

  ZStackObjectHandle& operator=(ZStackObjectHandle &&rhs) = delete;
  ZStackObjectHandle& operator=(const ZStackObjectHandle&) = delete;

private:
  static uint64_t GetNextHandle();

private:
  uint64_t m_handle = 0;
  static std::atomic<uint64_t> m_currentHandle;
};

#endif // ZSTACKOBJECTHANDLE_H
