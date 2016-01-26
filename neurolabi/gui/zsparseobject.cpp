#include "zsparseobject.h"
#include <QPen>
#include "zpainter.h"
#include "tz_math.h"
#include "zstack.hxx"
#include "neutubeconfig.h"

const ZLabelColorTable ZSparseObject::m_colorTable;

ZINTERFACE_DEFINE_CLASS_NAME(ZSparseObject)

ZSparseObject::ZSparseObject()
{
  setLabel(-1);
  m_type = ZStackObject::TYPE_SPARSE_OBJECT;
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
void ZSparseObject::setLabel(int label)
{
  m_label = label;
  m_color = m_colorTable.getColor(label);
}

void ZSparseObject::labelStack(ZStack *stack) const
{
  int offset[3];
  offset[0] = stack->getOffset().getX();
  offset[1] = stack->getOffset().getY();
  offset[2] = stack->getOffset().getZ();

  drawStack(stack->c_stack(), m_label, offset);
}

void ZSparseObject::display(ZPainter &painter, int z, EDisplayStyle option,
                            NeuTube::EAxis sliceAxis) const
{
  if (m_stackGrid.isEmpty() || z < 0) {
    ZObject3dScan::display(painter, z, option, sliceAxis);
  } else {
    if (sliceAxis != m_sliceAxis) {
      return;
    }

    UNUSED_PARAMETER(option);
    z -= iround(painter.getZOffset());
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
            ZGeometry::shiftSliceAxisInverse(wx, wy, wz, m_sliceAxis);
            int v = getVoxelValue(x, y, z);
            painter.setPen(QColor(v, v, v));
            painter.drawPoint(QPoint(x, y));
          }
        }
      }
    }
  }
}

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
  return m_stackGrid.getValue(x, y, z);
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
