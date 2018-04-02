#include "zdviddataslicehelper.h"
#include "zrect2d.h"
#include "zintcuboid.h"
#include "misc/miscutility.h"

ZDvidDataSliceHelper::ZDvidDataSliceHelper()
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
  int maxZoomLevel = getDvidTarget().getMaxGrayscaleZoom();
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

int ZDvidDataSliceHelper::getScale() const
{
  return misc::GetZoomScale(getZoom());
}

void ZDvidDataSliceHelper::setViewParam(const ZStackViewParam &viewParam)
{
  m_currentViewParam = viewParam;
}

ZStackViewParam ZDvidDataSliceHelper::getValidViewParam(
    const ZStackViewParam &viewParam) const
{
  ZStackViewParam newViewParam = viewParam;

  int maxZoomLevel = getDvidTarget().getMaxGrayscaleZoom();
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
  int maxZoomLevel = getDvidTarget().getMaxGrayscaleZoom();

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

void ZDvidDataSliceHelper::setBoundBox(const ZRect2d &rect)
{
  m_currentViewParam.setViewPort(
        QRect(rect.getX0(), rect.getY0(), rect.getWidth(), rect.getHeight()));
}
