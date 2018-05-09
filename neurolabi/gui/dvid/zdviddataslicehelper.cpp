#include "zdviddataslicehelper.h"
#include "zrect2d.h"
#include "zintcuboid.h"
#include "misc/miscutility.h"

ZDvidDataSliceHelper::ZDvidDataSliceHelper(ZDvidData::ERole role) :
  m_dataRole(role)
{
}

void ZDvidDataSliceHelper::clear()
{
  m_reader.clear();
}

void ZDvidDataSliceHelper::setDvidTarget(const ZDvidTarget &target)
{
  m_reader.open(target);
}

void ZDvidDataSliceHelper::setMaxZoom(int maxZoom)
{
  m_maxZoom = maxZoom;
}

int ZDvidDataSliceHelper::getMaxZoom() const
{
  switch (m_dataRole) {
  case ZDvidData::ROLE_GRAY_SCALE:
    return getDvidTarget().getMaxGrayscaleZoom();
  case ZDvidData::ROLE_LABEL_BLOCK:
  case ZDvidData::ROLE_BODY_LABEL:
    return getDvidTarget().getMaxLabelZoom();
  case ZDvidData::ROLE_MULTISCALE_2D:
    return m_maxZoom;
  default:
    return 0;
  }

  return 0;
}

bool ZDvidDataSliceHelper::validateSize(int *width, int *height) const
{
  bool changed = false;

  int area = (*width) * (*height);
  if (area > m_maxWidth * m_maxHeight) {
    if (*width > m_maxWidth) {
      *width = m_maxWidth;
    }
    if (*height > m_maxHeight) {
      *height = m_maxHeight;
    }
    changed = true;
  }

  return changed;
}

int ZDvidDataSliceHelper::updateParam(ZStackViewParam *param)
{
  int maxZoomLevel = getMaxZoom();
  if (maxZoomLevel < 3) {
    int width = param->getViewPort().width();
    int height = param->getViewPort().height();
    if (validateSize(&width, &height)) {
      param->resize(width, height);
    }
  }

  return maxZoomLevel;
}

QRect ZDvidDataSliceHelper::getViewPort() const
{
  return getViewParam().getViewPort();
}

int ZDvidDataSliceHelper::getX() const
{
  return getViewPort().left();
}

int ZDvidDataSliceHelper::getY() const
{
  return getViewPort().top();
}

int ZDvidDataSliceHelper::getZ() const
{
  return getViewParam().getZ();
}

void ZDvidDataSliceHelper::setZ(int z)
{
  m_currentViewParam.setZ(z);
}

int ZDvidDataSliceHelper::getWidth() const
{
  return getViewPort().width();
}

int ZDvidDataSliceHelper::getHeight() const
{
  return getViewPort().height();
}

int ZDvidDataSliceHelper::getCenterCutWidth() const
{
  return m_centerCutWidth;
}

int ZDvidDataSliceHelper::getCenterCutHeight() const
{
  return m_centerCutHeight;
}

int ZDvidDataSliceHelper::getScale() const
{
  return misc::GetZoomScale(getZoom());
}

bool ZDvidDataSliceHelper::hasMaxSize(int width, int height) const
{
  return getMaxWidth() == width && getMaxHeight() == height;
}

bool ZDvidDataSliceHelper::getMaxArea() const
{
  return getMaxWidth() * getMaxHeight();
}

void ZDvidDataSliceHelper::setViewParam(const ZStackViewParam &viewParam)
{
  m_currentViewParam = viewParam;
}

ZStackViewParam ZDvidDataSliceHelper::getValidViewParam(
    const ZStackViewParam &viewParam) const
{
  ZStackViewParam newViewParam = viewParam;

  int maxZoomLevel = getMaxZoom();
  if (maxZoomLevel < 3) {
    int width = viewParam.getViewPort().width();
    int height = viewParam.getViewPort().height();
    if (validateSize(&width, &height)) {
      newViewParam.resize(width, height);
    }
  }

  return newViewParam;
}

bool ZDvidDataSliceHelper::hasNewView(const ZStackViewParam &viewParam) const
{
  int maxZoomLevel = getMaxZoom();

  return !m_currentViewParam.contains(viewParam) ||
      (viewParam.getZoomLevel(maxZoomLevel) <
      m_currentViewParam.getZoomLevel(maxZoomLevel));
}

ZIntCuboid ZDvidDataSliceHelper::GetBoundBox(const QRect &viewPort, int z)
{
  ZIntCuboid box;
  box.setFirstCorner(viewPort.left(), viewPort.top(), z);
  box.setSize(viewPort.width(), viewPort.height(), 1);

  return box;
}

ZIntCuboid ZDvidDataSliceHelper::getBoundBox() const
{
  return GetBoundBox(getViewPort(), getZ());
}

void ZDvidDataSliceHelper::setMaxSize(int maxW, int maxH)
{
  m_maxWidth = maxW;
  m_maxHeight = maxH;
}

void ZDvidDataSliceHelper::setCenterCut(int width, int height)
{
  m_centerCutWidth = width;
  m_centerCutHeight = height;
}

void ZDvidDataSliceHelper::setBoundBox(const ZRect2d &rect)
{
  m_currentViewParam.setViewPort(
        QRect(rect.getX0(), rect.getY0(), rect.getWidth(), rect.getHeight()));
}
