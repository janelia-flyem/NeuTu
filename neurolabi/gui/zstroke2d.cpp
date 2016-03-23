#include "zstroke2d.h"

#include <QPainter>
#include <QPen>
#include <QBitmap>
#include "neutubeconfig.h"
#include "tz_math.h"
#include "zintpoint.h"
#include "zstack.hxx"
#include "zobject3d.h"
#include "zjsonobject.h"
#include "tz_geometry.h"
#include "zpainter.h"
#include "geometry/zgeometry.h"

const double ZStroke2d::m_minWidth = 1.0;
const double ZStroke2d::m_maxWidth = 100.0;
//const QVector<QColor> ZStroke2d::m_colorTable = ZStroke2d::constructColorTable();
//const QColor ZStroke2d::m_blackColor = Qt::black;
const ZLabelColorTable ZStroke2d::m_colorTable;

ZStroke2d::ZStroke2d() :
  m_width(10.0), m_z(-1), m_isFilled(true), m_hideStart(false),
  m_isPenetrating(true)
{
  setLabel(1);
  m_type = ZStackObject::TYPE_STROKE;
  setSliceAxis(NeuTube::Z_AXIS);
  //setEraser(m_isEraser);
}

ZStroke2d::ZStroke2d(const ZStroke2d &stroke) : ZStackObject(stroke)
{
  m_pointArray = stroke.m_pointArray;
  m_width = stroke.m_width;
  m_label = stroke.m_label;
  m_originalLabel = stroke.m_originalLabel;
//  setLabel(stroke.m_label);
  //m_label = stroke.m_label;
  m_z = stroke.m_z;
  //m_isEraser = stroke.m_isEraser;
  m_isFilled = stroke.m_isFilled;
  m_isPenetrating = stroke.m_isPenetrating;
  m_type = stroke.m_type;
  m_hideStart = stroke.m_hideStart;
  m_sliceAxis = stroke.m_sliceAxis;
}

ZStroke2d::~ZStroke2d()
{
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZStroke2d)

void ZStroke2d::save(const char */*filePath*/)
{

}

bool ZStroke2d::load(const char */*filePath*/)
{
  return false;
}

void ZStroke2d::append(double x, double y)
{
  if (m_pointArray.empty()) {
    m_pointArray.push_back(QPointF(x, y));
  } else {
    if (x != m_pointArray.back().x() || y != m_pointArray.back().y()) {
      m_pointArray.push_back(QPointF(x, y));
    }
  }
}

void ZStroke2d::set(const QPoint &pt)
{
  set(pt.x(), pt.y());
}

void ZStroke2d::set(double x, double y)
{
  clear();
  append(x, y);
}

void ZStroke2d::setLast(double x, double y)
{
  if (isEmpty()) {
    append(x, y);
  } else {
    QPointF &pt = m_pointArray[m_pointArray.size() - 1];
    pt.setX(x);
    pt.setY(y);
  }
}

void ZStroke2d::setLabel(int label)
{
  m_label = label;
  m_originalLabel = label;
  m_color = getLabelColor();
}

void ZStroke2d::toggleLabel(bool toggling)
{
  if (toggling) {
    if (m_label == m_originalLabel) {
      m_label++;
    }
  } else {
    m_label = m_originalLabel;
  }

  m_color = getLabelColor();
}

int ZStroke2d::getLabel() const
{
  return m_label;
}

void ZStroke2d::setEraser(bool enabled)
{
  if (enabled) {
    setLabel(0);
  } else if (m_label == 0) {
    setLabel(1);
  }
  /*
  if (enabled) {
    m_label = 0;
  } else {
    if (m_label == 0) {
      m_label = 1;
    }
  }
 // m_isEraser = isEraser;
  if (isEraser()) {
    m_color.setRgb(255, 255, 255);
  } else {
    m_color.setRgb(255, 0, 0);
  }
  m_color.setAlpha(128);
  */
}

