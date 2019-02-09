#ifndef MEMORYSTREAM_H
#define MEMORYSTREAM_H

#include <istream>
#include <streambuf>
#include <boost/iostreams/stream.hpp>

using ZMemoryStreamDevice = boost::iostreams::basic_array_source<char>;
using ZMemoryInputStreamBase = boost::iostreams::stream<ZMemoryStreamDevice>;

class ZMemoryInputStream : public ZMemoryInputStreamBase
{
public:
  ZMemoryInputStream(const char *data, size_t n) : ZMemoryInputStreamBase(data, n),
    m_data(data) {}

  const char* getCurrentDataPointer();

  void move(std::streampos pos);

  template <typename T>
  friend ZMemoryInputStream& operator >> (ZMemoryInputStream &stream, T &value);

private:
  const char *m_data = nullptr;
};

template <typename T>
ZMemoryInputStream& operator >> (ZMemoryInputStream &stream, T &value)
{
  stream.read(reinterpret_cast<char*>(&value), sizeof(T));
//  if (stream.good()) {
//    stream.m_data += sizeof(T);
//  } else {
//    stream.m_data = nullptr;
//  }

  return stream;
}

#if 0

struct ZMemoryStreamBuf : std::streambuf
{
  ZMemoryStreamBuf(const char *base, size_t size) {
    char* p(const_cast<char*>(base));
    this->setg(p, p, p + size);
  }
};


class ZMemoryInputStream : virtual ZMemoryStreamBuf, std::istream
{
public:
  ZMemoryInputStream(const char *data, size_t size);

  const char* getCurrentDataPointer();

  void move(std::streampos pos);

  template <typename T>
  friend ZMemoryInputStream& operator >> (ZMemoryInputStream &stream, T &value);

private:
  const char *m_data = nullptr;
};

template <typename T>
ZMemoryInputStream& operator >> (ZMemoryInputStream &stream, T &value)
{
  stream.read(reinterpret_cast<char*>(&value), sizeof(T));
  if (stream.good()) {
    stream.m_data += sizeof(T);
  } else {
    stream.m_data = nullptr;
  }

  return stream;
}
#endif

#endif // MEMORYSTREAM_H
