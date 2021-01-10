#ifndef ZDVIDINFO_H
#define ZDVIDINFO_H

#include <vector>
#include <string>
#include "geometry/zintpoint.h"
#include "geometry/zintpointarray.h"
#include "zresolution.h"

class ZIntCuboidArray;
class ZObject3dScan;
class ZIntCuboid;

class ZDvidInfo
{
public:
  ZDvidInfo();

  /*!
   * \brief Set info from a json string
   *
   * Properties without correspondng fields will be set to default values.
   *
   * \param str Input json string.
   */
  void setFromJsonString(const std::string &str);
  void set(const ZJsonObject &obj);

  void print() const;

  /*!
   * \brief Get block index of a certain point.
   *
   * \param x X coordinate of the point.
   * \param y Y coordinate of the point.
   * \param z Z coordinate of the point.
   *
   * \return Empty array if the point is out of range.
   */
  ZIntPoint getBlockIndex(double x, double y, double z) const;

  ZIntPoint getBlockIndex(int x, int y, int z) const;
  ZIntPoint getBlockIndex(const ZIntPoint &pt) const;

  /*!
   * \brief Get the first corner of a block.
   *
   * It returns the coordinates of the first corner of the block index
   * (\a ix, \a iy, \a iz).
   */
  ZIntPoint getBlockCoord(int ix, int iy, int iz) const;

  int getBlockIndexZ(int z) const;

  int getCoordZ(int zIndex) const;

  /*!
   * \brief Get the indices of all blocks containing at least one voxel of an object
   */
  ZObject3dScan getBlockIndex(const ZObject3dScan &obj) const;

  /*!
   * \brief Get the indices of blocks covering a cuboid
   */
  ZObject3dScan getBlockIndex(const ZIntCuboid &box) const;
  ZObject3dScan getBlockIndex(const ZIntCuboidArray &boxArray) const;

  inline const ZResolution& getVoxelResolution() const {
    return m_voxelResolution;
  }

  inline const ZIntPoint& getStartCoordinates() const {
    return m_startCoordinates;
  }

  ZIntPoint getEndCoordinates() const;

  inline const std::vector<int>& getStackSize() const {
    return m_stackSize;
  }

  inline const ZIntPoint& getStartBlockIndex() const {
    return m_startBlockIndex;
  }

  inline const ZIntPoint& getEndBlockIndex() const {
    return m_endBlockIndex;
  }

  /*!
   * \brief Get the bound box of a block
   *
   * Get the bound box of a block in DVID coordinates. (\a ix, \a iy, \a iz) is
   * the block index.
   */
  ZIntCuboid getBlockBox(int ix, int iy, int iz) const;
  ZIntCuboid getBlockBox(const ZIntPoint &blockIndex) const;

  ZIntCuboid getBlockBox(int x0, int x1, int y, int z) const;

  ZIntPoint getBlockSize() const;
  ZIntPoint getGridSize() const;

  int getBlockLevel() const;

  bool isValidBlockIndex(const ZIntPoint &pt);

  int getMinZ() const;
  int getMaxZ() const;

  int getMinX() const;
  int getMaxX() const;

  int getMinY() const;
  int getMaxY() const;

  ZIntCuboid getDataRange() const;

  void clear();

  bool isValid() const;

  void setStackSize(int width, int height, int depth);
  void setStartBlockIndex(int x, int y, int z);
  void setEndBlockIndex(int x, int y, int z);

  void setBlockSize(int width, int height, int depth);

  void downsampleBlock(int xintv, int yintv, int zintv);

  void setDvidNode(const std::string &address,
                   int port, const std::string &uuid) {
    m_dvidAddress = address;
    m_dvidPort = port;
    m_dvidUuid = uuid;
  }

  const std::string& getDvidAddress() const {
    return m_dvidAddress;
  }

  int getDvidPort() const {
    return m_dvidPort;
  }

  const std::string& getDvidUuid() const {
    return m_dvidUuid;
  }

  bool isJpegCompression() const;
  bool isLz4Compression() const;

  void setCompression(const std::string &compression);

private:
  static int CoordToBlockIndex(int x, int s);
  /*!
   * \brief Get the first corner of the block.
   */
  static int BlockIndexToCoord(int index, int s);

  void setExtents(const ZJsonObject &obj);

private:
  std::vector<int> m_stackSize;
  //std::vector<double> m_voxelResolution;
  ZResolution m_voxelResolution;
  ZIntPoint m_startCoordinates;
  ZIntPoint m_startBlockIndex;
  ZIntPoint m_endBlockIndex;
  std::vector<int> m_blockSize;
  std::string m_compression;

  std::string m_dvidAddress;
  int m_dvidPort;
  std::string m_dvidUuid;

  const static int m_defaultBlockSize;

  //Json keys
  const static char* KEY_MIN_POINT;
  const static char* KEY_MAX_POINT;
  const static char* m_blockSizeKey;
  const static char* m_voxelSizeKey;
  const static char* m_voxelUnitKey;
  const static char* m_blockMinIndexKey;
  const static char* m_blockMaxIndexKey;
  const static char* KEY_COMPRESSION;
};

#endif // ZDVIDINFO_H
