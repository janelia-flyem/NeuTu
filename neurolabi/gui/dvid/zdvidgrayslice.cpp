#include "zdvidgrayslice.h"
#include "zdvidurl.h"
#include "zdvidbufferreader.h"
#include "zrect2d.h"
#include "zpainter.h"

ZDvidGraySlice::ZDvidGraySlice() : m_x(0), m_y(0), m_z(0), m_width(0),
  m_height(0)
{
  setTarget(ZStackObject::TARGET_STACK_CANVAS);
  m_type = ZStackObject::TYPE_DVID_GRAY_SLICE;
}

ZDvidGraySlice::~ZDvidGraySlice()
{
  clear();
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidGraySlice)

void ZDvidGraySlice::clear()
{
  m_dvidTarget.clear();
//  m_image.clear();
}

void ZDvidGraySlice::display(
    ZPainter &painter, int slice, EDisplayStyle /*option*/,
    NeuTube::EAxis sliceAxis) const
{
  if (sliceAxis != NeuTube::Z_AXIS) {
    return;
  }
  //if (!m_image.isNull()) {
    int z = painter.getZOffset() + slice;
    //m_latestZ = z;

    const_cast<ZDvidGraySlice&>(*this).update(z);

    if (z == m_z && !m_image.isNull()) {
      painter.drawImage(getX(), getY(), m_image);
    }
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
}

void ZDvidGraySlice::setBoundBox(const ZRect2d &rect)
{
  m_x = rect.getX0();
  m_y = rect.getY0();
  m_width = rect.getWidth();
  m_height = rect.getHeight();
}

bool ZDvidGraySlice::isRegionConsistent() const
{
  return (m_x == -m_image.getTransform().getTx()) &&
      (m_y == -m_image.getTransform().getTy()) &&
      (m_width == m_image.width()) && (m_height == m_image.height());
}

void ZDvidGraySlice::update(int z)
{
  if (m_z != z || !isRegionConsistent()) {
    m_z = z;
    updateImage();
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
  m_dvidTarget = target;
}

ZRect2d ZDvidGraySlice::getBoundBox() const
{
  ZRect2d rect;
  rect.set(getX(), getY(), getWidth(), getHeight());

  return rect;
}