void ZStroke2d::display(ZPainter &painter, int slice, EDisplayStyle option,
                        NeuTube::EAxis sliceAxis) const
{
  if (sliceAxis != getSliceAxis()) {
    return;
  }

  //UNUSED_PARAMETER(z);
  UNUSED_PARAMETER(option);

  int z = slice + iround(painter.getZOffset());

  if (!(isSliceVisible(z, sliceAxis) || (slice < 0))) {
    return;
  }

  painter.save();

  QColor color = m_color;
//  if (m_z >= 0 && m_z != z) {
//    if (isEraser()) {
//      return;
//    }
//    //color.setAlphaF(color.alphaF() / (1.2 + abs(m_z - z) / 5.0));
//    return;
//  }
  QPen pen(color);
  QBrush brush(color);

  pen.setCosmetic(m_usingCosmeticPen);

  if (isEraser()) {
    painter.setCompositionMode(QPainter::CompositionMode_Source);
  } else {
    painter.setCompositionMode(QPainter::CompositionMode_Source);
  }

  double radius = m_width * 0.5;
  if (!m_isFilled) {
    radius += getPenWidth() * 0.5;
  }

  if (!m_pointArray.empty()) {
    if (m_pointArray.size() == 1) {
      if (m_isFilled) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(brush);
      } else {
        pen.setWidthF(getPenWidth());
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
      }
      painter.drawEllipse(QPointF(m_pointArray[0]), radius, radius);
    } else {

      if (m_isFilled) {
        pen.setCapStyle(Qt::RoundCap);
        pen.setWidthF(m_width);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.setOpacity(1.0);
        painter.drawPolyline(&(m_pointArray[0]), m_pointArray.size());
      } else {
        pen.setWidthF(getPenWidth());
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

        for (size_t i =0; i < m_pointArray.size(); ++i) {
          if (i == 0) {
            if (!m_hideStart) {
              painter.drawEllipse(
                    QPointF(m_pointArray[i]), radius, radius);
            }
          } else {
            painter.drawEllipse(
                  QPointF(m_pointArray[i]), radius, radius);
            painter.drawLine(m_pointArray[i-1], m_pointArray[i]);
          }
        }
      }
    }

    if (isSelected()) {
      painter.setPen(QColor(255, 255, 0));
      double radius = m_width / 3.0;
      painter.drawEllipse(m_pointArray.front(), radius, radius);
      if (m_pointArray.size() > 1) {
        painter.drawEllipse(m_pointArray.back(), radius, radius);
        painter.drawPolyline(&(m_pointArray[0]), m_pointArray.size());
      }
    }
  }

  painter.restore();
}

bool ZStroke2d::display(QPainter *rawPainter, int z, EDisplayStyle option,
                        EDisplaySliceMode sliceMode, NeuTube::EAxis sliceAxis) const
{
  if (sliceAxis != getSliceAxis()) {
    return false;
  }

  //UNUSED_PARAMETER(z);
  UNUSED_PARAMETER(option);

  bool painted = false;

  if (rawPainter == NULL || !isVisible()) {
    return painted;
  }

#ifdef _DEBUG_2
  std::cout << "Draw stroke" << std::endl;
#endif

  QPainter &painter = *rawPainter;

  //z -= iround(painter.getOffset().z());

  QColor color = m_color;
  if (sliceMode == DISPLAY_SLICE_SINGLE && m_z != z) {
    if (isEraser()) {
      return painted;
    }
    color.setAlphaF(color.alphaF() / (1.2 + abs(m_z - z) / 5.0));
  }

  QPen pen(color);
  QBrush brush(color);

  if (isEraser()) {
    painter.setCompositionMode(QPainter::CompositionMode_Source);
  } else {
    painter.setCompositionMode(QPainter::CompositionMode_Source);
  }

  if (!m_pointArray.empty()) {
    if (m_pointArray.size() == 1) {
      if (m_isFilled) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(brush);
      } else {
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
      }
      painter.drawEllipse(QPointF(m_pointArray[0]), m_width / 2, m_width / 2);
      painted = true;

#ifdef _DEBUG_
      std::cout << "Paint stroke: " << m_width << std::endl;
      qDebug() << brush;
#endif
    } else {
      pen.setCapStyle(Qt::RoundCap);
      pen.setWidthF(m_width);
      painter.setPen(pen);
      painter.setBrush(Qt::NoBrush);
      painter.setOpacity(1.0);
      painter.drawPolyline(&(m_pointArray[0]), m_pointArray.size());
      painted = true;
    }
  }

  return painted;
}

