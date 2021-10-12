#include "zdvidgrayslice.h"

#include <QElapsedTimer>

#include "zdvidurl.h"
#include "zdvidbufferreader.h"
#include "zrect2d.h"
#include "zpainter.h"
#include "zstackviewparam.h"
#include "misc/miscutility.h"
#include "imgproc/zstackprocessor.h"
#include "geometry/zintcuboid.h"
#include "neutubeconfig.h"
#include "zdviddataslicehelper.h"
#include "zutils.h"
#include "zstack.hxx"
#include "flyem/zdvidgrayslicehighrestask.h"
#include "vis2d/utilities.h"
#include "data3d/displayconfig.h"

/* Implementation details
 *
 * ZDvidGraySlice reads a slice from DVID and convert it into the form ready
 * for painting. The conversion has two parts, incliuding m_imageCanvas for
 * final rendering and m_image for host original grayscale values. These two are
 * supposed to be synced all the time, i.e. whenever m_image is updated,
 * m_imageCanvas should be updated too.
 */

ZDvidGraySlice::ZDvidGraySlice()
{
  setTarget(neutu::data3d::ETarget::TILE_CANVAS);
  m_type = GetType();

  m_helper = std::make_unique<ZDvidDataSliceHelper>(ZDvidData::ERole::GRAYSCALE);
//  getHelper()->useCenterCut(false);
}

ZDvidGraySlice::~ZDvidGraySlice()
{
  clear();
}

void ZDvidGraySlice::clear()
{ 
  m_helper->clear();
}

std::shared_ptr<ZDvidGraySlice::DisplayBuffer>
ZDvidGraySlice::getDisplayBuffer(int viewId) const
{
  std::lock_guard<std::mutex> guard(m_displayBufferMutex);

  if (m_displayBufferMap.count(viewId) == 0) {
    m_displayBufferMap[viewId] = std::shared_ptr<DisplayBuffer>(new DisplayBuffer);
  }

  return m_displayBufferMap.at(viewId);
}

const ZDvidReader& ZDvidGraySlice::getDvidReader() const
{
  return m_helper->getDvidReader();
}

const ZDvidReader& ZDvidGraySlice::getWorkDvidReader() const
{
  return m_helper->getWorkDvidReader();
}

const ZDvidTarget& ZDvidGraySlice::getDvidTarget() const
{
  return getDvidReader().getDvidTarget();
}

/*
int ZDvidGraySlice::getZ() const
{
  return m_helper->getZ();
}
*/

/*
int ZDvidGraySlice::getWidth(int viewId) const
{
  return m_helper->getWidth(viewId);
}

int ZDvidGraySlice::getHeight(int viewId) const
{
  return m_helper->getHeight(viewId);
}
*/

/*
int ZDvidGraySlice::getZoom() const
{
  return m_helper->getZoom();
}
*/

ZSliceCanvas& ZDvidGraySlice::getImageCanvas(int viewId) const
{
  return getDisplayBuffer(viewId)->m_imageCanvas;
}

ZImage& ZDvidGraySlice::getImage(int viewId) const
{
  return getDisplayBuffer(viewId)->m_image;
}

bool ZDvidGraySlice::display_inner(
    QPainter *painter, const DisplayConfig &config) const
{
#ifdef _DEBUG_2
    m_imageCanvas.save(GET_TEST_DATA_DIR + "/_test.png");
#endif
  const_cast<ZDvidDataSliceHelper*>(getHelper())->setViewParamActive(
        painter, config);
  return getImageCanvas(config.getViewId()).paintTo(painter, config.getTransform());
}

void ZDvidGraySlice::updateContrast(bool highContrast)
{
  m_usingContrastProtocol = highContrast;
  updateContrast();
}

void ZDvidGraySlice::updateContrast(const ZJsonObject &obj)
{
  m_contrastProtocol.load(obj);
  updateContrast();
}

void ZDvidGraySlice::updateContrast()
{
  std::lock_guard<std::mutex> guard(m_displayBufferMutex);
  for (auto &buffer : m_displayBufferMap) {
    auto &image = buffer.second->m_image;
    image.setContrastProtocol(m_contrastProtocol);
    image.updateContrast(m_usingContrastProtocol);
    buffer.second->m_imageCanvas.fromImage(image);
  }
//  m_image.setContrastProtocol(m_contrastProtocol);
//  m_image.updateContrast(m_usingContrastProtocol);
//  m_imageCanvas.fromImage(m_image);
}

