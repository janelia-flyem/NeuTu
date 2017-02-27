#ifndef ZRESOLUTION_H
#define ZRESOLUTION_H

#include <string>
#include "neutube_def.h"

class ZResolution {
public:
  ZResolution();

  inline double voxelSize() const {
    return m_voxelSize[0] * m_voxelSize[1] * m_voxelSize[2]; }
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

  enum EUnit {
    UNIT_PIXEL, UNIT_MICRON, UNIT_NANOMETER
  };

  inline void setUnit(char unit) { m_unit = unit; }
  void setUnit(const std::string &unit);
  std::string getUnitName() const;

  void convertUnit(char unit);

  EUnit getUnit() const;
  void setUnit(EUnit unit);

  /*!
   * \brief Get the voxel size along a certain dimension
   *
   * It computes the size base on the following rules:
   *   1. If \a unit is the same as the unit of the object, there is no conversion
   *   2. If \a unit is pixel and the unit of the object is um or mm,
   *      it returns 1
   *   3. If \a unit is um or mm and the unit of the object is pixel, then it
   *      returns 1
   *   4. If \a unit is um or mm and the unit of the object is mm or um, then do
   *      the unit conversion
   *
   * \return Voxel size based on the unit.
   */
  double getVoxelSize(NeuTube::EAxis axis, EUnit unit) const;

  /*!
   * \brief Get voxel size on a plane.
   *
   * \return area of a voxel on the plane.
   */
  double getPlaneVoxelSize(NeuTube::EPlane plane, EUnit unit) const;


private:
  double m_voxelSize[3];
  char m_unit; //'p' for pixel, 'u' for um, 'n' for nm
};


#endif // ZRESOLUTION_H
