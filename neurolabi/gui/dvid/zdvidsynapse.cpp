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
  m_projectionVisible = false;
  m_kind = KIND_INVALID;
  setDefaultRadius();
}

void ZDvidSynapse::display(ZPainter &painter, int slice, EDisplayStyle option,
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

void ZDvidSynapse::setPosition(const ZIntPoint &pos)
{
  m_position = pos;
}

void ZDvidSynapse::setPosition(int x, int y, int z)
{
  m_position.set(x, y, z);
}

// temporary?
std::vector<ZIntPoint> ZDvidSynapse::getPartners() {
    return std::vector<ZIntPoint>(m_partnerHint);
}

double ZDvidSynapse::GetDefaultRadius(EKind kind)
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

void ZDvidSynapse::setDefaultRadius()
{
  m_radius = GetDefaultRadius(m_kind);
}

QColor ZDvidSynapse::GetDefaultColor(EKind kind)
{
  switch (kind) {
  case KIND_POST_SYN:
    return QColor(0, 0, 255);
  case KIND_PRE_SYN:
    return QColor(0, 255, 0);
  case KIND_UNKNOWN:
    return QColor(0, 200, 200);
  default:
    break;
  }

  return QColor(128, 128, 128);
}

void ZDvidSynapse::setDefaultColor()
{
  setColor(GetDefaultColor(m_kind));
}

bool ZDvidSynapse::hit(double x, double y, double z)
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

bool ZDvidSynapse::hit(double x, double y)
{
  double dx = x - m_position.getX();
  double dy = y - m_position.getY();

  double d2 = dx * dx + dy * dy;

  return d2 <= m_radius * m_radius;
}

void ZDvidSynapse::clear()
{
  m_position.set(0, 0, 0);
  m_kind = KIND_INVALID;
  m_tagArray.clear();
  setDefaultRadius();
}

bool ZDvidSynapse::isValid() const
{
  return getKind() != KIND_INVALID;
}

void ZDvidSynapse::loadJsonObject(
    const ZJsonObject &obj, NeuTube::FlyEM::ESynapseLoadMode mode)
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
      setKind(KIND_UNKNOWN);
    }

    if (obj.hasKey("Tags")) {
      ZJsonArray tagJson(obj["Tags"], ZJsonValue::SET_INCREASE_REF_COUNT);
      for (size_t i = 0; i < tagJson.size(); ++i) {
        m_tagArray.push_back(ZJsonParser::stringValue(tagJson.at(i)));
      }
    }

    clearPartner();
    if (mode == NeuTube::FlyEM::LOAD_PARTNER_LOCATION) {
      if (obj.hasKey("Rels")) {
        ZJsonArray jsonArray(obj.value("Rels"));
        if (jsonArray.size() > 0) {
          for (size_t i = 0; i < jsonArray.size(); ++i) {
            ZJsonObject partnerJson(jsonArray.value(i));
            if (partnerJson.hasKey("To") && partnerJson.hasKey("Rel")) {
              std::string rel = ZJsonParser::stringValue(partnerJson["Rel"]);
              if ((getKind() == KIND_POST_SYN && rel == "PostSynTo") ||
                  (getKind() == KIND_PRE_SYN && rel == "PreSynTo")) {
                ZJsonArray posJson(partnerJson.value("To"));
                std::vector<int> coords = posJson.toIntegerArray();
                addPartner(coords[0], coords[1], coords[2]);
              }
            }
          }
        }
      }
    }

    setDefaultRadius();
    setDefaultColor();

    if (obj.hasKey("Prop")) {
      m_propertyJson.setValue(obj.value("Prop").clone());
    }
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

void ZDvidSynapse::clearPartner()
{
  m_partnerHint.clear();
}

void ZDvidSynapse::addPartner(int x, int y, int z)
{
  m_partnerHint.push_back(ZIntPoint(x, y, z));
}

void ZDvidSynapse::addTag(const std::string &tag)
{
  m_tagArray.push_back(tag);
}

std::string ZDvidSynapse::GetKindName(EKind kind)
{
  switch (kind) {
  case ZDvidSynapse::KIND_POST_SYN:
    return "PostSyn";
  case ZDvidSynapse::KIND_PRE_SYN:
    return "PreSyn";
  default:
    break;
  }

  return "Unknown";
}

ZDvidSynapse::EKind ZDvidSynapse::GetKind(const std::string &name)
{
  if (name == "PostSyn") {
    return ZDvidSynapse::KIND_POST_SYN;
  } else if (name == "PreSyn") {
    return ZDvidSynapse::KIND_PRE_SYN;
  }

  return ZDvidSynapse::KIND_UNKNOWN;
}

