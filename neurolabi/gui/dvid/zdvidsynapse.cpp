#include "zdvidsynapse.h"
#include <QtCore>
#include <QPen>

#include "common/math.h"
#include "common/utilities.h"
#include "logging/zqslog.h"
#include "data3d/displayconfig.h"
#include "vis2d/zslicepainter.h"
#include "vis2d/utilities.h"

//#include "zpainter.h"
#include "zjsonobject.h"
#include "zjsonparser.h"
#include "zjsonfactory.h"
#include "zjsonarray.h"
#include "zstackball.h"
#include "c_json.h"
#include "zlinesegmentobject.h"
#include "dvid/zdvidannotation.h"
//#include "tz_constant.h"
#include "dvid/zdvidreader.h"
#include "zvaa3dmarker.h"
#include "flyem/zflyemmisc.h"

const double ZDvidSynapse::DEFAULT_CONFIDENCE = 1.0;

ZDvidSynapse::ZDvidSynapse()
{
  init();
}

void ZDvidSynapse::init()
{
  m_type = GetType();
  m_projectionVisible = false;
  m_kind = EKind::KIND_INVALID;
  m_usingCosmeticPen = true;
  setDefaultRadius();
  setDefaultColor();
}

/*
ZDvidSynapse::ZDvidSynapse(const ZDvidSynapse &synapse)
  : ZDvidAnnotation(synapse)
{
  m_isPartnerVerified = synapse.m_isPartnerVerified;
  m_partnerKind = synapse.m_partnerKind;
  m_partnerStatus = synapse.m_partnerStatus;
}
*/

bool ZDvidSynapse::hasConfidenceProperty() const
{
  return (m_propertyJson.hasKey("confidence")) || m_propertyJson.hasKey("conf");
}

void ZDvidSynapse::removeConfidenceProperty()
{
  RemoveConfidenceProp(m_propertyJson);
}

double ZDvidSynapse::getConfidence() const
{
  double c = DEFAULT_CONFIDENCE;

  std::string confStr = getConfidenceStr();
  if (!confStr.empty()) {
    c = std::atof(confStr.c_str());
  }

  return c;
}

std::string ZDvidSynapse::getConfidenceStr() const
{
  std::string confStr;
  if (m_propertyJson.hasKey("confidence")) {
    confStr = ZJsonParser::stringValue(m_propertyJson["confidence"]);
  } else if (m_propertyJson.hasKey("conf")) {
    confStr = ZJsonParser::stringValue(m_propertyJson["conf"]);
  }

  return confStr;
}

void ZDvidSynapse::setConfidence(const std::string str)
{
  SetConfidenceProp(m_propertyJson, str);
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
#if 0
  if (m_propertyJson.hasKey("confidence")) {
    m_propertyJson.removeKey("confidence");
  }

  // remember, store props as strings!
  std::ostringstream stream;
  stream << c;
  m_propertyJson.setEntry("conf", stream.str());
#endif

  SetConfidenceProp(m_propertyJson, c);
}

void ZDvidSynapse::RemoveConfidenceProp(ZJsonObject &json)
{
  json.removeKey("confidence");
  json.removeKey("conf");
}

void ZDvidSynapse::SetConfidenceProp(ZJsonObject &propJson, std::string conf)
{
  propJson.removeKey("confidence");

  if (!conf.empty()) {
    propJson.setEntry("conf", conf);
  } else {
    propJson.removeKey("conf");
  }
}

void ZDvidSynapse::SetConfidenceProp(ZJsonObject &propJson, double conf)
{
  // remember, store props as strings!
  std::ostringstream stream;
  stream << conf;
  SetConfidenceProp(propJson, stream.str());
}

void ZDvidSynapse::SetConfidence(ZJsonObject &json, double conf)
{
  ZJsonObject propJson = json.value("Prop");
  SetConfidenceProp(propJson, conf);
  if (!propJson.hasKey("Prop")) {
    json.setEntry("Prop", propJson);
  }
}

void ZDvidSynapse::SetConfidence(ZJsonObject &json, std::string conf)
{
  ZJsonObject propJson = json.value("Prop");
  SetConfidenceProp(propJson, conf);
  if (!propJson.hasKey("Prop")) {
    json.setEntry("Prop", propJson);
  }
}

