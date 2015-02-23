#include "zdvidresolution.h"
#include <iostream>

ZDvidResolution::ZDvidResolution() : m_level(0)
{
}

int ZDvidResolution::getScale() const
{
  int scale = 1;
  for (int level = 0; level  < m_level; ++level) {
    scale *= 2;
  }

  return scale;
}

void ZDvidResolution::print() const
{
  std::cout << "Level: " << getLevel() << std::endl;
  std::cout << "Scale: " << getScale() << std::endl;
}