ZJsonObject ZDvidSynapse::MakeRelJson(
    const ZIntPoint &pt, const std::string &rel)
{
  ZJsonObject relJson;
  relJson.setEntry("Rel", rel);

  ZJsonArray posJson = ZJsonFactory::MakeJsonArray(pt);
  relJson.setEntry("To", posJson);

  return relJson;
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

int ZDvidSynapse::AddRelation(ZJsonArray &json, const ZJsonArray &relJson)
{
  int count = 0;
  for (size_t i = 0; i < relJson.size(); ++i) {
    if (AddRelation(json, ZJsonObject(relJson.value(i)))) {
      ++count;
    }
  }

  return count;
}

int ZDvidSynapse::AddRelation(ZJsonObject &json, const ZJsonArray &relJson)
{
  int count = 0;
  for (size_t i = 0; i < relJson.size(); ++i) {
    if (AddRelation(json, ZJsonObject(relJson.value(i)))) {
      ++count;
    }
  }

  return count;
}

ZJsonArray ZDvidSynapse::GetRelationJson(ZJsonObject &json)
{
  ZJsonArray relArrayJson;
  bool hasRels = false;
  if (json.hasKey("Rels")) {
    relArrayJson = ZJsonArray(json.value("Rels"));
    if (!relArrayJson.isEmpty()) {
      hasRels = true;
    }
  }

  if (!hasRels) {
    relArrayJson = ZJsonArray(C_Json::makeArray(), ZJsonValue::SET_AS_IT_IS);
    json.setEntry("Rels", relArrayJson);
  }

  return relArrayJson;
}

bool ZDvidSynapse::AddRelation(ZJsonObject &json, const ZJsonObject &relJson)
{
  if (relJson.isEmpty()) {
    return false;
  }

  ZJsonArray relArrayJson = GetRelationJson(json);

  return AddRelation(relArrayJson, relJson);
}

bool ZDvidSynapse::RemoveRelation(ZJsonArray &relArrayJson, const ZIntPoint &pt)
{
  bool removed = false;

  for (size_t i = 0; i < relArrayJson.size(); ++i) {
    ZJsonObject toJson(relArrayJson.value(i));
    ZIntPoint to = ZJsonParser::toIntPoint(toJson["To"]);
    if (pt == to) {
      relArrayJson.remove(i);
      removed = true;
      break;
    }
  }

  return removed;
}

bool ZDvidSynapse::RemoveRelation(ZJsonObject &json, const ZIntPoint &pt)
{
  ZJsonArray relationArray = GetRelationJson(json);
  return RemoveRelation(relationArray, pt);
}

bool ZDvidSynapse::AddRelation(
    ZJsonArray &relArrayJson, const ZJsonObject &relJson)
{
  if (relJson.isEmpty()) {
    return false;
  }

  ZIntPoint to = ZJsonParser::toIntPoint(relJson["To"]);

  bool adding = true;
  for (size_t i = 0; i < relArrayJson.size(); ++i) {
    ZJsonObject toJson(relArrayJson.value(i));
    ZIntPoint pt = ZJsonParser::toIntPoint(toJson["To"]);
    if (pt == to) {
      adding = false;
      break;
    }

  }

  if (adding) {
    relArrayJson.append(relJson);
  }

  return adding;
}

bool ZDvidSynapse::AddRelation(
    ZJsonArray &relArrayJson, const ZIntPoint &to, const std::string &rel)
{
  if (rel.empty()) {
    return false;
  }

  bool adding = true;
  for (size_t i = 0; i < relArrayJson.size(); ++i) {
    ZIntPoint pt = ZJsonParser::toIntPoint(ZJsonObject(relArrayJson.value(i))["To"]);
    if (pt == to) {
      adding = false;
      break;
    }
  }

  if (adding) {
    relArrayJson.append(MakeRelJson(to, rel));
  }

  return adding;
}

bool ZDvidSynapse::AddRelation(
    ZJsonObject &json, const ZIntPoint &to, const std::string &rel)
{
  if (rel.empty()) {
    return false;
  }

  ZJsonArray relArrayJson = GetRelationJson(json);

  return AddRelation(relArrayJson, to, rel);
}


ZJsonObject ZDvidSynapse::toJsonObject() const
{
  ZJsonObject obj;

  ZJsonArray posJson = ZJsonFactory::MakeJsonArray(m_position);
  obj.setEntry("Pos", posJson);
  obj.setEntry("Kind", GetKindName(getKind()));
  if (!m_partnerHint.empty()) {
    ZJsonArray relArrayJson;
    for (std::vector<ZIntPoint>::const_iterator iter = m_partnerHint.begin();
         iter != m_partnerHint.end(); ++iter) {
      const ZIntPoint &pt = *iter;
      relArrayJson.append(makeRelJson(pt));
    }
    obj.setEntry("Rels", relArrayJson);
  }

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
    ZJsonObject propJson = m_propertyJson.clone();
    obj.setEntry("Prop", propJson);
  }

  return obj;
}

bool ZDvidSynapse::isVisible(int z, NeuTube::EAxis sliceAxis) const
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

double ZDvidSynapse::getRadius(int z, NeuTube::EAxis sliceAxis) const
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

void ZDvidSynapse::setUserName(const std::string &name)
{
  m_propertyJson.setEntry("user", name);
}

std::string ZDvidSynapse::getUserName() const
{
  return ZJsonParser::stringValue(m_propertyJson["user"]);
}

void ZDvidSynapse::AddProperty(
    ZJsonObject &json, const std::string &key, const std::string &value)
{
  ZDvidAnnotation::AddProperty(json, key, value);
}

int ZDvidSynapse::getX() const
{
  return getPosition().getX();
}

int ZDvidSynapse::getY() const
{
  return getPosition().getY();
}

int ZDvidSynapse::getZ() const
{
  return getPosition().getZ();
}


ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidSynapse)


///////////////
ZJsonObject ZDvidSynapse::Relation::toJsonObject() const
{
  return ZDvidSynapse::MakeRelJson(m_to, GetName(m_relation));
}

std::string ZDvidSynapse::Relation::GetName(ZDvidSynapse::Relation::ERelation rel)
{
  switch (rel) {
  case RELATION_POSTSYN_TO:
    return "PostSynTo";
  case RELATION_PRESYN_TO:
    return "PreSynTo";
  case RELATION_CONVERGENT_TO:
    return "ConvergentTo";
  case RELATION_GROUPED_WITH:
    return "GroupedWith";
  default:
    break;
  }

  return "UnknownRelationship";
}
