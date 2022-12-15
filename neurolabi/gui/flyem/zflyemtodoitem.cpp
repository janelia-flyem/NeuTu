#include "zflyemtodoitem.h"

#include <QColor>
#include <iostream>

#include "QsLog/QsLog.h"
#include "common/math.h"
#include "common/utilities.h"
#include "vis2d/zslicepainter.h"
#include "vis2d/utilities.h"
#include "data3d/displayconfig.h"

#include "zpainter.h"
#include "zjsonparser.h"
#include "flyem/zflyemmisc.h"
#include "zstring.h"

const char* ZFlyEmToDoItem::KEY_ACTION = "action";
const char* ZFlyEmToDoItem::ACTION_GENERAL = "normal";
const char* ZFlyEmToDoItem::ACTION_SPLIT = "to split";
const char* ZFlyEmToDoItem::ACTION_SUPERVOXEL_SPLIT = "to split supervoxel";
const char* ZFlyEmToDoItem::ACTION_IRRELEVANT = "irrelevant";
const char* ZFlyEmToDoItem::ACTION_SPLIT_TAG = "split";
const char* ZFlyEmToDoItem::ACTION_SUPERVOXEL_SPLIT_TAG = "svsplit";
const char* ZFlyEmToDoItem::ACTION_IRRELEVANT_TAG = "irrelevant";
const char* ZFlyEmToDoItem::ACTION_MERGE = "to merge";
const char* ZFlyEmToDoItem::ACTION_MERGE_TAG = "merge";
const char* ZFlyEmToDoItem::ACTION_TRACE_TO_SOMA = "trace to soma";
const char* ZFlyEmToDoItem::ACTION_TRACE_TO_SOMA_TAG = "trace_to_soma";
const char* ZFlyEmToDoItem::ACTION_NO_SOMA = "no soma";
const char* ZFlyEmToDoItem::ACTION_NO_SOMA_TAG = "no_soma";
const char* ZFlyEmToDoItem::ACTION_DIAGNOSTIC = "diagnostic";
const char* ZFlyEmToDoItem::ACTION_DIAGNOSTIC_TAG = "diagnostic";
const char* ZFlyEmToDoItem::ACTION_SEGMENTATION_DIAGNOSTIC =
    "segmentation-diagnostic";
const char* ZFlyEmToDoItem::ACTION_SEGMENTATION_DIAGNOSTIC_TAG =
    "segmentation_diagnostic";
const char* ZFlyEmToDoItem::ACTION_TIP_DETECTOR = "tip detector";
const char* ZFlyEmToDoItem::ACTION_TIP_DETECTOR_TAG = "tip_detector";
const char * ZFlyEmToDoItem::TODO_STATE_CHECKED = "checked";
const char * ZFlyEmToDoItem::TODO_STATE_UNCHECKED = "unchecked";
// after Slack discussion (https://flyem2pt0.slack.com/archives/C15S8GGVD/p1671119229175209) we
//    decided to mangle the English language in this way:
const char * ZFlyEmToDoItem::TODO_STATE_CHECKED_WONTFIX = "checked--wontfix";

const std::map<std::string, neutu::EToDoAction> ZFlyEmToDoItem::m_actionMap ={
  {ZFlyEmToDoItem::ACTION_GENERAL, neutu::EToDoAction::TO_DO},
  {ZFlyEmToDoItem::ACTION_MERGE, neutu::EToDoAction::TO_MERGE},
  {ZFlyEmToDoItem::ACTION_SPLIT, neutu::EToDoAction::TO_SPLIT},
  {ZFlyEmToDoItem::ACTION_SUPERVOXEL_SPLIT, neutu::EToDoAction::TO_SUPERVOXEL_SPLIT},
  {ZFlyEmToDoItem::ACTION_IRRELEVANT, neutu::EToDoAction::TO_DO_IRRELEVANT},
  {ZFlyEmToDoItem::ACTION_TRACE_TO_SOMA, neutu::EToDoAction::TO_TRACE_TO_SOMA},
  {ZFlyEmToDoItem::ACTION_NO_SOMA, neutu::EToDoAction::NO_SOMA},
  {ZFlyEmToDoItem::ACTION_DIAGNOSTIC, neutu::EToDoAction::DIAGNOSTIC},
  {ZFlyEmToDoItem::ACTION_SEGMENTATION_DIAGNOSTIC,
   neutu::EToDoAction::SEGMENTATION_DIAGNOSIC},
  {ZFlyEmToDoItem::ACTION_TIP_DETECTOR, neutu::EToDoAction::TIP_DETECTOR}
};