void ZStroke2d::labelImage(QImage *image) const
{
  if (image != NULL) {
    image->fill(Qt::black);
    QPen pen(Qt::white);
    QBrush brush(Qt::white);
    QPainter painter(image);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    if (!m_pointArray.empty()) {
      if (m_pointArray.size() == 1) {
        if (m_isFilled) {
          painter.setPen(Qt::NoPen);
          painter.setBrush(brush);
        } else {
          painter.setPen(pen);
          painter.setBrush(Qt::NoBrush);
        }
        painter.drawEllipse(QPointF(m_pointArray[0]), m_width / 2, m_width / 2);
      } else {
        pen.setCapStyle(Qt::RoundCap);
        pen.setWidthF(m_width);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.setOpacity(1.0);
        painter.drawPolyline(&(m_pointArray[0]), m_pointArray.size());
      }
    }
  }
}

void ZStroke2d::labelGrey(Stack *stack, int label) const
{
  if (stack == NULL || C_Stack::kind(stack) != GREY ||
      m_z < 0 || m_z >= C_Stack::depth(stack)) {
    return;
  }

  CLIP_VALUE(label, 0, 255);

  //QBitmap image(C_Stack::width(stack), C_Stack::height(stack));
  QImage image(C_Stack::width(stack), C_Stack::height(stack),
               QImage::Format_RGB16);
  labelImage(&image);

  uint8_t* array = C_Stack::array8(stack);
  size_t area = C_Stack::width(stack) * C_Stack::height(stack);
  size_t offset = area * m_z;
  for (int j = 0; j < image.height(); ++j) {
    for (int i = 0; i < image.width(); ++i) {
      QRgb color = image.pixel(i, j);
      if (qRed(color) > 0 || qGreen(color) > 0 || qBlue(color) > 0) {
        array[offset] = label;
      }
      offset++;
    }
  }
}

void ZStroke2d::labelGrey(Stack *stack) const
{
  labelGrey(stack, m_label);
}

void ZStroke2d::labelBinary(Stack *stack) const
{
  labelGrey(stack, 1);

#ifdef _DEBUG_2
  image.save((GET_DATA_DIR + "/test.tif").c_str());
#endif

#if 0
  QColor color = m_color;
  if (m_z >= 0 && m_z != z) {
    if (isEraser()) {
      return;
    }
    color.setAlpha(color.alpha() / 2);
  }
  QPen pen(color);
  QBrush brush(color);

  if (isEraser()) {
    painter.setCompositionMode(QPainter::CompositionMode_Source);
  } else {
    painter.setCompositionMode(QPainter::CompositionMode_Source);
  }

  if (!m_pointArray.empty()) {
    if (m_pointArray.size() == 1) {
      if (m_isFilled) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(brush);
      } else {
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
      }
      painter.drawEllipse(QPointF(m_pointArray[0]), m_width / 2, m_width / 2);
    } else {
      pen.setCapStyle(Qt::RoundCap);
      pen.setWidthF(m_width);
      painter.setPen(pen);
      painter.setBrush(Qt::NoBrush);
      painter.setOpacity(1.0);
      painter.drawPolyline(&(m_pointArray[0]), m_pointArray.size());

      /*
        painter.setPen(Qt::NoPen);
        painter.setBrush(brush);
        painter.drawEllipse(QPointF(m_pointArray.back()), m_width / 2, m_width / 2);
        */
    }
  }
#endif
}

void ZStroke2d::clear()
{
  m_pointArray.clear();
}

bool ZStroke2d::isEmpty() const
{
  return m_pointArray.empty();
}

ZStroke2d* ZStroke2d::clone()
{
  ZStroke2d *stroke = new ZStroke2d(*this);

  return stroke;
}


void ZStroke2d::addWidth(double dw)
{
  m_width += dw;
  if (m_width < m_minWidth) {
    m_width = m_minWidth;
  } else if (m_width > m_maxWidth) {
    m_width = m_maxWidth;
  }
}

bool ZStroke2d::getLastPoint(int *x, int *y) const
{
  if (m_pointArray.empty()) {
    return false;
  }

  *x = iround(m_pointArray.back().x());
  *y = iround(m_pointArray.back().y());

  return true;
}

bool ZStroke2d::getLastPoint(double *x, double *y) const
{
  if (m_pointArray.empty()) {
    return false;
  }

  *x = m_pointArray.back().x();
  *y = m_pointArray.back().y();

  return true;
}

bool ZStroke2d::getPoint(double *x, double *y, size_t index) const
{
  if (index >= m_pointArray.size()) {
    return false;
  }

  *x = m_pointArray[index].x();
  *y = m_pointArray[index].y();

  return true;
}


