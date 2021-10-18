#ifndef ZFUNCTIONSTACKSOURCE_H
#define ZFUNCTIONSTACKSOURCE_H

#include<functional>

#include "zstacksource.h"

class ZFunctionStackSource : public ZStackSource
{
public:
  ZFunctionStackSource();
  ZFunctionStackSource(std::function<int(int, int, int)> intValue);

  int getIntValue(int x, int y, int z) const override;

private:
  std::function<int(int, int, int)> m_getIntValue;
};

#endif // ZFUNCTIONSTACKSOURCE_H