ZFlyEmToDoItem::ZFlyEmToDoItem()
{
  init(EKind::KIND_INVALID);
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
  init(EKind::KIND_NOTE);
  setPosition(pos);
}

ZFlyEmToDoItem::ZFlyEmToDoItem(int x, int y, int z)
{
  init(EKind::KIND_NOTE);
  setPosition(x, y, z);
}


std::ostream& operator<< (std::ostream &stream, const ZFlyEmToDoItem &item)
{
  //"Kind": (x, y, z)
  switch (item.getKind()) {
  case ZDvidAnnotation::EKind::KIND_POST_SYN:
    stream << "PostSyn";
    break;
  case ZDvidAnnotation::EKind::KIND_PRE_SYN:
    stream << "PreSyn";
    break;
  case ZDvidAnnotation::EKind::KIND_NOTE:
    stream << "Note";
    break;
  case ZDvidAnnotation::EKind::KIND_INVALID:
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
    switch (getAction()) {
    case neutu::EToDoAction::TO_DO:
      color.setRgb(255, 0, 0, 192);
      break;
    case neutu::EToDoAction::TO_MERGE:
      color.setRgb(255, 164, 0, 192);
      break;
    case neutu::EToDoAction::TO_SPLIT:
      color.setRgb(200, 0, 255, 192);
      break;
    case neutu::EToDoAction::TO_SUPERVOXEL_SPLIT:
      color.setRgb(255, 80, 164, 192);
      break;
    case neutu::EToDoAction::TO_DO_IRRELEVANT:
      color.setRgb(Qt::gray);
      break;
    case neutu::EToDoAction::TO_TRACE_TO_SOMA:
      color.setRgb(160, 80, 0, 192);
      break;
    case neutu::EToDoAction::DIAGNOSTIC:
    case neutu::EToDoAction::SEGMENTATION_DIAGNOSIC:
      color.setRgb(128, 0, 128, 192);
      break;
    case neutu::EToDoAction::TIP_DETECTOR:
      color.setRgb(255, 128, 128, 192);
      break;
    case neutu::EToDoAction::NO_SOMA:
      break;
    }
  }

  return color;
}

neutu::EToDoAction ZFlyEmToDoItem::getAction() const
{
  std::string actionName = getActionName();
  neutu::EToDoAction action = neutu::EToDoAction::TO_DO;
  if (m_actionMap.count(actionName) > 0) {
    action = m_actionMap.at(actionName);
  }

  return action;

#if 0
  return m_actionMap[getActionName()];

//  const char *key = "action"; //coupled with setAction
  std::string value = getActionName();
  neutu::EToDoAction action = neutu::EToDoAction::TO_DO;
  if (value == ACTION_MERGE) {
    action = neutu::EToDoAction::TO_MERGE;
  } else if (value == ACTION_SPLIT) {
    action = neutu::EToDoAction::TO_SPLIT;
  } else if (value == ACTION_SUPERVOXEL_SPLIT) {
    action = neutu::EToDoAction::TO_SUPERVOXEL_SPLIT;
  } else if (value == ACTION_IRRELEVANT) {
    action = neutu::EToDoAction::TO_DO_IRRELEVANT;
  } else if (value == )

  return action;
#endif
}

std::string ZFlyEmToDoItem::getActionName() const
{
  const char *key = "action"; //coupled with setAction
  std::string value = getProperty<std::string>(key);

  return value;
}

