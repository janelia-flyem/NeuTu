#include "zsparseobject.h"

#include <QPen>

#include "neulib/math/utilities.h"
#include "zpainter.h"
#include "zstack.hxx"
#include "neutubeconfig.h"
#include "data3d/displayconfig.h"
#include "vis2d/zslicepainter.h"
#include "imgproc/zstacksource.h"

const ZLabelColorTable ZSparseObject::m_colorTable;

//ZINTERFACE_DEFINE_CLASS_NAME(ZSparseObject)

ZSparseObject::ZSparseObject()
{
  m_type = GetType();
  m_uLabel = -1;
  m_color = m_colorTable.getColor(m_uLabel);
}

void ZSparseObject::setStackSource(std::shared_ptr<ZStackSource> source)
{
  m_stackSource = source;
}

bool ZSparseObject::display(QPainter *painter, const DisplayConfig &config) const
{
  if (!m_stackSource) {
    return ZObject3dScan::display(painter, config);
  } else if (getSliceAxis() == config.getSliceAxis()) {
    ZSlice2dPainter s2Painter;
    ZModelViewTransform t = config.getWorldViewTransform();
    s2Painter.setViewPlaneTransform(config.getViewCanvasTransform());
    neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});

    int z = neulib::iround(config.getCutDepth(ZPoint::ORIGIN));
    ZObject3dScan slice = getSlice(z);
    size_t stripeNumber = slice.getStripeNumber();
    QPen pen(getColor());
    pen.setCosmetic(false);
    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing, false);

    ZPoint center = t.getCutCenter();
    center.shiftSliceAxis(getSliceAxis());
    std::vector<QLineF> lineArray;
    std::vector<QPointF> pointArray;
    size_t stride = 1;
    for (size_t i = 0; i < stripeNumber; i += stride) {
      const ZObject3dStripe &stripe = slice.getStripe(i);
      int nseg = stripe.getSegmentNumber();
      for (int j = 0; j < nseg; ++j) {
        int x0 = stripe.getSegmentStart(j);
        int x1 = stripe.getSegmentEnd(j);
        double y = stripe.getY() - center.getY();

        for (int x = x0; x <= x1; ++x) {
//          int value = m_stackGrid.getValue(x, y, stripe.getZ());
          int value = m_stackSource->getIntValue(
                x, stripe.getY(), stripe.getZ()) % 255;
          neutu::SetPenColor(painter, QColor(value, value, value));
//          neutu::SetPenColor(painter, QColor(255, 0, 0));
#ifdef _DEBUG_0
          std::cout << "Paint point: " << x << " " << stripe.getY()
                    << " " << stripe.getZ() << " " << value << std::endl;
#endif
          s2Painter.drawPoint(painter, double(x) - center.getX(), y);
        }
      }
    }

    return s2Painter.getPaintedHint();
  }

  return false;
}

#if 0
void ZSparseObject::display(ZPainter &painter, int z, Display_Style option) const
{
  UNUSED_PARAMETER(option);
#if _QT_GUI_USED_
  z -= iround(painter.getOffset().z());

  QPen pen(m_color);
  painter.setPen(pen);

  size_t stripeNumber = m_obj.getStripeNumber();
  for (size_t i = 0; i < stripeNumber; ++i) {
    const ZObject3dStripe &stripe = m_obj.getStripe(i);
    if (stripe.getZ() == z || z < 0) {
      int nseg = stripe.getSegmentNumber();
      for (int j = 0; j < nseg; ++j) {
        int x0 = stripe.getSegmentStart(j);
        int x1 = stripe.getSegmentEnd(j);
        int y = stripe.getY();
        painter.drawLine(x0, y, x1, y);
      }
    }
  }
#else
  UNUSED_PARAMETER(&painter);
  UNUSED_PARAMETER(z);
  UNUSED_PARAMETER(option);
#endif
}
#endif
void ZSparseObject::setLabel(uint64_t label)
{
  m_uLabel = label;
  m_color = m_colorTable.getColor(label);
}