bool ZDvidSynapse::isVerified() const
{
  const std::string &userName = getUserName();
  if (!userName.empty()) {
    if (userName[0] != '$') {
      return true;
    }
  }

  return false;
}

bool ZDvidSynapse::isProtocolVerified(const ZDvidTarget &target) const
{
  if (!isVerified()) {
    return false;
  }

  bool v = true;

  if (getKind() == EKind::KIND_PRE_SYN) {
    std::vector<ZIntPoint> psdArray = getPartners();
    if (!psdArray.empty()) {
      ZDvidReader reader;
      if (reader.open(target)) {
        for (std::vector<ZIntPoint>::const_iterator iter = psdArray.begin();
             iter != psdArray.end(); ++iter) {
          const ZIntPoint &pt = *iter;
          ZDvidSynapse synapse =
              reader.readSynapse(pt, dvid::EAnnotationLoadMode::NO_PARTNER);
          if (!synapse.isVerified()) {
            v = false;
            break;
          }
        }
      }
    }
  } else {
    v = false;
  }

  return v;
}

void ZDvidSynapse::setVerified(const std::string &userName)
{
  setUserName(userName);
}

QColor ZDvidSynapse::GetArrowColor(bool verified)
{
  QColor color(255, 0, 0);
  if (verified) {
    color = QColor(0, 255, 0);
  }
  color.setAlpha(100);

  return color;
}