bool ZFlyEmToDoItem::hasSomaAction() const
{
  neutu::EToDoAction action = getAction();

  return (action == neutu::EToDoAction::TO_TRACE_TO_SOMA) ||
      (action == neutu::EToDoAction::NO_SOMA);
}

void ZFlyEmToDoItem::setAction(const std::string &action)
{
  if (m_actionMap.count(action) > 0) {
    setAction(m_actionMap.at(action));
  }
}

void ZFlyEmToDoItem::setAction(neutu::EToDoAction action)
{
  removeActionTag();

  if (action == neutu::EToDoAction::TO_DO) {
    removeProperty(KEY_ACTION);
  } else {
    auto iter = std::find_if(
          m_actionMap.begin(), m_actionMap.end(),
          [action](const std::map<std::string, neutu::EToDoAction>::value_type& v) {
      return v.second == action;});
    if (iter != m_actionMap.end()) {
      addProperty(KEY_ACTION, iter->first);
    };
  }

  syncActionTag();
}

std::string ZFlyEmToDoItem::GetActionTag(neutu::EToDoAction action)
{
  std::string tag;

  auto make_tag = [](const char *actionTag) {
    return std::string(KEY_ACTION) + ":" + actionTag; };

  switch (action) {
  case neutu::EToDoAction::TO_DO:
    break;
  case neutu::EToDoAction::TO_MERGE:
    tag = make_tag(ACTION_MERGE_TAG);
    break;
  case neutu::EToDoAction::TO_SPLIT:
    tag = make_tag(ACTION_SPLIT_TAG);
    break;
  case neutu::EToDoAction::TO_SUPERVOXEL_SPLIT:
    tag = make_tag(ACTION_SUPERVOXEL_SPLIT_TAG);
    break;
  case neutu::EToDoAction::TO_DO_IRRELEVANT:
    tag = make_tag(ACTION_IRRELEVANT_TAG);
    break;
  case neutu::EToDoAction::TO_TRACE_TO_SOMA:
    tag = make_tag(ACTION_TRACE_TO_SOMA_TAG);
    break;
  case neutu::EToDoAction::NO_SOMA:
    tag = make_tag(ACTION_NO_SOMA_TAG);
    break;
  case neutu::EToDoAction::DIAGNOSTIC:
    tag = make_tag(ACTION_DIAGNOSTIC_TAG);
    break;
  case neutu::EToDoAction::SEGMENTATION_DIAGNOSIC:
    tag = make_tag(ACTION_SEGMENTATION_DIAGNOSTIC_TAG);
    break;
  case neutu::EToDoAction::TIP_DETECTOR:
    tag = make_tag(ACTION_TIP_DETECTOR_TAG);
    break;
  }

  return tag;
}

void ZFlyEmToDoItem::syncActionTag()
{
  std::string tag;

  if (!isChecked()) {
    tag = GetActionTag(getAction());
  }

  if (tag.empty()) {
    removeActionTag();
  } else {
    addTag(tag);
  }
}

void ZFlyEmToDoItem::removeActionTag()
{
  for (std::set<std::string>::iterator iter = m_tagSet.begin();
       iter != m_tagSet.end(); ) {
    if (ZString(*iter).startsWith(std::string(KEY_ACTION) + ":")) {
      iter = m_tagSet.erase(iter);
    } else {
      ++iter;
    }
  }
}

QList<std::vector<QPointF> > ZFlyEmToDoItem::makeOutlineDecoration(double x, double y, double radius) const
{
  QList<std::vector<QPointF> > outlineArray;

  switch (getAction()) {
  case neutu::EToDoAction::DIAGNOSTIC:
    outlineArray.append(flyem::MakeCrossKey(QPointF(x, y), radius, 0.2));
    break;
  case neutu::EToDoAction::SEGMENTATION_DIAGNOSIC:
  {
    outlineArray.append(flyem::MakeCrossKey(QPointF(x, y), radius, 0.2));
    double dr1 = radius * 0.2;
    double dr2 = radius - dr1;
    outlineArray.append({QPointF(x - dr1, y - dr2), QPointF(x + dr1, y - dr2)});
    outlineArray.append({QPointF(x - dr1, y + dr2), QPointF(x + dr1, y + dr2)});
    outlineArray.append({QPointF(x - dr2, y - dr1), QPointF(x - dr2, y + dr1)});
    outlineArray.append({QPointF(x + dr2, y - dr1), QPointF(x + dr2, y + dr1)});
  }
    break;
  case neutu::EToDoAction::TIP_DETECTOR:
    outlineArray.append(flyem::MakeStar(QPointF(x, y), radius, 0.1));
    break;
  default:
    outlineArray.append(flyem::MakeStar(QPointF(x, y), radius, 0.25));
    break;
  }

  return outlineArray;
}

