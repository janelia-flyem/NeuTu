#ifndef ZFLYEMCOORDINATECONVERTER_H
#define ZFLYEMCOORDINATECONVERTER_H

#include <vector>

class ZFlyEmCoordinateConverter
{
public:
  ZFlyEmCoordinateConverter();

  enum ESpace {
    RAVELER_SPACE, /* Raveler space */
    IMAGE_SPACE, /* Image space */
    PHYSICAL_SPACE, /* resolution scale from image space */
    ROI_SPACE, /* Set the origin as the first corner of the ROI bound box. */
  };

  void setStackSize(int xDim, int yDim, int zDim);
  void setVoxelResolution(double xRes, double yRes, double zRes);
  void setZStart(int zStart);
  void setMargin(int margin);

  void convert(double *x, double *y, double *z, ESpace source, ESpace target);

private:
  void convertFromRavelerSpace(double *x, double *y, double *z, ESpace target);
  void convertFromImageSpace(double *x, double *y, double *z, ESpace target);
  void convertFromPhysicalSpace(double *x, double *y, double *z, ESpace target);
  void convertFromRoiSpace(double *x, double *y, double *z, ESpace target);

private:
  std::vector<int> m_stackSize;
  int m_zStart; //The starting Z of the raveler space
  int m_margin;
  std::vector<double> m_voxelResolution;
};

#endif // ZFLYEMCOORDINATECONVERTER_H