#if 0
void ZDvidSynapse::display(ZPainter &painter, int slice, EDisplayStyle option,
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
    QPen pen;

    QColor color = getColor();

    double alpha = 1.0;
    if (option == EDisplayStyle::SKELETON) {
      alpha = 0.1;
    }

    if (!isFocused) {
      alpha *= radius / m_radius;
      alpha *= alpha * 0.5;
      alpha += 0.1;  
    }
    color.setAlphaF(alpha * color.alphaF());

    pen.setColor(color);

    painter.setPen(pen);

    painter.setBrush(Qt::NoBrush);
    if (isFocused) {
      int x = center.getX();
      int y = center.getY();
      painter.drawLine(QPointF(x - 1, y), QPointF(x + 1, y));
      painter.drawLine(QPointF(x, y - 1), QPointF(x, y + 1));

      if (getStatus() == EStatus::DUPLICATED) {
        painter.drawEllipse(
              QPointF(center.getX(), center.getY()), 1, 1);
      }
    }
    if (radius > 0.0) {
      double oldWidth = pen.widthF();
      QColor oldColor = pen.color();
      if (getKind() == EKind::KIND_POST_SYN) {
        if (option != EDisplayStyle::SKELETON) {
          pen.setWidthF(oldWidth + 1.0);
        }
        if (isSelected()) {
          pen.setColor(QColor(255, 0, 255, oldColor.alpha()));
        }
      } else {
        if (isSelected()) {
          pen.setWidthF(pen.widthF() + 1.0);
        }
      }

      painter.setPen(pen);
      painter.drawEllipse(QPointF(center.getX(), center.getY()),
                          radius, radius);
      pen.setWidthF(oldWidth);
      pen.setColor(oldColor);
    }
//    QString decorationText;

    if (isVerified()) {
      //        decorationText = "U";
      color.setRgb(0, 0, 0);

      if (isSelected()) {
        if (getKind() == EKind::KIND_PRE_SYN) {
          color.setRgb(0, 255, 0);
          size_t index = 0;
          for (std::vector<bool>::const_iterator iter = m_isPartnerVerified.begin();
               iter != m_isPartnerVerified.end(); ++iter, ++index) {
            bool verified = *iter;
            if (m_partnerKind[index] == EKind::KIND_POST_SYN) {
              if (!verified) {
                color.setRgb(0, 0, 0);
                break;
              }
            }
          }
        }
      }

      color.setAlphaF(alpha);
      pen.setColor(color);
      pen.setWidthF(pen.widthF() + 0.5);
      painter.setPen(pen);
      double margin = 0.5;
      painter.drawLine(QPointF(center.getX(), center.getY() + radius - margin),
                       QPointF(center.getX() + radius - margin, center.getY()));
      painter.drawLine(QPointF(center.getX(), center.getY() + radius - margin),
                       QPointF(center.getX() - radius + margin, center.getY()));
    }


    double conf  = getConfidence();
    if (conf < 1.0) {
//      double lineWidth = radius * conf * 0.5 + 1.0;
      double lineWidth = radius * 0.5;
      double red = 1.0 - conf;
      double green = conf;
      QColor color;
      color.setRedF(red);
      if (getKind() == ZDvidAnnotation::EKind::KIND_POST_SYN) {
        color.setBlueF(green);
      } else {
        color.setGreenF(green);
      }
      color.setAlphaF(alpha);
      painter.setPen(color);
      double x = center.getX();
      double y = center.getY();

      int startAngle = 0;
      int spanAngle = neutu::iround((1.0 - conf) * 180) * 16;
      painter.drawArc(QRectF(QPointF(x - lineWidth, y - lineWidth),
                             QPointF(x + lineWidth, y + lineWidth)),
                      startAngle, spanAngle);
    }

#if 0
    int height = iround(getRadius() * 1.5);
    int width = decorationText.size() * height;

    if (decorationText !=   m_textDecoration.text()) {
      m_textDecoration.setText(decorationText);
      m_textDecoration.setTextWidth(width);
    }

    if (!decorationText.isEmpty()) {
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
      painter.drawStaticText(center.getX() - height / 2, center.getY(),
                             m_textDecoration);
//      painter.drawText(center.getX() - height / 2, center.getY(), width, height,
//                       Qt::AlignLeft, decorationText);
      painter.setPen(oldColor);
    }
#endif
  }


  QPen pen;
  pen.setCosmetic(m_usingCosmeticPen);

  bool drawingBoundBox = false;
  bool drawingArrow = false;
  if (isSelected()) {
    if (visible) {
      drawingBoundBox = true;
    } else {
      drawingArrow = true;
    }

    QColor color;
    color.setRgb(255, 255, 0, 255);
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

  if (drawingArrow) {
    painter.setPen(pen);
    QRectF rect(center.getX() - m_radius, center.getY() - m_radius,
                m_radius + m_radius, m_radius + m_radius);

//    pen.setStyle(Qt::SolidLine);
//    pen.setColor(GetArrowColor(isVerified()));
//    painter.setPen(pen);
    QPointF ptArray[4];
    //      double s = 5.0;
    if (z > center.getZ()) {
      flyem::MakeTriangle(rect, ptArray, neutu::ECardinalDirection::NORTH);
      /*
        pt[0] = QPointF(rect.center().x() - rect.width() / s,
                        rect.top() + rect.height() / s);
        pt[1] = QPointF(rect.center().x(),
                        rect.top() - rect.height() / s);
        pt[2] = QPointF(rect.center().x() + rect.width() / s,
                        rect.top() + rect.height() / s);
*/
    } else {
      flyem::MakeTriangle(rect, ptArray, neutu::ECardinalDirection::SOUTH);
      /*
        pt[0] = QPointF(rect.center().x() - rect.width() / s,
                        rect.bottom() - rect.height() / s);
        pt[1] = QPointF(rect.center().x(),
                        rect.bottom() + rect.height() / s);
        pt[2] = QPointF(rect.center().x() + rect.width() / s,
                        rect.bottom() - rect.height() / s);
                        */
    }
    painter.drawPolyline(ptArray, 4);
//      painter.drawLine(pt[0], pt[1]);
//      painter.drawLine(pt[1], pt[2]);
//      painter.drawLine(pt[0], pt[2]);
  }

  if (isSelected()) {
    pen.setStyle(Qt::SolidLine);

    size_t index = 0;
    if (m_isPartnerVerified.size() == m_partnerHint.size()) {
      for (std::vector<ZIntPoint>::const_iterator iter = m_partnerHint.begin();
           iter != m_partnerHint.end(); ++iter, ++index) {
        pen.setColor(GetArrowColor(m_isPartnerVerified[index]));
        painter.setPen(pen);

        const ZIntPoint &partner = *iter;
        double len = 0.0;
        if (partner.getZ() < z && getPosition().getZ() < z) {
          len = -1.0;
        } else if (partner.getZ() > z && getPosition().getZ() > z) {
          len = 1.0;
        }

        if (len != 0.0) {
          QPointF pt[3];
          pt[0].setX(partner.getX() - len);
          pt[0].setY(partner.getY() - len);

          pt[1].setX(partner.getX() + len);
          pt[1].setY(partner.getY() - len);

          pt[2].setX(partner.getX());
          pt[2].setY(partner.getY() + len);


          painter.drawLine(pt[0], pt[1]);
          painter.drawLine(pt[1], pt[2]);
          painter.drawLine(pt[0], pt[2]);
        }

        if (m_partnerKind[index] == EKind::KIND_POST_SYN) {
          ZDvidSynapse partnerSynapse;
          partnerSynapse.setKind(EKind::KIND_POST_SYN);
          partnerSynapse.setStatus(m_partnerStatus[index]);
          partnerSynapse.setPosition(partner);
          partnerSynapse.setDefaultColor();
          partnerSynapse.setDefaultRadius();
          painter.save();
          partnerSynapse.display(
                painter, slice, ZStackObject::EDisplayStyle::NORMAL, sliceAxis);
          painter.restore();
        }
      }
    }

    index = 0;
    for (std::vector<ZIntPoint>::const_iterator iter = m_partnerHint.begin();
         iter != m_partnerHint.end(); ++iter, ++index) {
      ZLineSegmentObject line;
      line.setStartPoint(getPosition());
      line.setEndPoint(*iter);
      if (getKind() == EKind::KIND_PRE_SYN && m_partnerKind[index] == EKind::KIND_PRE_SYN) {
        line.setColor(QColor(0, 255, 255));
        line.setFocusColor(QColor(0, 255, 255));
      } else if (m_partnerKind[index] == EKind::KIND_UNKNOWN) {
        line.setColor(QColor(164, 0, 0));
        line.setFocusColor(QColor(255, 0, 0));
      } else {
        line.setColor(QColor(255, 255, 0));
        line.setFocusColor(QColor(255, 0, 255));
      }

      line.setVisualEffect(neutu::display::Line::VE_LINE_PROJ);
//      line.display(painter, slice, option, sliceAxis);

      /*
      ZIntPoint pos = *iter;
      painter.drawLine(getPosition().getX(), getPosition().getY(),
                       pos.getX(), pos.getY());
                       */
    }
  }
}
#endif

