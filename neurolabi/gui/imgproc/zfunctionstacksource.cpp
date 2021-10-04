#include "zfunctionstacksource.h"

ZFunctionStackSource::ZFunctionStackSource()
{

}

ZFunctionStackSource::ZFunctionStackSource(
    std::function<int(int, int, int)> intValue)
{
  m_getIntValue = intValue;
}

int ZFunctionStackSource::getIntValue(int x, int y, int z) const
{
  return m_getIntValue ? m_getIntValue(x, y, z) : 0;
}
