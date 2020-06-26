#ifndef DISPLAYCONFIG_H
#define DISPLAYCONFIG_H

#include "geometry/zaffinerect.h"

#include "zsliceviewtransform.h"

namespace neutu {
namespace data3d {

enum class EDisplayStyle {
  NORMAL, SOLID, BOUNDARY, SKELETON
};

enum class EDisplaySliceMode {
  PROJECTION, //Display Z-projection of the object
  SINGLE      //Display a cross section of the object
};

class ViewSpaceAlignedDisplayConfig
{
public:
  ViewSpaceAlignedDisplayConfig();

//  int getZ() const;
  EDisplayStyle getStyle() const;
  EDisplaySliceMode getSliceMode() const;
  ZViewPlaneTransform getTransform() const;

//  void setZ(int z);
  void setStyle(EDisplayStyle style);
  void setSliceMode(EDisplaySliceMode mode);
  void setTransform(const ZViewPlaneTransform &transform);
  void setTransform(double dx, double dy, double s);

//  int getSlice(int z0) const;

private:
//  int m_z = 0;
  EDisplayStyle m_style = EDisplayStyle::SOLID;
  EDisplaySliceMode m_sliceMode = EDisplaySliceMode::SINGLE;
  ZViewPlaneTransform m_transform;
};

class DisplayConfig
{
public:
  DisplayConfig();

//  int getZ() const;
//  int getSlice(int z0);
  neutu::EAxis getSliceAxis() const;

  EDisplayStyle getStyle() const;
  EDisplaySliceMode getSliceMode() const;
  ZAffinePlane getCutPlane() const;
  ZAffineRect getCutRect(double width, double height) const;
  ZAffineRect getCutRect(
      double width, double height, neutu::data3d::ESpace sizeSpace) const;
//  double getCutDepth() const;

//  void setZ(int z);
  void setStyle(EDisplayStyle style);
  void setSliceMode(EDisplaySliceMode mode);

  double getCutDepth(const ZPoint &origin) const;

  void setCutPlane(const ZAffinePlane &plane);
  void setCutPlane(neutu::EAxis sliceAxis, double cutDepth);
  void setCanvasRange(double width, double height);

  void setViewCanvasTransform(double dx, double dy, double s);

  ZSliceViewTransform getTransform() const;
  void setTransform(const ZSliceViewTransform &transform);

  ZModelViewTransform getWorldViewTransform() const;
  ZViewPlaneTransform getViewCanvasTransform() const;

//  void setCutRect(const ZAffineRect &rect);
//  void setSliceAxis(neutu::EAxis axis);

private:
  ZModelViewTransform m_transform;
  ViewSpaceAlignedDisplayConfig m_alignedConfig;
};

class DisplayConfigBuilder {

public:
  DisplayConfigBuilder();

  operator DisplayConfig();
//  void z(int z);
  DisplayConfigBuilder& style(EDisplayStyle style);
  DisplayConfigBuilder& sliceMode(EDisplaySliceMode mode);
//  void axis(neutu::EAxis axis);
  DisplayConfigBuilder& cutPlane(neutu::EAxis axis, double cutDepth);

private:
  DisplayConfig m_result;
};

}
}

#endif // DISPLAYCONFIG_H
