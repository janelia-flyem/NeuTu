#include "zdvidblockstream.h"

ZDvidBlockInputStream::ZDvidBlockInputStream(const char *data, size_t size) :
  ZDvidBlockStreamBuf(data, size),
  std::istream(static_cast<std::streambuf*>(this))
{
}
