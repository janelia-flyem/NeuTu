#include "zdviddataslicehelper.h"

#include "neulib/math/utilities.h"
#include "common/math.h"
#include "zrect2d.h"
#include "geometry/zintcuboid.h"
#include "misc/miscutility.h"
#include "zarbsliceviewparam.h"
#include "zdvidglobal.h"

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
  switch (m_dataRole) {
  case ZDvidData::ERole::GRAYSCALE:
  case ZDvidData::ERole::MULTISCALE_2D:
    m_dvidInfo = ZDvidGlobal::Memo::ReadGrayscaleInfo(m_reader.getDvidTarget());
    break;
  case ZDvidData::ERole::SEGMENTATION:
  case ZDvidData::ERole::SPARSEVOL:
    m_dvidInfo = ZDvidGlobal::Memo::ReadSegmentationInfo(
          m_reader.getDvidTarget());
  default:
    break;
  }
  m_workReader.openRaw(m_reader.getDvidTarget());

  updateCenterCut();
}

const ZStackViewParam& ZDvidDataSliceHelper::getViewParam(int viewId) const
{
  return getViewParamBuffer(viewId).m_viewParam;
}

bool ZDvidDataSliceHelper::isViewParamBuffered(int viewId) const
{
  return m_currentViewParamMap.count(viewId) > 0;
}

const ZDvidDataSliceHelper::ViewParamBuffer&
ZDvidDataSliceHelper::getViewParamBuffer(int viewId) const
{
  if (m_currentViewParamMap.count(viewId) > 0) {
    return m_currentViewParamMap.at(viewId);
  }

  return m_emptyViewParamBuffer;
}

ZDvidDataSliceHelper::ViewParamBuffer&
ZDvidDataSliceHelper::getViewParamBuffer(int viewId)
{
  return const_cast<ZDvidDataSliceHelper::ViewParamBuffer&>(
        static_cast<const ZDvidDataSliceHelper&>(*this).getViewParamBuffer(viewId));
}

void ZDvidDataSliceHelper::setMaxZoom(int maxZoom)
{
  m_maxZoom = maxZoom;
}

void ZDvidDataSliceHelper::updateMaxZoom()
{
  switch (m_dataRole) {
  case ZDvidData::ERole::GRAYSCALE:
    m_maxZoom = ZDvidGlobal::Memo::ReadMaxGrayscaleZoom(m_reader.getDvidTarget());
    m_reader.updateMaxGrayscaleZoom(m_maxZoom);
    m_workReader.updateMaxGrayscaleZoom(m_maxZoom);
//    m_reader.updateMaxGrayscaleZoom(
//          ZDvidGlobal::Memo::ReadMaxGrayscaleZoom(m_reader.getDvidTarget()));
//    m_reader.updateMaxGrayscaleZoom();
    break;
  case ZDvidData::ERole::SEGMENTATION:
  case ZDvidData::ERole::SPARSEVOL:
    m_maxZoom = ZDvidGlobal::Memo::ReadMaxLabelZoom(m_reader.getDvidTarget());
    m_reader.updateMaxLabelZoom(m_maxZoom);
    m_workReader.updateMaxLabelZoom(m_maxZoom);
//    m_reader.updateMaxLabelZoom(
//          ZDvidGlobal::Memo::ReadMaxLabelZoom(m_reader.getDvidTarget()));
//    m_reader.updateMaxLabelZoom();
    break;
  default:
    break;
  }
}