bool ZDvidSynapse::display(QPainter *painter, const DisplayConfig &config) const
{
  bool painted = false;
  this->_hit = [](const ZStackObject*,double,double,double) { return false; };

  if (isVisible()) {
    ZSlice3dPainter s3Painter = neutu::vis2d::Get3dSlicePainter(config);

    neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});

    QPen pen(getColor());
    pen.setCosmetic(m_usingCosmeticPen);
    painter->setPen(pen);

    const double depthScale = 2.0;
    const double fadingFactor = 1.0;
    ZPoint pos = getPosition().toPoint();
    s3Painter.drawBall(painter, pos, getRadius(), depthScale, fadingFactor);

    if (isSelected()) {
      neutu::SetPenColor(painter, Qt::yellow);
      s3Painter.drawBoundBox(painter, pos, getRadius(), depthScale);

      size_t index = 0;
      for (std::vector<ZIntPoint>::const_iterator iter = m_partnerHint.begin();
           iter != m_partnerHint.end(); ++iter, ++index) {
        if (getKind() == EKind::KIND_PRE_SYN && m_partnerKind[index] == EKind::KIND_PRE_SYN) {
          neutu::SetPenColor(painter, QColor(0, 255, 255));
        } else if (m_partnerKind[index] == EKind::KIND_UNKNOWN) {
          neutu::SetPenColor(painter, QColor(255, 0, 0));
        } else {
          neutu::SetPenColor(painter, QColor(255, 0, 255));
        }
        ZLineSegment line(getPosition().toPoint(), iter->toPoint());
        s3Painter.drawLine(painter, line);
      }
    }

    if (s3Painter.getPaintedHint()) {
      this->_hit = [=](const ZStackObject *obj, double x, double y ,double z) {
        auto s = dynamic_cast<const ZDvidSynapse*>(obj);
        return ZSlice3dPainter::BallHitTest(
              x, y, z, s->getX(), s->getY(), s->getZ(), s->getRadius(),
              config.getWorldViewTransform(), depthScale);
      };
      /*
      this->_hit = s3Painter.getBallHitFunc(
            getX(), getY(), getZ(), getRadius(), depthScale);
            */
    }

    painted = s3Painter.getPaintedHint();
  }

  return false;
}

