#include "zindexstacksource.h"

#include "common/utilities.h"

ZIndexStackSource::ZIndexStackSource()
{

}

ZIndexStackSource::ZIndexStackSource(int width, int height, int depth) :
  m_width(width), m_height(height), m_depth(depth)
{

}

void ZIndexStackSource::setSize(int width, int height, int depth)
{
  m_width = width;
  m_height = height;
  m_depth = depth;
}

int ZIndexStackSource::getIntValue(int x, int y, int z) const
{
  if (neutu::WithinCloseOpenRange(x, 0, m_width) &&
      neutu::WithinCloseOpenRange(y, 0, m_height) &&
      neutu::WithinCloseOpenRange(z, 0, m_depth)) {
    return m_width * m_height * z + m_width * y + x;
  }

  return 0;
}
