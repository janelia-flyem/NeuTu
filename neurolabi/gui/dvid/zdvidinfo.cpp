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
  std::cout << "Dvid server: " << m_dvidAddress << " " << m_dvidPort << " "
            << m_dvidUuid << std::endl;
  std::cout << "Stack size: " << m_stackSize[0] << " " << m_stackSize[1] << " "
            << m_stackSize[2] << std::endl;
  std::cout << "Voxel resolution: " << m_voxelResolution[0] << " x "
            << m_voxelResolution[1] << " x " << m_voxelResolution[2] << std::endl;
  std::cout << "Start coordinates: " << m_startCoordinates[0] << " "
            << m_startCoordinates[1] << " " << m_startCoordinates[2] << std::endl;
  std::cout << "Start block: " << m_startBlockIndex[0] << " "
            << m_startBlockIndex[1] << " " << m_startBlockIndex[2] << std::endl;
  std::cout << "Block size: " << m_blockSize[0] << " x "
            << m_blockSize[1] << " x " << m_blockSize[2] << std::endl;
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

  std::vector<int> pt(3);

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

ZIntPointArray ZDvidInfo::getBlockIndex(const ZObject3dScan &obj)
{
  std::set<ZIntPoint> blockSet;
  for (size_t i = 0; i < obj.getStripeNumber(); ++i) {
    const ZObject3dStripe &stripe = obj.getStripe(i);
    for (int j = 0; j < stripe.getSegmentNumber(); ++j) {
      ZIntPoint block1 = getBlockIndex(
            stripe.getSegmentStart(j), stripe.getY(), stripe.getZ());
      ZIntPoint block2 = getBlockIndex(
            stripe.getSegmentEnd(j), stripe.getY(), stripe.getZ());
      for (int y = block1.getY(); y <= block2.getY(); ++y) {
        ZIntPoint block(block1.getX(), y, block1.getZ());
        blockSet.insert(block);
      }
    }
  }

  ZIntPointArray blockArray;
  blockArray.append(blockSet.begin(), blockSet.end());

  return blockArray;
}