std::ostream& operator<< (std::ostream &stream, const ZDvidSynapse &synapse)
{
  //"Kind": (x, y, z)
  switch (synapse.getKind()) {
  case ZDvidSynapse::EKind::KIND_POST_SYN:
    stream << "PostSyn";
    break;
  case ZDvidSynapse::EKind::KIND_PRE_SYN:
    stream << "PreSyn";
    break;
  case ZDvidSynapse::EKind::KIND_INVALID:
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
  case ZDvidSynapse::EKind::KIND_POST_SYN:
    rel = "PostSynTo";
    break;
  case ZDvidSynapse::EKind::KIND_PRE_SYN:
    rel = "PreSynTo";
    break;
  default:
    rel = "UnknownRelationship";
  }

  return MakeRelJson(pt, rel);
}

ZDvidAnnotation::EKind ZDvidSynapse::getPartnerKind(size_t i) const
{
  EKind kind = EKind::KIND_INVALID;
  if (i < m_partnerKind.size()) {
    kind = m_partnerKind[i];
  }

  return kind;
}

ZVaa3dMarker ZDvidSynapse::toVaa3dMarker(double radius) const
{
  ZVaa3dMarker marker;

  marker.setCenter(
        getPosition().getX(), getPosition().getY(), getPosition().getZ());
  if (getKind() == EKind::KIND_PRE_SYN) {
    marker.setColor(255, 255, 0);
    marker.setType(1);
  } else {
    marker.setColor(128, 128, 128);
    marker.setType(2);
  }

  std::ostringstream commentStream;
  commentStream << getBodyId();
  marker.setName(commentStream.str());

  marker.setRadius(radius);

  return marker;
}

void ZDvidSynapse::updatePartnerProperty(const ZDvidReader &reader)
{
  m_isPartnerVerified.resize(m_partnerHint.size(), false);
  m_partnerKind.resize(m_partnerHint.size(), EKind::KIND_UNKNOWN);
  m_partnerStatus.resize(m_partnerHint.size(), EStatus::NORMAL);

  if (reader.good()) {
    for (size_t i = 0; i < m_partnerHint.size(); ++i) {
      ZDvidSynapse synapse = reader.readSynapse(
            m_partnerHint[i], dvid::EAnnotationLoadMode::PARTNER_LOCATION);
      if (synapse.isValid()) {
        if (synapse.hasPartner(getPosition())) {
          m_isPartnerVerified[i] = synapse.isVerified();
          m_partnerKind[i] = synapse.getKind();
          m_partnerStatus[i] = synapse.getStatus();
        }/* else {
          LWARN() << "Inconsistent synapse link:" << getPosition().toString()
                  << "->" << synapse.getPosition().toString();
        }*/
      } else {
        m_isPartnerVerified[i] = false;
        m_partnerKind[i] = ZDvidSynapse::EKind::KIND_INVALID;
      }
    }
  }
}

std::string ZDvidSynapse::getConnString(
    const std::unordered_map<ZIntPoint, uint64_t> &labelMap) const
{
  std::string name = std::to_string(getBodyId());
  switch (getKind()) {
  case ZDvidSynapse::EKind::KIND_PRE_SYN:
    name += "->";
    break;
  case ZDvidSynapse::EKind::KIND_POST_SYN:
    name += "<-";
    break;
  default:
    name += "-";
    break;
  }

  for (const ZIntPoint &pt : m_partnerHint) {
    auto iter = labelMap.find(pt);
    if (iter != labelMap.end()) {
      name += "[" + std::to_string(iter->second) + "]";
    }
  }

  return name;
}

void ZDvidSynapse::updateProperty(const ZJsonObject &propJson)
{
  removeConfidenceProperty();

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

//ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidSynapse)


