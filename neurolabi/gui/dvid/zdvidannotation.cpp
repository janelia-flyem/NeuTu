#include "zdvidannotation.h"

#include <QColor>

#include "tz_math.h"
#include "zjsonarray.h"
#include "zpainter.h"
#include "zjsonparser.h"
#include "zjsonfactory.h"
#include "c_json.h"
#include "zcuboid.h"

ZDvidAnnotation::ZDvidAnnotation()
{
  init();
}

void ZDvidAnnotation::init()
{
  m_type = GetType();
  m_projectionVisible = false;
  m_kind = KIND_INVALID;
  m_bodyId = 0;
  setDefaultRadius();
  setDefaultColor();
}

void ZDvidAnnotation::setRadius(double r)
{
  m_radius = r;
}

void ZDvidAnnotation::display(ZPainter &painter, int slice, EDisplayStyle /*option*/,
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
    return 3.0;
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
  if (isSliceVisible(z, NeuTube::Z_AXIS)) {
    double dx = x - m_position.getX();
    double dy = y - m_position.getY();

    double d2 = dx * dx + dy * dy;

    double radius = getRadius(z, NeuTube::Z_AXIS);

    return d2 <= radius * radius;
  }

  return false;
}

bool ZDvidAnnotation::hit(double x, double y, NeuTube::EAxis axis)
{
  ZIntPoint shiftedCenter = m_position;
  shiftedCenter.shiftSliceAxis(axis);

  double dx = x - m_position.getX();
  double dy = y - m_position.getY();

  double d2 = dx * dx + dy * dy;

  return d2 <= m_radius * m_radius;
}

void ZDvidAnnotation::setProperty(ZJsonObject propJson)
{
  if (propJson.hasKey("conf") && m_propertyJson.hasKey("confidence")) {
    m_propertyJson.removeKey("confidence");
  }

  std::map<std::string, json_t*> entryMap = propJson.toEntryMap();
  for (std::map<std::string, json_t*>::iterator iter = entryMap.begin();
       iter != entryMap.end(); ++iter) {
    const std::string &key = iter->first;
    bool goodKey = true;
    if (key == "annotation") {
      if (strlen(ZJsonParser::stringValue(iter->second)) == 0) {
        m_propertyJson.removeKey("annotation");
        goodKey = false;
      }
    }

    if (goodKey) {
      m_propertyJson.setEntry(key.c_str(), iter->second);
    }
  }
}

void ZDvidAnnotation::clear()
{
  m_position.set(0, 0, 0);
  m_kind = KIND_INVALID;
  m_tagArray.clear();
  m_partnerHint.clear();
  m_relJson.clear();
  m_propertyJson.clear();
  setDefaultRadius();
}

ZIntPoint ZDvidAnnotation::GetPosition(const ZJsonObject &json)
{
  ZIntPoint pt;
  if (json.hasKey("Pos")) {
    json_t *value = json.value("Pos").getData();
    pt.setX(ZJsonParser::integerValue(value, 0));
    pt.setY(ZJsonParser::integerValue(value, 1));
    pt.setZ(ZJsonParser::integerValue(value, 2));
  }

  return pt;
}

ZIntPoint ZDvidAnnotation::GetRelPosition(const ZJsonObject &json)
{
  ZIntPoint pt;
  pt.invalidate();

  if (json.hasKey("To")) {
    ZJsonArray posJson(json.value("To"));
    std::vector<int> coords = posJson.toIntegerArray();
    pt.set(coords[0], coords[1], coords[2]);
  }

  return pt;
}

std::string ZDvidAnnotation::GetRelationType(const ZJsonObject &relJson)
{
  std::string type;
  if (relJson.hasKey("Rel")) {
    type = ZJsonParser::stringValue(relJson["Rel"]);
  }

  return type;
}

ZDvidAnnotation::EKind ZDvidAnnotation::GetKind(const ZJsonObject &obj)
{
  EKind kind = KIND_INVALID;

  if (obj.hasKey("Kind")) {
    kind = GetKind(ZJsonParser::stringValue(obj["Kind"]));
  }

  return kind;
}

void ZDvidAnnotation::updatePartner(const ZJsonArray &jsonArray)
{
  m_partnerHint.clear();

  for (size_t i = 0; i < jsonArray.size(); ++i) {
    ZJsonObject partnerJson(jsonArray.value(i));
    if (partnerJson.hasKey("To") && partnerJson.hasKey("Rel")) {
//              std::string rel = ZJsonParser::stringValue(partnerJson["Rel"]);
      /*
      if ((getKind() == KIND_POST_SYN && rel == "PostSynTo") ||
          (getKind() == KIND_PRE_SYN && rel == "PreSynTo")) {
          */
      ZJsonArray posJson(partnerJson.value("To"));
      std::vector<int> coords = posJson.toIntegerArray();
      addPartner(coords[0], coords[1], coords[2]);
//              }
    }
  }
}

