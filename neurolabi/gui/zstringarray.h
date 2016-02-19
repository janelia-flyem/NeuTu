#ifndef ZSTRINGARRAY_H
#define ZSTRINGARRAY_H

#include <vector>
#include "zstring.h"

class ZStringArray : public std::vector<ZString>
{
public:
  ZStringArray();

  static void print(const std::vector<std::string> &strArray);
};

#endif // ZSTRINGARRAY_H
