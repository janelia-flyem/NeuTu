#include "zintcuboidobj.h"

#include <QColor>
#include <QPen>
#include <QBrush>

#include "zpainter.h"
#include "geometry/zintpoint.h"
#include "geometry/zcuboid.h"
#include "data3d/displayconfig.h"
#include "vis2d/zslicepainter.h"
#include "qt/gui/utilities.h"

ZIntCuboidObj::ZIntCuboidObj()
{
  m_type = GetType();
  useCosmeticPen(true);
}

bool ZIntCuboidObj::isSliceVisible(int /*z*/, neutu::EAxis /*sliceAxis*/) const
{
  return true;
}

bool ZIntCuboidObj::isVisible_inner(const DisplayConfig &config) const
{
  if (!m_cuboid.isEmpty() && (config.hasRegularSliceAxis())) {
    return true;
  }

  return false;
}

bool ZIntCuboidObj::display_inner(
    QPainter *painter, const DisplayConfig &config) const
{
//  if (config.getSliceAxis() == neutu::EAxis::Z) {
  if (config.hasRegularSliceAxis()) {
//    neutu::SetPenColor(painter, getColor());

#ifdef _DEBUG_
    std::cout << "cosmetic: " << painter->pen().isCosmetic() << std::endl;
#endif
    ZCuboid cuboid = ZCuboid::FromIntCuboid(m_cuboid);
    ZPoint minCorner =
        config.getWorldViewTransform().transform(cuboid.getMinCorner());
    ZPoint maxCorner =
        config.getWorldViewTransform().transform(cuboid.getMaxCorner());

    Qt::PenStyle penStyle = Qt::DashLine;
    if (minCorner.getZ() * maxCorner.getZ() <= 0) {
      penStyle = Qt::SolidLine;
    }

    this->m_hitMap[config.getViewId()] =
        [=](const ZStackObject */*obj*/, double x, double y, double z) {
      ZPoint pt = config.getWorldViewTransform().transform({x, y, z});
      if (minCorner.getZ() * maxCorner.getZ() <= 0) {
        constexpr double margin = 5.0;
        return ((pt.getX() >= minCorner.getX() - margin &&
                 pt.getY() >= minCorner.getY() - margin &&
                 pt.getX() < maxCorner.getX() + margin &&
                 pt.getY() < maxCorner.getY() + margin) &&
                !(pt.getX() >= minCorner.getX() + margin &&
                  pt.getY() >= minCorner.getY() + margin &&
                  pt.getX() < maxCorner.getX() - margin &&
                  pt.getY() < maxCorner.getY() - margin));
      }
      return false;
    };

    neutu::RevisePen(painter, [&](QPen &pen) {
      pen.setStyle(penStyle);
    });

    ZSlice2dPainter s2Painter;
    s2Painter.setViewPlaneTransform(config.getViewCanvasTransform());
    s2Painter.setPixelCentered(false);
    s2Painter.drawRect(painter, minCorner.getX(), minCorner.getY(),
                       maxCorner.getX(), maxCorner.getY());
    if (isSelected()) {
      s2Painter.drawLine(
            painter, minCorner.getX(), minCorner.getY(),
            maxCorner.getX(), maxCorner.getY());
    }
    return s2Painter.getPaintedHint();
  }

  return false;
}

const ZIntPoint& ZIntCuboidObj::getFirstCorner() const
{
  return m_cuboid.getMinCorner();
}

const ZIntPoint& ZIntCuboidObj::getLastCorner() const
{
  return m_cuboid.getMaxCorner();
}

void ZIntCuboidObj::setFirstCorner(const ZIntPoint &firstCorner)
{
  m_cuboid.setMinCorner(firstCorner);
}

void ZIntCuboidObj::setLastCorner(const ZIntPoint &lastCorner)
{
  m_cuboid.setMaxCorner(lastCorner);
}

void ZIntCuboidObj::setFirstCorner(int x, int y, int z)
{
  m_cuboid.setMinCorner(x, y, z);
}

void ZIntCuboidObj::setLastCorner(int x, int y, int z)
{
  m_cuboid.setMaxCorner(x, y, z);
}

int ZIntCuboidObj::getWidth() const
{
  return m_cuboid.getWidth();
}

int ZIntCuboidObj::getHeight() const
{
  return m_cuboid.getHeight();
}

int ZIntCuboidObj::getDepth() const
{
  return m_cuboid.getDepth();
}

bool ZIntCuboidObj::isOnSlice(int z, neutu::EAxis sliceAxis) const
{
  return z >= getFirstCorner().getCoord(sliceAxis) &&
      z <= getLastCorner().getCoord(sliceAxis);
}

