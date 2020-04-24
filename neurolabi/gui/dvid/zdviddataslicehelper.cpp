#include "zdviddataslicehelper.h"
#include "zrect2d.h"
#include "geometry/zintcuboid.h"
#include "misc/miscutility.h"
#include "zarbsliceviewparam.h"

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
  m_workReader.openRaw(m_reader.getDvidTarget());
  updateCenterCut();
}

void ZDvidDataSliceHelper::setMaxZoom(int maxZoom)
{
  m_maxZoom = maxZoom;
}

void ZDvidDataSliceHelper::updateMaxZoom()
{
  switch (m_dataRole) {
  case ZDvidData::ERole::GRAYSCALE:
    m_reader.updateMaxGrayscaleZoom();
    break;
  case ZDvidData::ERole::SEGMENTATION:
  case ZDvidData::ERole::SPARSEVOL:
    m_reader.updateMaxLabelZoom();
    break;
  default:
    break;
  }
}

int ZDvidDataSliceHelper::getMaxZoom() const
{
  switch (m_dataRole) {
  case ZDvidData::ERole::GRAYSCALE:
    return getDvidTarget().getMaxGrayscaleZoom();
  case ZDvidData::ERole::SEGMENTATION:
  case ZDvidData::ERole::SPARSEVOL:
    return getDvidTarget().getMaxLabelZoom();
  case ZDvidData::ERole::MULTISCALE_2D:
    return m_maxZoom;
  default:
    return 0;
  }

  return 0;
}

neutu::EDataSliceUpdatePolicy ZDvidDataSliceHelper::getUpdatePolicy() const
{
  return m_updatePolicy;
}

void ZDvidDataSliceHelper::setUpdatePolicy(neutu::EDataSliceUpdatePolicy policy)
{
  m_updatePolicy = policy;
}