void ZSparseObject::labelStack(ZStack *stack) const
{
  int offset[3];
  offset[0] = stack->getOffset().getX();
  offset[1] = stack->getOffset().getY();
  offset[2] = stack->getOffset().getZ();

  drawStack(stack->c_stack(), getLabel(), offset);
}

#if 0
void ZSparseObject::display(ZPainter &painter, int z, EDisplayStyle option,
                            neutu::EAxis sliceAxis) const
{
  if (m_stackGrid.isEmpty() || z < 0) {
    ZObject3dScan::display(painter, z, option, sliceAxis);
  } else {
    if (sliceAxis != m_sliceAxis) {
      return;
    }

//    UNUSED_PARAMETER(option);
    z -= painter.getZOffset();
    QPen pen(m_color);
    painter.setPen(pen);

    size_t stripeNumber = getStripeNumber();
    for (size_t i = 0; i < stripeNumber; ++i) {
      const ZObject3dStripe &stripe = getStripe(i);
      if (stripe.getZ() == z) {
        int nseg = stripe.getSegmentNumber();
        for (int j = 0; j < nseg; ++j) {
          int x0 = stripe.getSegmentStart(j);
          int x1 = stripe.getSegmentEnd(j);
          int y = stripe.getY();
          for (int x = x0; x <= x1; ++x) {
            int wx = x;
            int wy = y;
            int wz = z;
            zgeom::shiftSliceAxisInverse(wx, wy, wz, m_sliceAxis);
            int v = getVoxelValue(x, y, z);
            painter.setPen(QColor(v, v, v));
            painter.drawPoint(QPoint(x, y));
          }
        }
      }
    }
  }
}
#endif

void ZSparseObject::append(const ZObject3dScan &obj)
{
  for (size_t i = 0; i < obj.getStripeNumber(); ++i) {
    m_stripeArray.push_back(obj.getStripe(i));
  }
}

int ZSparseObject::getVoxelValue(int x, int y, int z) const
{
#if defined(_USE_OPENVDB_2)
  if (m_voxelValueObject.isEmpty()) {
    return 0;
  }
  return m_voxelValueObject.getValue(x, y, z);
#else
  return m_stackSource->getIntValue(x, y, z);
//  return m_stackGrid.getValue(x, y, z);
#endif
}

#if 0
void ZSparseObject::setVoxelValue(ZStack *stack)
{
#if defined(_USE_OPENVDB_2)

#ifdef _DEBUG_2
  stack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif

  if (stack != NULL && stack->kind() == GREY) {
    int z0 = stack->getOffset().getZ();
    int z1 = z0 + stack->depth() - 1;
    int y0 = stack->getOffset().getY();
    int y1 = y0 + stack->height() - 1;
    int x0 = stack->getOffset().getX();
    int x1 = x0 + stack->width() - 1;

#ifdef _DEBUG_2
  clear();
  for (int y = y0; y <= y1; ++y) {
    addStripe(z0, y);
    addSegment(x0, x1);
  }

#endif

    size_t area = stack->width() * stack->height();
    uint8_t *array = stack->array8();
    for (size_t i = 0; i < getStripeNumber(); ++i) {
      const ZObject3dStripe &stripe = getStripe(i);
      int y = stripe.getY();
      int z = stripe.getZ();
      if (IS_IN_CLOSE_RANGE(z, z0, z1) &&
          IS_IN_CLOSE_RANGE(y, y0, y1)) {
        for (int j = 0; j < stripe.getSegmentNumber(); ++j) {
          int tx0 = imax2(x0, stripe.getSegmentStart(j));
          int tx1 = imin2(x1, stripe.getSegmentEnd(j));

          size_t offset = area * (z - z0) + stack->width() * (y - y0) +
              tx0 - x0;
          for (int x = tx0; x <= tx1; ++x) {
            m_voxelValueObject.setValue(x, y, z, array[offset++]);
          }
        }
      }
    }
    m_voxelValueObject.repack();
  }
#endif
}
#endif
