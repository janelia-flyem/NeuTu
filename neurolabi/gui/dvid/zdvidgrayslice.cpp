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
  setTarget(ZStackObject::ETarget::TILE_CANVAS);
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
//  m_reader.clear();
  m_helper->clear();
//  invalidateCanvas();
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


int ZDvidGraySlice::getZ() const
{
  return m_helper->getZ();
}

int ZDvidGraySlice::getWidth() const
{
  return m_helper->getWidth();
}

int ZDvidGraySlice::getHeight() const
{
  return m_helper->getHeight();
}

int ZDvidGraySlice::getZoom() const
{
  return m_helper->getZoom();
}

bool ZDvidGraySlice::display(
    QPainter *painter, const DisplayConfig &config) const
{
#ifdef _DEBUG_2
    m_imageCanvas.save(GET_TEST_DATA_DIR + "/_test.png");
#endif
  return m_imageCanvas.paintTo(painter, config.getTransform());
}

#if 0
void ZDvidGraySlice::display(
    ZPainter &painter, int slice, EDisplayStyle /*option*/,
    neutu::EAxis sliceAxis) const
{
  if (sliceAxis != getSliceAxis()) {
    return;
  }

  int z = painter.getZOffset() + slice;
#if 0
  if (getSliceAxis() == neutube::Z_AXIS) {
    const_cast<ZDvidGraySlice&>(*this).update(z);
  }
#endif

  if (z == getZ() && !m_image.isNull()) {
    const_cast<ZDvidGraySlice&>(*this).updatePixmap();
    painter.drawPixmap(getX(), getY(), m_pixmap);
  }
}
#endif

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
  m_image.setContrastProtocol(m_contrastProtocol);
  m_image.updateContrast(m_usingContrastProtocol);
  m_imageCanvas.fromImage(m_image);
//  invalidateCanvas();
#if 0
  if (!m_contrastProtocal.isEmpty()) {
    m_image.enhanceContrast(false);
  } else {
    m_image.enhanceContrast(true);
  }
#endif
}

bool ZDvidGraySlice::hasLowresRegion() const
{
  if (getZoom() > 0) {
    return true;
  }

//  QRect viewport = getViewPort();//m_currentViewParam.getViewPort();
  ZAffineRect rect = getViewParam().getIntCutRect();
  if (rect.getWidth() > getHelper()->getCenterCutWidth() ||
      rect.getHeight() > getHelper()->getCenterCutHeight()) {
    return true;
  }

  return false;
}

/*
void ZDvidGraySlice::invalidateCanvas()
{
  m_isCanvasValid = false;
}

void ZDvidGraySlice::validatePixmap(bool v)
{
  m_isCanvasValid = v;
}

bool ZDvidGraySlice::isCanvasValid() const
{
  return m_isCanvasValid;
}

void ZDvidGraySlice::validatePixmap()
{
  m_isCanvasValid = true;
}
*/

void ZDvidGraySlice::updateImage(
    const ZStack *stack, const ZAffineRect &rect)
{
//  QElapsedTimer timer;
//  timer.start();

  if (stack != NULL) {
    ZSliceViewTransform t;

    if (getHelper()->getSliceAxis() == neutu::EAxis::ARB) {
      t.setCutPlane(rect.getAffinePlane());
    } else {
      t.setCutPlane(getHelper()->getSliceAxis(), rect.getCenter());
    }

    m_image = neutu::vis2d::GetSlice(*stack, stack->getOffset().getZ());
//    m_image.setContrastProtocol(m_contrastProtocol);
//    m_image.updateContrast(m_usingContrastProtocol);

//    t.setCutPlane(rect.getAffinePlane());
    t.setScale(1.0 / zgeom::GetZoomScale(getZoom()));
    //Assuming lowtis uses right integer center
    t.setAnchor((stack->width()) / 2, (stack->height()) / 2);

    m_imageCanvas.setTransform(t);
    updateContrast();

//    m_imageCanvas.fromImage(m_image);
    /*
    if (stack->width() != m_image.width() ||
        stack->height() != m_image.height()) {
      m_image = ZImage(stack->width(), stack->height(), QImage::Format_Indexed8);
    }
    m_image.setOffset(-stack->getOffset().getX(), -stack->getOffset().getY());
    m_image.setData(stack->array8());
    */

//    updateContrast();
  }/* else {
    invalidateCanvas();
  }*/

//  std::cout << "Grayscale udpating time: " << timer.elapsed() << std::endl;

#ifdef _DEBUG_2
  m_image.save((GET_TEST_DATA_DIR + "/test.tif").c_str());
#endif
}

