#include "zdvidgrayslice.h"
#include "zdvidurl.h"
#include "zdvidbufferreader.h"
#include "zrect2d.h"
#include "zpainter.h"
#include "zstackviewparam.h"
#include "misc/miscutility.h"
#include "imgproc/zstackprocessor.h"
#include "zintcuboid.h"
#include "neutubeconfig.h"

ZDvidGraySlice::ZDvidGraySlice()
{
  setTarget(ZStackObject::TARGET_TILE_CANVAS);
  m_type = GetType();
  m_zoom = 0;
  m_maxWidth = 512;
  m_maxHeight = 512;
}

ZDvidGraySlice::~ZDvidGraySlice()
{
  clear();
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidGraySlice)

void ZDvidGraySlice::clear()
{ 
  m_reader.clear();
  invalidatePixmap();
//  delete m_reader;
//  m_reader = NULL;
//  m_dvidTarget.clear();
//  m_image.clear();
}

void ZDvidGraySlice::display(
    ZPainter &painter, int slice, EDisplayStyle /*option*/,
    neutube::EAxis sliceAxis) const
{
  if (sliceAxis != getSliceAxis()) {
    return;
  }

  int z = painter.getZOffset() + slice;
  if (getSliceAxis() == neutube::Z_AXIS) {
    const_cast<ZDvidGraySlice&>(*this).update(z);
  }

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
  if (m_zoom > 0) {
    return true;
  }

  QRect viewport = m_currentViewParam.getViewPort();
  if (viewport.width() > m_centerCutWidth ||
      viewport.height() > m_centerCutHeight) {
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
  }
}

void ZDvidGraySlice::setBoundBox(const ZRect2d &rect)
{
  m_currentViewParam.setViewPort(
        QRect(rect.getX0(), rect.getY0(), rect.getWidth(), rect.getHeight()));
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
    ZStackViewParam viewParam = m_currentViewParam;
    viewParam.moveSlice(z - m_currentViewParam.getZ());
    forceUpdate(viewParam);
//    m_currentViewParam.setZ(z);
//    updateImage();
  }
}

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

template<typename T>
int ZDvidGraySlice::updateParam(T *param)
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