void ZDvidAnnotation::updatePartner()
{
  updatePartner(m_relJson);
}

void ZDvidAnnotation::loadJsonObject(
    const ZJsonObject &obj,
    FlyEM::EDvidAnnotationLoadMode mode)
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

    if (mode != FlyEM::LOAD_NO_PARTNER) {
      if (obj.hasKey("Rels")) {
        ZJsonArray jsonArray(obj.value("Rels"));
        switch (mode) {
        case FlyEM::LOAD_PARTNER_RELJSON:
          m_relJson = jsonArray;
          break;
        case FlyEM::LOAD_PARTNER_LOCATION:
//          m_relJson = jsonArray;
          updatePartner(jsonArray);
#if 0
          for (size_t i = 0; i < jsonArray.size(); ++i) {
            ZJsonObject partnerJson(jsonArray.value(i));
            if (partnerJson.hasKey("To") && partnerJson.hasKey("Rel")) {
//              std::string rel = ZJsonParser::stringValue(partnerJson["Rel"]);
              /*
              if ((getKind() == KIND_POST_SYN && rel == "PostSynTo") ||
                  (getKind() == KIND_PRE_SYN && rel == "PreSynTo")) {
                  */
              ZJsonArray posJson(partnerJson.value("To"));
              std::vector<int> coords = posJson.toIntegerArray();
              addPartner(coords[0], coords[1], coords[2]);
//              }
            }
          }
#endif
          break;
        default:
          break;
        }


      }
    }

    setDefaultRadius();
    setDefaultColor();

    if (obj.hasKey("Prop")) {
      m_propertyJson.setValue(obj.value("Prop").clone());
#ifdef _DEBUG_2
      std::cout << m_propertyJson.dumpString(2) << std::endl;
#endif
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

void ZDvidAnnotation::clearPartner()
{
  m_partnerHint.clear();
}

void ZDvidAnnotation::addPartner(int x, int y, int z)
{
  m_partnerHint.push_back(ZIntPoint(x, y, z));
}

void ZDvidAnnotation::addTag(const std::string &tag)
{
  m_tagArray.push_back(tag);
}

ZJsonObject ZDvidAnnotation::MakeRelJson(
    const ZIntPoint &pt, const std::string &rel)
{
  ZJsonObject relJson;
  relJson.setEntry("Rel", rel);

  ZJsonArray posJson = ZJsonFactory::MakeJsonArray(pt);
  relJson.setEntry("To", posJson);

  return relJson;
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

bool ZDvidAnnotation::isSliceVisible(int z, NeuTube::EAxis sliceAxis) const
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
  if (name.empty()) {
    m_propertyJson.removeKey("user");
  } else {
    m_propertyJson.setEntry("user", name);
  }
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
    ZJsonObject &json, const std::string &key, const char *value)
{
  ZJsonObject propJson = json.value("Prop");
  propJson.setEntry(key, value);
  if (!propJson.hasKey("Prop")) {
    json.setEntry("Prop", propJson);
  }
}

void ZDvidAnnotation::Annotate(ZJsonObject &json, const std::string &annot)
{
  AddProperty(json, "annotation", annot);
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

std::vector<ZIntPoint> ZDvidAnnotation::GetPartners(const ZJsonObject &json)
{
  std::vector<ZIntPoint> partnerArray;

  ZJsonArray jsonArray(json.value("Rels"));

  for (size_t i = 0; i < jsonArray.size(); ++i) {
    ZJsonObject partnerJson(jsonArray.value(i));
    if (partnerJson.hasKey("To") && partnerJson.hasKey("Rel")) {
      ZJsonArray posJson(partnerJson.value("To"));
      std::vector<int> coords = posJson.toIntegerArray();

      partnerArray.push_back(ZIntPoint(coords[0], coords[1], coords[2]));
    }
  }

  return partnerArray;
}

std::vector<ZIntPoint> ZDvidAnnotation::GetPartners(
    const ZJsonObject &json, const std::string &relation)
{
  std::vector<ZIntPoint> partnerArray;

  ZJsonArray jsonArray(json.value("Rels"));

  for (size_t i = 0; i < jsonArray.size(); ++i) {
    ZJsonObject partnerJson(jsonArray.value(i));
    if (partnerJson.hasKey("To") && partnerJson.hasKey("Rel")) {
      std::string relationType =
          ZJsonParser::stringValue(partnerJson.value("Rel").getData());

      if (relationType == relation) {
        ZJsonArray posJson(partnerJson.value("To"));
        std::vector<int> coords = posJson.toIntegerArray();

        partnerArray.push_back(ZIntPoint(coords[0], coords[1], coords[2]));
      }
    }
  }

  return partnerArray;
}

int ZDvidAnnotation::AddRelation(ZJsonArray &json, const ZJsonArray &relJson)
{
  int count = 0;
  for (size_t i = 0; i < relJson.size(); ++i) {
    if (AddRelation(json, ZJsonObject(relJson.value(i)))) {
      ++count;
    }
  }

  return count;
}

int ZDvidAnnotation::AddRelation(ZJsonObject &json, const ZJsonArray &relJson)
{
  int count = 0;
  for (size_t i = 0; i < relJson.size(); ++i) {
    if (AddRelation(json, ZJsonObject(relJson.value(i)))) {
      ++count;
    }
  }

  return count;
}

ZJsonArray ZDvidAnnotation::GetRelationJson(ZJsonObject &json)
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

bool ZDvidAnnotation::AddRelation(ZJsonObject &json, const ZJsonObject &relJson)
{
  if (relJson.isEmpty()) {
    return false;
  }

  ZJsonArray relArrayJson = GetRelationJson(json);

  return AddRelation(relArrayJson, relJson);
}

void ZDvidAnnotation::SetProperty(ZJsonObject &json, ZJsonObject propJson)
{
  if (propJson.isEmpty()) {
    json.removeKey("Prop");
  } else {
    json.setEntry("Prop", propJson);
  }
}

bool ZDvidAnnotation::RemoveRelation(ZJsonArray &relArrayJson, const ZIntPoint &pt)
{
  bool removed = false;

  for (size_t i = 0; i < relArrayJson.size(); ++i) {
    ZJsonObject toJson(relArrayJson.value(i));
    ZIntPoint to = ZJsonParser::toIntPoint(toJson["To"]);
    if (pt == to) {
      relArrayJson.remove(i);
      --i;
      removed = true;
//      break;
    }
  }

  return removed;
}

bool ZDvidAnnotation::RemoveRelation(
    ZJsonArray &relArrayJson, const std::string &rel)
{
  bool removed = false;

  size_t removeCount = 0;
  size_t relCount = relArrayJson.size();
  for (size_t i = 0; i < relCount; ++i) {
    ZJsonObject toJson(relArrayJson.value(i - removeCount));
    std::string relationType = ZJsonParser::stringValue(toJson["Rel"]);
    if (relationType == rel) {
      relArrayJson.remove(i - removeCount);
      ++removeCount;
      removed = true;
    }
  }

  return removed;
}

bool ZDvidAnnotation::RemoveRelation(ZJsonObject &json, const ZIntPoint &pt)
{
  ZJsonArray relationArray = GetRelationJson(json);
  return RemoveRelation(relationArray, pt);
}

bool ZDvidAnnotation::RemoveRelation(ZJsonObject &json, const std::string &rel)
{
  ZJsonArray relationArray = GetRelationJson(json);
  return RemoveRelation(relationArray, rel);
}

bool ZDvidAnnotation::AddRelation(
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

std::string ZDvidAnnotation::GetMatchingRelation(const std::string &relType)
{
  std::string type = "UnknownRelationship";

  if (relType == "PreSynTo" || relType == "ConvergentTo") {
    type = "PostSynTo";
  } else if (relType == "PostSynTo") {
    type = "PreSynTo";
  } else if (relType == "GroupedWith") {
    type = relType;
  }

  return type;
}

int ZDvidAnnotation::MatchRelation(
    const ZJsonArray &relArray, const ZIntPoint &pos, const std::string &relType)
{
  int index = -1;

  for (size_t i = 0; i < relArray.size(); ++i) {
    ZJsonObject testRel(relArray.value(i));
    if (GetRelPosition(testRel) == pos) {
      std::string testRelType = GetRelationType(testRel);
      if (relType == "PreSynTo" || relType == "ConvergentTo") {
        if (testRelType == "PostSynTo") {
          index = i;
          break;
        }
      } else if (relType == "PostSynTo") {
        if (testRelType == "PreSynTo") {
          index = i;
          break;
        }
      } else if (relType == testRelType) {
        index = i;
        break;
      }
    }
  }

  return index;
}


int ZDvidAnnotation::MatchRelation(
    const ZJsonArray &relArray, const ZIntPoint &pos, const ZJsonObject &rel)
{
  std::string relType = GetRelationType(rel);

  return MatchRelation(relArray, pos, relType);
}

bool ZDvidAnnotation::AddRelation(
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

bool ZDvidAnnotation::AddRelation(
    ZJsonObject &json, const ZIntPoint &to, const std::string &rel)
{
  if (rel.empty()) {
    return false;
  }

  ZJsonArray relArrayJson = GetRelationJson(json);

  return AddRelation(relArrayJson, to, rel);
}

ZCuboid ZDvidAnnotation::getBoundBox() const
{
  ZCuboid box;
  box.setFirstCorner(getPosition().toPoint() - getRadius());
  box.setLastCorner(getPosition().toPoint() + getRadius());

  return box;
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidAnnotation)

///////////////
ZJsonObject ZDvidAnnotation::Relation::toJsonObject() const
{
  return ZDvidAnnotation::MakeRelJson(m_to, GetName(m_relation));
}

std::string ZDvidAnnotation::Relation::GetName(
    ZDvidAnnotation::Relation::ERelation rel)
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
