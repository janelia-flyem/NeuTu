#include "zdvidannotation.h"

#include <cmath>
#include <sstream>
#include <QColor>

#include "common/utilities.h"
#include "common/math.h"
#include "geometry/zcuboid.h"
#include "zjsonarray.h"
#include "zjsonobjectparser.h"
#include "zpainter.h"
#include "zjsonparser.h"
#include "zjsonfactory.h"
#include "c_json.h"
#include "zresolution.h"
#include "zdvidutil.h"

const char* ZDvidAnnotation::KEY_COMMENT = "comment";
const char* ZDvidAnnotation::KEY_CREATED_TIME = "createdTime";
const char* ZDvidAnnotation::KEY_MODIFIED_TIME = "modifiedTime";
const char* ZDvidAnnotation::KEY_CHECKED_TIME = "checkedTime";

double ZDvidAnnotation::DEFAULT_PRE_SYN_RADIUS = 7.0;
double ZDvidAnnotation::DEFAULT_POST_SYN_RADIUS = 3.0;

ZDvidAnnotation::ZDvidAnnotation()
{
  init();
}

ZDvidAnnotation::ZDvidAnnotation(const ZDvidAnnotation &annotation)
{
  m_position = annotation.m_position;
  m_kind = annotation.m_kind;
  m_radius = annotation.m_radius;
  m_bodyId = annotation.m_bodyId;
  m_status = annotation.m_status;
  m_partnerHint = annotation.m_partnerHint;
  m_tagSet = annotation.m_tagSet;
  m_propertyJson = ZJsonObject(annotation.m_propertyJson.clone());
  m_relJson = ZJsonArray(annotation.m_relJson.clone());
}

void ZDvidAnnotation::init()
{
  m_type = GetType();
  m_projectionVisible = false;
  m_kind = EKind::KIND_INVALID;
  m_bodyId = 0;
  m_status = EStatus::NORMAL;
  setDefaultRadius();
  setDefaultColor();
}

ZDvidAnnotation* ZDvidAnnotation::clone() const
{
  return new ZDvidAnnotation(*this);
}

void ZDvidAnnotation::setRadius(double r)
{
  m_radius = r;
}

