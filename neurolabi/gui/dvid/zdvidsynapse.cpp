#include "zdvidsynapse.h"
#include <QtCore>
#include "zpainter.h"
#include "zjsonobject.h"
#include "zjsonparser.h"

ZDvidSynapse::ZDvidSynapse()
{
  init();
}

void ZDvidSynapse::init()
{
  m_projectionVisible = false;
  m_kind = KIND_UNKNOWN;
  setDefaultRadius();
}

void ZDvidSynapse::display(
    ZPainter &painter, int slice, EDisplayStyle /*option*/) const
{
  bool visible = true;

  if (slice < 0) {
    visible = isProjectionVisible();
  } else {
    int z = painter.getZ(slice);
    visible = (z == m_position.getZ());
  }

  if (visible) {
    painter.setPen(getColor());
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(QPointF(m_position.getX(), m_position.getY()),
                        m_radius, m_radius);
  }

  QPen pen = painter.getPen();

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
    rect.setLeft(m_position.getX() - halfSize);
    rect.setTop(m_position.getY() - halfSize);
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
}

void ZDvidSynapse::setPosition(const ZIntPoint &pos)
{
  m_position = pos;
}

void ZDvidSynapse::setPosition(int x, int y, int z)
{
  m_position.set(x, y, z);
}

void ZDvidSynapse::setDefaultRadius()
{
  switch (m_kind) {
  case KIND_POST_SYN:
    setRadius(3.0);
    break;
  case KIND_PRE_SYN:
    setRadius(5.0);
    break;
  default:
    setRadius(5.0);
    break;
  }
}

void ZDvidSynapse::setDefaultColor()
{
  switch (m_kind) {
  case KIND_POST_SYN:
    setColor(0, 0, 255);
    break;
  case KIND_PRE_SYN:
    setColor(0, 255, 0);
    break;
  default:
    setColor(0, 255, 255);
    break;
  }
}

bool ZDvidSynapse::hit(double x, double y, double z)
{
  return m_position.distanceTo(x, y, z) <= m_radius;
}

bool ZDvidSynapse::hit(double x, double y)
{
  double dx = x - m_position.getX();
  double dy = y = m_position.getY();

  double d2 = dx * dx * dy * dy;

  return d2 <= m_radius * m_radius;
}

void ZDvidSynapse::clear()
{
  m_position.set(0, 0, 0);
  m_kind = KIND_UNKNOWN;
  m_tagArray.clear();
  setDefaultRadius();
}

void ZDvidSynapse::loadJsonObject(const ZJsonObject &obj)
{
  clear();
  if (obj.hasKey("Pos")) {
    json_t *value = obj.value("Pos").getData();
    m_position.setX(ZJsonParser::integerValue(value, 0));
    m_position.setY(ZJsonParser::integerValue(value, 1));
    m_position.setZ(ZJsonParser::integerValue(value, 2));

    if (obj.hasKey("Kind")) {
      setKind(ZJsonParser::stringValue(obj["Kind"]));
    }

    if (obj.hasKey("Tags")) {
      ZJsonArray tagJson(obj["Tags"], ZJsonValue::SET_INCREASE_REF_COUNT);
      for (size_t i = 0; i < tagJson.size(); ++i) {
        m_tagArray.push_back(ZJsonParser::stringValue(tagJson.at(i), i));
      }
    }

    setDefaultRadius();
    setDefaultColor();
  }
}

void ZDvidSynapse::setKind(const std::string &kind)
{
  if (kind == "PostSyn") {
    setKind(KIND_POST_SYN);
  } else if (kind == "PreSyn") {
    setKind(KIND_PRE_SYN);
  } else {
    setKind(KIND_UNKNOWN);
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
  default:
    stream << "Unknown";
    break;
  }

  stream << ": " << "(" << synapse.getPosition().getX() << ", "
         << synapse.getPosition().getY() << ", "
         << synapse.getPosition().getZ() << ")";

  return stream;
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidSynapse)
