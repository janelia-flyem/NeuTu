#include "displayconfig.h"

neutu::data3d::ViewSpaceAlignedDisplayConfig::ViewSpaceAlignedDisplayConfig()
{

}

/*
int zstackobject::ViewSpaceAlignedDisplayConfig::getZ() const
{
//  return m_z;
}
*/

neutu::data3d::EDisplayStyle
neutu::data3d::ViewSpaceAlignedDisplayConfig::getStyle() const
{
  return m_style;
}

neutu::data3d::EDisplaySliceMode
neutu::data3d::ViewSpaceAlignedDisplayConfig::getSliceMode() const
{
  return m_sliceMode;
}

ZViewPlaneTransform
neutu::data3d::ViewSpaceAlignedDisplayConfig::getTransform() const
{
  return m_transform;
}

/*
void zstackobject::ViewSpaceAlignedDisplayConfig::setZ(int z)
{
  m_z = z;
}
*/

void neutu::data3d::ViewSpaceAlignedDisplayConfig::setStyle(EDisplayStyle style)
{
  m_style = style;
}

void neutu::data3d::ViewSpaceAlignedDisplayConfig::setSliceMode(
    EDisplaySliceMode mode)
{
  m_sliceMode = mode;
}

void neutu::data3d::ViewSpaceAlignedDisplayConfig::setTransform(
    const ZViewPlaneTransform &transform)
{
  m_transform = transform;
}
void neutu::data3d::ViewSpaceAlignedDisplayConfig::setTransform(
    double dx, double dy, double s)
{
  m_transform.set(dx, dy, s);
}

/*
int zstackobject::ViewSpaceAlignedDisplayConfig::getSlice(int z0) const
{
  return getSliceMode() ==
      EDisplaySliceMode::SINGLE ? (getZ() - z0) : -1;
}
*/

//DisplayConfig

neutu::data3d::DisplayConfig::DisplayConfig()
{

}

/*
int zstackobject::DisplayConfig::getZ() const
{
  return m_transform.getCutDepth();
//  return m_alignedConfig.getZ();
}
*/

/*
int zstackobject::DisplayConfig::getSlice(int z0)
{
  return m_alignedConfig.getSlice(z0);
}
*/

neutu::EAxis neutu::data3d::DisplayConfig::getSliceAxis() const
{
  return m_transform.getSliceAxis();
}

neutu::data3d::EDisplayStyle neutu::data3d::DisplayConfig::getStyle() const
{
  return m_alignedConfig.getStyle();
}

neutu::data3d::EDisplaySliceMode neutu::data3d::DisplayConfig::getSliceMode() const
{
  return m_alignedConfig.getSliceMode();
}

ZAffinePlane neutu::data3d::DisplayConfig::getCutPlane() const
{
  return m_transform.getCutPlane();
}

ZAffineRect neutu::data3d::DisplayConfig::getCutRect(double width, double height) const
{
  ZAffineRect rect;
  rect.setPlane(m_transform.getCutPlane());
  rect.setSize(width, height);

  return rect;
}

ZAffineRect neutu::data3d::DisplayConfig::getCutRect(
    double width, double height, neutu::data3d::ESpace sizeSpace) const
{
  return getTransform().getCutRect(width, height, sizeSpace);
}

double neutu::data3d::DisplayConfig::getCutDepth(const ZPoint &origin) const
{
  return m_transform.getCutDepth(origin);
}

/*
double zstackobject::DisplayConfig::getCutDepth() const
{
  return m_transform.getCutDepth();
}
*/

/*
void zstackobject::DisplayConfig::setZ(int z)
{
  m_transform.setD
//  m_alignedConfig.setZ(z);
}
*/

void neutu::data3d::DisplayConfig::setStyle(EDisplayStyle style)
{
  m_alignedConfig.setStyle(style);
}

void neutu::data3d::DisplayConfig::setSliceMode(EDisplaySliceMode mode)
{
  m_alignedConfig.setSliceMode(mode);
}

void neutu::data3d::DisplayConfig::setCutPlane(const ZAffinePlane &plane)
{
  m_transform.setCutPlane(plane);
}

void neutu::data3d::DisplayConfig::setCutPlane(
    neutu::EAxis sliceAxis, double cutDepth)
{
  m_transform.setCutPlane(sliceAxis, cutDepth);
}

int neutu::data3d::DisplayConfig::getViewId() const
{
  return m_viewId;
}

void neutu::data3d::DisplayConfig::setViewId(int id)
{
  m_viewId = id;
}

ZModelViewTransform neutu::data3d::DisplayConfig::getWorldViewTransform() const
{
  return m_transform;
}

ZViewPlaneTransform neutu::data3d::DisplayConfig::getViewCanvasTransform() const
{
  return m_alignedConfig.getTransform();
}

void neutu::data3d::DisplayConfig::setViewCanvasTransform(
    double dx, double dy, double s)
{
  m_alignedConfig.setTransform(dx, dy, s);
}

ZSliceViewTransform neutu::data3d::DisplayConfig::getTransform() const
{
  return ZSliceViewTransform(m_transform, m_alignedConfig.getTransform());
}

void neutu::data3d::DisplayConfig::setTransform(
    const ZSliceViewTransform &transform)
{
  m_alignedConfig.setTransform(transform.getViewCanvasTransform());
  m_transform = transform.getModelViewTransform();
}

/*
void zstackobject::DisplayConfig::setCutRect(const ZAffineRect &rect)
{

//  m_cutRect = rect;
}

void zstackobject::DisplayConfig::setSliceAxis(neutu::EAxis axis)
{
//  m_sliceAxis = axis;
}
*/

//DisplayConfigBuilder

neutu::data3d::DisplayConfigBuilder::DisplayConfigBuilder()
{
}

neutu::data3d::DisplayConfigBuilder::operator DisplayConfig()
{
  return m_result;
}

neutu::data3d::DisplayConfigBuilder&
neutu::data3d::DisplayConfigBuilder::style(EDisplayStyle style)
{
  m_result.setStyle(style);
  return *this;
}

neutu::data3d::DisplayConfigBuilder&
neutu::data3d::DisplayConfigBuilder::sliceMode(EDisplaySliceMode mode)
{
  m_result.setSliceMode(mode);
  return *this;
}


neutu::data3d::DisplayConfigBuilder &neutu::data3d::DisplayConfigBuilder::cutPlane(
    neutu::EAxis axis, double cutDepth)
{
  m_result.setCutPlane(axis, cutDepth);
  return *this;
}