bool ZDvidGraySlice::update(const ZStackViewParam &viewParam)
{
  if (viewParam.getSliceAxis() != m_sliceAxis) {
    return false;
  }

  if (viewParam.getViewPort().isEmpty()) {
    return false;
  }

  bool updated = false;

  ZStackViewParam newViewParam = viewParam;

  int maxZoomLevel = updateParam(&newViewParam);

  /*
  int maxZoomLevel = getDvidTarget().getMaxGrayscaleZoom();
  if (maxZoomLevel < 3) {
    int width = viewParam.getViewPort().width();
    int height = viewParam.getViewPort().height();
    if (validateSize(&width, &height)) {
      newViewParam.resize(width, height);
    }
  }
  */

  if (!m_currentViewParam.contains(newViewParam) ||
      viewParam.getZoomLevel(maxZoomLevel) <
      m_currentViewParam.getZoomLevel(maxZoomLevel)) {
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
int ZDvidGraySlice::getZoom() const
{
  return m_zoom;
}

void ZDvidGraySlice::setZoom(int zoom)
{
  m_zoom = zoom;
}

void ZDvidGraySlice::setContrastProtocol(const ZContrastProtocol &cp)
{
  m_contrastProtocal = cp;
}

int ZDvidGraySlice::getScale() const
{
  return misc::GetZoomScale(getZoom());
}

void ZDvidGraySlice::setCenterCut(int width, int height)
{
  m_centerCutWidth = width;
  m_centerCutHeight = height;
}

/*
void ZDvidGraySlice::setArbitraryAxis(const ZPoint &v1, const ZPoint &v2)
{
  m_sliceAxis = neutube::A_AXIS;
  if (v1.isPendicularTo(v2)) {
    m_v1 = v1;
    m_v2 = v2;
    m_v1.normalize();
    m_v2.normalize();
  }
}
*/

void ZDvidGraySlice::forceUpdate(const QRect &viewPort, int z)
{
  ZIntCuboid box;
  box.setFirstCorner(viewPort.left(), viewPort.top(), z);
  box.setSize(viewPort.width(), viewPort.height(), 1);

  int cx = m_centerCutWidth;
  int cy = m_centerCutHeight;
//  int z = box.getFirstCorner().getZ();


  ZStack *stack = NULL;

  if (getSliceAxis() == neutube::Z_AXIS) {
    int zoom = m_zoom;
    if (hasLowresRegion()) {
      ++zoom;
    }

    int scale = misc::GetZoomScale(zoom);
    int remain = z % scale;
    stack = m_reader.readGrayScaleLowtis(
          box.getFirstCorner().getX(), box.getFirstCorner().getY(),
          z, box.getWidth(), box.getHeight(),
          m_zoom, cx, cy);
    if (scale > 1) {
      if (remain > 0) {
        //        int z1 = z + scale - remain;
        int z1 = z - remain + scale;
        ZStack *stack2 = m_reader.readGrayScaleLowtis(
              box.getFirstCorner().getX(), box.getFirstCorner().getY(),
              z1, box.getWidth(), box.getHeight(), m_zoom, cx, cy);
        //        double lambda = double(remain) / scale;
        ZStackProcessor::IntepolateFovia(
              stack, stack2, cx, cy, scale, z, z1, z, stack);
        //        ZStackProcessor::Intepolate(stack, stack2, lambda, stack);
        delete stack2;
      }
    }
  } /*else if (getSliceAxis() == neutube::A_AXIS) {
    //Assume no rotation happens
    ZArbSliceViewParam sliceViewParam = m_sliceViewParam;

    if (m_currentViewParam.isValid()) {
      QPoint oldCenter = m_currentViewParam.getViewPort().center();
      QPoint newCenter = viewPort.center();
      int dz = z - m_currentViewParam.getZ();
      int dx = newCenter.x() - oldCenter.x();
      int dy = newCenter.y() - oldCenter.y();

      m_sliceViewParam.move(dx, dy, dz);
    } else {
      sliceViewParam.setCenter(box.getCenter());
      sliceViewParam.setSize(box.getWidth(), box.getHeight());
    }
    forceUpdate(sliceViewParam);
  }*/

  updateImage(stack);

  delete stack;
}

void ZDvidGraySlice::forceUpdate(const ZStackViewParam &viewParam)
{
  if (viewParam.getSliceAxis() != m_sliceAxis) {
    return;
  }

  if (m_sliceAxis != neutube::Z_AXIS && m_sliceAxis != neutube::A_AXIS) {
    return;
  }

  if (isVisible()) {
    m_zoom = viewParam.getZoomLevel(getDvidTarget().getMaxGrayscaleZoom());
    if (m_sliceAxis == neutube::Z_AXIS) {
      QRect viewPort = viewParam.getViewPort();
      forceUpdate(viewPort, viewParam.getZ());
    } else if (m_sliceAxis == neutube::A_AXIS) {
      m_zoom = 0; //Temporary fix for the crashing problem in grayscale retrieval
      forceUpdate(viewParam.getSliceViewParam());
      //Align the image with the view port, which is used by the painter
      m_image.setOffset(viewParam.getViewPort().left(),
                        viewParam.getViewPort().top());
    }
  } else {
    invalidatePixmap();
  }

  m_currentViewParam = viewParam;
}

void ZDvidGraySlice::forceUpdate(const ZArbSliceViewParam &viewParam)
{
  if (m_sliceAxis != neutube::A_AXIS || !viewParam.isValid()) {
    return;
  }

  if (isVisible()) {
//    m_zoom = 0;
#ifdef _DEBUG_
    std::cout << "Gray center: " << viewParam.getCenter().toString() << std::endl;
#endif

    ZStack *stack = m_reader.readGrayScaleLowtis(
          viewParam.getCenter(), viewParam.getPlaneV1(), viewParam.getPlaneV2(),
          viewParam.getWidth(), viewParam.getHeight(),
          m_zoom, m_centerCutWidth, m_centerCutHeight);
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
//  m_dvidTarget = target;
  m_reader.open(target);
}

ZRect2d ZDvidGraySlice::getBoundBox() const
{
  ZRect2d rect;
  rect.set(getX(), getY(), getWidth(), getHeight());

  return rect;
}