void ZIntCuboidObj::setGridInterval(int intv)
{
  m_gridIntv = intv;
}

#if 0
namespace {
void NextMark(int &x, int intv, int &remain)
{
  x += intv;
  if (remain > 0) {
    ++x;
    --remain;
  }
}
}

void ZIntCuboidObj::display(
    ZPainter &painter, int slice, EDisplayStyle /*option*/,
    neutu::EAxis sliceAxis) const
{
  if (m_cuboid.isEmpty()) {
    return;
  }

  int z = painter.getZ(slice);

  if (!(isSliceVisible(z, sliceAxis) || (slice < 0))) {
    return;
  }

  QColor color = m_color;
  QPen pen(color);

  pen.setWidthF(getPenWidth());

  if(isOnSlice(z, sliceAxis) || slice < 0) {
    if (isSelected()) {
      pen.setStyle(Qt::DashLine);
    } else {
      pen.setStyle(Qt::SolidLine);
    }
  } else {
    pen.setStyle(Qt::DotLine);
  }

  if (isSelected()) {
    pen.setWidth(pen.width() + 5);
  }

  pen.setCosmetic(m_usingCosmeticPen);

  painter.setPen(pen);
  painter.setBrush(Qt::NoBrush);

  ZIntPoint firstCorner = getFirstCorner();
  ZIntPoint lastCorner = getLastCorner();

  firstCorner.shiftSliceAxis(sliceAxis);
  lastCorner.shiftSliceAxis(sliceAxis);
  painter.drawRect(firstCorner.getX(), firstCorner.getY(),
                   lastCorner.getX() - firstCorner.getX() + 1,
                   lastCorner.getY() - firstCorner.getY() + 1);

//  pen.setWidthF(getDefaultPenWidth());
//  painter.setPen(pen);

  if (hasVisualEffect(neutu::display::Box::VE_GRID)) {
    const int gridIntervalHint = m_gridIntv;

    int x1 = firstCorner.getX();
    int y1 = firstCorner.getY();
    int x2 = lastCorner.getX() + 1;
    int y2 = lastCorner.getY() + 1;

    int colCount = getWidth() / gridIntervalHint;
    if (colCount > 1) {
      int colInterval = getWidth() / colCount;
      int colRemain = getWidth() % colCount;
      int x = x1;
      for (int i = 1; i < colCount; ++i) {
        NextMark(x, colInterval, colRemain);
        painter.drawLine(x, y1, x, y2);
      }
    }

    int rowCount = getHeight() / gridIntervalHint;
    if (rowCount > 1) {
      int rowInterval = getHeight() / rowCount;
      int rowRemain = getHeight() % rowCount;
      int y = y1;
      for (int i = 1; i < rowCount; ++i) {
        NextMark(y, rowInterval, rowRemain);
        painter.drawLine(x1, y, x2, y);
      }
    }
  }
}
#endif

bool ZIntCuboidObj::hit(double x, double y, neutu::EAxis axis)
{
  ZIntPoint firstCorner = getFirstCorner();
  ZIntPoint lastCorner = getLastCorner();

  firstCorner.shiftSliceAxis(axis);
  lastCorner.shiftSliceAxis(axis);

  return ((x >= firstCorner.getX() - 5 && y >= firstCorner.getY() - 5 &&
           x < lastCorner.getX() + 5 && y < lastCorner.getY() + 5) &&
          !(x >= firstCorner.getX() + 5 && y >= firstCorner.getY() + 5 &&
            x < lastCorner.getX() - 5 && y < lastCorner.getY() - 5));
}

/*
bool ZIntCuboidObj::hit(double x, double y, double z)
{
  if (isOnSlice(z, neutu::EAxis::Z)) {
    return hit(x, y, neutu::EAxis::Z);
  }

  return false;
}
*/

void ZIntCuboidObj::clear()
{
  m_cuboid.set(0, 0, 0, -1, -1, -1);
}

bool ZIntCuboidObj::isValid() const
{
  return !m_cuboid.isEmpty();
}

void ZIntCuboidObj::join(const ZIntCuboid &cuboid)
{
  m_cuboid.join(cuboid);
}

void ZIntCuboidObj::boundBox(ZIntCuboid *box) const
{
  if (box != NULL) {
    *box = getCuboid();
  }
}

ZCuboid ZIntCuboidObj::getBoundBox() const
{
  return ZCuboid::FromIntCuboid(m_cuboid);
}

//ZSTACKOBJECT_DEFINE_CLASS_NAME(ZIntCuboidObj)
