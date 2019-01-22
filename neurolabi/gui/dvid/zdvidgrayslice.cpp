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

ZDvidGraySlice::ZDvidGraySlice()
{
  setTarget(ZStackObject::TARGET_TILE_CANVAS);
  m_type = GetType();
//  m_zoom = 0;
//  m_maxWidth = 512;
//  m_maxHeight = 512;

  m_helper = std::make_unique<ZDvidDataSliceHelper>(ZDvidData::ERole::GRAY_SCALE);
  getHelper()->useCenterCut(false);
}

ZDvidGraySlice::~ZDvidGraySlice()
{
  clear();
}

//ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidGraySlice)

void ZDvidGraySlice::clear()
{ 
//  m_reader.clear();
  m_helper->clear();
  invalidatePixmap();
//  delete m_reader;
//  m_reader = NULL;
//  m_dvidTarget.clear();
//  m_image.clear();
}

const ZDvidReader& ZDvidGraySlice::getDvidReader() const
{
  return m_helper->getDvidReader();
}

const ZDvidTarget& ZDvidGraySlice::getDvidTarget() const
{
  return getDvidReader().getDvidTarget();
}

int ZDvidGraySlice::getX() const
{
  return m_helper->getX();
}

int ZDvidGraySlice::getY() const
{
  return m_helper->getY();
}

int ZDvidGraySlice::getZ() const
{
  return m_helper->getZ();
}

