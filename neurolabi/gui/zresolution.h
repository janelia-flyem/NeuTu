#ifndef ZRESOLUTION_H
#define ZRESOLUTION_H

#include <string>
#include "common/neutudefs.h"
#include "geometry/zpoint.h"

class ZJsonObject;

class ZResolution {
public:
  ZResolution();

  inline double voxelSize() const {
    return m_voxelSize.getX() * m_voxelSize.getY() * m_voxelSize.getZ(); }
  inline double voxelSizeX() const { return m_voxelSize[0]; }
  inline double voxelSizeY() const { return m_voxelSize[1]; }
  inline double voxelSizeZ() const { return m_voxelSize[2]; }
  inline char unit() const { return m_unit; }

  inline const double& operator[] (int index) const {
    return m_voxelSize[index];
  }
  inline double& operator[] (int index) {
    return m_voxelSize[index];
  }

  inline void setVoxelSize(double x, double y, double z) {
    m_voxelSize[0] = x;
    m_voxelSize[1] = y;
    m_voxelSize[2] = z;
  }

  enum class EUnit {
    UNIT_PIXEL, UNIT_MICRON, UNIT_NANOMETER
  };

  inline void setUnit(char unit) { m_unit = unit; }
  void setUnit(const std::string &unit);
  std::string getUnitName() const;

  void convertUnit(char unit);
  void loadJsonObject(const ZJsonObject &obj);

  void reset();

  EUnit getUnit() const;
  void setUnit(EUnit unit);

  /*!
   * \brief Get the voxel size along a certain dimension
   *
   * It computes the size along \a axis base on the following rules:
   *   1. If \a unit is the same as the unit of the object, or,
   *      either \a unit is pixel or the unit of the object is pixel and the
   *      other is not, it returns the orignal value
   *   2. If \a unit is um or mm and the unit of the object is mm or um, then do
   *      the unit conversion
   *
   * \return Voxel size based on the unit.
   */
  double getVoxelSize(neutu::EAxis axis, EUnit unit) const;

  ZPoint getVoxelDims(EUnit unit) const;

  /*!
   * \brief Get the unit voxel size after unit conversion
   *
   * It computes the size base on the following rules:
   *   1. If \a unit is the same as the unit of the object, or,
   *      either \a unit is pixel or the unit of the object is pixel and the
   *      other is not, it returns the orignal value
   *   2. If \a unit is um or mm and the unit of the object is mm or um, then do
   *      the unit conversion
   *
   * \return Voxel size based on the unit.
   */
  double getUnitVoxelSize(EUnit unit) const;

  /*!
   * \brief Get voxel size on a plane.
   *
   * \return area of a voxel on the plane.
   */
  double getPlaneVoxelSize(neutu::EPlane plane, EUnit unit) const;

  double getPlaneVoxelSize(neutu::EPlane plane) const;

  /*!
   * \brief Get sqrt(voxel area)
   */
  double getPlaneVoxelSpan(neutu::EPlane plane, EUnit unit) const;
  double getPlaneVoxelSpan(neutu::EPlane plane) const;

  bool isValid() const;

private:
  ZPoint m_voxelSize;
  char m_unit; //'p' for pixel, 'u' for um, 'n' for nm
};


#endif // ZRESOLUTION_H