bool ZDvidGraySlice::hasLowresRegion(int viewId) const
{
  if (getHelper()->getZoom(viewId, ZDvidDataSliceHelper::EViewParamOption::ACTIVE) > 0) {
    return true;
  }

  if (!getHelper()->usingCenterCut()) {
    return false;
  }

  ZStackViewParam param = getHelper()->getViewParam(
        viewId, ZDvidDataSliceHelper::EViewParamOption::ACTIVE);

  ZAffineRect rect = param.getIntCutRect(
        getHelper()->getDataRange(), getHelper()->getCenterCutWidth(),
        getHelper()->getCenterCutHeight(), getHelper()->usingCenterCut());

  return (rect.getWidth() > getHelper()->getCenterCutWidth() ||
      rect.getHeight() > getHelper()->getCenterCutHeight());
}

void ZDvidGraySlice::updateImage(
    const ZStack *stack, const ZStackViewParam &viewParam,
    int zoom, int centerCutX, int centerCutY, bool usingCenterCut)
{
  if (stack != NULL) {
    auto buffer = getDisplayBuffer(viewParam.getViewId());
    buffer->m_image =
        neutu::vis2d::GetSlice(*stack, stack->getOffset().getZ());
    ZAffineRect rect = viewParam.getIntCutRect(
          getHelper()->getDataRange(), centerCutX, centerCutY, usingCenterCut);

    ZSliceViewTransform t = getHelper()->getCanvasTransform(
          viewParam.getSliceAxis(), rect.getAffinePlane(),
          stack->width(), stack->height(), zoom);
    buffer->m_imageCanvas.setTransform(t);
    buffer->m_imageCanvas.setOriginalCut(viewParam.getCutRect());
    updateContrast();
  }
}

#if 0
void ZDvidGraySlice::updateImage(
    const ZStack *stack, neutu::EAxis axis,
    const ZAffinePlane &ap, int zoom, int viewId)
{
  if (stack != NULL) {
    auto buffer = getDisplayBuffer(viewId);
    buffer->m_image =
        neutu::vis2d::GetSlice(*stack, stack->getOffset().getZ());

    ZSliceViewTransform t = getHelper()->getCanvasTransform(
          axis, ap, stack->width(), stack->height(), zoom);

    buffer->m_imageCanvas.setTransform(t);
    updateContrast();

#ifdef _DEBUG_2
        std::cout << "Saving image canvas ..." << std::endl;
        m_image.save((GET_TEST_DATA_DIR + "/_test.png").c_str());
#endif
  }

//  std::cout << "Grayscale udpating time: " << timer.elapsed() << std::endl;

#ifdef _DEBUG_2
  m_image.save((GET_TEST_DATA_DIR + "/test.tif").c_str());
#endif
}
#endif

void ZDvidGraySlice::saveImage(const std::string &path, int viewId)
{
  getImage(viewId).save(path.c_str());
}

void ZDvidGraySlice::savePixmap(const std::string &path, int viewId)
{
  getImageCanvas(viewId).save(path.c_str());
}

#if 0
void ZDvidGraySlice::updateImage()
{
  if (getWidth() != m_image.width() ||
      getHeight() != m_image.height()) {
    m_image = ZImage(getWidth(), getHeight());
  }
  m_image.setOffset(-getX(), -getY());

  ZDvidUrl dvidUrl(getDvidTarget());
  ZDvidBufferReader bufferReader;
  bufferReader.read(dvidUrl.getGrayscaleUrl(
                      getWidth(), getHeight(), 1,
                      getX(), getY(), getZ()).c_str());
  const QByteArray &buffer = bufferReader.getBuffer();

  if (!buffer.isEmpty()) {
    if (m_image.width() * m_image.height() == buffer.size()) {
      m_image.setData((uint8_t*) buffer.data()/*, 1.5, 0*/);
    }
  }
  updateContrast();
}
#endif

#if 0
void ZDvidGraySlice::updatePixmap()
{
  if (!isPixmapValid()) {
    m_pixmap.detach();
    m_pixmap.convertFromImage(m_image);
    double scale = 1.0 / getScale();
    m_pixmap.setScale(scale, scale);
//    m_pixmap.setOffset(-getX(), -getY());
    validatePixmap();

#ifdef _DEBUG_2
  std::cout << "gray slice pixmap offset: "
            << m_pixmap.getTransform().getTx() << " "
            << m_pixmap.getTransform().getTy() << std::endl;

  std::cout << "gray slice pixmap scale: "
            << m_pixmap.getTransform().getSx() << " "
            << m_pixmap.getTransform().getSy() << std::endl;
#endif
  }
}
#endif

void ZDvidGraySlice::forEachViewParam(
    std::function<void (const ZStackViewParam &)> f)
{
  getHelper()->forEachViewParam(f);
}

/*
ZStackViewParam ZDvidGraySlice::getViewParam(int viewId) const
{
  return getHelper()->getViewParamLastUpdate(viewId);
}
*/

ZIntCuboid ZDvidGraySlice::getDataRange() const
{
  return getHelper()->getDataRange();
}

