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
  updateCenterCut();
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

void ZDvidDataSliceHelper::updateCenterCut()
{
  switch (m_dataRole) {
  case ZDvidData::ROLE_GRAY_SCALE:
    m_reader.setGrayCenterCut(m_centerCutWidth, m_centerCutHeight);
    break;
  case ZDvidData::ROLE_LABEL_BLOCK:
    m_reader.setLabelCenterCut(m_centerCutWidth, m_centerCutHeight);
    break;
  default:
    break;
  }
}

bool ZDvidDataSliceHelper::validateSize(int *width, int *height) const
{
  bool changed = false;

  if (hasMaxSize()) {
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

size_t ZDvidDataSliceHelper::getViewPortArea() const
{
  return size_t(getWidth()) * size_t(getHeight());
}

size_t ZDvidDataSliceHelper::getViewDataSize() const
{
  int scale = getScale();
  return getViewPortArea() / scale / scale;
}

size_t ZDvidDataSliceHelper::GetViewDataSize(
    const ZStackViewParam &viewParam, int zoom)
{
  int scale = misc::GetZoomScale(zoom);

  return viewParam.getArea() / scale / scale;
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

void ZDvidDataSliceHelper::setZoom(int zoom)
{
  m_zoom = std::max(0, std::min(zoom, m_maxZoom));
}

int ZDvidDataSliceHelper::getLowresZoom() const
{
  int zoom = getZoom() + 1;
  if (zoom > getMaxZoom()) {
    zoom -= 1;
  }

  return zoom;
}

bool ZDvidDataSliceHelper::hasMaxSize(int width, int height) const
{
  return getMaxWidth() == width && getMaxHeight() == height;
}

bool ZDvidDataSliceHelper::hasMaxSize() const{
  return m_maxWidth > 0 && m_maxHeight > 0;
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

bool ZDvidDataSliceHelper::hasNewView(
    const ZStackViewParam &viewParam, int centerCutX, int centerCutY) const
{
  bool newView = hasNewView(viewParam);
  if (newView == false) {
    if (centerCutX < m_centerCutWidth || centerCutY < m_centerCutHeight) {
      newView = true;
    }
  }

  return newView;
}

bool ZDvidDataSliceHelper::containedIn(
    const ZStackViewParam &viewParam, int zoom, int centerCutX, int centerCutY,
    bool centerCut)
const
{
  bool contained = false;

  if (viewParam.contains(m_currentViewParam)) {
    if (zoom == m_zoom) {
      if (!centerCut && !usingCenterCut()) {
        if (centerCutX >= m_centerCutWidth && centerCutY >= m_centerCutHeight) {
          contained = true;
        }
      } else {
        contained = usingCenterCut();
      }
    } else if (zoom < m_zoom) {
      contained = true;
    }
  }

  return contained;
}

void ZDvidDataSliceHelper::invalidateViewParam()
{
  m_currentViewParam.invalidate();
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
  updateCenterCut();
}

void ZDvidDataSliceHelper::setUnlimitedSize()
{
  setMaxSize(0, 0);
}

void ZDvidDataSliceHelper::setBoundBox(const ZRect2d &rect)
{
  m_currentViewParam.setViewPort(
        QRect(rect.getX0(), rect.getY0(), rect.getWidth(), rect.getHeight()));
}
