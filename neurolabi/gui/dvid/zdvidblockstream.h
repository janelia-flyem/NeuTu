#ifndef ZDVIDBLOCKSTREAM_H
#define ZDVIDBLOCKSTREAM_H

#include <istream>
#include <streambuf>

struct ZDvidBlockStreamBuf : std::streambuf
{
  ZDvidBlockStreamBuf(const char *base, size_t size) {
    char* p(const_cast<char*>(base));
    this->setg(p, p, p + size);
  }
};

struct ZDvidBlockBuffer {
  char *m_data = nullptr;
  size_t m_length = 0;
};

class ZDvidBlockInputStream : virtual ZDvidBlockStreamBuf, std::istream
{
public:
  ZDvidBlockInputStream(const char *data, size_t size);

  template <typename T>
  friend ZDvidBlockInputStream& operator >> (ZDvidBlockInputStream &stream, T &value);

//  friend ZDvidBlockInputStream& operator >> (
//      ZDvidBlockInputStream &stream, ZDvidBlockBuffer &buffer);
};

template <typename T>
ZDvidBlockInputStream& operator >> (ZDvidBlockInputStream &stream, T &value)
{
  stream.read(reinterpret_cast<char*>(&value), sizeof(T));

  return stream;
}

#endif // ZDVIDBLOCKSTREAM_H