void ZDvidGraySlice::trackViewParam(const ZStackViewParam &viewParam)
{
  getHelper()->setViewParamActive(viewParam);
}

/*
QRect ZDvidGraySlice::getViewPort() const
{
  return getHelper()->getViewPort();
}
*/

#if 0
void ZDvidGraySlice::setBoundBox(const ZRect2d &rect)
{
  getHelper()->setBoundBox(rect);
  /*
  m_currentViewParam.setViewPort(
        QRect(rect.getX0(), rect.getY0(), rect.getWidth(), rect.getHeight()));
        */
}
#endif
#if 0
bool ZDvidGraySlice::isRegionConsistent() const
{
  return (getX() == -m_image.getTransform().getTx()) &&
      (getY() == -m_image.getTransform().getTy()) &&
      (getWidth() == m_image.width()) && (getHeight() == m_image.height());
}
#endif

bool ZDvidGraySlice::highResUpdateNeeded(int viewId) const
{
  return getHelper()->highResUpdateNeeded(viewId);
}

void ZDvidGraySlice::processHighResParam(
    int viewId,
    std::function<void(
      const ZStackViewParam &/*viewParam*/, int /*zoom*/,
      int /*centerCutX*/, int /*centerCutY*/,
      bool /*usingCenterCut*/)> f) const
{
  f(getHelper()->getViewParamActive(viewId),
    getHelper()->getHighresZoom(
      viewId, ZDvidDataSliceHelper::EViewParamOption::ACTIVE), 0, 0, false);
}

template<typename T>
int ZDvidGraySlice::updateParam(T *param)
{
  int maxZoomLevel = getDvidTarget().getMaxGrayscaleZoom();
  if (maxZoomLevel < 3) {
    int width = param->getViewPort().width();
    int height = param->getViewPort().height();
    if (getHelper()->validateSize(&width, &height)) {
      param->resize(width, height);
    }
  }

  return maxZoomLevel;
}

bool ZDvidGraySlice::update(const ZStackViewParam &viewParam)
{
  /*
  if (viewParam.getSliceAxis() != getSliceAxis()) {
    return false;
  }
  */

  if (viewParam.isViewportEmpty()) {
    return false;
  }

  bool updated = false;

//  ZStackViewParam newViewParam = getHelper()->getValidViewParam(viewParam);
  if (getHelper()->hasNewView(viewParam)) {
    forceUpdate(viewParam);
    updated = true;
  }

  return updated;
}

/*
bool ZDvidGraySlice::update(const ZArbSliceViewParam &viewParam)
{
  if (m_sliceAxis != neutube::A_AXIS) {
    return false;
  }

  if (!viewParam.isValid()) {
    return false;
  }

  bool updated = false;

  if (!m_currentViewParam.getSliceViewParam().contains(viewParam)) {
    forceUpdate(viewParam);
    updated = true;
  }

  return updated;
}
*/

/*
void ZDvidGraySlice::setZoom(int zoom)
{
  getHelper()->setZoom(zoom);
//  m_zoom = zoom;
}
*/

void ZDvidGraySlice::setContrastProtocol(const ZContrastProtocol &cp)
{
  m_contrastProtocol = cp;
}

int ZDvidGraySlice::getActiveScale(int viewId) const
{
  return getHelper()->getScale(
        viewId, ZDvidDataSliceHelper::EViewParamOption::ACTIVE);
}

void ZDvidGraySlice::setCenterCut(int width, int height)
{
  getHelper()->setCenterCut(width, height);
}

bool ZDvidGraySlice::containedIn(
    const ZStackViewParam &viewParam, int zoom, int centerCutX, int centerCutY,
    bool centerCut) const
{
  return getHelper()->actualContainedIn(
        viewParam, zoom, centerCutX, centerCutY, centerCut);
}


bool ZDvidGraySlice::consume(
    ZStack *stack, const ZStackViewParam &viewParam, int zoom,
    int centerCutX, int centerCutY, bool usingCenterCut)
{
  bool succ = false;
  if (stack != NULL) {
    if (containedIn(viewParam, zoom, centerCutX, centerCutY, usingCenterCut)) {
//      getHelper()->setZoom(zoom);
      getHelper()->setActualQuality(
            zoom, centerCutX, centerCutY, usingCenterCut, viewParam.getViewId());
//      getHelper()->setViewParam(viewParam);
//      getHelper()->setCenterCut(centerCutX, centerCutY);
//      ZAffineRect rect = viewParam.getIntCutRect(
//            getHelper()->getDataRange(), centerCutX, centerCutY, usingCenterCut);
//      updateImage(stack, viewParam.getSliceAxis(), rect.getAffinePlane(),
//                  zoom, viewParam.getViewId());
      updateImage(stack, viewParam, zoom, centerCutX, centerCutY, usingCenterCut);
#ifdef _DEBUG_2
      std::cout << "Saving image canvas ..." << std::endl;
      m_imageCanvas.save((GET_TEST_DATA_DIR + "/_test.png").c_str());
#endif

      succ = true;
    }

    delete stack;
  }
  return succ;
}

