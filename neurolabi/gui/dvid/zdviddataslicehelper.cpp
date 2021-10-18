#include "zdviddataslicehelper.h"

#include <QPainter>

#include "neulib/math/utilities.h"
#include "common/math.h"
#include "common/debug.h"
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

const ZStackViewParam& ZDvidDataSliceHelper::getViewParamLastUpdate(int viewId) const
{
  return getViewParamBuffer(viewId).m_viewParam;
}

ZStackViewParam ZDvidDataSliceHelper::getViewParamActive(int viewId) const
{
  std::lock_guard<std::mutex> guard(m_activeViewParamMutex);
  if (m_activeViewParam.count(viewId) > 0) {
    return m_activeViewParam.at(viewId);
  }

  return ZStackViewParam();
}

bool ZDvidDataSliceHelper::isViewParamBuffered(int viewId) const
{
  return m_lastUpdateParam.count(viewId) > 0;
}

const ZDvidDataSliceHelper::ViewParamBuffer&
ZDvidDataSliceHelper::getViewParamBuffer(int viewId) const
{
  if (m_lastUpdateParam.count(viewId) > 0) {
    return m_lastUpdateParam.at(viewId);
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
  return neulib::iround(getViewParamLastUpdate(viewId).getCutDepth(ZPoint(0, 0, 0)));
}

int ZDvidDataSliceHelper::getZ() const
{
  if (!m_lastUpdateParam.empty()) {
    ZPoint center =
        (*m_lastUpdateParam.begin()).second.m_viewParam.getCutCenter();
    return neulib::iround(center.getZ());
  }

  return 0;
}

size_t ZDvidDataSliceHelper::getViewPortArea(int viewId, EViewParamOption option) const
{
  return size_t(getWidth(viewId, option)) * size_t(getHeight(viewId, option));
}

size_t ZDvidDataSliceHelper::getViewDataSize(
    int viewId, EViewParamOption option) const
{
  int scale = getScale(viewId, option);
  return getViewPortArea(viewId, option) / scale / scale;
}

double ZDvidDataSliceHelper::getPixelScale(int viewId, EViewParamOption option) const
{
  return getViewParam(viewId, option).getSliceViewTransform().getScale();
}

int ZDvidDataSliceHelper::getWidth(int viewId, EViewParamOption option) const
{
  return getViewParam(viewId, option).getIntWidth(neutu::data3d::ESpace::MODEL);
}

int ZDvidDataSliceHelper::getHeight(int viewId, EViewParamOption option) const
{
  return getViewParam(viewId, option).getIntHeight(neutu::data3d::ESpace::MODEL);
}

ZStackViewParam ZDvidDataSliceHelper::getViewParam(
    int viewId, EViewParamOption option) const
{
  switch (option) {
  case EViewParamOption::LAST_UPDATE:
    return getViewParamLastUpdate(viewId);
  case EViewParamOption::ACTIVE:
    return getViewParamActive(viewId);
  }

  return m_emptyViewParamBuffer.m_viewParam;
}

size_t ZDvidDataSliceHelper::GetViewDataSize(
    const ZStackViewParam &viewParam, int zoom)
{
  int scale = zgeom::GetZoomScale(zoom);

  return viewParam.getArea(neutu::data3d::ESpace::MODEL) / scale / scale;
}

neutu::EAxis ZDvidDataSliceHelper::getSliceAxis(
    int viewId, EViewParamOption option) const
{
  return getViewParam(viewId, option).getSliceAxis();
}

void ZDvidDataSliceHelper::forEachViewParam(
    std::function<void (const ZStackViewParam &)> f)
{
  for (const auto &param : m_lastUpdateParam) {
    f(param.second.m_viewParam);
  }
}

void ZDvidDataSliceHelper::closeViewPort(int viewId, EViewParamOption option)
{
  switch (option) {
  case EViewParamOption::LAST_UPDATE:
    if (m_lastUpdateParam.count(viewId) > 0) {
      m_lastUpdateParam[viewId].m_viewParam.closeViewPort();
    }
    break;
  case EViewParamOption::ACTIVE:
    if (m_activeViewParam.count(viewId) > 0) {
      m_activeViewParam[viewId].closeViewPort();
    }
    break;
  }
}

void ZDvidDataSliceHelper::openViewPort(int viewId, EViewParamOption option)
{
  switch (option) {
  case EViewParamOption::LAST_UPDATE:
    if (m_lastUpdateParam.count(viewId) > 0) {
      m_lastUpdateParam[viewId].m_viewParam.openViewPort();
    }
    break;
  case EViewParamOption::ACTIVE:
    if (m_activeViewParam.count(viewId) > 0) {
      m_activeViewParam[viewId].openViewPort();
    }
    break;
  }
  /*
  if (m_lastUpdateParam.count(viewId) > 0) {
    getViewParamLastUpdate(viewId).openViewPort();
  }
  */
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

int ZDvidDataSliceHelper::getScale(int viewId, EViewParamOption option) const
{
  return zgeom::GetZoomScale(getZoom(viewId, option));
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

int ZDvidDataSliceHelper::getZoom(int viewId, EViewParamOption option) const
{
  int zoom = getViewParam(viewId, option).getZoomLevel(getMaxZoom());
  if (zoom < 0) {
    zoom = 0;
  }
  return zoom;
}

/*
void ZDvidDataSliceHelper::setZoom(int zoom)
{
  m_zoom = std::max(0, std::min(zoom, getMaxZoom()));
}
*/

int ZDvidDataSliceHelper::getLowresZoom(int viewId, EViewParamOption option) const
{
  int zoom = getZoom(viewId, option) + 1;
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

void ZDvidDataSliceHelper::setViewParamLastUpdate(const ZStackViewParam &viewParam)
{
  m_lastUpdateParam[viewParam.getViewId()].m_viewParam = viewParam;
}

void ZDvidDataSliceHelper::setViewParamActive(const ZStackViewParam &viewParam)
{
  std::lock_guard<std::mutex> guard(m_activeViewParamMutex);
  m_activeViewParam[viewParam.getViewId()] = viewParam;
#ifdef _DEBUG_0
  std::cout << OUTPUT_HIGHTLIGHT_1 << " " << __FUNCTION__ << ": "
            << viewParam.getViewId() << " " << viewParam.getCutRect()
            << std::endl;
#endif
}

void ZDvidDataSliceHelper::setViewParamActive(
    QPainter *painter, const neutu::data3d::DisplayConfig &config)
{
  std::lock_guard<std::mutex> guard(m_activeViewParamMutex);
  ZStackViewParam viewParam(
        {config.getWorldViewTransform(), config.getViewCanvasTransform()},
        painter->device()->width(), painter->device()->height(),
        neutu::data3d::ESpace::CANVAS);
  viewParam.setViewId(config.getViewId());
  m_activeViewParam[viewParam.getViewId()] = viewParam;
#ifdef _DEBUG_0
  std::cout << OUTPUT_HIGHTLIGHT_1 << " " << __FUNCTION__ << ": "
            << viewParam.getViewId() << " " << viewParam.getCutRect()
            << std::endl;
#endif
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
  if (m_lastUpdateParam.count(viewId) > 0) {
    const auto &currentViewParam =
        m_lastUpdateParam.at(viewId);
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
  if (m_lastUpdateParam.count(viewId) > 0) {
    return getViewParamLastUpdate(viewId).contains(x, y, z);
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

bool ZDvidDataSliceHelper::actualContainedIn(
    const ZStackViewParam &viewParam, int zoom, int centerCutX, int centerCutY,
    bool centerCut) const
{
  if (m_lastUpdateParam.count(viewParam.getViewId()) == 0) {
    return true;
  }

  bool contained = false;

  const ViewParamBuffer &currentViewParamBuffer = m_lastUpdateParam.at(
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

#ifdef _DEBUG_0
  std::cout << OUTPUT_HIGHTLIGHT_1 << __FUNCTION__ << std::endl;
  std::cout << OUTPUT_HIGHTLIGHT_1 << rect1 << std::endl;
  std::cout << OUTPUT_HIGHTLIGHT_1 << rect2 << std::endl;
#endif

  if (rect2 == rect1) {
    contained = ZDvidDataSliceHelper::IsResIncreasing(
          currentViewParamBuffer.m_actualZoom,
          currentViewParamBuffer.m_actualCenterCutWidth,
          currentViewParamBuffer.m_actualCenterCutHeight,
          currentViewParamBuffer.m_actualUsingCenterCut,
          zoom, centerCutX, centerCutY, centerCut,
          getWidth(viewParam.getViewId(), EViewParamOption::LAST_UPDATE),
          getHeight(viewParam.getViewId(), EViewParamOption::LAST_UPDATE),
          getMaxZoom());
  } else if (rect2.contains(rect1) &&
             rect1.getAffinePlane().hasSamePlane(rect2.getAffinePlane())) {
    contained = !ZDvidDataSliceHelper::IsResIncreasing(
          zoom, centerCutX, centerCutY, centerCut,
          getWidth(viewParam.getViewId(), EViewParamOption::LAST_UPDATE),
          getHeight(viewParam.getViewId(), EViewParamOption::LAST_UPDATE),
          currentViewParamBuffer.m_actualZoom,
          currentViewParamBuffer.m_actualCenterCutWidth,
          currentViewParamBuffer.m_actualCenterCutHeight,
          currentViewParamBuffer.m_actualUsingCenterCut, getMaxZoom());
  }

  if (!contained) { // check active view
    ZStackViewParam activeViewParam = getViewParamActive(viewParam.getViewId());
    if (activeViewParam.isValid()) {
#ifdef _DEBUG_0
      std::cout << OUTPUT_HIGHTLIGHT_1 << __FUNCTION__ << " Active view check" << std::endl;
      std::cout << OUTPUT_HIGHTLIGHT_1 << viewParam.getCutRect() << std::endl;
      std::cout << OUTPUT_HIGHTLIGHT_1 << activeViewParam.getCutRect() << std::endl;
#endif
      ZAffineRect rect = viewParam.getCutRect();
      if (rect1 == rect2) {
        contained = true;
      } else if (viewParam.getSliceViewTransform().getIntCutPlane().hasSamePlane(
                   activeViewParam.getSliceViewTransform().getIntCutPlane())) {
        rect.addSize(1, 1);
        contained = rect.contains(activeViewParam.getCutRect());
      }
    }
  }

#ifdef _DEBUG_0
  std::cout << OUTPUT_HIGHTLIGHT_1 <<  "Contained " << contained << std::endl;
#endif

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
    const ZAffinePlane &ap, int width, int height, int zoom, int viewId,
    EViewParamOption option) const
{
  ZSliceViewTransform t;

  if (getSliceAxis(viewId, option) == neutu::EAxis::ARB) {
    t.setCutPlane(ap);
  } else {
    t.setCutPlane(getSliceAxis(viewId, option), ap.getOffset());
  }

  t.setScale(1.0 / zgeom::GetZoomScale(zoom));
  //Assuming lowtis uses left integer center
  t.setAnchor(width / 2, height / 2);

  return t;
}

ZSliceViewTransform ZDvidDataSliceHelper::getCanvasTransform(
    neutu::EAxis axis, const ZAffinePlane &ap,
    int width, int height, int zoom) const
{
  ZSliceViewTransform t;

  if (axis == neutu::EAxis::ARB) {
    t.setCutPlane(ap);
  } else {
    t.setCutPlane(axis, ap.getOffset());
  }

  t.setScale(1.0 / zgeom::GetZoomScale(zoom));
  //Assuming lowtis uses left integer center
  t.setAnchor(width / 2, height / 2);

  return t;
}

ZAffineRect ZDvidDataSliceHelper::getIntCutRect(
    int viewId, EViewParamOption option) const
{
  ZAffineRect rect = getViewParam(viewId, option).getIntCutRect(
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

/*
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
*/

int ZDvidDataSliceHelper::getHighresZoom(int viewId, EViewParamOption option) const
{
  int targetZoom = getZoom(viewId, option);
  if (targetZoom > 0 && getPixelScale(viewId, option) < 0.9) {
    targetZoom -= 1;
  }

  return targetZoom;
}

bool ZDvidDataSliceHelper::highResUpdateNeeded(int viewId) const
{
  if (getViewDataSize(viewId, EViewParamOption::ACTIVE) == 0) {
    return true;
  }

  auto vpb = getViewParamBuffer(viewId);
  ZAffineRect currentViewport =  getViewParamActive(viewId).getCutRect();
  ZAffineRect bufferedViewport = vpb.m_viewParam.getCutRect();
  bufferedViewport.addSize(1, 1);
  if (!bufferedViewport.contains(currentViewport)) {
#ifdef _DEBUG_0
    std::cout << OUTPUT_HIGHTLIGHT_1 << __FUNCTION__
              << " Fetch highres because of viewport update" << std::endl;
#endif
    return true;
  }

  return IsResIncreasing(
        vpb.m_actualZoom, vpb.m_actualCenterCutWidth, vpb.m_actualCenterCutHeight,
        vpb.m_actualUsingCenterCut,
        getHighresZoom(viewId, EViewParamOption::LAST_UPDATE),
        m_centerCutWidth, m_centerCutHeight, false,
        getWidth(viewId, EViewParamOption::LAST_UPDATE),
        getHeight(viewId, EViewParamOption::LAST_UPDATE), getMaxZoom());
}

#if 0
bool ZDvidDataSliceHelper::updateNeeded(
    const ZStackViewParam &viewParam, int zoom,
    int centerCutX, int centerCutY, bool usingCenterCut) const
{
  int viewId = viewParam.getViewId();
  if (getViewDataSize(viewId) == 0) {
    return true;
  }

  auto vpb = getViewParamBuffer(viewId);
  ZAffineRect currentViewport =  getViewParamActive(viewId).getCutRect();
  ZAffineRect bufferedViewport = vpb.m_viewParam.getCutRect();
  bufferedViewport.addSize(1, 1);
  if (!bufferedViewport.contains(currentViewport)) {
#ifdef _DEBUG_0
    std::cout << OUTPUT_HIGHTLIGHT_1 << __FUNCTION__
              << " Fetch highres because of viewport update" << std::endl;
#endif
    return true;
  }

  return IsResIncreasing(
        vpb.m_actualZoom, vpb.m_actualCenterCutWidth, vpb.m_actualCenterCutHeight,
        vpb.m_actualUsingCenterCut,
        zoom, centerCutX, centerCutY, usingCenterCut,
        getWidth(viewId), getHeight(viewId), getMaxZoom());
}
#endif

void ZDvidDataSliceHelper::invalidateViewParam(int viewId)
{
  if (m_lastUpdateParam.count(viewId) > 0) {
    m_lastUpdateParam[viewId].m_viewParam.invalidate();
  }
//  m_currentViewParam.invalidate();
}

void ZDvidDataSliceHelper::invalidateAllViewParam()
{
  for (auto &p : m_lastUpdateParam) {
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
  if (m_lastUpdateParam.count(viewId) > 0) {
    return zgeom::GetIntBoundBox(
          m_lastUpdateParam.at(viewId).m_viewParam.getCutRect());
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
  m_lastUpdateParam[viewId].m_actualZoom = zoom;
  m_lastUpdateParam[viewId].m_actualCenterCutWidth = ccw;
  m_lastUpdateParam[viewId].m_actualCenterCutHeight = cch;
  m_lastUpdateParam[viewId].m_actualUsingCenterCut = centerCut;
}

/*
void ZDvidDataSliceHelper::syncActualQuality(int viewId)
{
  setActualQuality(
        getZoom(), m_centerCutWidth, m_centerCutHeight, m_usingCenterCut, viewId);
}
*/

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