void ZFlyEmToDoItem::PaintOutline(
    ZPainter &painter, const QList<std::vector<QPointF> > &outline)
{
  for(const auto &polyline : outline) {
    painter.drawPolyline(polyline);
  }
}

#if 0
void ZFlyEmToDoItem::viewSpaceAlignedDisplayC(
      QPainter *painter, const ViewSpaceAlignedDisplayConfig &config,
      const ZPoint &center) const
{
  bool visible = true;
  int z = config.z;

  if (config.sliceMode == EDisplaySliceMode::PROJECTION) {
    visible = isProjectionVisible();
  } else {
    visible = isSliceVisible(z, neutu::EAxis::Z);
  }

  double radius = getRadius(z, neutu::EAxis::Z);

//  ZIntPoint center = getPosition();

  bool isFocused = (z == neutu::iround(center.getZ()));

  double basePenWidth = getBasePenWidth();

  QPen pen = painter->pen();
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
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    if (radius > 0.0) {
      double x = center.getX();
      double y = center.getY();
      pen.setWidthF(basePenWidth * 0.5);
      painter->setPen(pen);

      std::vector<QPointF> vertexArray(5);
      vertexArray[0] = QPointF(x - radius, y);
      vertexArray[1] = QPointF(x, y - radius);
      vertexArray[2] = QPointF(x + radius, y);
      vertexArray[3] = QPointF(x, y + radius);
      vertexArray[4] = vertexArray[0];

      painter->drawLine(vertexArray[0], vertexArray[2]);
      painter->drawLine(vertexArray[1], vertexArray[3]);

      pen.setWidthF(basePenWidth);
      painter->setPen(pen);

      QList<std::vector<QPointF>> outline = makeOutlineDecoration(x, y, radius);
      if (hasSomaAction()) {
        outline.append(vertexArray);
      }

      for(const auto &polyline : outline) {
        painter->drawPolyline(polyline.data(), polyline.size());
      }

      if (getPriority() > 0) {
        painter->save();
        double p = double(getPriority()) / 9;

        painter->drawEllipse(
              QPointF(x, y - ((0.5 - p) * radius)), radius * 0.05, radius * 0.05);

        painter->restore();

      }
    }

  }

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
    double halfSize = getRadius();
    if (m_usingCosmeticPen) {
      halfSize += 0.5;
    }
    rect.setLeft(center.getX() - halfSize);
    rect.setTop(center.getY() - halfSize);
    rect.setWidth(halfSize * 2);
    rect.setHeight(halfSize * 2);

    painter->setBrush(Qt::NoBrush);
    pen.setWidthF(basePenWidth * 0.5);
    if (visible) {
      pen.setStyle(Qt::SolidLine);
    } else {
      pen.setStyle(Qt::DotLine);
    }
    painter->setPen(pen);
    painter->drawRect(rect);
  }
}


void ZFlyEmToDoItem::viewSpaceAlignedDisplay(
      QPainter *painter, const ViewSpaceAlignedDisplayConfig &config) const
{
  viewSpaceAlignedDisplayC(painter, config, getPosition().toPoint());
}
#endif