ZIntCuboid ZDvidDataSliceHelper::getDataRange() const
{
  return m_dvidInfo.getDataRange();
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

int ZDvidDataSliceHelper::updateParam(ZStackViewParam *param) const
{
  int width = param->getIntWidth(neutu::data3d::ESpace::MODEL);
  int height = param->getIntHeight(neutu::data3d::ESpace::MODEL);
  if (!param->getCutCenter().hasIntCoord()) {
    ++width;
    ++height;
    param->setCutCenter(param->getCutCenter().roundToIntPoint());
  }

  int maxZoomLevel = getMaxZoom();
  if (maxZoomLevel < 3) {
    validateSize(&width, &height);
  }

  param->setSize(width, height, neutu::data3d::ESpace::MODEL);

  return maxZoomLevel;
}

int ZDvidDataSliceHelper::getZ(int viewId) const
{
  return neulib::iround(getViewParam(viewId).getCutDepth(ZPoint(0, 0, 0)));
}

int ZDvidDataSliceHelper::getZ() const
{
  if (!m_currentViewParamMap.empty()) {
    ZPoint center =
        (*m_currentViewParamMap.begin()).second.m_viewParam.getCutCenter();
    return neulib::iround(center.getZ());
  }

  return 0;
}

size_t ZDvidDataSliceHelper::getViewPortArea(int viewId) const
{
  return size_t(getWidth(viewId)) * size_t(getHeight(viewId));
}

size_t ZDvidDataSliceHelper::getViewDataSize(int viewId) const
{
  int scale = getScale();
  return getViewPortArea(viewId) / scale / scale;
}

double ZDvidDataSliceHelper::getPixelScale(int viewId) const
{
  return getViewParam(viewId).getSliceViewTransform().getScale();
}

int ZDvidDataSliceHelper::getWidth(int viewId) const
{
  return getViewParam(viewId).getIntWidth(neutu::data3d::ESpace::MODEL);
}

int ZDvidDataSliceHelper::getHeight(int viewId) const
{
  return getViewParam(viewId).getIntHeight(neutu::data3d::ESpace::MODEL);
}

size_t ZDvidDataSliceHelper::GetViewDataSize(
    const ZStackViewParam &viewParam, int zoom)
{
  int scale = zgeom::GetZoomScale(zoom);

  return viewParam.getArea(neutu::data3d::ESpace::MODEL) / scale / scale;
}

neutu::EAxis ZDvidDataSliceHelper::getSliceAxis(int viewId) const
{
  return getViewParam(viewId).getSliceAxis();
}

void ZDvidDataSliceHelper::forEachViewParam(
    std::function<void (const ZStackViewParam &)> f)
{
  for (const auto &param : m_currentViewParamMap) {
    f(param.second.m_viewParam);
  }
}

void ZDvidDataSliceHelper::closeViewPort(int viewId)
{
  if (m_currentViewParamMap.count(viewId) > 0) {
    getViewParam(viewId).closeViewPort();
  }
}

void ZDvidDataSliceHelper::openViewPort(int viewId)
{
  if (m_currentViewParamMap.count(viewId) > 0) {
    getViewParam(viewId).openViewPort();
  }
//  m_currentViewParam.openViewPort();
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

int ZDvidDataSliceHelper::getActualScale(int viewId) const
{
  return zgeom::GetZoomScale(getActualZoom(viewId));
}

int ZDvidDataSliceHelper::getActualZoom(int viewId) const
{
  return getViewParamBuffer(viewId).m_actualZoom;
//  return m_actualZoom;
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
  m_currentViewParamMap[viewParam.getViewId()].m_viewParam = viewParam;
}

ZStackViewParam ZDvidDataSliceHelper::getValidViewParam(
    const ZStackViewParam &viewParam) const
{
  ZStackViewParam newViewParam = viewParam;

  updateParam(&newViewParam);

  return newViewParam;
}

bool ZDvidDataSliceHelper::hasNewView(const ZStackViewParam &viewParam) const
{
  return hasNewView(viewParam, getDataRange());
  /*
  int maxZoomLevel = getMaxZoom();

  return !m_currentViewParam.contains(viewParam) ||
      (viewParam.getZoomLevel(maxZoomLevel) <
      m_currentViewParam.getZoomLevel(maxZoomLevel));
      */
}

bool ZDvidDataSliceHelper::hasNewView(
    const ZStackViewParam &viewParam, const ZIntCuboid &modelRange) const
{
  int viewId = viewParam.getViewId();
  if (m_currentViewParamMap.count(viewId) > 0) {
    const auto &currentViewParam =
        m_currentViewParamMap.at(viewId);
    ZAffineRect currentCutRect =
        currentViewParam.m_viewParam.getIntCutRect(
          modelRange, m_centerCutWidth, m_centerCutHeight, m_usingCenterCut);
    ZAffineRect newCutRect = viewParam.getIntCutRect(
          modelRange, m_centerCutWidth, m_centerCutHeight, m_usingCenterCut);
    if (currentCutRect.contains(newCutRect)) {
      int maxZoomLevel = getMaxZoom();
      return viewParam.getZoomLevel(maxZoomLevel) <
          currentViewParam.m_viewParam.getZoomLevel(maxZoomLevel);
    }
  }

  return true;
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

void ZDvidDataSliceHelper::CanonizeQuality(
    int *zoom, int *centerCutX, int *centerCutY,
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

bool ZDvidDataSliceHelper::hit(double x, double y, double z, int viewId) const
{
  if (m_currentViewParamMap.count(viewId) > 0) {
    return getViewParam(viewId).contains(x, y, z);
  }

  return false;
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
  if (m_currentViewParamMap.count(viewParam.getViewId()) == 0) {
    return true;
  }

  bool contained = false;

  const ViewParamBuffer &currentViewParamBuffer = m_currentViewParamMap.at(
        viewParam.getViewId());

  // Current fov
  ZAffineRect rect1 = currentViewParamBuffer.m_viewParam.
      getIntCutRect(getDataRange(),
                    currentViewParamBuffer.m_actualCenterCutWidth,
                    currentViewParamBuffer.m_actualCenterCutHeight,
                    currentViewParamBuffer.m_actualUsingCenterCut);

  // New fov. Use center cut adjustment if the current fov uses it.
  ZAffineRect rect2 = viewParam.getIntCutRect(
        getDataRange(), centerCutX, centerCutY,
        centerCut || currentViewParamBuffer.m_actualUsingCenterCut);

  if (rect2 == rect1) {
    contained = ZDvidDataSliceHelper::IsResIncreasing(
          currentViewParamBuffer.m_actualZoom,
          currentViewParamBuffer.m_actualCenterCutWidth,
          currentViewParamBuffer.m_actualCenterCutHeight,
          currentViewParamBuffer.m_actualUsingCenterCut,
          zoom, centerCutX, centerCutY, centerCut,
          getWidth(viewParam.getViewId()), getHeight(viewParam.getViewId()),
          getMaxZoom());
  } else if (rect2.contains(rect1)) {
    contained = !ZDvidDataSliceHelper::IsResIncreasing(
          zoom, centerCutX, centerCutY, centerCut,
          getWidth(viewParam.getViewId()), getHeight(viewParam.getViewId()),
          currentViewParamBuffer.m_actualZoom,
          currentViewParamBuffer.m_actualCenterCutWidth,
          currentViewParamBuffer.m_actualCenterCutHeight,
          currentViewParamBuffer.m_actualUsingCenterCut, getMaxZoom());
  }

  return contained;
}

/*
ZSliceViewTransform ZDvidDataSliceHelper::getCanvasTransform(
    const ZAffinePlane &ap, int width, int height) const
{
  return getCanvasTransform(ap, width, height, getZoom());
}
*/

ZSliceViewTransform ZDvidDataSliceHelper::getCanvasTransform(
    const ZAffinePlane &ap, int width, int height, int zoom, int viewId) const
{
  ZSliceViewTransform t;

  if (getSliceAxis(viewId) == neutu::EAxis::ARB) {
    t.setCutPlane(ap);
  } else {
    t.setCutPlane(getSliceAxis(viewId), ap.getOffset());
  }

  t.setScale(1.0 / zgeom::GetZoomScale(zoom));
  //Assuming lowtis uses left integer center
  t.setAnchor(width / 2, height / 2);

  return t;
}

ZAffineRect ZDvidDataSliceHelper::getIntCutRect(int viewId) const
{
  ZAffineRect rect = getViewParam(viewId).getIntCutRect(
        getDataRange(), m_centerCutWidth, m_centerCutHeight, m_usingCenterCut);
  if (m_maxZoom < 3) {
    int width = rect.getWidth();
    int height = rect.getHeight();
    if (validateSize(&width, &height)) {
      rect.setSize(width, height);
    }
  }

  return rect;
}

bool ZDvidDataSliceHelper::isResolutionReached(int viewId) const
{
  if (getViewDataSize(viewId) == 0) {
    return false;
  }

  auto vpb = getViewParamBuffer(viewId);

  return !IsResIncreasing(
        vpb.m_actualZoom, vpb.m_actualCenterCutWidth, vpb.m_actualCenterCutHeight,
        vpb.m_actualUsingCenterCut,
        m_zoom, m_centerCutWidth, m_centerCutHeight, m_usingCenterCut,
        getWidth(viewId), getHeight(viewId), getMaxZoom());
}

int ZDvidDataSliceHelper::getHighresZoom(int viewId) const
{
  int targetZoom = getZoom();
  if (targetZoom > 0 && getPixelScale(viewId) < 0.9) {
    targetZoom -= 1;
  }

  return targetZoom;
}

bool ZDvidDataSliceHelper::needHighResUpdate(int viewId) const
{
  if (getViewDataSize(viewId) == 0) {
    return true;
  }

  auto vpb = getViewParamBuffer(viewId);
  return IsResIncreasing(
        vpb.m_actualZoom, vpb.m_actualCenterCutWidth, vpb.m_actualCenterCutHeight,
        vpb.m_actualUsingCenterCut,
        getHighresZoom(viewId), m_centerCutWidth, m_centerCutHeight, false,
        getWidth(viewId), getHeight(viewId), getMaxZoom());
}

void ZDvidDataSliceHelper::invalidateViewParam(int viewId)
{
  if (m_currentViewParamMap.count(viewId) > 0) {
    m_currentViewParamMap[viewId].m_viewParam.invalidate();
  }
//  m_currentViewParam.invalidate();
}

void ZDvidDataSliceHelper::invalidateAllViewParam()
{
  for (auto &p : m_currentViewParamMap) {
    p.second.m_viewParam.invalidate();
  }
}

ZIntCuboid ZDvidDataSliceHelper::GetBoundBox(const QRect &viewPort, int z)
{
  ZIntCuboid box;
  box.setMinCorner(viewPort.left(), viewPort.top(), z);
  box.setSize(viewPort.width(), viewPort.height(), 1);

  return box;
}

ZIntCuboid ZDvidDataSliceHelper::getBoundBox(int viewId) const
{
  if (m_currentViewParamMap.count(viewId) > 0) {
    return zgeom::GetIntBoundBox(
          m_currentViewParamMap.at(viewId).m_viewParam.getCutRect());
  }

  return ZIntCuboid();
//  return GetBoundBox(getViewPort(), getZ());
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

/*
void ZDvidDataSliceHelper::setBoundBox(const ZRect2d &rect)
{
  m_currentViewParam.setViewPort(
        QRect(rect.getX0(), rect.getY0(), rect.getWidth(), rect.getHeight()));
}
*/

void ZDvidDataSliceHelper::setActualQuality(
    int zoom, int ccw, int cch, bool centerCut, int viewId)
{
  m_currentViewParamMap[viewId].m_actualZoom = zoom;
  m_currentViewParamMap[viewId].m_actualCenterCutWidth = ccw;
  m_currentViewParamMap[viewId].m_actualCenterCutHeight = cch;
  m_currentViewParamMap[viewId].m_actualUsingCenterCut = centerCut;
}

void ZDvidDataSliceHelper::syncActualQuality(int viewId)
{
  setActualQuality(
        getZoom(), m_centerCutWidth, m_centerCutHeight, m_usingCenterCut, viewId);
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