void ZDvidDataSliceHelper::updateCenterCut()
{
  switch (m_dataRole) {
  case ZDvidData::ERole::GRAYSCALE:
    m_reader.setGrayCenterCut(m_centerCutWidth, m_centerCutHeight);
    break;
  case ZDvidData::ERole::SEGMENTATION:
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
  if (getViewPort().isEmpty()) {
    return 0;
  }

  return getViewPort().width();
}

int ZDvidDataSliceHelper::getHeight() const
{
  if (getViewPort().isEmpty()) {
    return 0;
  }

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
  int scale = zgeom::GetZoomScale(zoom);

  return viewParam.getArea() / scale / scale;
}

void ZDvidDataSliceHelper::closeViewPort()
{
  m_currentViewParam.closeViewPort();
}

void ZDvidDataSliceHelper::openViewPort()
{
  m_currentViewParam.openViewPort();
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
  return zgeom::GetZoomScale(getZoom());
}

int ZDvidDataSliceHelper::getActualScale() const
{
  return zgeom::GetZoomScale(getActualZoom());
}

int ZDvidDataSliceHelper::getActualZoom() const
{
  return m_actualZoom;
}

void ZDvidDataSliceHelper::setZoom(int zoom)
{
  m_zoom = std::max(0, std::min(zoom, getMaxZoom()));
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

void ZDvidDataSliceHelper::CanonizeQuality(int *zoom, int *centerCutX, int *centerCutY,
    bool *centerCut, int viewWidth, int viewHeight, int maxZoom)
{
  if (*zoom < 0) { //full resolution
    *zoom = 0;
    *centerCut = false;
  } else if (*zoom >= maxZoom) {
    *zoom = maxZoom;
    *centerCut = false;
  } else {
    if (*centerCut) {
      if (*centerCutX == 0 || *centerCutY == 0) { //no center cut area
        *zoom += 1;
        *centerCut = false;
      } else if (*centerCutX >= viewWidth && *centerCutY >= viewHeight) { //full center cut
        *centerCut = false;
      }
    }
  }
}

bool ZDvidDataSliceHelper::IsResIncreasing(
    int sourceZoom, int sourceCenterCutX, int sourceCenterCutY, bool sourceCenterCut,
    int targetZoom, int targetCenterCutX, int targetCenterCutY, bool targetCenterCut,
    int viewWidth, int viewHeight, int maxZoom)
{
  bool result = false;
  CanonizeQuality(
        &sourceZoom, &sourceCenterCutX, &sourceCenterCutY, &sourceCenterCut,
        viewWidth, viewHeight, maxZoom);
  CanonizeQuality(
        &targetZoom, &targetCenterCutX, &targetCenterCutY, &targetCenterCut,
        viewWidth, viewHeight, maxZoom);

  if (sourceZoom == targetZoom) {
    if (!targetCenterCut && sourceCenterCut) {
      result = true;
    } else if (targetCenterCut) {
      if (targetCenterCutX > sourceCenterCutX &&
          targetCenterCutY > sourceCenterCutY) {
        result = true;
      }
    }
  } else if (sourceZoom > targetZoom) {
    result = true;
  }

  return result;
}

/*
bool ZDvidDataSliceHelper::containedIn(
    const ZStackViewParam &viewParam, int zoom, int centerCutX, int centerCutY,
    bool centerCut) const
{
  bool contained = false;

  if (m_currentViewParam.getViewPort().isEmpty() &&
      !viewParam.getViewPort().isEmpty()) {
    contained = true;
  } else if (viewParam.contains(m_currentViewParam)) {
    contained = ZDvidDataSliceHelper::IsResIncreasing(
          getZoom(), getCenterCutWidth(), getCenterCutHeight(), usingCenterCut(),
          zoom, centerCutX, centerCutY, centerCut,
          getWidth(), getHeight(), getMaxZoom());
  }

  return contained;
}
*/

bool ZDvidDataSliceHelper::actualContainedIn(
    const ZStackViewParam &viewParam, int zoom, int centerCutX, int centerCutY,
    bool centerCut) const
{
  bool contained = false;

  if (m_currentViewParam.getViewPort().isEmpty() &&
      !viewParam.getViewPort().isEmpty()) {
    if (m_currentViewParam.getSliceAxis() == neutu::EAxis::ARB) {
      //Must be on the same plane to be contained
      if (m_currentViewParam.getSliceViewParam().hasSamePlaneCenter(
            viewParam.getSliceViewParam())) {
        contained = true;
      }
    } else {
      if (viewParam.getZ() == m_currentViewParam.getZ()) {
        contained = true;
      }
    }
  } else if (viewParam.contains(m_currentViewParam)) {
    contained = ZDvidDataSliceHelper::IsResIncreasing(
          m_actualZoom, m_actualCenterCutWidth, m_actualCenterCutHeight,
          m_actualUsingCenterCut,
          zoom, centerCutX, centerCutY, centerCut,
          getWidth(), getHeight(), getMaxZoom());
  }

  return contained;
}

bool ZDvidDataSliceHelper::needHighResUpdate() const
{
  if (getViewDataSize() == 0) {
    return true;
  }

  return IsResIncreasing(
        m_actualZoom, m_actualCenterCutWidth, m_actualCenterCutHeight,
        m_actualUsingCenterCut,
        m_zoom, m_centerCutWidth, m_centerCutHeight, m_usingCenterCut,
        getWidth(), getHeight(), getMaxZoom());
}

void ZDvidDataSliceHelper::invalidateViewParam()
{
  m_currentViewParam.invalidate();
}

ZIntCuboid ZDvidDataSliceHelper::GetBoundBox(const QRect &viewPort, int z)
{
  ZIntCuboid box;
  box.setMinCorner(viewPort.left(), viewPort.top(), z);
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

void ZDvidDataSliceHelper::setActualQuality(
    int zoom, int ccw, int cch, bool centerCut)
{
  m_actualZoom = zoom;
  m_actualCenterCutWidth = ccw;
  m_actualCenterCutHeight = cch;
  m_actualUsingCenterCut = centerCut;
}

void ZDvidDataSliceHelper::syncActualQuality()
{
  setActualQuality(getZoom(), m_centerCutWidth, m_centerCutHeight, m_usingCenterCut);
}

void ZDvidDataSliceHelper::setPreferredUpdatePolicy(
    neutu::EDataSliceUpdatePolicy policy)
{
  m_preferredUpdatePolicy = policy;
}

neutu::EDataSliceUpdatePolicy ZDvidDataSliceHelper::getPreferredUpdatePolicy() const
{
  return m_preferredUpdatePolicy;
}

void ZDvidDataSliceHelper::inferUpdatePolicy(neutu::EAxis axis)
{
  if (getMaxZoom() == 0) {
    if (axis == neutu::EAxis::ARB) {
      setUpdatePolicy(neutu::EDataSliceUpdatePolicy::HIDDEN);
    } else {
      setUpdatePolicy(neutu::EDataSliceUpdatePolicy::SMALL);
    }
  } else {
    if (axis == neutu::EAxis::ARB) {
      setUpdatePolicy(getPreferredUpdatePolicy());
    } else {
      setUpdatePolicy(neutu::EDataSliceUpdatePolicy::LOWRES);
    }
  }
}
