#include "zdvidinfo.h"

#include <iostream>
#include <cmath>

#include "zstring.h"
#include "zjsonobject.h"
#include "zjsonparser.h"
#include "zjsonobjectparser.h"
#include "zobject3dfactory.h"
#include "geometry/zintcuboidarray.h"
#include "geometry/zintcuboid.h"
#include "zobject3dscan.h"

const int ZDvidInfo::m_defaultBlockSize = 32;

const char* ZDvidInfo::KEY_MIN_POINT = "MinPoint";
const char* ZDvidInfo::KEY_MAX_POINT = "MaxPoint";
const char* ZDvidInfo::m_blockSizeKey = "BlockSize";
const char* ZDvidInfo::m_voxelSizeKey = "VoxelSize";
const char* ZDvidInfo::m_voxelUnitKey = "VoxelUnits";
const char* ZDvidInfo::m_blockMinIndexKey = "MinIndex";
const char* ZDvidInfo::m_blockMaxIndexKey = "MaxIndex";
const char* ZDvidInfo::KEY_COMPRESSION = "Compression";

ZDvidInfo::ZDvidInfo() : m_dvidPort(7000)
{
  m_stackSize.resize(3, 0);
  //m_voxelResolution.resize(3, 1.0);
  m_blockSize.resize(3, m_defaultBlockSize);
}

void ZDvidInfo::clear()
{
  for (size_t i = 0; i < m_stackSize.size(); ++i) {
    m_stackSize[i] = 0;
  }

  for (size_t i = 0; i < m_blockSize.size(); ++i) {
    m_blockSize[i] = m_defaultBlockSize;
  }

  m_startCoordinates.set(0, 0, 0);
  m_startBlockIndex.set(0, 0, 0);
  m_endBlockIndex.set(0, 0, 0);
}

bool ZDvidInfo::isValid() const
{
  return (m_stackSize[0] > 0 && m_stackSize[1] > 0 && m_stackSize[2] > 0);
}