#if 0
void ZFlyEmToDoItem::display(ZPainter &painter, const DisplayConfig &config) const
{
  if (config.sliceAxis == neutu::EAxis::ARB) {
    ZIntPoint center = getPosition();
    viewSpaceAlignedDisplayC(
          painter.getPainter(), config.alignedConfig,
          config.cutPlane.getAffinePlane().align(center.toPoint()));
  } else {
    display(painter, config.getSlice(painter.getZOffset()),
            config.getStyle(), config.sliceAxis);
  }
}

void ZFlyEmToDoItem::display(ZPainter &painter, int slice, EDisplayStyle option,
                           neutu::EAxis sliceAxis) const
{
  ViewSpaceAlignedDisplayConfig config;
  config.z = painter.getZ(slice);
  config.style = option;
  config.sliceMode =
      slice < 0 ? EDisplaySliceMode::PROJECTION : EDisplaySliceMode::SINGLE;

  if (sliceAxis == neutu::EAxis::Z) {
    viewSpaceAlignedDisplay(painter.getPainter(), config);
  } else {
    if (sliceAxis != neutu::EAxis::ARB) {
      ZFlyEmToDoItem aligned(*this);
      ZIntPoint center = getPosition();
      center.shiftSliceAxis(sliceAxis);
      aligned.viewSpaceAlignedDisplayC(
            painter.getPainter(), config, center.toPoint());
    }
  }
#if 0
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
    painter.setBrush(Qt::NoBrush);

    if (radius > 0.0) {
      int x = center.getX();
      int y = center.getY();
      pen.setWidthF(basePenWidth * 0.5);
      painter.setPen(pen);

      std::vector<QPointF> vertexArray(5);
      vertexArray[0] = QPointF(x - radius, y);
      vertexArray[1] = QPointF(x, y - radius);
      vertexArray[2] = QPointF(x + radius, y);
      vertexArray[3] = QPointF(x, y + radius);
      vertexArray[4] = vertexArray[0];

      painter.drawLine(vertexArray[0], vertexArray[2]);
      painter.drawLine(vertexArray[1], vertexArray[3]);

      pen.setWidthF(basePenWidth);
      painter.setPen(pen);

      QList<std::vector<QPointF>> outline = makeOutlineDecoration(x, y, radius);
      if (hasSomaAction()) {
        outline.append(vertexArray);
      }

      PaintOutline(painter, outline);

      /*
      if (getAction() != neutu::EToDoAction::DIAGNOSTIC) {
        QPointF ptArray[9];
        flyem::MakeStar(QPointF(x, y), radius, ptArray);
        painter.drawPolyline(ptArray, 9);
      } else {
        QVector<QPointF> ptArray = flyem::MakeCrossKey(QPointF(x, y), radius);
        painter.drawPolyline(ptArray.constData(), ptArray.size());
      }
      */

      /*
      if (hasSomaAction()) {
        painter.drawPolyline(vertexArray);
      }
      */

      if (getPriority() > 0) {
        painter.save();
        double p = double(getPriority()) / 9;

//        QPen priorityPen = pen;
//        priorityPen.setColor(
//              QColor(int(std::round((1.0 - p) * 127)) + 128, 0, 0, color.alpha()));
//        painter.setPen(priorityPen);

        painter.drawEllipse(
              QPointF(x, y - ((0.5 - p) * radius)), radius * 0.05, radius * 0.05);


        painter.restore();

      }
    }

  }

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
#endif
}
#endif

bool ZFlyEmToDoItem::isChecked() const
{
   // we switched from boolean (stored as "0" and "1") to an enum (with longer strings) but support both
  if (m_propertyJson.hasKey("checked")) {
    std::string checkedStr = std::string(ZJsonParser::stringValue(m_propertyJson["checked"]));
    return (checkedStr == "1" || checkedStr == TODO_STATE_CHECKED ||
        checkedStr == TODO_STATE_CHECKED_WONTFIX);
  }

  return false;
}

