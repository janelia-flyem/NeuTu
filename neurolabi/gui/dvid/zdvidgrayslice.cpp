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
  getHelper()->useCenterCut(false);
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

int ZDvidGraySlice::getWidth(int viewId) const
{
  return m_helper->getWidth(viewId);
}

int ZDvidGraySlice::getHeight(int viewId) const
{
  return m_helper->getHeight(viewId);
}

int ZDvidGraySlice::getZoom() const
{
  return m_helper->getZoom();
}

ZSliceCanvas& ZDvidGraySlice::getImageCanvas(int viewId) const
{
  return getDisplayBuffer(viewId)->m_imageCanvas;
}

ZImage& ZDvidGraySlice::getImage(int viewId) const
{
  return getDisplayBuffer(viewId)->m_image;
}

bool ZDvidGraySlice::display(
    QPainter *painter, const DisplayConfig &config) const
{
#ifdef _DEBUG_2
    m_imageCanvas.save(GET_TEST_DATA_DIR + "/_test.png");
#endif
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
  if (getZoom() > 0) {
    return true;
  }

  ZAffineRect rect =
      getViewParam(viewId).getIntCutRect(getHelper()->getDataRange());

  return (rect.getWidth() > getHelper()->getCenterCutWidth() ||
      rect.getHeight() > getHelper()->getCenterCutHeight());
}

void ZDvidGraySlice::updateImage(
    const ZStack *stack, const ZAffinePlane &ap, int zoom, int viewId)
{
  if (stack != NULL) {
    auto buffer = getDisplayBuffer(viewId);
    buffer->m_image =
        neutu::vis2d::GetSlice(*stack, stack->getOffset().getZ());

    ZSliceViewTransform t = getHelper()->getCanvasTransform(
          ap, stack->width(), stack->height(), zoom, viewId);

/*
    if (getHelper()->getSliceAxis() == neutu::EAxis::ARB) {
      t.setCutPlane(ap.getAffinePlane());
    } else {
      t.setCutPlane(getHelper()->getSliceAxis(), ap.getCenter());
    }

    t.setScale(1.0 / zgeom::GetZoomScale(getZoom()));
    //Assuming lowtis uses left integer center
    t.setAnchor((stack->width()) / 2, (stack->height()) / 2);
*/

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

ZStackViewParam ZDvidGraySlice::getViewParam(int viewId) const
{
  return getHelper()->getViewParam(viewId);
}

ZIntCuboid ZDvidGraySlice::getDataRange() const
{
  return getHelper()->getDataRange();
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
  if (viewParam.getSliceAxis() != getSliceAxis()) {
    return false;
  }

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

void ZDvidGraySlice::setZoom(int zoom)
{
  getHelper()->setZoom(zoom);
//  m_zoom = zoom;
}

void ZDvidGraySlice::setContrastProtocol(const ZContrastProtocol &cp)
{
  m_contrastProtocol = cp;
}

int ZDvidGraySlice::getScale() const
{
  return getHelper()->getScale();
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
      ZAffineRect rect = viewParam.getIntCutRect(getHelper()->getDataRange());
      updateImage(stack, rect.getAffinePlane(), zoom, viewParam.getViewId());
      delete stack;
#ifdef _DEBUG_2
      std::cout << "Saving image canvas ..." << std::endl;
      m_imageCanvas.save((GET_TEST_DATA_DIR + "/_test.png").c_str());
#endif

      succ = true;
    } else {
      delete stack;
    }
  }
  return succ;
}

ZTask* ZDvidGraySlice::makeFutureTask(ZStackDoc *doc, int viewId)
{
  ZDvidGraySliceHighresTask *task = nullptr;
  const int maxSize = 1024*1024;
  if (getHelper()->needHighResUpdate(viewId)
      && getHelper()->getViewDataSize(viewId) < maxSize) {
    task = new ZDvidGraySliceHighresTask;
    task->setViewParam(getHelper()->getViewParam(viewId));
    task->setZoom(getHelper()->getZoom());
    task->useCenterCut(false);
    task->setDelay(50);
    task->setDoc(doc);
    task->setHandle(getSource());
    task->setName(this->getSource().c_str());
  }

  return task;
}

void ZDvidGraySlice::forceUpdate(const ZStackViewParam &viewParam)
{
  if (viewParam.getSliceAxis() != m_sliceAxis) {
    return;
  }

  getHelper()->setViewParam(viewParam);
  setZoom(viewParam.getZoomLevel());

  if (isVisible()) {
    ZAffineRect rect = getHelper()->getIntCutRect(viewParam.getViewId());

    if (!rect.isEmpty()) {
      ZStack *stack = getDvidReader().readGrayScaleLowtis(
            rect,
            getZoom(), getHelper()->getCenterCutWidth(),
            getHelper()->getCenterCutHeight(), true);
      getHelper()->setActualQuality(
            getZoom(), getHelper()->getCenterCutWidth(),
            getHelper()->getCenterCutHeight(), true, viewParam.getViewId());
      updateImage(
            stack, rect.getAffinePlane(), getHelper()->getZoom(), viewParam.getViewId());
#ifdef _DEBUG_2
      std::cout << "Saving stack" << std::endl;
      stack->save(GET_TEST_DATA_DIR + "/_test.tif");
#endif
      delete stack;
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
