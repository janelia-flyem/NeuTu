#ifndef ZINDEXSTACKSOURCE_H
#define ZINDEXSTACKSOURCE_H

#include "zstacksource.h"

class ZIndexStackSource : public ZStackSource
{
public:
  ZIndexStackSource();
  ZIndexStackSource(int width, int height, int depth);

  void setSize(int width, int height, int depth);

  int getIntValue(int x, int y, int z) const override;

private:
  int m_width = 0;
  int m_height = 0;
  int m_depth = 0;
};

#endif // ZINDEXSTACKSOURCE_H