void ZDvidGraySlice::saveImage(const std::string &path)
{
  m_image.save(path.c_str());
}

void ZDvidGraySlice::savePixmap(const std::string &path)
{
  m_imageCanvas.save(path.c_str());
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

ZStackViewParam ZDvidGraySlice::getViewParam() const
{
  return getHelper()->getViewParam();
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

/*
void ZDvidGraySlice::update(int z)
{
  if (getZ() != z) {
//    ZStackViewParam viewParam = m_currentViewParam;
    ZStackViewParam viewParam = getViewParam();
    viewParam.moveCutDepth(z - viewParam.getCutDepth(ZPoint(0, 0, 0)));
    forceUpdate(viewParam);
//    m_currentViewParam.setZ(z);
//    updateImage();
  }
}
*/

/*
bool ZDvidGraySlice::validateSize(int *width, int *height)
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
*/

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

#if 0
void ZDvidGraySlice::forceUpdate(const QRect &viewPort, int z)
{
  ZIntCuboid box;
  box.setMinCorner(viewPort.left(), viewPort.top(), z);
  box.setSize(viewPort.width(), viewPort.height(), 1);

  int cx = getHelper()->getCenterCutWidth();
  int cy = getHelper()->getCenterCutHeight();
//  int z = box.getFirstCorner().getZ();


  ZStack *stack = NULL;

  if (getSliceAxis() == neutu::EAxis::Z) {
    int zoom = getZoom();
    if (hasLowresRegion()) {
      ++zoom;
    }

    int scale = zgeom::GetZoomScale(zoom);
    int remain = z % scale;
    stack = getDvidReader().readGrayScaleLowtis(
          box.getMinCorner().getX(), box.getMinCorner().getY(),
          z, box.getWidth(), box.getHeight(),
          getZoom(), cx, cy, true);
    if (scale > 1) {
      if (remain > 0) {
        //        int z1 = z + scale - remain;
        int z1 = z - remain + scale;
        ZStack *stack2 = getDvidReader().readGrayScaleLowtis(
              box.getMinCorner().getX(), box.getMinCorner().getY(),
              z1, box.getWidth(), box.getHeight(), getZoom(), cx, cy,
              true);
        //        double lambda = double(remain) / scale;
        ZStackProcessor::IntepolateFovia(
              stack, stack2, cx, cy, scale, z, z1, z, stack);
        //        ZStackProcessor::Intepolate(stack, stack2, lambda, stack);
        delete stack2;
      }
    }

    getHelper()->setActualQuality(
          getZoom(), cx, cy, true);
  }

  updateImage(stack);

  delete stack;
}
#endif

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
      getHelper()->setActualQuality(zoom, centerCutX, centerCutY, usingCenterCut);
      getHelper()->setViewParam(viewParam);
//      getHelper()->setCenterCut(centerCutX, centerCutY);
      updateImage(stack, viewParam.getIntCutRect());
      succ = true;
    } else {
      delete stack;
    }
  }
  return succ;
}

