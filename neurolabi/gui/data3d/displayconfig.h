#ifndef DISPLAYCONFIG_H
#define DISPLAYCONFIG_H

#include "geometry/zaffinerect.h"

#include "zworldviewtransform.h"
#include "zviewplanetransform.h"

namespace zstackobject {

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

//  void setZ(int z);
  void setStyle(EDisplayStyle style);
  void setSliceMode(EDisplaySliceMode mode);
  void setTransform(const ZViewPlaneTransform &transform);

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

  EDisplayStyle getStyle();
  ZAffinePlane getCutPlane() const;
  ZAffineRect getCutRect() const;

//  void setZ(int z);
  void setStyle(EDisplayStyle style);
  void setSliceMode(EDisplaySliceMode mode);

  void setCutPlane(const ZAffinePlane &plane);
  void setCutPlane(neutu::EAxis sliceAxis, double cutDepth);
  void setCanvasRange(double width, double height);

//  void setCutRect(const ZAffineRect &rect);
//  void setSliceAxis(neutu::EAxis axis);

private:
  ZWorldViewTransform m_transform;
  double m_width;
  double m_height;
//  neutu::EAxis m_sliceAxis = neutu::EAxis::Z;
//  ZAffineRect m_cutRect;
  ViewSpaceAlignedDisplayConfig m_alignedConfig;
};

class DisplayConfigBuilder {

public:
  DisplayConfigBuilder();

  operator DisplayConfig();
//  void z(int z);
  void style(EDisplayStyle style);
  void sliceMode(EDisplaySliceMode mode);
//  void axis(neutu::EAxis axis);
  void cutPlane(neutu::EAxis axis, double cutDepth);

private:
  DisplayConfig m_result;
};

}

#endif // DISPLAYCONFIG_H
