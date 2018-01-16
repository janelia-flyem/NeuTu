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
  m_type = ZStackObject::TYPE_DVID_GRAY_SLICE;
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
  if (sliceAxis != neutube::Z_AXIS) {
    return;
  }
  //if (!m_image.isNull()) {
  int z = painter.getZOffset() + slice;
  //m_latestZ = z;

  const_cast<ZDvidGraySlice&>(*this).update(z);

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
  if (stack->width() != m_image.width() ||
      stack->height() != m_image.height()) {
    m_image = ZImage(stack->width(), stack->height(), QImage::Format_Indexed8);
  }
  m_image.setOffset(-stack->getOffset().getX(), -stack->getOffset().getY());
  m_image.setData(stack->array8());
  updateContrast();

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
    m_currentViewParam.setZ(z);
    updateImage();
  }
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

  int maxZoomLevel = getDvidTarget().getMaxGrayscaleZoom();
  if (maxZoomLevel < 3) {
    int width = viewParam.getViewPort().width();
    int height = viewParam.getViewPort().height();
    int area = width * height;
    if (area > m_maxWidth * m_maxHeight) {
      if (width > m_maxWidth) {
        width = m_maxWidth;
      }
      if (height > m_maxHeight) {
        height = m_maxHeight;
      }
      newViewParam.resize(width, height);
    }
  }


  if (!m_currentViewParam.contains(newViewParam) ||
      viewParam.getZoomLevel(maxZoomLevel) <
      m_currentViewParam.getZoomLevel(maxZoomLevel)) {
    forceUpdate(newViewParam);
    updated = true;

    m_currentViewParam = newViewParam;
  }

  return updated;
}

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

void ZDvidGraySlice::forceUpdate(const ZStackViewParam &viewParam)
{
  if (viewParam.getSliceAxis() != m_sliceAxis) {
    return;
  }

  if (m_sliceAxis != neutube::Z_AXIS) {
    return;
  }

  m_currentViewParam = viewParam;

//  QMutexLocker locker(&m_updateMutex);

  if (isVisible()) {
    m_zoom = viewParam.getZoomLevel(getDvidTarget().getMaxGrayscaleZoom());
//    int zoomRatio = pow(2, zoom);

    QRect viewPort = viewParam.getViewPort();

    ZIntCuboid box;
    box.setFirstCorner(viewPort.left(), viewPort.top(), viewParam.getZ());
    box.setSize(viewPort.width(), viewPort.height(), 1);


#if defined(_ENABLE_LOWTIS_)
    int cx = m_centerCutWidth;
    int cy = m_centerCutHeight;
    int z = box.getFirstCorner().getZ();
    int zoom = m_zoom;
    if (hasLowresRegion()) {
      ++zoom;
    }
    int scale = misc::GetZoomScale(zoom);
    int remain = z % scale;
    ZStack *stack = m_reader.readGrayScaleLowtis(
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
#else
    ZStack *stack = m_reader.readGrayScale(
          box.getFirstCorner().getX(), box.getFirstCorner().getY(),
          box.getFirstCorner().getZ(), box.getWidth(), box.getHeight(), 1,
          m_zoom);

    if (m_zoom > 0) {
      int z = box.getFirstCorner().getZ();
      int scale = misc::GetZoomScale(m_zoom);
      int remain = z % scale;
      if (remain > 0) {
        int z1 = z + scale;
        ZStack *stack2 = m_reader.readGrayScale(
              box.getFirstCorner().getX(), box.getFirstCorner().getY(),
              z1, box.getWidth(), box.getHeight(), 1, m_zoom);
        double lambda = double(remain) / scale;
        ZStackProcessor::Intepolate(stack, stack2, lambda, stack);
        delete stack2;
      }
    }
#endif
    updateImage(stack);
    delete stack;
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