void ZDvidAnnotation::display(ZPainter &painter, int slice, EDisplayStyle /*option*/,
                           neutu::EAxis sliceAxis) const
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
  } else if (hasVisualEffect(neutu::display::Sphere::VE_BOUND_BOX)) {
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

double ZDvidAnnotation::GetDefaultRadius(
    EKind kind, const ZResolution &resolution)
{
  double r = GetDefaultRadius(kind);

  if (resolution.getUnit() == ZResolution::EUnit::UNIT_PIXEL) {
    r *= sqrt(resolution.getPlaneVoxelSize(neutu::EPlane::XY));
  } else {
    r *= sqrt(resolution.getPlaneVoxelSize(
                neutu::EPlane::XY, ZResolution::EUnit::UNIT_NANOMETER)) / 8.0;
  }

  return r;
}

double ZDvidAnnotation::GetDefaultRadius(EKind kind)
{
  switch (kind) {
  case EKind::KIND_POST_SYN:
    return DEFAULT_POST_SYN_RADIUS;
  case EKind::KIND_PRE_SYN:
    return DEFAULT_PRE_SYN_RADIUS;
  default:
    break;
  }

  return DEFAULT_PRE_SYN_RADIUS;
}

void ZDvidAnnotation::setDefaultRadius()
{
  m_radius = GetDefaultRadius(m_kind);
}

void ZDvidAnnotation::setDefaultRadius(const ZResolution &resolution)
{
  m_radius = GetDefaultRadius(m_kind, resolution);
}

QColor ZDvidAnnotation::GetDefaultColor(EKind kind)
{
  switch (kind) {
  case EKind::KIND_POST_SYN:
    return QColor(0, 0, 255);
  case EKind::KIND_PRE_SYN:
    return QColor(0, 255, 0);
  case EKind::KIND_NOTE:
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
  if (isSliceVisible(z, neutu::EAxis::Z)) {
    double dx = x - m_position.getX();
    double dy = y - m_position.getY();

    double d2 = dx * dx + dy * dy;

    double radius = getRadius(z, neutu::EAxis::Z);

    return d2 <= radius * radius;
  }

  return false;
}

bool ZDvidAnnotation::hit(double x, double y, neutu::EAxis axis)
{
  ZIntPoint shiftedCenter = m_position;
  shiftedCenter.shiftSliceAxis(axis);

  double dx = x - m_position.getX();
  double dy = y - m_position.getY();

  double d2 = dx * dx + dy * dy;

  return d2 <= m_radius * m_radius;
}

/*
void ZDvidAnnotation::setProperty(ZJsonObject propJson)
{
  if (propJson.hasKey("conf") && m_propertyJson.hasKey("confidence")) {
    m_propertyJson.removeKey("confidence");
  }

  std::map<std::string, json_t*> entryMap = propJson.toEntryMap(false);
  for (std::map<std::string, json_t*>::iterator iter = entryMap.begin();
       iter != entryMap.end(); ++iter) {
    const std::string &key = iter->first;
    bool goodKey = true;
    if (key == "annotation") {
      if (ZJsonParser::stringValue(iter->second).empty()) {
        m_propertyJson.removeKey("annotation");
        goodKey = false;
      }
    }

    if (goodKey) {
      m_propertyJson.setEntry(key.c_str(), iter->second);
    }
  }
}
*/

void ZDvidAnnotation::clear()
{
  m_position.set(0, 0, 0);
  m_kind = EKind::KIND_INVALID;
  m_tagSet.clear();
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
  EKind kind = EKind::KIND_INVALID;

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
    dvid::EAnnotationLoadMode mode)
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
      setKind(EKind::KIND_INVALID);
    }

    if (obj.hasKey("Tags")) {
      ZJsonArray tagJson(obj["Tags"], ZJsonValue::SET_INCREASE_REF_COUNT);
      for (size_t i = 0; i < tagJson.size(); ++i) {
        addTag(ZJsonParser::stringValue(tagJson.at(i)));
      }
    }

    if (mode != dvid::EAnnotationLoadMode::NO_PARTNER) {
      if (obj.hasKey("Rels")) {
        ZJsonArray jsonArray(obj.value("Rels"));
        switch (mode) {
        case dvid::EAnnotationLoadMode::PARTNER_RELJSON:
          m_relJson = jsonArray;
          break;
        case dvid::EAnnotationLoadMode::PARTNER_LOCATION:
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
  return getKind() != EKind::KIND_INVALID;
}

void ZDvidAnnotation::setKind(const std::string &kind)
{
  if (kind == "PostSyn") {
    setKind(EKind::KIND_POST_SYN);
  } else if (kind == "PreSyn") {
    setKind(EKind::KIND_PRE_SYN);
  } else if (kind == "Note") {
    setKind(EKind::KIND_NOTE);
  } else {
    setKind(EKind::KIND_INVALID);
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

bool ZDvidAnnotation::hasPartner(const ZIntPoint &pos)
{
  for (std::vector<ZIntPoint>::const_iterator iter = m_partnerHint.begin();
       iter != m_partnerHint.end(); ++iter) {
    const ZIntPoint &pt = *iter;
    if (pt == pos) {
      return true;
    }
  }

  return false;
}

void ZDvidAnnotation::addTag(const std::string &tag)
{
  if (!tag.empty()) {
    m_tagSet.insert(tag);
  }
}

void ZDvidAnnotation::addBodyIdTag()
{
  addTag(dvid::GetBodyIdTag(getBodyId()));
}

void ZDvidAnnotation::removeTag(const std::string &tag)
{
  m_tagSet.erase(tag);
}

bool ZDvidAnnotation::hasTag(const std::string &tag) const
{
  return m_tagSet.count(tag) > 0;
}

std::set<std::string> ZDvidAnnotation::getTagSet() const
{
  return m_tagSet;
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

  ZJsonArray relJson = ZJsonArray(
        C_Json::clone(m_relJson.getData()), ZJsonValue::SET_AS_IT_IS);
  obj.setEntry("Rels", relJson);

  ZJsonArray tagJson;
  if (!m_tagSet.empty()) {
    for (std::set<std::string>::const_iterator iter = m_tagSet.begin();
         iter != m_tagSet.end(); ++iter) {
      const std::string &tag = *iter;
      tagJson.append(tag);
    }
  }
  obj.setEntry("Tags", tagJson);

  if (!m_propertyJson.isEmpty()) {
    ZJsonValue propJson = m_propertyJson.clone();
    obj.setEntry("Prop", propJson);
  }

  return obj;
}

bool ZDvidAnnotation::isSliceVisible(int z, neutu::EAxis sliceAxis) const
{
  if (sliceAxis == neutu::EAxis::ARB) {
    return false;
  }

  int dz = 0;
  switch (sliceAxis) {
  case neutu::EAxis::X:
    dz = abs(getPosition().getX() - z);
    break;
  case neutu::EAxis::Y:
    dz = abs(getPosition().getY() - z);
    break;
  case neutu::EAxis::Z:
  case neutu::EAxis::ARB:
    dz = abs(getPosition().getZ() - z);
    break;
  }

  return dz < neutu::iround(getRadius());
}

double ZDvidAnnotation::getRadius(int z, neutu::EAxis sliceAxis) const
{
  if (sliceAxis == neutu::EAxis::ARB) {
    return 0.0;
  }

  int dz = 0;
  switch (sliceAxis) {
  case neutu::EAxis::X:
    dz = abs(getPosition().getX() - z);
    break;
  case neutu::EAxis::Y:
    dz = abs(getPosition().getY() - z);
    break;
  case neutu::EAxis::Z:
  case neutu::EAxis::ARB:
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

template < >
std::string ZDvidAnnotation::getProperty<std::string>(const std::string &key)
const
{
  std::string prop;
  prop = ZJsonParser::stringValue(m_propertyJson.value(key.c_str()).getData());

  return prop;
}

bool ZDvidAnnotation::hasProperty(const std::string &key) const
{
  return m_propertyJson.hasKey(key.c_str());
}

void ZDvidAnnotation::addProperty(
    const std::string &key, const std::string &value)
{
  if (!key.empty()) {
    m_propertyJson.setEntry(key, value);
  }
}

void ZDvidAnnotation::removeProperty(const std::string &key)
{
  if (m_propertyJson.hasKey(key.c_str())) {
    m_propertyJson.removeKey(key.c_str());
  }
}

void ZDvidAnnotation::setComment(const std::string &comment)
{
  if (comment.empty()) {
    removeProperty(KEY_COMMENT);
  } else {
    addProperty(KEY_COMMENT, comment);
  }
}

std::string ZDvidAnnotation::getComment() const
{
  return getProperty<std::string>(KEY_COMMENT);
}

std::string ZDvidAnnotation::GetKindName(EKind kind)
{
  switch (kind) {
  case ZDvidAnnotation::EKind::KIND_POST_SYN:
    return "PostSyn";
  case ZDvidAnnotation::EKind::KIND_PRE_SYN:
    return "PreSyn";
  case ZDvidAnnotation::EKind::KIND_NOTE:
    return "Note";
  case ZDvidAnnotation::EKind::KIND_UNKNOWN:
    return "Unknown";
  default:
    break;
  }

  return "Invalid";
}

ZDvidAnnotation::EKind ZDvidAnnotation::GetKind(const std::string &name)
{
  if (name == "PostSyn") {
    return ZDvidAnnotation::EKind::KIND_POST_SYN;
  } else if (name == "PreSyn") {
    return ZDvidAnnotation::EKind::KIND_PRE_SYN;
  } else if (name == "Note") {
    return ZDvidAnnotation::EKind::KIND_NOTE;
  } else if (name == "Unknown") {
    return ZDvidAnnotation::EKind::KIND_UNKNOWN;
  }

  return ZDvidAnnotation::EKind::KIND_INVALID;
}

namespace {

bool includes_same_prop(const ZJsonObject &prop1, const ZJsonObject &prop2)
{
  return prop1.all([&](const std::string &key) {
    if (key == "user" || key == "checked" || key == "timestamp" ||
        key == "custom" ||
        key == ZDvidAnnotation::KEY_CREATED_TIME ||
        key == ZDvidAnnotation::KEY_MODIFIED_TIME ||
        key == ZDvidAnnotation::KEY_CHECKED_TIME) {
      return true;
    }

    return ZJsonObjectParser::GetValue(prop1, key, "") ==
        ZJsonObjectParser::GetValue(prop2, key, "");
  });
}

bool is_same_prop(const ZJsonObject &prop1, const ZJsonObject &prop2)
{
  return includes_same_prop(prop1, prop2) && includes_same_prop(prop2, prop1);
}

ZJsonObject get_prop(const ZJsonObject &json)
{
  return ZJsonObject(json.value("Prop"));
}

ZJsonObject add_prop(ZJsonObject &json)
{
  ZJsonObject propJson = json.value("Prop");

  if (!json.hasKey("Prop")) {
    json.setEntry("Prop", propJson);
  }

  return propJson;
}

std::string get_created_time(const ZJsonObject &json)
{
  return ZJsonObjectParser::GetValue(
              get_prop(json), ZDvidAnnotation::KEY_CREATED_TIME, "");
}

std::string get_modified_time(const ZJsonObject &json)
{
  return ZJsonObjectParser::GetValue(
              get_prop(json), ZDvidAnnotation::KEY_MODIFIED_TIME, "");
}

std::string get_checked_time(const ZJsonObject &json)
{
  return ZJsonObjectParser::GetValue(
              get_prop(json), ZDvidAnnotation::KEY_CHECKED_TIME, "");
}

}

std::string ZDvidAnnotation::GetProperty(
    ZJsonObject &json, const std::string &key)
{
  return ZJsonObjectParser::GetValue(get_prop(json), key, "");
}

bool ZDvidAnnotation::IsChecked(const ZJsonObject &json)
{
  ZJsonObject propJson = get_prop(json);
  std::string checked = ZJsonObjectParser::GetValue(propJson, "checked", "0");

  return checked == "1";
}

bool ZDvidAnnotation::HasSameSubProp(
    const ZJsonObject &json1, const ZJsonObject &json2)
{
  return is_same_prop(get_prop(json1), get_prop(json2));
}

void ZDvidAnnotation::UpdateTime(ZJsonObject &json, const ZJsonObject &oldJson)
{
  std::string timeString = neutu::GetUtcTimeString();
  std::string modifiedTime;
  std::string createdTime;
  std::string checkedTime;

  if (oldJson.isEmpty()) {
    createdTime = timeString;
//    AddProperty(json, ZDvidAnnotation::KEY_CREATED_TIME, timeString);
  } else {
    if (HasSameSubProp(json, oldJson) == false) {
      modifiedTime = timeString;
    } else if (get_modified_time(json).empty()) {
      // use old modified time if the new json doesn't have one
      modifiedTime = get_modified_time(oldJson);
    }

    if (get_created_time(json).empty()) {
      createdTime = get_created_time(oldJson);
    }
  }

  if (IsChecked(json)) {
    if (IsChecked(oldJson) == false) {
      checkedTime = timeString;
//      AddProperty(json, ZDvidAnnotation::KEY_CHECKED_TIME, timeString);
    } else {
      if (get_checked_time(json).empty()) {
        checkedTime = get_checked_time(oldJson);
      }
    }
  } else {
    get_prop(json).removeKey("checkedTime");
  }

  if (createdTime.empty() == false) {
    AddProperty(json, ZDvidAnnotation::KEY_CREATED_TIME, createdTime);
  }
  if (modifiedTime.empty() == false) {
    AddProperty(json, ZDvidAnnotation::KEY_MODIFIED_TIME, modifiedTime);
  }
  if (checkedTime.empty() == false) {
    AddProperty(json, ZDvidAnnotation::KEY_CHECKED_TIME, checkedTime);
  }
}

void ZDvidAnnotation::AddProperty(
    ZJsonObject &json, const std::string &key, const std::string &value)
{
  ZJsonObject propJson = add_prop(json);
  propJson.setEntry(key, value);
/*
  ZJsonObject propJson = json.value("Prop");
  propJson.setEntry(key, value);
  if (!json.hasKey("Prop")) {
    json.setEntry("Prop", propJson);
  }
  */
}

void ZDvidAnnotation::AddProperty(
    ZJsonObject &json, const std::string &key, const char *value)
{
  ZJsonObject propJson = add_prop(json);
  propJson.setEntry(key, value);
}

void ZDvidAnnotation::Annotate(ZJsonObject &json, const std::string &annot)
{
  AddProperty(json, "annotation", annot);
}

void ZDvidAnnotation::AddProperty(
    ZJsonObject &json, const std::string &key, bool value)
{
  if (value) {
    AddProperty(json, key, "1");
  } else {
    AddProperty(json, key, "0");
  }
}

void ZDvidAnnotation::AddProperty(
    ZJsonObject &json, const std::string &key, int value)
{
  AddProperty(json, key, std::to_string(value));
}

void ZDvidAnnotation::RemoveProperty(ZJsonObject &json, const std::string &key)
{
  get_prop(json).removeKey(key.c_str());
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
  box.setMinCorner(getPosition().toPoint() - getRadius());
  box.setMaxCorner(getPosition().toPoint() + getRadius());

  return box;
}

//ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidAnnotation)

///////////////
ZJsonObject ZDvidAnnotation::Relation::toJsonObject() const
{
  return ZDvidAnnotation::MakeRelJson(m_to, GetName(m_relation));
}

std::string ZDvidAnnotation::Relation::GetName(
    ZDvidAnnotation::Relation::ERelation rel)
{
  switch (rel) {
  case ERelation::POSTSYN_TO:
    return "PostSynTo";
  case ERelation::PRESYN_TO:
    return "PreSynTo";
  case ERelation::CONVERGENT_TO:
    return "ConvergentTo";
  case ERelation::GROUPED_WITH:
    return "GroupedWith";
  default:
    break;
  }

  return "UnknownRelationship";
}
