#include "zdvidinfo.h"

#include <iostream>
#include "zjsonobject.h"
#include "zjsonparser.h"
#include "tz_math.h"

const int ZDvidInfo::m_defaultBlockSize = 32;

const char* ZDvidInfo::m_minPointKey = "MinPoint";
const char* ZDvidInfo::m_maxPointKey = "MaxPoint";
const char* ZDvidInfo::m_blockSizeKey = "BlockSize";
const char* ZDvidInfo::m_voxelSizeKey = "VoxelSize";
const char* ZDvidInfo::m_blockMinIndexKey = "MinIndex";
const char* ZDvidInfo::m_blockMaxIndexKey = "MaxIndex";

ZDvidInfo::ZDvidInfo() : m_dvidPort(7000)
{
  m_stackSize.resize(3, 0);
  m_voxelResolution.resize(3, 1.0);
  m_blockSize.resize(3, m_defaultBlockSize);
}

void ZDvidInfo::setFromJsonString(const std::string &str)
{
  ZJsonObject obj;
  if (obj.decode(str)) {
    if (obj.hasKey(m_minPointKey)) {
      ZJsonArray array(obj[m_minPointKey], false);
      std::vector<int> startCoordinates = array.toIntegerArray();
      if (startCoordinates.size() == 3) {
        m_startCoordinates.set(startCoordinates);
      } else {
        m_startCoordinates.set(0, 0, 0);
      }
    }

    if (obj.hasKey(m_maxPointKey)) {
      ZJsonArray array(obj[m_maxPointKey], false);
      std::vector<int> endCoordinates = array.toIntegerArray();
      if (endCoordinates.size() == 3) {
        for (int i = 0; i < 3; ++i) {
          m_stackSize[i] =  endCoordinates[i] - m_startCoordinates[i] + 1;
        }
      } else {
        for (int i = 0; i < 3; ++i) {
          m_stackSize[i] = 0;
        }
      }
    }

    if (obj.hasKey(m_blockMinIndexKey)) {
      ZJsonArray array(obj[m_blockMinIndexKey], false);
      std::vector<int> startBlockIndex = array.toIntegerArray();
      if (startBlockIndex.size() == 3) {
        m_startBlockIndex.set(startBlockIndex);
      } else {
        m_startBlockIndex.set(0, 0, 0);
      }
    }

    if (obj.hasKey(m_blockMaxIndexKey)) {
      ZJsonArray array(obj[m_blockMaxIndexKey], false);
      std::vector<int> blockIndex = array.toIntegerArray();
      if (blockIndex.size() == 3) {
        m_endBlockIndex.set(blockIndex);
      } else {
        m_endBlockIndex.set(0, 0, 0);
      }
    }

    if (obj.hasKey(m_blockSizeKey)) {
      ZJsonArray array(obj[m_blockSizeKey], false);
      m_blockSize = array.toIntegerArray();
      if (m_blockSize.size() != 3) {
        m_blockSize.resize(m_defaultBlockSize);
      }
    }

    if (obj.hasKey(m_voxelSizeKey)) {
      ZJsonArray array(obj[m_voxelSizeKey], false);
      m_voxelResolution = array.toNumberArray();
      if (m_voxelResolution.size() != 3) {
        m_voxelResolution.resize(3, 1);
      }
    }
  }
}

void ZDvidInfo::print() const
{
  std::cout << "Dvid server: ";
  if (m_dvidAddress.empty()) {
    std::cout << "N/A" << std::endl;
  } else {
    std::cout << m_dvidAddress << std::endl;
  }

  std::cout << "Port: " << m_dvidPort << std::endl;
  if (!m_dvidUuid.empty()) {
    std::cout << "UUID: " << m_dvidUuid << std::endl;
  }
  std::cout << "Stack size: " << m_stackSize[0] << " " << m_stackSize[1] << " "
            << m_stackSize[2] << std::endl;
  std::cout << "Voxel resolution: " << m_voxelResolution[0] << " x "
            << m_voxelResolution[1] << " x " << m_voxelResolution[2] << std::endl;
  std::cout << "Start coordinates: " << "(" << m_startCoordinates[0] << ", "
            << m_startCoordinates[1] << ", " << m_startCoordinates[2]
            << ")" << std::endl;
  std::cout << "Start block: (" << m_startBlockIndex[0] << ", "
            << m_startBlockIndex[1] << ", " << m_startBlockIndex[2]
            << ")" << std::endl;
  std::cout << "Block size: " << m_blockSize[0] << " x "
            << m_blockSize[1] << " x " << m_blockSize[2] << std::endl;
}

ZIntPoint ZDvidInfo::getBlockSize() const
{
  return ZIntPoint(m_blockSize[0], m_blockSize[1], m_blockSize[2]);
}

ZIntPoint ZDvidInfo::getBlockIndex(int x, int y, int z)
{
  ZIntPoint blockIndex(-1, -1, -1);

  if (x < m_startCoordinates[0] ||
      x >= m_startCoordinates[0] + m_stackSize[0]) {
    return blockIndex;
  }
  if (y < m_startCoordinates[1] ||
      y >= m_startCoordinates[1] + m_stackSize[1]) {
    return blockIndex;
  }
  if (z < m_startCoordinates[2] ||
      z >= m_startCoordinates[2] + m_stackSize[2]) {
    return blockIndex;
  }

  blockIndex.set(
        (x - m_startCoordinates[0]) / m_blockSize[0] + m_startBlockIndex[0],
      (y - m_startCoordinates[1]) / m_blockSize[1] + m_startBlockIndex[1],
      (z - m_startCoordinates[2]) / m_blockSize[2] + m_startBlockIndex[2]);

#if 0
  for (int i = 0; i < 3; ++i) {
    blockIndex[i] = (pt[i] - m_startCoordinates[i]) /
        m_blockSize[i] + m_startBlockIndex[i];
  }
#endif
  return blockIndex;
}