void ZDvidGraySlice::setZ(int z)
{
  m_helper->setZ(z);
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

void ZDvidGraySlice::display(
    ZPainter &painter, int slice, EDisplayStyle /*option*/,
    neutube::EAxis sliceAxis) const
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

void ZDvidGraySlice::updateContrast(bool highContrast)
{
  m_usingContrastProtocol = highContrast;
  updateContrast();
}

void ZDvidGraySlice::updateContrast(const ZJsonObject &obj)
{
  m_contrastProtocal.load(obj);
  updateContrast();
}

void ZDvidGraySlice::updateContrast()
{
  m_image.setContrastProtocol(m_contrastProtocal);
  m_image.updateContrast(m_usingContrastProtocol);
  invalidatePixmap();
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

  QRect viewport = getViewPort();//m_currentViewParam.getViewPort();
  if (viewport.width() > getHelper()->getCenterCutWidth() ||
      viewport.height() > getHelper()->getCenterCutHeight()) {
    return true;
  }

  return false;
}

void ZDvidGraySlice::invalidatePixmap()
{
  m_isPixmapValid = false;
}

void ZDvidGraySlice::validatePixmap(bool v)
{
  m_isPixmapValid = v;
}

bool ZDvidGraySlice::isPixmapValid() const
{
  return m_isPixmapValid;
}

void ZDvidGraySlice::validatePixmap()
{
  m_isPixmapValid = true;
}

void ZDvidGraySlice::updateImage(const ZStack *stack)
{
//  QElapsedTimer timer;
//  timer.start();

  if (stack != NULL) {
    if (stack->width() != m_image.width() ||
        stack->height() != m_image.height()) {
      m_image = ZImage(stack->width(), stack->height(), QImage::Format_Indexed8);
    }
    m_image.setOffset(-stack->getOffset().getX(), -stack->getOffset().getY());
    m_image.setData(stack->array8());
    updateContrast();
  } else {
    m_image.clear();
    invalidatePixmap();
  }

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
  m_pixmap.save(path.c_str());
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

void ZDvidGraySlice::updatePixmap()
{
  if (!isPixmapValid()) {
    m_pixmap.detach();
    m_pixmap.convertFromImage(m_image);
    double scale = 1.0 / getScale();
    m_pixmap.setScale(scale, scale);
    m_pixmap.setOffset(-getX(), -getY());
    validatePixmap();

#ifdef _DEBUG_
  std::cout << "gray slice pixmap offset: "
            << m_pixmap.getTransform().getTx() << " "
            << m_pixmap.getTransform().getTy() << std::endl;

  std::cout << "gray slice pixmap scale: "
            << m_pixmap.getTransform().getSx() << " "
            << m_pixmap.getTransform().getSy() << std::endl;
#endif
  }
}

ZStackViewParam ZDvidGraySlice::getViewParam() const
{
  return getHelper()->getViewParam();
}

QRect ZDvidGraySlice::getViewPort() const
{
  return getHelper()->getViewPort();
}

void ZDvidGraySlice::setBoundBox(const ZRect2d &rect)
{
  getHelper()->setBoundBox(rect);
  /*
  m_currentViewParam.setViewPort(
        QRect(rect.getX0(), rect.getY0(), rect.getWidth(), rect.getHeight()));
        */
}

#if 0
bool ZDvidGraySlice::isRegionConsistent() const
{
  return (getX() == -m_image.getTransform().getTx()) &&
      (getY() == -m_image.getTransform().getTy()) &&
      (getWidth() == m_image.width()) && (getHeight() == m_image.height());
}
#endif

void ZDvidGraySlice::update(int z)
{
  if (getZ() != z) {
//    ZStackViewParam viewParam = m_currentViewParam;
    ZStackViewParam viewParam = getViewParam();
    viewParam.moveSlice(z - viewParam.getZ());
    forceUpdate(viewParam);
//    m_currentViewParam.setZ(z);
//    updateImage();
  }
}

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

  if (viewParam.getViewPort().isEmpty()) {
    return false;
  }

  bool updated = false;

  ZStackViewParam newViewParam = getHelper()->getValidViewParam(viewParam);
  if (getHelper()->hasNewView(newViewParam)) {
    forceUpdate(newViewParam);
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
  m_contrastProtocal = cp;
}

int ZDvidGraySlice::getScale() const
{
  return getHelper()->getScale();
}

void ZDvidGraySlice::setCenterCut(int width, int height)
{
  getHelper()->setCenterCut(width, height);
}

void ZDvidGraySlice::forceUpdate(const QRect &viewPort, int z)
{
  ZIntCuboid box;
  box.setFirstCorner(viewPort.left(), viewPort.top(), z);
  box.setSize(viewPort.width(), viewPort.height(), 1);

  int cx = getHelper()->getCenterCutWidth();
  int cy = getHelper()->getCenterCutHeight();
//  int z = box.getFirstCorner().getZ();


  ZStack *stack = NULL;

  if (getSliceAxis() == neutube::EAxis::Z) {
    int zoom = getZoom();
    if (hasLowresRegion()) {
      ++zoom;
    }

    int scale = zgeom::GetZoomScale(zoom);
    int remain = z % scale;
    stack = getDvidReader().readGrayScaleLowtis(
          box.getFirstCorner().getX(), box.getFirstCorner().getY(),
          z, box.getWidth(), box.getHeight(),
          getZoom(), cx, cy, true);
    if (scale > 1) {
      if (remain > 0) {
        //        int z1 = z + scale - remain;
        int z1 = z - remain + scale;
        ZStack *stack2 = getDvidReader().readGrayScaleLowtis(
              box.getFirstCorner().getX(), box.getFirstCorner().getY(),
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
      updateImage(stack);
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
    task->setDelay(100);
    task->setDoc(doc);
  }

  return task;
}

void ZDvidGraySlice::forceUpdate(const ZStackViewParam &viewParam)
{
  if (viewParam.getSliceAxis() != m_sliceAxis) {
    return;
  }

  if (m_sliceAxis != neutube::EAxis::Z && m_sliceAxis != neutube::EAxis::ARB) {
    return;
  }

  if (isVisible()) {
    setZoom(viewParam.getZoomLevel(getDvidTarget().getMaxGrayscaleZoom()));
//    m_zoom = viewParam.getZoomLevel(getDvidTarget().getMaxGrayscaleZoom());
    if (m_sliceAxis == neutube::EAxis::Z) {
      QRect viewPort = viewParam.getViewPort();
      forceUpdate(viewPort, viewParam.getZ());
    } else if (m_sliceAxis == neutube::EAxis::ARB) {
//      setZoom(0); //Temporary fix for the crashing problem in grayscale retrieval
      forceUpdate(viewParam.getSliceViewParam());
      //Align the image with the view port, which is used by the painter
      m_image.setOffset(viewParam.getViewPort().left(),
                        viewParam.getViewPort().top());
    }
  } else {
    invalidatePixmap();
  }

  getHelper()->setViewParam(viewParam);
//  m_currentViewParam = viewParam;
}

void ZDvidGraySlice::forceUpdate(const ZArbSliceViewParam &viewParam)
{
  if (m_sliceAxis != neutube::EAxis::ARB || !viewParam.isValid()) {
    return;
  }

  if (isVisible()) {
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
    updateImage(stack);
    delete stack;
  } else {
    invalidatePixmap();
  }
}

void ZDvidGraySlice::printInfo() const
{
  std::cout << "Dvid grayscale: " << std::endl;
//  m_res.print();
  std::cout << "Offset: " << getX() << ", " << getY() << ", " << getZ() << std::endl;
  std::cout << "Size: " << m_image.width() << " x " << m_image.height()
            << std::endl;

}

void ZDvidGraySlice::setDvidTarget(const ZDvidTarget &target)
{
  getHelper()->setDvidTarget(target);
  getHelper()->setMaxZoom(target.getMaxGrayscaleZoom());
//  m_dvidTarget = target;
//  getDvidReader().open(target);
}

ZRect2d ZDvidGraySlice::getBoundBox() const
{
  ZRect2d rect;
  rect.set(getX(), getY(), getWidth(), getHeight());

  return rect;
}
