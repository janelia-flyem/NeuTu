#ifndef ZRESOLUTION_H
#define ZRESOLUTION_H

#include <string>

class ZJsonObject;

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

  void set(double x, double y, double z, char unit);

  inline void setVoxelSize(double x, double y, double z) {
    m_voxelSize[0] = x;
    m_voxelSize[1] = y;
    m_voxelSize[2] = z;
  }
  inline void setUnit(char unit) { m_unit = unit; }
  void setUnit(const std::string &unit);
  std::string getUnitName() const;

  void convertUnit(char unit);

  void loadJsonObject(const ZJsonObject &obj);

  void reset();

private:
  double m_voxelSize[3];
  char m_unit; //'p' for pixel, 'u' for um, 'n' for nm
};


#endif // ZRESOLUTION_H