void ZStroke2d::print() const
{
  foreach (QPointF point, m_pointArray) {
    std::cout << point.x() << " " << point.y() << std::endl;
  }
}

const QColor& ZStroke2d::getLabelColor() const
{
  return m_colorTable.getColor(m_label);

#if 0
  if (m_label == 255) {
    return m_blackColor;
  }

  int index = m_label % m_colorTable.size();
  return m_colorTable[index];
#endif
}

void ZStroke2d::translate(const ZPoint &offset)
{
  for (std::vector<QPointF>::iterator iter = m_pointArray.begin();
       iter != m_pointArray.end(); ++iter) {
    QPointF &pt = *iter;
    pt += QPointF(offset.x(), offset.y());
  }
  m_z += iround(offset.z());
}

void ZStroke2d::translate(const ZIntPoint &offset)
{
  for (std::vector<QPointF>::iterator iter = m_pointArray.begin();
       iter != m_pointArray.end(); ++iter) {
    QPointF &pt = *iter;
    pt += QPointF(offset.getX(), offset.getY());
  }
  m_z += iround(offset.getZ());
}


ZObject3d* ZStroke2d::toObject3d() const
{
  ZObject3d *obj = NULL;
  ZStack *stack = toStack();
  if (stack != NULL) {
    obj = new ZObject3d;
    obj->loadStack(stack);
    obj->setLabel(getLabel());
  }

  return obj;
}

ZCuboid ZStroke2d::getBoundBox() const
{
  ZCuboid box;
  if (!isEmpty()) {
    double x0 = m_pointArray[0].x();
    double y0 = m_pointArray[0].y();
    double x1 = x0;
    double y1 = y0;

    double r = m_width / 2 + 1.0;

    foreach(const QPointF &pt, m_pointArray) {
      if (x0 > pt.x()) {
        x0 = pt.x();
      }
      if (x1 < pt.x()) {
        x1 = pt.x();
      }
      if (y0 > pt.y()) {
        y0 = pt.y();
      }
      if (y1 < pt.y()) {
        y1 = pt.y();
      }
    }

    box.set(x0 - r, y0 - r, m_z, x1 + r, y1 + r, m_z + 1.0);
  }
  return box;
}

ZStack* ZStroke2d::toStack() const
{
  if (isEmpty()) {
    return NULL;

  }
  const static int margin = 5;

  //Estimate bound box
  double x0 = m_pointArray[0].x();
  double y0 = m_pointArray[0].y();
  double x1 = x0;
  double y1 = y0;

  foreach(const QPointF &pt, m_pointArray) {
    if (x0 > pt.x()) {
      x0 = pt.x();
    }
    if (x1 < pt.x()) {
      x1 = pt.x();
    }
    if (y0 > pt.y()) {
      y0 = pt.y();
    }
    if (y1 < pt.y()) {
      y1 = pt.y();
    }
  }

  Cuboid_I boundBox;
  int r = iround(m_width / 2);
  boundBox.cb[0] = iround(x0) - margin - r;
  boundBox.cb[1] = iround(y0) - margin - r;
  boundBox.cb[2] = m_z;
  boundBox.ce[0] = iround(x1) + margin + r;
  boundBox.ce[1] = iround(y1) + margin + r;
  boundBox.ce[2] = m_z;

  int width = Cuboid_I_Width(&boundBox);
  int height = Cuboid_I_Height(&boundBox);
  int depth = Cuboid_I_Depth(&boundBox);

  ZStack *stack = new ZStack(GREY, width, height, depth, 1);
  stack->setOffset(boundBox.cb[0], boundBox.cb[1], boundBox.cb[2]);
  ZStroke2d tmpStroke = *this;
  tmpStroke.translate(-stack->getOffset());
  tmpStroke.labelGrey(stack->c_stack());

  return stack;
}

void ZStroke2d::labelStack(ZStack *stack) const
{
  if (stack != NULL) {
    ZStroke2d tmpStroke = *this;
    tmpStroke.translate(-stack->getOffset());
    tmpStroke.labelGrey(stack->c_stack());
  }
}

