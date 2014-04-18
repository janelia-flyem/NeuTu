#ifndef ZSTRINGARRAY_H
#define ZSTRINGARRAY_H

#include <vector>
#include "zstring.h"

class ZStringArray : public std::vector<ZString>
{
public:
  ZStringArray();
};

#endif // ZSTRINGARRAY_H