ZIntPoint ZDvidInfo::getBlockIndex(double x, double y, double z)
{
  ZIntPoint blockIndex(-1, -1, -1);

  if (x < m_startCoordinates[0] ||
      x >= m_startCoordinates[0] + m_stackSize[0]) {
    return blockIndex;
  }
  if (y < m_startCoordinates[1] ||
      y >= m_startCoordinates[1] + m_stackSize[1]) {
    return blockIndex;
  }
  if (z < m_startCoordinates[2] ||
      z >= m_startCoordinates[2] + m_stackSize[2]) {
    return blockIndex;
  }

  int pt[3];

  pt[0] = iround(x);
  pt[1] = iround(y);
  pt[2] = iround(z);

  for (int i = 0; i < 3; ++i) {
    blockIndex[i] = (pt[i] - m_startCoordinates[i]) /
        m_blockSize[i] + m_startBlockIndex[i];
  }

  return blockIndex;
}

bool ZDvidInfo::isValidBlockIndex(const ZIntPoint &pt)
{
  for (int i = 0; i < 3; ++i) {
    if (pt[i] < m_startBlockIndex[i]) {
      return false;
    }
  }

  return true;
}

ZObject3dScan ZDvidInfo::getBlockIndex(const ZObject3dScan &obj)
{
  ZIntPoint gridSize = m_endBlockIndex - m_startBlockIndex + 1;
  size_t area = ((size_t) gridSize.getX()) * gridSize.getY();
  size_t blockNumber = area * gridSize.getZ();
  std::vector<bool> isAdded(blockNumber, false);

  //std::set<ZIntPoint> blockSet;
  //ZIntPointArray blockArray;
  ZObject3dScan blockObj;

  for (size_t i = 0; i < obj.getStripeNumber(); ++i) {
#ifdef _DEBUG_
    if (i % 10000 == 0) {
      std::cout << i << "/" << obj.getStripeNumber() << std::endl;
    }
#endif
    const ZObject3dStripe &stripe = obj.getStripe(i);
    int y = stripe.getY();
    int z = stripe.getZ();
    for (int j = 0; j < stripe.getSegmentNumber(); ++j) {
      ZIntPoint block1 = getBlockIndex(stripe.getSegmentStart(j), y, z);
      size_t blockIndex1 = area * block1.getZ() +
          gridSize.getY() * block1.getY() + block1.getX();
      ZIntPoint block2 = getBlockIndex(stripe.getSegmentEnd(j), y, z);
      size_t blockIndex2 = area * block2.getZ() +
          gridSize.getY() * block2.getY() + block2.getX();

      if (!isAdded[blockIndex1] || !isAdded[blockIndex2]) {
        blockObj.addSegment(
              block1.getZ(), block1.getY(), block1.getX(), block2.getX(), false);
        isAdded[blockIndex1] = true;
        isAdded[blockIndex2] = true;
      }
      //ZDVIDINFO_ADD_BLOCK(block1);
      //ZDVIDINFO_ADD_BLOCK(block2);

#if 0
      for (int x = block1.getX(); x <= block2.getX(); ++x) {
        ZIntPoint block(x, block1.getY(), block1.getZ());
        size_t blockIndex =
            area * block.getZ() + gridSize.getY() * block.getY() +
            block.getX();
        if (!isAdded[blockIndex]) {
          blockArray.append(block);
          isAdded[blockIndex] = true;
        }
          //blockSet.insert(block);
      }
#endif
    }
  }

  //blockArray.append(blockSet.begin(), blockSet.end());
  blockObj.canonize();

  return blockObj;
}

ZIntPoint ZDvidInfo::getGridSize() const
{
  return ZIntPoint(m_endBlockIndex.getX() - m_startBlockIndex.getX() + 1,
                   m_endBlockIndex.getY() - m_startBlockIndex.getY() + 1,
                   m_endBlockIndex.getZ() - m_startBlockIndex.getZ() + 1);
}

int ZDvidInfo::getMinZ() const
{
  return m_startCoordinates.getZ();
}

int ZDvidInfo::getMaxZ() const
{
  return getMinZ() + m_stackSize[2] - 1;
}

ZIntCuboid ZDvidInfo::getBlockBox(int x, int y, int z) const
{
  x -= m_startBlockIndex.getX();
  y -= m_startBlockIndex.getY();
  z -= m_startBlockIndex.getZ();

  ZIntCuboid cuboid;

  cuboid.setFirstCorner(x * m_blockSize[0] + m_startCoordinates.getX(),
      y * m_blockSize[1] + m_startCoordinates.getY(),
      z * m_blockSize[2] + m_startCoordinates.getZ());
  cuboid.setSize(m_blockSize[0], m_blockSize[1], m_blockSize[2]);

  return cuboid;
}
