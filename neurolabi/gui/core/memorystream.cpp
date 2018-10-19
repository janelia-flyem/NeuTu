#include "memorystream.h"

#include <iostream>

/*
ZMemoryInputStream::ZMemoryInputStream(const char *data, size_t size) :
  ZMemoryStreamBuf(data, size),
  std::istream(static_cast<std::streambuf*>(this)),
  m_data(data)
{
}
*/

const char* ZMemoryInputStream::getCurrentDataPointer()
{
  if (tellg() < 0 || eof()) {
    return nullptr;
  }

  return m_data + tellg();
}


void ZMemoryInputStream::move(std::streampos pos)
{
  seekg(pos, std::ios_base::cur);
//  if (good()) {
//    m_data += pos;
//  } else {
//    m_data = nullptr;
//  }

#ifdef _DEBUG_2
  std::cout << "tellg: " << tellg() << std::endl;
#endif
}