void ZDvidInfo::setExtents(const ZJsonObject &obj)
{
  if (obj.hasKey(KEY_MIN_POINT)) {
    ZJsonArray array(obj[KEY_MIN_POINT], ZJsonValue::SET_INCREASE_REF_COUNT);
    std::vector<int> startCoordinates = array.toIntegerArray();
    if (startCoordinates.size() == 3) {
      m_startCoordinates.set(startCoordinates);
    } else {
      m_startCoordinates.set(0, 0, 0);
    }
  }

  if (obj.hasKey(KEY_MAX_POINT)) {
    ZJsonArray array(obj[KEY_MAX_POINT], ZJsonValue::SET_INCREASE_REF_COUNT);
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
}

void ZDvidInfo::set(const ZJsonObject &rootObj)
{
  clear();
  if (!rootObj.isEmpty()) {
    if (rootObj.hasKey("Base")) {
      ZJsonObjectParser parser;
      setCompression(
            parser.getValue(rootObj.value("Base"), KEY_COMPRESSION, ""));
    }

    ZJsonObject obj;

    if (rootObj.hasKey("Extents")) {
      obj.set(rootObj.value("Extents"));
      setExtents(obj);
    }

    if (rootObj.hasKey("Extended")) {
      obj.set(rootObj.value("Extended"));
    } else {
      obj = rootObj;
    }

    if (getDataRange().getVolume() == 0) {
      setExtents(obj);
    }
    /*
    if (obj.hasKey(m_minPointKey)) {
      ZJsonArray array(obj[m_minPointKey], ZJsonValue::SET_INCREASE_REF_COUNT);
      std::vector<int> startCoordinates = array.toIntegerArray();
      if (startCoordinates.size() == 3) {
        m_startCoordinates.set(startCoordinates);
      } else {
        m_startCoordinates.set(0, 0, 0);
      }
    }

    if (obj.hasKey(m_maxPointKey)) {
      ZJsonArray array(obj[m_maxPointKey], ZJsonValue::SET_INCREASE_REF_COUNT);
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
    */

    if (obj.hasKey(m_blockMinIndexKey)) {
      ZJsonArray array(obj[m_blockMinIndexKey], ZJsonValue::SET_INCREASE_REF_COUNT);
      std::vector<int> startBlockIndex = array.toIntegerArray();
      if (startBlockIndex.size() == 3) {
        m_startBlockIndex.set(startBlockIndex);
      } else {
        m_startBlockIndex.set(0, 0, 0);
      }
    }

    if (obj.hasKey(m_blockMaxIndexKey)) {
      ZJsonArray array(obj[m_blockMaxIndexKey], ZJsonValue::SET_INCREASE_REF_COUNT);
      std::vector<int> blockIndex = array.toIntegerArray();
      if (blockIndex.size() == 3) {
        m_endBlockIndex.set(blockIndex);
      } else {
        m_endBlockIndex.set(0, 0, 0);
      }
    }

    if (obj.hasKey(m_blockSizeKey)) {
      ZJsonArray array(obj[m_blockSizeKey], ZJsonValue::SET_INCREASE_REF_COUNT);
      m_blockSize = array.toIntegerArray();
      if (m_blockSize.size() != 3) {
        m_blockSize.resize(m_defaultBlockSize);
      }
    }

    if (obj.hasKey(m_voxelSizeKey)) {
      ZJsonArray array(obj[m_voxelSizeKey], ZJsonValue::SET_INCREASE_REF_COUNT);
      std::vector<double> resolution = array.toNumberArray();
      if (resolution.size() != 3) {
        m_voxelResolution.setVoxelSize(1, 1, 1);
        m_voxelResolution.setUnit('p');
      } else {
        m_voxelResolution.setVoxelSize(
              resolution[0], resolution[1], resolution[2]);
      }
    }

    if (obj.hasKey(m_voxelUnitKey)) {
      ZJsonArray array(obj[m_voxelUnitKey], ZJsonValue::SET_INCREASE_REF_COUNT);
      std::string unit = ZJsonParser::stringValue(array.at(0));
      if (!unit.empty()) {
        m_voxelResolution.setUnit(unit);
      }
    }
  }
}

void ZDvidInfo::setFromJsonString(const std::string &str)
{
  ZJsonObject rootObj;
  rootObj.decode(str, false);
  set(rootObj);
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
  std::cout << "Voxel resolution: " << m_voxelResolution.voxelSizeX() << " x "
            << m_voxelResolution.voxelSizeY() << " x "
            << m_voxelResolution.voxelSizeZ() << std::endl;
  std::cout << "Start coordinates: " << "(" << m_startCoordinates[0] << ", "
            << m_startCoordinates[1] << ", " << m_startCoordinates[2]
            << ")" << std::endl;
  std::cout << "Start block: (" << m_startBlockIndex[0] << ", "
            << m_startBlockIndex[1] << ", " << m_startBlockIndex[2]
            << ")" << std::endl;
  std::cout << "Block size: " << m_blockSize[0] << " x "
            << m_blockSize[1] << " x " << m_blockSize[2] << std::endl;
}

bool ZDvidInfo::isLz4Compression() const
{
  return ZString(m_compression).startsWith("LZ4");
}

bool ZDvidInfo::isJpegCompression() const
{
  return ZString(m_compression).startsWith("jpeg");
}

void ZDvidInfo::setCompression(const std::string &compression)
{
  m_compression = compression;
}

ZIntPoint ZDvidInfo::getBlockSize() const
{
  return ZIntPoint(m_blockSize[0], m_blockSize[1], m_blockSize[2]);
}

int ZDvidInfo::getBlockLevel() const
{
  return zgeom::GetZoomLevel(m_blockSize[0]);
}

int ZDvidInfo::CoordToBlockIndex(int x, int s)
{
  if (x >= 0) {
    return x / s;
  } else {
    return (x + 1) / s - 1;
  }
}

int ZDvidInfo::BlockIndexToCoord(int index, int s)
{
  if (index >= 0) {
    return index * s;
  } else {
    return (index + 1) * s - 1;
  }
}

ZIntPoint ZDvidInfo::getBlockIndex(const ZIntPoint &blockIndex) const
{
  return getBlockIndex(blockIndex.getX(), blockIndex.getY(), blockIndex.getZ());
}

ZIntPoint ZDvidInfo::getBlockIndex(int x, int y, int z) const
{
  ZIntPoint blockIndex(-1, -1, -1);

  blockIndex.set(CoordToBlockIndex(x, m_blockSize[0]),
      CoordToBlockIndex(y, m_blockSize[1]),
      CoordToBlockIndex(z, m_blockSize[2]));

  return blockIndex;
}

ZIntPoint ZDvidInfo::getBlockIndex(double x, double y, double z) const
{
  return getBlockIndex(
        int(std::floor(x)), int(std::floor(y)), int(std::floor(z)));
}

void ZDvidInfo::setBlockSize(int width, int height, int depth)
{
  m_blockSize[0] = width;
  m_blockSize[1] = height;
  m_blockSize[2] = depth;
}

int ZDvidInfo::getCoordZ(int zIndex) const
{
  return BlockIndexToCoord(zIndex, m_blockSize[2]);
}

int ZDvidInfo::getBlockIndexZ(int z) const
{
  return CoordToBlockIndex(z, m_blockSize[2]);
}

bool ZDvidInfo::isValidBlockIndex(const ZIntPoint &pt)
{
  for (int i = 0; i < 3; ++i) {
    if (pt[i] < m_startBlockIndex[i]) {
      return false;
    }
    if (pt[i] > m_endBlockIndex[i]) {
      return false;
    }
  }

  return true;
}

ZObject3dScan ZDvidInfo::getBlockIndex(const ZIntCuboid &box) const
{
  ZIntPoint startIndex = getBlockIndex(box.getMinCorner());
  ZIntPoint endIndex = getBlockIndex(box.getMaxCorner());


  return ZObject3dFactory::MakeObject3dScan(ZIntCuboid(startIndex, endIndex));
}

ZObject3dScan ZDvidInfo::getBlockIndex(const ZIntCuboidArray &boxArray) const
{
  ZObject3dScan obj;
  for (ZIntCuboidArray::const_iterator iter = boxArray.begin();
       iter != boxArray.end(); ++iter) {
    const ZIntCuboid &box = *iter;
    obj.unify(getBlockIndex(box));
  }

  return obj;
}

ZObject3dScan ZDvidInfo::getBlockIndex(const ZObject3dScan &obj) const
{
//  ZIntPoint gridSize = m_endBlockIndex - m_startBlockIndex + 1;
//  size_t area = ((size_t) gridSize.getX()) * gridSize.getY();
//  size_t blockNumber = area * gridSize.getZ();
//  std::vector<bool> isAdded(blockNumber, false);

  //std::set<ZIntPoint> blockSet;
  //ZIntPointArray blockArray;
  ZObject3dScan blockObj;

  for (size_t i = 0; i < obj.getStripeNumber(); ++i) {
#ifdef _DEBUG_2
    if (i % 10000 == 0) {
      std::cout << i << "/" << obj.getStripeNumber() << std::endl;
    }
#endif
    const ZObject3dStripe &stripe = obj.getStripe(i);
    int y = stripe.getY();
    int z = stripe.getZ();
    for (int j = 0; j < stripe.getSegmentNumber(); ++j) {
      int x0 = stripe.getSegmentStart(j);
      int x1 = stripe.getSegmentEnd(j);

      ZIntPoint block1 = getBlockIndex(x0, y, z);
      ZIntPoint block2 = getBlockIndex(x1, y, z);
      blockObj.addSegment(
            block1.getZ(), block1.getY(), block1.getX(), block2.getX(), false);

      }
  }

  blockObj.canonize();

  return blockObj;
}

ZIntPoint ZDvidInfo::getGridSize() const
{
  return ZIntPoint(m_endBlockIndex.getX() - m_startBlockIndex.getX() + 1,
                   m_endBlockIndex.getY() - m_startBlockIndex.getY() + 1,
                   m_endBlockIndex.getZ() - m_startBlockIndex.getZ() + 1);
}

int ZDvidInfo::getMinX() const
{
  return m_startCoordinates.getX();
}

int ZDvidInfo::getMinY() const
{
  return m_startCoordinates.getY();
}

int ZDvidInfo::getMinZ() const
{
  return m_startCoordinates.getZ();
}

int ZDvidInfo::getMaxZ() const
{
  return getMinZ() + m_stackSize[2] - 1;
}

int ZDvidInfo::getMaxX() const
{
  return getMinX() + m_stackSize[0] - 1;
}

int ZDvidInfo::getMaxY() const
{
  return getMinY() + m_stackSize[1] - 1;
}

ZIntCuboid ZDvidInfo::getDataRange() const
{
  return ZIntCuboid(getMinX(), getMinY(), getMinZ(),
                    getMaxX(), getMaxY(), getMaxZ());
}

ZIntPoint ZDvidInfo::getBlockCoord(int ix, int iy, int iz) const
{
  return ZIntPoint(BlockIndexToCoord(ix, m_blockSize[0]),
      BlockIndexToCoord(iy, m_blockSize[1]),
      BlockIndexToCoord(iz, m_blockSize[2]));
}

ZIntCuboid ZDvidInfo::getBlockBox(int ix, int iy, int iz) const
{
  ZIntCuboid cuboid;

  cuboid.setMinCorner(getBlockCoord(ix, iy, iz));
  cuboid.setSize(m_blockSize[0], m_blockSize[1], m_blockSize[2]);

  return cuboid;
}

ZIntCuboid ZDvidInfo::getBlockBox(const ZIntPoint &blockIndex) const
{
  return getBlockBox(blockIndex.getX(), blockIndex.getY(), blockIndex.getZ());
}

ZIntCuboid ZDvidInfo::getBlockBox(int x0, int x1, int y, int z) const
{
  ZIntCuboid cuboid = getBlockBox(x0, y, z);
  cuboid.setWidth(cuboid.getWidth() * (x1 - x0 + 1));

  return cuboid;
}

ZIntPoint ZDvidInfo::getEndCoordinates() const
{
  ZIntPoint pt = getStartCoordinates();
  pt += ZIntPoint(m_stackSize[0], m_stackSize[1], m_stackSize[2]) - 1;

  return pt;
}

void ZDvidInfo::setStackSize(int width, int height, int depth)
{
  m_stackSize[0] = width;
  m_stackSize[1] = height;
  m_stackSize[2] = depth;
}

void ZDvidInfo::setStartBlockIndex(int x, int y, int z)
{
  m_startBlockIndex.set(x, y, z);
}

void ZDvidInfo::setEndBlockIndex(int x, int y, int z)
{
  m_endBlockIndex.set(x, y, z);
}

void ZDvidInfo::downsampleBlock(int xintv, int yintv, int zintv)
{
  m_startBlockIndex /= ZIntPoint(xintv + 1, yintv + 1, zintv + 1);
  m_endBlockIndex /= ZIntPoint(xintv + 1, yintv + 1, zintv + 1) +
      ZIntPoint(1, 1, 1);
  m_blockSize[0] *= xintv + 1;
  m_blockSize[1] *= yintv + 1;
  m_blockSize[2] *= zintv + 1;
}