ZJsonObject ZStroke2d::toJsonObject() const
{
  ZJsonObject obj;

  obj.setEntry("label", m_label);
  obj.setEntry("width", m_width);
  obj.setEntry("z", m_z);
  obj.setEntry("z_order", m_zOrder);

  ZJsonArray arrayJson;
  for (std::vector<QPointF>::const_iterator iter = m_pointArray.begin();
       iter != m_pointArray.end(); ++iter) {
    const QPointF &pt = *iter;
    ZJsonArray ptJson;
    ptJson.append(pt.x());
    ptJson.append(pt.y());

    arrayJson.append(ptJson);
  }

  obj.setEntry("stroke", arrayJson);

  return obj;
}

void ZStroke2d::loadJsonObject(const ZJsonObject &obj)
{
  clear();
  if (obj.hasKey("label")) {
    setLabel(ZJsonParser::integerValue(obj["label"]));
  }
  if (obj.hasKey("width")) {
    setWidth(ZJsonParser::numberValue(obj["width"]));
  }
  if (obj.hasKey("z")) {
    setZ(ZJsonParser::integerValue(obj["z"]));
  }

  if (obj.hasKey("z_order")) {
    setZOrder(ZJsonParser::integerValue(obj["z_order"]));
  }

  if (obj.hasKey("stroke")) {
    ZJsonArray ptArray(obj["stroke"], ZJsonValue::SET_INCREASE_REF_COUNT);
    for (size_t i = 0; i < ptArray.size(); ++i) {
      ZJsonArray ptJson(ptArray.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
      double x = ZJsonParser::numberValue(ptJson.at(0));
      double y = ZJsonParser::numberValue(ptJson.at(1));
      append(x, y);
    }
  }
}

bool ZStroke2d::isSliceVisible(int z, NeuTube::EAxis sliceAxis) const
{
  if (isVisible() && !isEmpty() && (sliceAxis == getSliceAxis())) {
    if (m_isPenetrating || m_z == z) {
      return true;
    }
  }

  return false;
}

bool ZStroke2d::hitTest(double x, double y, NeuTube::EAxis axis) const
{
  if (axis != getSliceAxis()) {
    return false;
  }

  bool hit = false;

  for (std::vector<QPointF>::const_iterator iter = m_pointArray.begin();
       iter != m_pointArray.end(); ++iter) {
    const QPointF &pt = *iter;
    double dx = pt.x() - x;
    double dy = pt.y() - y;
    double dist = sqrt(dx * dx + dy * dy);
    if (dist * 2 < m_width) {
      hit = true;
      break;
    }
  }

  if (!hit) {
    for (size_t i = 1; i < m_pointArray.size(); ++i) {
      const QPointF &pt1 = m_pointArray[i - 1];
      const QPointF &pt2 = m_pointArray[i];

      double dx = pt2.x() - pt1.x();
      double dy = pt2.y() - pt1.y();

      double length = sqrt(dx * dx + dy * dy);

      if (length > 1.0) {
        double tx = x;
        double ty = y;
        tx -= pt1.x();
        ty -= pt1.y();
        double angle = -Vector_Angle(dx, dy);
        double cosAngle = cos(angle);
        double sinAngle = sin(angle);

        double tmpX = tx;
        tx = cosAngle * tx - sinAngle * ty;
        ty = sinAngle * tmpX + cosAngle * ty;

        /*
        QTransform transform;
        transform.translate(-pt1.x(), -pt1.y());
        transform.rotateRadians(-Vector_Angle(dx, dy));

        qreal tx = 0;
        qreal ty = 0;
        transform.map(x, y, &tx, &ty);
        */

        if (tx >= 0 && tx <= length && fabs(ty) <= m_width * 0.5) {
          hit = true;
          break;
        }
      }
    }
  }

  return hit;
}

bool ZStroke2d::hitTest(double x, double y, double z) const
{
  bool hit = false;

  ZGeometry::shiftSliceAxis(x, y, z, getSliceAxis());

  if (iround(z) == m_z) {
    hit = hitTest(x, y, getSliceAxis());
  }

  return hit;
}

bool ZStroke2d::hit(double x, double y, NeuTube::EAxis axis)
{
  return hitTest(x, y, axis);
}

bool ZStroke2d::hit(double x, double y, double z)
{
  return hitTest(x, y, z);
}

void ZStroke2d::getBoundBox(ZIntCuboid *box) const
{
  if (box != NULL) {
    ZCuboid cuboid = getBoundBox();
    box->setFirstCorner(cuboid.firstCorner().toIntPoint());
    box->setLastCorner(cuboid.lastCorner().toIntPoint());
  }
}
