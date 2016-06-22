#include "zdvidannotation.h"

#include <QColor>

#include "tz_math.h"
#include "zjsonarray.h"
#include "zpainter.h"
#include "zjsonparser.h"
#include "zjsonfactory.h"

ZDvidAnnotation::ZDvidAnnotation()
{
  m_type = GetType();
  init();
}

void ZDvidAnnotation::init()
{
  m_projectionVisible = false;
  m_kind = KIND_INVALID;
  setDefaultRadius();
}

void ZDvidAnnotation::display(ZPainter &painter, int slice, EDisplayStyle /*option*/,
                           NeuTube::EAxis sliceAxis) const
{
  bool visible = true;
  int z = painter.getZ(slice);

  if (slice < 0) {
    visible = isProjectionVisible();
  } else {
    visible = isVisible(z, sliceAxis);
  }

  double radius = getRadius(z, sliceAxis);

  ZIntPoint center = m_position;
  center.shiftSliceAxis(sliceAxis);

  bool isFocused = (z == center.getZ());

  if (visible) {
    QColor color = getColor();
    if (!isFocused) {
      double alpha = radius / m_radius;
      alpha *= alpha * 0.5;
      alpha += 0.1;
      color.setAlphaF(alpha * color.alphaF());
    }

    painter.setPen(color);
    painter.setBrush(Qt::NoBrush);

    if (isFocused) {
      int x = center.getX();
      int y = center.getY();
      painter.drawLine(QPointF(x - 1, y), QPointF(x + 1, y));
      painter.drawLine(QPointF(x, y - 1), QPointF(x, y + 1));
    }
    if (radius > 0.0) {
      painter.drawEllipse(QPointF(center.getX(), center.getY()),
                          radius, radius);
    }
    if (!getUserName().empty()) {
      QString decorationText = "u";
      int width = decorationText.size() * 25;
      int height = 25;
      QColor oldColor = painter.getPen().color();
      painter.setPen(QColor(0, 0, 0));
      painter.drawText(center.getX(), center.getY(), width, height,
                       Qt::AlignLeft, decorationText);
      painter.setPen(oldColor);
    }
  }

  QPen pen = painter.getPen();
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
}

void ZDvidAnnotation::setPosition(const ZIntPoint &pos)
{
  m_position = pos;
}

void ZDvidAnnotation::setPosition(int x, int y, int z)
{
  m_position.set(x, y, z);
}

double ZDvidAnnotation::GetDefaultRadius(EKind kind)
{
  switch (kind) {
  case KIND_POST_SYN:
    return 5.0;
  case KIND_PRE_SYN:
    return 7.0;
  default:
    break;
  }

  return 7.0;
}

void ZDvidAnnotation::setDefaultRadius()
{
  m_radius = GetDefaultRadius(m_kind);
}

QColor ZDvidAnnotation::GetDefaultColor(EKind kind)
{
  switch (kind) {
  case KIND_POST_SYN:
    return QColor(0, 0, 255);
  case KIND_PRE_SYN:
    return QColor(0, 255, 0);
  case KIND_NOTE:
    return QColor(0, 200, 200);
  default:
    break;
  }

  return QColor(128, 128, 128);
}

void ZDvidAnnotation::setDefaultColor()
{
  setColor(GetDefaultColor(m_kind));
}

bool ZDvidAnnotation::hit(double x, double y, double z)
{
  if (isVisible(z, NeuTube::Z_AXIS)) {
    double dx = x - m_position.getX();
    double dy = y - m_position.getY();

    double d2 = dx * dx + dy * dy;

    double radius = getRadius(z, NeuTube::Z_AXIS);

    return d2 <= radius * radius;
  }

  return false;
}

bool ZDvidAnnotation::hit(double x, double y)
{
  double dx = x - m_position.getX();
  double dy = y - m_position.getY();

  double d2 = dx * dx + dy * dy;

  return d2 <= m_radius * m_radius;
}

void ZDvidAnnotation::clear()
{
  m_position.set(0, 0, 0);
  m_kind = KIND_INVALID;
  m_tagArray.clear();
  setDefaultRadius();
}

void ZDvidAnnotation::loadJsonObject(const ZJsonObject &obj)
{
  clear();
  if (obj.hasKey("Pos")) {
    json_t *value = obj.value("Pos").getData();
    m_position.setX(ZJsonParser::integerValue(value, 0));
    m_position.setY(ZJsonParser::integerValue(value, 1));
    m_position.setZ(ZJsonParser::integerValue(value, 2));

    if (obj.hasKey("Kind")) {
      setKind(ZJsonParser::stringValue(obj["Kind"]));
    } else {
      setKind(KIND_INVALID);
    }

    if (obj.hasKey("Tags")) {
      ZJsonArray tagJson(obj["Tags"], ZJsonValue::SET_INCREASE_REF_COUNT);
      for (size_t i = 0; i < tagJson.size(); ++i) {
        m_tagArray.push_back(ZJsonParser::stringValue(tagJson.at(i)));
      }
    }

    if (obj.hasKey("Rels")) {
      ZJsonArray jsonArray(obj.value("Rels"));
      m_relJson = jsonArray;
    }

    setDefaultRadius();
    setDefaultColor();

    if (obj.hasKey("Prop")) {
      m_propertyJson.setValue(obj.value("Prop").clone());
    }
  }
}