bool ZFlyEmToDoItem::display(QPainter *painter, const DisplayConfig &config) const
{
  this->m_hitMap[config.getViewId()] = [](const ZStackObject*,double,double,double) {
    return false;
  };

  if (isVisible()) {
    ZSlice3dPainter s3Painter = neutu::vis2d::Get3dSlicePainter(config);

    neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});

    QPen pen(getDisplayColor());
    pen.setCosmetic(m_usingCosmeticPen);
    painter->setPen(pen);

    const double depthScale = 2.0;
    const double fadingFactor = 1.0;

    ZPoint center = getPosition().toPoint();
    s3Painter.drawStar(
          painter, center, getRadius(), depthScale, fadingFactor);
    if (isSelected()) {
      neutu::SetPenColor(painter, Qt::yellow);
      s3Painter.drawBoundBox(painter, center, getRadius(), depthScale);
    }

    if (s3Painter.getPaintedHint()) {
      this->m_hitMap[config.getViewId()] =
          [=](const ZStackObject *obj, double x, double y ,double z) {
        auto s = dynamic_cast<const ZFlyEmToDoItem*>(obj);
        ZIntPoint center = s->getPosition();
        return ZSlice3dPainter::BallHitTest(
              x, y, z, center.getX(), center.getY(), center.getZ(), s->getRadius(),
              config.getWorldViewTransform(), depthScale);
      };

//      this->_hit = s3Painter.getBallHitFunc(
//            getX(), getY(), getZ(), getRadius(), depthScale);
    }/* else {
      this->_hit = [](const ZStackObject*,double,double,double) { return false; };
    }*/

    return s3Painter.getPaintedHint();
  }

  return false;
}

/*
bool ZFlyEmToDoItem::isCheckable() const
{

}
*/

void ZFlyEmToDoItem::setChecked(bool checked)
{
  // we switched from boolean (stored as "0" and "1") to an enum (with longer strings) but support both
  // note you can only set state CHECKED_WONT_DO using the other setChecked() method
  std::string checkedStr = TODO_STATE_UNCHECKED;
  if (checked) {
    checkedStr = TODO_STATE_CHECKED;
  }
  m_propertyJson.setEntry("checked", checkedStr);
  syncActionTag();
}

void ZFlyEmToDoItem::setChecked(std::string state) {
    // this is a little risky, as I don't check that the state matches a known constant;
    //  but it is expected to be called from UI code that restricts the input values
    m_propertyJson.setEntry("checked", state);
    syncActionTag();
}

void ZFlyEmToDoItem::setCheckedWontfix() {
    // convenience, so I don't have to include this enum distantly
    m_propertyJson.setEntry("checked", TODO_STATE_CHECKED_WONTFIX);
    syncActionTag();
}

std::string ZFlyEmToDoItem::getChecked() const {
    if (m_propertyJson.hasKey("checked")) {
        std::string checkedString = ZJsonParser::stringValue(m_propertyJson["checked"]);
        // support old "0", "1" as well as full strings
        if (checkedString == "0") {
            return TODO_STATE_UNCHECKED;
        } else if (checkedString == "1") {
            return TODO_STATE_CHECKED;
        } else {
        return checkedString;
        }
    }
    return "";
}

std::string ZFlyEmToDoItem::GetPriorityName(int p)
{
  if (p > 0) {
    switch (p) {
    case 1:
    case 2:
    case 3:
      return "High";
    case 4:
    case 5:
      return "Medium";
    default:
      return "Low";
    }
  }

  return "Unknown";
}

std::string ZFlyEmToDoItem::getPriorityName() const
{
  return GetPriorityName(getPriority());
}

int ZFlyEmToDoItem::getPriority() const
{
  int p = 0;

  if (m_propertyJson.hasKey("priority")) {
    try {
      p = std::stoi(ZJsonParser::stringValue(m_propertyJson["priority"]));
    } catch (const std::invalid_argument &ia) {
      LERROR() << "Invalid priority:" << ia.what();
    }
  }

  return p;
}

void ZFlyEmToDoItem::setPriority(int p)
{
  if (p > 0) {
    addProperty("priority", std::to_string(p));
  } else {
    removeProperty("priority");
  }
}


//ZSTACKOBJECT_DEFINE_CLASS_NAME(ZFlyEmToDoItem)
