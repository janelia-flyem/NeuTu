#include "zstroke2d.h"

#include <QPainter>
#include <QPen>
#include <QBitmap>
#include "neutubeconfig.h"
#include "tz_math.h"

const double ZStroke2d::m_minWidth = 1.0;
const double ZStroke2d::m_maxWidth = 100.0;
const QVector<QColor> ZStroke2d::m_colorTable = ZStroke2d::constructColorTable();
const QColor ZStroke2d::m_blackColor = Qt::black;

QVector<QColor> ZStroke2d::constructColorTable()
{
  QVector<QColor> colorTable(7);

  colorTable[0] = QColor(Qt::white);
  colorTable[1] = QColor(Qt::red);
  colorTable[2] = QColor(Qt::green);
  colorTable[3] = QColor(Qt::blue);
  colorTable[4] = QColor(Qt::cyan);
  colorTable[5] = QColor(Qt::magenta);
  colorTable[6] = QColor(Qt::yellow);


  for (QVector<QColor>::iterator iter = colorTable.begin();
       iter != colorTable.end(); ++iter) {
    iter->setAlpha(128);
  }

  return colorTable;
}

ZStroke2d::ZStroke2d() : m_width(10.0), m_z(-1), m_isFilled(true)
{
  setLabel(1);
  //setEraser(m_isEraser);
}

ZStroke2d::ZStroke2d(const ZStroke2d &stroke) :
  ZInterface(stroke), ZDocumentable(stroke), ZStackDrawable(stroke)
{
  m_pointArray = stroke.m_pointArray;
  m_width = stroke.m_width;
  m_label = stroke.m_label;
  m_z = stroke.m_z;
  //m_isEraser = stroke.m_isEraser;
  m_isFilled = stroke.m_isFilled;
}

ZStroke2d::~ZStroke2d()
{
}

ZINTERFACE_DEFINE_CLASS_NAME(ZStroke2d)

void ZStroke2d::save(const char */*filePath*/)
{

}

void ZStroke2d::load(const char */*filePath*/)
{

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

void ZStroke2d::setLabel(int label)
{
  m_label = label;
  m_color = getLabelColor();
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

void ZStroke2d::display(ZPainter &painter, int z, Display_Style option) const
{
  //UNUSED_PARAMETER(z);
  UNUSED_PARAMETER(option);

  z -= iround(painter.getOffset().z());

  QColor color = m_color;
  if (m_z >= 0 && m_z != z) {
    if (isEraser()) {
      return;
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

bool ZStroke2d::isEmpty()
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

void ZStroke2d::print() const
{
  foreach (QPointF point, m_pointArray) {
    std::cout << point.x() << " " << point.y() << std::endl;
  }
}

const QColor& ZStroke2d::getLabelColor() const
{
  if (m_label == 255) {
    return m_blackColor;
  }

  int index = m_label % m_colorTable.size();
  return m_colorTable[index];
}

void ZStroke2d::translate(const ZPoint offset)
{
  for (std::vector<QPointF>::iterator iter = m_pointArray.begin();
       iter != m_pointArray.end(); ++iter) {
    QPointF &pt = *iter;
    pt += QPointF(offset.x(), offset.y());
  }
  m_z += iround(offset.z());
}
