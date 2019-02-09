#include "zdvidroi.h"

ZDvidRoi::ZDvidRoi()
{
}

ZDvidRoi::~ZDvidRoi()
{
  clear();
}

void ZDvidRoi::clear()
{
  m_roi.clear();
  m_blockSize.set(0, 0, 0);
}

void ZDvidRoi::setName(const std::string &name)
{
  m_name = name;
}

bool ZDvidRoi::isEmpty() const
{
  return m_roi.isEmpty() || (m_blockSize.getX() == 0) ||
      (m_blockSize.getY() == 0) || (m_blockSize.getZ() == 0);
}


void ZDvidRoi::setBlockSize(const ZIntPoint &blockSize)
{
  m_blockSize = blockSize;
}

void ZDvidRoi::setBlockSize(int s)
{
  setBlockSize(s, s, s);
}

void ZDvidRoi::setBlockSize(int w, int h, int d)
{
  setBlockSize(ZIntPoint(w, h, d));
}

ZIntPoint ZDvidRoi::getBlockSize() const
{
  return m_blockSize;
}

std::string ZDvidRoi::getName() const
{
  return m_name;
}

size_t ZDvidRoi::getVolume() const
{
  return m_roi.getVoxelNumber() * m_blockSize.getX() * m_blockSize.getY()
      * m_blockSize.getZ();
}

bool ZDvidRoi::contains(const ZIntPoint &pt) const
{
  return contains(pt.getX(), pt.getY(), pt.getZ());
}

bool ZDvidRoi::contains(int x, int y, int z) const
{
  if (isEmpty()) {
    return false;
  }

  x /= m_blockSize.getX();
  y /= m_blockSize.getY();
  z /= m_blockSize.getZ();

  return const_cast<ZObject3dScan&>(m_roi).contains(x, y, z);
}

