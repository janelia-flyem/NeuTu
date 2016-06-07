#include "zdvidsynapse.h"
#include <QtCore>
#include <QPen>

#include "zpainter.h"
#include "zjsonobject.h"
#include "zjsonparser.h"
#include "zjsonfactory.h"
#include "zjsonarray.h"
#include "tz_math.h"
#include "zstackball.h"
#include "c_json.h"
#include "zlinesegmentobject.h"
#include "dvid/zdvidannotation.h"

ZDvidSynapse::ZDvidSynapse()
{
  init();
}

void ZDvidSynapse::init()
{
  m_type = GetType();
  m_projectionVisible = false;
  m_kind = KIND_INVALID;
  setDefaultRadius();
  setDefaultColor();
}

double ZDvidSynapse::getConfidence() const
{
  double c = 1.0;

  if (m_propertyJson.hasKey("confidence")) {
    const char *confStr =
        ZJsonParser::stringValue(m_propertyJson["confidence"]);
    c = std::atof(confStr);
  } else if (m_propertyJson.hasKey("conf")) {
    const char *confStr =
        ZJsonParser::stringValue(m_propertyJson["conf"]);
    c = std::atof(confStr);
  }

  return c;
}

std::string ZDvidSynapse::getAnnotation() const
{
  std::string annotation;
  if (m_propertyJson.hasKey("annotation")) {
    annotation = ZJsonParser::stringValue(m_propertyJson["annotation"]);
  }

  return annotation;
}

void ZDvidSynapse::setConfidence(double c)
{
  m_propertyJson.setEntry("conf", c);
}

void ZDvidSynapse::display(ZPainter &painter, int slice, EDisplayStyle option,
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

  ZIntPoint center = m_position;
  center.shiftSliceAxis(sliceAxis);

  bool isFocused = (z == center.getZ());
  QPen pen;

  if (visible) {
    QColor color = getColor();
    double alpha = 1.0;
    if (!isFocused) {
      alpha = radius / m_radius;
      alpha *= alpha * 0.5;
      alpha += 0.1;
      color.setAlphaF(alpha * color.alphaF());
    }

    pen.setColor(color);

    painter.setPen(pen);

    painter.setBrush(Qt::NoBrush);
    if (isFocused) {
      int x = center.getX();
      int y = center.getY();
      painter.drawLine(QPointF(x - 1, y), QPointF(x + 1, y));
      painter.drawLine(QPointF(x, y - 1), QPointF(x, y + 1));
    }
    if (radius > 0.0) {
      if (getKind() == KIND_POST_SYN) {
        pen.setWidth(pen.width() + 1);
      }
      painter.setPen(pen);
      painter.drawEllipse(QPointF(center.getX(), center.getY()),
                          radius, radius);
      if (getKind() == KIND_POST_SYN) {
        pen.setWidth(pen.width() - 1);
      }
    }
    QString decorationText;

    if (!getUserName().empty()) {
      decorationText = "U";
    }


    double conf  = getConfidence();
    if (conf < 1.0) {
      decorationText += QString(".%1").arg(iround(conf * 10.0));
    }

    if (!decorationText.isEmpty()) {
      int height = iround(getRadius() * 1.5);
      int width = decorationText.size() * height;
      QFont font;
      font.setPixelSize(height);
      font.setWeight(QFont::Light);
      font.setStyleStrategy(QFont::PreferMatch);
      painter.setFont(font);

      QColor oldColor = painter.getPen().color();
      QColor color = QColor(0, 0, 0);
      color.setAlphaF(alpha);
      QPen pen = painter.getPen();
      pen.setColor(color);
      painter.setPen(pen);
      painter.drawText(center.getX() - height / 2, center.getY(), width, height,
                       Qt::AlignLeft, decorationText);
      painter.setPen(oldColor);
    }
  }


  pen.setCosmetic(m_usingCosmeticPen);

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
    double halfSize = m_radius;
    if (m_usingCosmeticPen) {
      halfSize += 0.5;
    }
    rect.setLeft(center.getX() - halfSize);
    rect.setTop(center.getY() - halfSize);
    rect.setWidth(halfSize * 2);
    rect.setHeight(halfSize * 2);

    painter.setBrush(Qt::NoBrush);
    pen.setWidthF(pen.widthF() * 0.5);
    if (visible) {
      pen.setStyle(Qt::SolidLine);
    } else {
      pen.setStyle(Qt::DotLine);
    }
    painter.setPen(pen);
    painter.drawRect(rect);
  }
  if (isSelected()) {
    for (std::vector<ZIntPoint>::const_iterator iter = m_partnerHint.begin();
         iter != m_partnerHint.end(); ++iter) {

      ZLineSegmentObject line;
      line.setStartPoint(getPosition());
      line.setEndPoint(*iter);
      line.setColor(QColor(255, 255, 0));
      line.setFocusColor(QColor(255, 0, 255));
      line.setVisualEffect(NeuTube::Display::Line::VE_LINE_PROJ);
      line.display(painter, slice, option, sliceAxis);

      /*
      ZIntPoint pos = *iter;
      painter.drawLine(getPosition().getX(), getPosition().getY(),
                       pos.getX(), pos.getY());
                       */
    }
  }
}

std::ostream& operator<< (std::ostream &stream, const ZDvidSynapse &synapse)
{
  //"Kind": (x, y, z)
  switch (synapse.getKind()) {
  case ZDvidSynapse::KIND_POST_SYN:
    stream << "PostSyn";
    break;
  case ZDvidSynapse::KIND_PRE_SYN:
    stream << "PreSyn";
    break;
  case ZDvidSynapse::KIND_INVALID:
    stream << "Invalid";
    break;
  default:
    stream << "Unknown";
    break;
  }

  stream << ": " << "(" << synapse.getPosition().getX() << ", "
         << synapse.getPosition().getY() << ", "
         << synapse.getPosition().getZ() << ")";

  return stream;
}

ZJsonObject ZDvidSynapse::makeRelJson(const ZIntPoint &pt) const
{
  std::string rel;
  switch (getKind()) {
  case ZDvidSynapse::KIND_POST_SYN:
    rel = "PostSynTo";
    break;
  case ZDvidSynapse::KIND_PRE_SYN:
    rel = "PreSynTo";
    break;
  default:
    rel = "UnknownRelationship";
  }

  return MakeRelJson(pt, rel);
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidSynapse)


