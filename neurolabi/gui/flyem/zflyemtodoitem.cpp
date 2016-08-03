#include "zflyemtodoitem.h"

#include <QColor>
#include <iostream>

#include "zpainter.h"
#include "zjsonparser.h"
#include "flyem/zflyemmisc.h"

ZFlyEmToDoItem::ZFlyEmToDoItem()
{
  init(KIND_INVALID);
}

void ZFlyEmToDoItem::init(EKind kind)
{
  m_type = GetType();
  setKind(kind);
  setDefaultColor();
  setDefaultRadius();
  useCosmeticPen(true);
  setBasePenWidth(2.0);
}

ZFlyEmToDoItem::ZFlyEmToDoItem(const ZIntPoint &pos)
{
  init(KIND_NOTE);
  setPosition(pos);
}


std::ostream& operator<< (std::ostream &stream, const ZFlyEmToDoItem &item)
{
  //"Kind": (x, y, z)
  switch (item.getKind()) {
  case ZDvidAnnotation::KIND_POST_SYN:
    stream << "PostSyn";
    break;
  case ZDvidAnnotation::KIND_PRE_SYN:
    stream << "PreSyn";
    break;
  case ZDvidAnnotation::KIND_NOTE:
    stream << "Note";
    break;
  case ZDvidAnnotation::KIND_INVALID:
    stream << "Invalid";
    break;
  default:
    stream << "Unknown";
    break;
  }

  stream << ": " << "(" << item.getPosition().getX() << ", "
         << item.getPosition().getY() << ", "
         << item.getPosition().getZ() << ")";

  return stream;
}

QColor ZFlyEmToDoItem::getDisplayColor() const
{
  QColor color = getColor();
  if (!isChecked()) {
    color.setRgb(255, 0, 0, 192);
  }

  return color;
}

void ZFlyEmToDoItem::display(ZPainter &painter, int slice, EDisplayStyle /*option*/,
                           NeuTube::EAxis sliceAxis) const
{
  bool visible = true;
  int z = painter.getZ(slice);

  if (slice < 0) {
    visible = isProjectionVisible();
  } else {
    visible = isSliceVisible(z, sliceAxis);
  }

  double radius = getRadius(z, sliceAxis);

  ZIntPoint center = getPosition();
  center.shiftSliceAxis(sliceAxis);

  bool isFocused = (z == center.getZ());

  double basePenWidth = getBasePenWidth();

  QPen pen = painter.getPen();
  pen.setCosmetic(m_usingCosmeticPen);

  if (visible) {
    QColor color = getDisplayColor();
    if (!isFocused) {
      double alpha = radius / getRadius();
      alpha *= alpha * 0.5;
      alpha += 0.1;
      color.setAlphaF(alpha * color.alphaF());
    }

    pen.setColor(color);
    painter.setPen(pen);
//    painter.setPen(color);
    painter.setBrush(Qt::NoBrush);

//    if (isFocused) {

//    }
    if (radius > 0.0) {
      int x = center.getX();
      int y = center.getY();
      pen.setWidthF(basePenWidth * 0.5);
      painter.setPen(pen);

      painter.drawLine(QPointF(x - radius, y), QPointF(x + radius, y));
      painter.drawLine(QPointF(x, y - radius), QPointF(x, y + radius));
//      painter.drawLine(QPointF(x - radius, y - radius),
//                       QPointF(x + radius, y + radius));
//      painter.drawLine(QPointF(x - radius, y + radius),
//                       QPointF(x + radius, y - radius));

      pen.setWidthF(basePenWidth);
      painter.setPen(pen);
      QPointF ptArray[9];
      ZFlyEmMisc::MakeStar(QPointF(x, y), radius, ptArray);
      painter.drawPolyline(ptArray, 9);
      /*
      painter.drawEllipse(QPointF(center.getX(), center.getY()),
                          radius, radius);
                          */
    }

    /*
    QString decorationText = "*";
    int width = decorationText.size() * 25;
    int height = 25;
    QColor oldColor = painter.getPen().color();
    painter.setPen(QColor(0, 0, 0, 128));
    painter.drawText(center.getX() - width / 2, center.getY() - height / 2,
                     width, height,
                     Qt::AlignCenter, decorationText);
    painter.setPen(oldColor);
    */
  }

  bool drawingBoundBox = false;

  if (isSelected()) {
    drawingBoundBox = true;
    QColor color;
    color.setRgb(255, 255, 0);
    pen.setColor(color);
    pen.setCosmetic(true);
  } else if (hasVisualEffect(NeuTube::Display::Sphere::VE_BOUND_BOX)) {
    drawingBoundBox = true;
    pen.setStyle(Qt::SolidLine);
    pen.setCosmetic(m_usingCosmeticPen);
  }

  if (drawingBoundBox) {
    QRectF rect;
    double halfSize = getRadius();
    if (m_usingCosmeticPen) {
      halfSize += 0.5;
    }
    rect.setLeft(center.getX() - halfSize);
    rect.setTop(center.getY() - halfSize);
    rect.setWidth(halfSize * 2);
    rect.setHeight(halfSize * 2);

    painter.setBrush(Qt::NoBrush);
    pen.setWidthF(basePenWidth * 0.5);
    if (visible) {
      pen.setStyle(Qt::SolidLine);
    } else {
      pen.setStyle(Qt::DotLine);
    }
    painter.setPen(pen);
    painter.drawRect(rect);
  }
}


bool ZFlyEmToDoItem::isChecked() const
{
  if (m_propertyJson.hasKey("checked")) {
    return std::string(ZJsonParser::stringValue(m_propertyJson["checked"])) == "1";
  }

  return false;
}

void ZFlyEmToDoItem::setChecked(bool checked)
{
  std::string checkedStr = "0";
  if (checked) {
    checkedStr = "1";
  }

  m_propertyJson.setEntry("checked", checkedStr);
}


ZSTACKOBJECT_DEFINE_CLASS_NAME(ZFlyEmToDoItem)
