#ifndef ZSTACKOBJECTHANDLE_H
#define ZSTACKOBJECTHANDLE_H

#include <atomic>

class ZStackObjectHandle
{
public:
  ZStackObjectHandle();
  ZStackObjectHandle(ZStackObjectHandle &&rhs) = default;
  ZStackObjectHandle(const ZStackObjectHandle&) = default;

  bool operator == (const ZStackObjectHandle &handle) const;
  bool operator != (const ZStackObjectHandle &handle) const;

  bool isValid() const;

  ZStackObjectHandle& operator=(ZStackObjectHandle &&rhs) = delete;
  ZStackObjectHandle& operator=(const ZStackObjectHandle&) = delete;

public: // for testing
  uint64_t _getValue_() const;
  static void _reset_();

private:
  static uint64_t GetNextHandle();
  uint64_t getValue() const;

private:
  const uint64_t m_handle = 0;
  static std::atomic<uint64_t> m_currentHandle;
};

#endif // ZSTACKOBJECTHANDLE_H
