#include "zflyemtodoitem.h"

#include <QColor>
#include <iostream>

#include "zpainter.h"
#include "zjsonparser.h"
#include "flyem/zflyemmisc.h"
#include "zstring.h"

const char* ZFlyEmToDoItem::ACTION_KEY = "action";
const char* ZFlyEmToDoItem::ACTION_SPLIT = "to split";
const char* ZFlyEmToDoItem::ACTION_SUPERVOXEL_SPLIT = "to split supervoxel";
const char* ZFlyEmToDoItem::ACTION_IRRELEVANT = "irrelevant";
const char* ZFlyEmToDoItem::ACTION_SPLIT_TAG = "split";
const char* ZFlyEmToDoItem::ACTION_SUPERVOXEL_SPLIT_TAG = "svsplit";
const char* ZFlyEmToDoItem::ACTION_IRRELEVANT_TAG = "irrelevant";
const char* ZFlyEmToDoItem::ACTION_MERGE = "to merge";
const char* ZFlyEmToDoItem::ACTION_MERGE_TAG = "merge";

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

ZFlyEmToDoItem::ZFlyEmToDoItem(int x, int y, int z)
{
  init(KIND_NOTE);
  setPosition(x, y, z);
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
    switch (getAction()) {
    case neutube::TO_DO:
      color.setRgb(255, 0, 0, 192);
      break;
    case neutube::TO_MERGE:
      color.setRgb(255, 164, 0, 192);
      break;
    case neutube::TO_SPLIT:
      color.setRgb(200, 0, 255, 192);
      break;
    case neutube::TO_SUPERVOXEL_SPLIT:
      color.setRgb(255, 80, 164, 192);
      break;
    case neutube::TO_DO_IRRELEVANT:
      color.setRgb(Qt::gray);
      break;
    }
  }

  return color;
}

neutube::EToDoAction ZFlyEmToDoItem::getAction() const
{
  const char *key = "action"; //coupled with setAction
  std::string value = getProperty<std::string>(key);
  neutube::EToDoAction action = neutube::TO_DO;
  if (value == ACTION_MERGE) {
    action = neutube::TO_MERGE;
  } else if (value == ACTION_SPLIT) {
    action = neutube::TO_SPLIT;
  } else if (value == ACTION_SUPERVOXEL_SPLIT) {
    action = neutube::TO_SUPERVOXEL_SPLIT;
  } else if (value == ACTION_IRRELEVANT) {
    action = neutube::TO_DO_IRRELEVANT;
  }

  return action;
}

void ZFlyEmToDoItem::setAction(neutube::EToDoAction action)
{
  switch (action) {
  case neutube::TO_DO:
    removeProperty(ACTION_KEY);
//    removeActionTag();
    break;
  case neutube::TO_MERGE:
    addProperty(ACTION_KEY, ACTION_MERGE);
//    addTag(std::string(ACTION_KEY) + ":" + ACTION_MERGE_TAG);
    break;
  case neutube::TO_SPLIT:
    addProperty(ACTION_KEY, ACTION_SPLIT);
//    addTag(std::string(ACTION_KEY) + ":" + ACTION_SPLIT_TAG);
    break;
  case neutube::TO_SUPERVOXEL_SPLIT:
    addProperty(ACTION_KEY, ACTION_SUPERVOXEL_SPLIT);
//    addTag(std::string(ACTION_KEY) + ":" + ACTION_SUPERVOXEL_SPLIT_TAG);
    break;
  case neutube::TO_DO_IRRELEVANT:
    addProperty(ACTION_KEY, ACTION_IRRELEVANT);
//    addTag(std::string(ACTION_KEY) + ":" + ACTION_IRRELEVANT_TAG);
    break;
  }

  syncActionTag();
}

std::string ZFlyEmToDoItem::GetActionTag(neutube::EToDoAction action)
{
  std::string tag;

  auto make_tag = [](const char *actionTag) {
    return std::string(ACTION_KEY) + ":" + actionTag; };

  switch (action) {
  case neutube::TO_DO:
    break;
  case neutube::TO_MERGE:
    tag = make_tag(ACTION_MERGE_TAG);
    break;
  case neutube::TO_SPLIT:
    tag = make_tag(ACTION_SPLIT_TAG);
    break;
  case neutube::TO_SUPERVOXEL_SPLIT:
    tag = make_tag(ACTION_SUPERVOXEL_SPLIT_TAG);
    break;
  case neutube::TO_DO_IRRELEVANT:
    tag = make_tag(ACTION_IRRELEVANT_TAG);
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
    if (ZString(*iter).startsWith(std::string(ACTION_KEY) + ":")) {
      iter = m_tagSet.erase(iter);
    } else {
      ++iter;
    }
  }
}

void ZFlyEmToDoItem::display(ZPainter &painter, int slice, EDisplayStyle /*option*/,
                           neutube::EAxis sliceAxis) const
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
  } else if (hasVisualEffect(neutube::display::Sphere::VE_BOUND_BOX)) {
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

  syncActionTag();
}


ZSTACKOBJECT_DEFINE_CLASS_NAME(ZFlyEmToDoItem)