ZTask* ZDvidGraySlice::makeFutureTask(ZStackDoc *doc)
{
  ZDvidGraySliceHighresTask *task = NULL;
  const int maxSize = 1024*1024;
  if (getHelper()->needHighResUpdate()
      && getHelper()->getViewDataSize() < maxSize) {
    task = new ZDvidGraySliceHighresTask;
    task->setViewParam(getHelper()->getViewParam());
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

  if (isVisible()) {
    int maxZoomLevel = getDvidTarget().getMaxGrayscaleZoom();
    setZoom(viewParam.getZoomLevel(maxZoomLevel));
    ZAffineRect rect = viewParam.getIntCutRect();
//    int scale = zgeom::GetZoomScale(getZoom());
    if (maxZoomLevel < 3) {
      int width = rect.getWidth();
      int height = rect.getHeight();
      if (getHelper()->validateSize(&width, &height)) {
        rect.setSize(width, height);
      }
    }

    ZStack *stack = getDvidReader().readGrayScaleLowtis(
          rect.getCenter().toIntPoint(), rect.getV1(), rect.getV2(),
          rect.getWidth(), rect.getHeight(),
          getZoom(), getHelper()->getCenterCutWidth(),
          getHelper()->getCenterCutHeight(), true);
    getHelper()->setActualQuality(
          getZoom(), getHelper()->getCenterCutWidth(),
          getHelper()->getCenterCutHeight(), true);
    updateImage(stack, rect);
#ifdef _DEBUG_
    std::cout << "Saving stack" << std::endl;
    stack->save(GET_TEST_DATA_DIR + "/_test.tif");
#endif
    delete stack;

//    forceUpdate(viewParam.toArbSliceViewParam());
//    m_zoom = viewParam.getZoomLevel(getDvidTarget().getMaxGrayscaleZoom());
    /*
    if (m_sliceAxis == neutu::EAxis::Z) {
      QRect viewPort = viewParam.getViewPort();
      forceUpdate(viewPort, viewParam.getZ());
    } else if (m_sliceAxis == neutu::EAxis::ARB) {
//      setZoom(0); //Temporary fix for the crashing problem in grayscale retrieval
      forceUpdate(viewParam.getSliceViewParam());
      //Align the image with the view port, which is used by the painter
      m_image.setOffset(viewParam.getViewPort().left(),
                        viewParam.getViewPort().top());
    }
    */
  }

  getHelper()->setViewParam(viewParam);
//  m_currentViewParam = viewParam;
}

#if 0
void ZDvidGraySlice::forceUpdate(const ZArbSliceViewParam &viewParam)
{
  if (!viewParam.isValid()) {
    return;
  }

  if (isVisible() && viewParam.isValid()) {
//    m_zoom = 0;
#ifdef _DEBUG_
    std::cout << "Gray center: " << viewParam.getCenter().toString() << std::endl;
#endif

    ZStack *stack = getDvidReader().readGrayScaleLowtis(
          viewParam.getCenter(), viewParam.getPlaneV1(), viewParam.getPlaneV2(),
          viewParam.getWidth(), viewParam.getHeight(),
          getZoom(), getHelper()->getCenterCutWidth(),
          getHelper()->getCenterCutHeight(), true);
    getHelper()->setActualQuality(
          getZoom(), getHelper()->getCenterCutWidth(),
          getHelper()->getCenterCutHeight(), true);
    updateImage(stack, viewParam);
    delete stack;
  }
}
#endif

void ZDvidGraySlice::printInfo() const
{
  std::cout << "Dvid grayscale: " << std::endl;
//  m_res.print();
//  std::cout << "Offset: " << getX() << ", " << getY() << ", " << getZ() << std::endl;
  std::cout << "Size: " << m_imageCanvas.getWidth() << " x "
            << m_imageCanvas.getHeight() << std::endl;

}

void ZDvidGraySlice::setDvidTarget(const ZDvidTarget &target)
{
  getHelper()->setDvidTarget(target);
  if (target.getMaxGrayscaleZoom() > 0) {
    getHelper()->setMaxZoom(target.getMaxGrayscaleZoom());
  } else {
    getHelper()->updateMaxZoom();
  }
//  m_dvidTarget = target;
//  getDvidReader().open(target);
}

ZCuboid ZDvidGraySlice::getBoundBox() const
{
  return ZCuboid::FromIntCuboid(getHelper()->getBoundBox());
  /*
  return ZCuboid::FromIntCuboid(
        ZIntCuboid(getX(), getY(), getZ(),
                   getX() + getWidth(), getY() + getHeight(), getZ()));
                   */
}

/*
ZRect2d ZDvidGraySlice::getBoundBox() const
{
  ZRect2d rect;
  rect.set(getX(), getY(), getWidth(), getHeight());

  return rect;
}
*/
