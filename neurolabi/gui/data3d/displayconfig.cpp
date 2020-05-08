#include "displayconfig.h"

zstackobject::ViewSpaceAlignedDisplayConfig::ViewSpaceAlignedDisplayConfig()
{

}

/*
int zstackobject::ViewSpaceAlignedDisplayConfig::getZ() const
{
//  return m_z;
}
*/

zstackobject::EDisplayStyle
zstackobject::ViewSpaceAlignedDisplayConfig::getStyle() const
{
  return m_style;
}

zstackobject::EDisplaySliceMode
zstackobject::ViewSpaceAlignedDisplayConfig::getSliceMode() const
{
  return m_sliceMode;
}

/*
void zstackobject::ViewSpaceAlignedDisplayConfig::setZ(int z)
{
  m_z = z;
}
*/

void zstackobject::ViewSpaceAlignedDisplayConfig::setStyle(EDisplayStyle style)
{
  m_style = style;
}

void zstackobject::ViewSpaceAlignedDisplayConfig::setSliceMode(
    EDisplaySliceMode mode)
{
  m_sliceMode = mode;
}

/*
int zstackobject::ViewSpaceAlignedDisplayConfig::getSlice(int z0) const
{
  return getSliceMode() ==
      EDisplaySliceMode::SINGLE ? (getZ() - z0) : -1;
}
*/

//DisplayConfig

zstackobject::DisplayConfig::DisplayConfig()
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

neutu::EAxis zstackobject::DisplayConfig::getSliceAxis() const
{
  return m_transform.getSliceAxis();
}

zstackobject::EDisplayStyle zstackobject::DisplayConfig::getStyle()
{
  return m_alignedConfig.getStyle();
}


ZAffinePlane zstackobject::DisplayConfig::getCutPlane() const
{
  return getCutRect().getAffinePlane();
}

ZAffineRect zstackobject::DisplayConfig::getCutRect() const
{
  ZAffineRect rect;
  rect.setPlane(m_transform.getCutPlane());
  rect.setSize(m_width, m_height);

  return rect;
}

/*
void zstackobject::DisplayConfig::setZ(int z)
{
  m_transform.setD
//  m_alignedConfig.setZ(z);
}
*/

void zstackobject::DisplayConfig::setStyle(EDisplayStyle style)
{
  m_alignedConfig.setStyle(style);
}

void zstackobject::DisplayConfig::setSliceMode(EDisplaySliceMode mode)
{
  m_alignedConfig.setSliceMode(mode);
}

void zstackobject::DisplayConfig::setCutPlane(const ZAffinePlane &plane)
{
  m_transform.setCutPlane(plane);
}

void zstackobject::DisplayConfig::setCutPlane(
    neutu::EAxis sliceAxis, double cutDepth)
{
  m_transform.setCutPlane(sliceAxis, cutDepth);
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

zstackobject::DisplayConfigBuilder::DisplayConfigBuilder()
{
}

zstackobject::DisplayConfigBuilder::operator DisplayConfig()
{
  return m_result;
}

void zstackobject::DisplayConfigBuilder::cutPlane(
    neutu::EAxis axis, double cutDepth)
{
  m_result.setCutPlane(axis, cutDepth);
}