ZTask* ZDvidGraySlice::makeFutureTask(ZStackDoc *doc, int viewId)
{
  ZDvidGraySliceHighresTask *task = nullptr;
  const int maxSize = 1024*1024;
  if (getHelper()->highResUpdateNeeded(viewId)
      && getHelper()->getViewDataSize(
        viewId, ZDvidDataSliceHelper::EViewParamOption::ACTIVE) < maxSize) {
    task = new ZDvidGraySliceHighresTask;
    task->setViewId(viewId);
//    task->setViewParam(getHelper()->getViewParamActive(viewId));
//    task->setZoom(getHelper()->getHighresZoom(viewId));
//    task->useCenterCut(false);
    task->setDelay(50);
    task->setDoc(doc);
    task->setHandle(getSource());
    task->setName(QString("%1-%2").arg(this->getSource().c_str()).arg(viewId));
  }

  return task;
}

void ZDvidGraySlice::_forceUpdate(const ZStackViewParam &viewParam)
{
  forceUpdate(viewParam);
}

void ZDvidGraySlice::forceUpdate(const ZStackViewParam &viewParam)
{
  /*
  if (viewParam.getSliceAxis() != m_sliceAxis) {
    return;
  }
  */

  getHelper()->setViewParamLastUpdate(viewParam);
//  setZoom(viewParam.getZoomLevel());

  if (isVisible()) {
    ZAffineRect rect = getHelper()->getIntCutRect(
          viewParam.getViewId(),
          ZDvidDataSliceHelper::EViewParamOption::LAST_UPDATE);
    int zoom = getHelper()->getZoom(
          viewParam.getViewId(),
          ZDvidDataSliceHelper::EViewParamOption::LAST_UPDATE);

    if (!rect.isEmpty()) {
      int cx = getHelper()->getCenterCutWidth();
      int cy = getHelper()->getCenterCutHeight();
#if 1
      ZStack *stack = getDvidReader().readGrayScaleLowtis(
            rect, zoom, cx, cy, getHelper()->usingCenterCut());
      if (stack) {
#if 1
        int scale = zgeom::GetZoomScale(zoom);
        if (scale > 1) {
          ZPoint normal = rect.getAffinePlane().getNormal();
          if (normal == ZPoint(0, 0, 1)) {
            int z = int(normal.dot(rect.getCenter()));
            int remain = z % scale;
            if (remain > 0) {
              int z1 = z - remain + scale;
              ZAffineRect rect2 = rect;
              rect2.translateDepth(scale - remain);
              ZStack *stack2 = getDvidReader().readGrayScaleLowtis(
                    rect2, zoom, cx, cy, getHelper()->usingCenterCut());
              ZStackProcessor::InterpolateFovia(
                    stack, stack2, cx, cy, scale * 2, z, z1, z, stack);
              delete stack2;
            }
          }
        }

        getHelper()->setActualQuality(
              zoom, getHelper()->getCenterCutWidth(),
              getHelper()->getCenterCutHeight(), true, viewParam.getViewId());
        updateImage(
              stack, viewParam, zoom, cx, cy, getHelper()->usingCenterCut());
//        updateImage(
//              stack, viewParam.getSliceAxis(), rect.getAffinePlane(),
//              getHelper()->getZoom(), viewParam.getViewId());
#endif
#ifdef _DEBUG_2
        std::cout << "Saving stack" << std::endl;
        stack->save(GET_TEST_DATA_DIR + "/_test.tif");
#endif
        delete stack;
      }
#endif
    }
  }
}

void ZDvidGraySlice::printInfo() const
{
  std::cout << "Dvid grayscale: " << std::endl;
//  m_res.print();
//  std::cout << "Offset: " << getX() << ", " << getY() << ", " << getZ() << std::endl;
  std::lock_guard<std::mutex> guard(m_displayBufferMutex);
  for (const auto &buffer : m_displayBufferMap) {
    std::cout << "Size: " << buffer.second->m_imageCanvas.getWidth() << " x "
              << buffer.second->m_imageCanvas.getHeight() << std::endl;
  }
}

void ZDvidGraySlice::setDvidTarget(const ZDvidTarget &target)
{
  getHelper()->setDvidTarget(target);
  if (target.getMaxGrayscaleZoom() > 0) {
    getHelper()->setMaxZoom(target.getMaxGrayscaleZoom());
  } else {
    getHelper()->updateMaxZoom();
  }
}

ZCuboid ZDvidGraySlice::getBoundBox(int viewId) const
{
  return ZCuboid::FromIntCuboid(getHelper()->getBoundBox(viewId));
}
