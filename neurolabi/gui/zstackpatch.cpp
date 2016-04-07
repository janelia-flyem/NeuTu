#include "zstackpatch.h"
#include "zstack.hxx"
#include "tz_math.h"
#include "zpainter.h"

ZStackPatch::ZStackPatch(ZStack *stack) : m_stack(stack), m_sx(1.0), m_sy(1.0)
{
  setTarget(ZStackObject::TARGET_OBJECT_CANVAS);
}

ZStackPatch::~ZStackPatch()
{
  delete m_stack;
}

ZPoint ZStackPatch::getFinalOffset() const
{
  ZPoint pt = m_offset;

  if (m_stack != NULL) {
    pt += ZPoint(m_stack->getOffset().getX() * m_sx,
           m_stack->getOffset().getY() * m_sy,
           m_stack->getOffset().getZ());
  }

  return pt;
}

void ZStackPatch::display(
    ZPainter &painter, int slice, EDisplayStyle /*option*/,
    NeuTube::EAxis sliceAxis) const
{
  if (sliceAxis != NeuTube::Z_AXIS) {
    return;
  }

  if (m_stack != NULL) {
    int dataFocus = iround(painter.getZOffset() + slice);

    ZImage image = getImage(dataFocus);
//    ZPoint pt = getFinalOffset() - painter.getOffset();

    painter.save();

    QTransform transform;
    transform.scale(m_sx, m_sy);
    transform.translate(m_offset.x() + m_stack->getOffset().getX(),
                        m_offset.y() + m_stack->getOffset().getY());
    transform = painter.getTransform() * transform;
    //transform.translate(pt.x(), pt.y());
    painter.setTransform(transform);
    painter.drawImage(0, 0, image);

    painter.restore();
  }
}

int ZStackPatch::getZOffset() const
{
  if (m_stack != NULL) {
    return m_stack->getOffset().getZ();
  }

  return 0;
}

ZImage ZStackPatch::getImage(int z) const
{
  if (m_stack != NULL) {
    int slice = z - getZOffset();
    if (slice >= 0 && slice < m_stack->depth()) {
      ZImage image(m_stack->width(), m_stack->height());
      switch (m_stack->kind()) {
      case GREY:
        image.setData((uint8_t*) m_stack->getDataPointer(0, slice), -1);
        break;
      case GREY16:
      {
        std::vector<ZImage::DataSource<uint16_t> > stackData16;
        for (int i=0; i<m_stack->channelNumber(); ++i) {
          stackData16.push_back(
                ZImage::DataSource<uint16_t>(
                  static_cast<uint16_t*>(m_stack->getDataPointer(i, slice)),
                  0.07, 0, m_stack->getChannelColor(i)));
        }
        image.setData(stackData16, 255, false);
      }
        break;
      }
      return image;
    }
  }

  return ZImage();
}

void ZStackPatch::setScale(double sx, double sy)
{
  m_sx = sx;
  m_sy = sy;
}

void ZStackPatch::setFinalOffset(double dx, double dy)
{
  m_stack->setOffset(0, 0, 0);
  m_offset.set(dx, dy, m_offset.z());
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZStackPatch)