bool ZDvidAnnotation::isValid() const
{
  return getKind() != KIND_INVALID;
}

void ZDvidAnnotation::setKind(const std::string &kind)
{
  if (kind == "PostSyn") {
    setKind(KIND_POST_SYN);
  } else if (kind == "PreSyn") {
    setKind(KIND_PRE_SYN);
  } else if (kind == "Note") {
    setKind(KIND_NOTE);
  } else {
    setKind(KIND_INVALID);
  }
}

ZJsonObject ZDvidAnnotation::toJsonObject() const
{
  ZJsonObject obj;

  ZJsonArray posJson = ZJsonFactory::MakeJsonArray(m_position);
  obj.setEntry("Pos", posJson);
  obj.setEntry("Kind", GetKindName(getKind()));
  ZJsonValue relJson = m_relJson.clone();
  obj.setEntry("Rels", relJson);

  if (!m_tagArray.empty()) {
    ZJsonArray tagJson;
    for (std::vector<std::string>::const_iterator iter = m_tagArray.begin();
         iter != m_tagArray.end(); ++iter) {
      const std::string &tag = *iter;
      tagJson.append(tag);
    }
    obj.setEntry("Tags", tagJson);
  }

  if (!m_propertyJson.isEmpty()) {
    ZJsonValue propJson = m_propertyJson.clone();
    obj.setEntry("Prop", propJson);
  }

  return obj;
}

bool ZDvidAnnotation::isVisible(int z, NeuTube::EAxis sliceAxis) const
{
  int dz = 0;
  switch (sliceAxis) {
  case NeuTube::X_AXIS:
    dz = abs(getPosition().getX() - z);
    break;
  case NeuTube::Y_AXIS:
    dz = abs(getPosition().getY() - z);
    break;
  case NeuTube::Z_AXIS:
    dz = abs(getPosition().getZ() - z);
    break;
  }

  return dz < iround(getRadius());
}

double ZDvidAnnotation::getRadius(int z, NeuTube::EAxis sliceAxis) const
{
  int dz = 0;
  switch (sliceAxis) {
  case NeuTube::X_AXIS:
    dz = abs(getPosition().getX() - z);
    break;
  case NeuTube::Y_AXIS:
    dz = abs(getPosition().getY() - z);
    break;
  case NeuTube::Z_AXIS:
    dz = abs(getPosition().getZ() - z);
    break;
  }

  return std::max(0.0, getRadius() - dz);
}

void ZDvidAnnotation::setUserName(const std::string &name)
{
  m_propertyJson.setEntry("user", name);
}

std::string ZDvidAnnotation::getUserName() const
{
  return ZJsonParser::stringValue(m_propertyJson["user"]);
}

int ZDvidAnnotation::getX() const
{
  return getPosition().getX();
}

int ZDvidAnnotation::getY() const
{
  return getPosition().getY();
}

int ZDvidAnnotation::getZ() const
{
  return getPosition().getZ();
}

std::string ZDvidAnnotation::GetKindName(EKind kind)
{
  switch (kind) {
  case ZDvidAnnotation::KIND_POST_SYN:
    return "PostSyn";
  case ZDvidAnnotation::KIND_PRE_SYN:
    return "PreSyn";
  case ZDvidAnnotation::KIND_NOTE:
    return "Note";
  case ZDvidAnnotation::KIND_UNKNOWN:
    return "Unknown";
  default:
    break;
  }

  return "Invalid";
}

ZDvidAnnotation::EKind ZDvidAnnotation::GetKind(const std::string &name)
{
  if (name == "PostSyn") {
    return ZDvidAnnotation::KIND_POST_SYN;
  } else if (name == "PreSyn") {
    return ZDvidAnnotation::KIND_PRE_SYN;
  } else if (name == "Note") {
    return ZDvidAnnotation::KIND_NOTE;
  } else if (name == "Unknown") {
    return ZDvidAnnotation::KIND_UNKNOWN;
  }

  return ZDvidAnnotation::KIND_INVALID;
}

void ZDvidAnnotation::AddProperty(
    ZJsonObject &json, const std::string &key, const std::string &value)
{
  ZJsonObject propJson = json.value("Prop");
  propJson.setEntry(key, value);
  if (!propJson.hasKey("Prop")) {
    json.setEntry("Prop", propJson);
  }
}

void ZDvidAnnotation::AddProperty(
    ZJsonObject &json, const std::string &key, bool value)
{
  ZJsonObject propJson = json.value("Prop");
  if (value == true) {
    propJson.setEntry(key, "1");
  } else {
    propJson.setEntry(key, "0");
  }
  if (!propJson.hasKey("Prop")) {
    json.setEntry("Prop", propJson);
  }
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidAnnotation)
