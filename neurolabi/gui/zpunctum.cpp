#include <QtGui>

#include <numeric>
#include <algorithm>
#include <sstream>

#include "zpunctum.h"
#include "zrandom.h"
#include "zstackball.h"

#define INIT_PUNCTUM m_score(1.0)

ZPunctum::ZPunctum()
{
  init(-1, -1, -1, 2.0);
}

ZPunctum::ZPunctum(double x, double y, double z, double r)
{
  init(x, y, z, r);
}

ZPunctum::ZPunctum(const ZPoint &center, double r)
{
  init(center.x(), center.y(), center.z(), r);
}

ZPunctum::ZPunctum(const ZIntPoint &center, double r)
{
  init(center.getX(), center.getY(), center.getZ(), r);
}


ZPunctum::~ZPunctum()
{
}

void ZPunctum::init(double x, double y, double z, double r)
{
  setColor(255, 255, 0, 255);
  setCenter(x, y, z);
  setRadius(r);
  setMaxIntensity(255);
  setMeanIntensity(255);
  setSDevOfIntensity(0);
  updateVolSize();
  updateMass();
  setVisualEffect(NeuTube::Display::Sphere::VE_OUT_FOCUS_DIM);
  m_type = ZStackObject::TYPE_PUNCTUM;
  m_score = 1.0;
}

#if 0
void ZPunctum::display(ZPainter &painter, int n, ZStackObject::Display_Style style) const
{
  if (!isVisible())
    return;

  bool isVisible = true;

  double dataFocus = n - painter.getOffset().z();

  if (!ZStackBall::isCuttingPlane(m_z, getRadius(), dataFocus, m_zScale)) {
    isVisible = false;
  }

  if (isVisible) {
    ZStackBall circle(m_x, m_y, m_z, m_radius);
    circle.setZScale(m_zScale);
    circle.setColor(m_color);
    circle.useCosmeticPen(m_usingCosmeticPen);
    circle.setVisualEffect(ZStackBall::VE_OUT_FOCUS_DIM);

    /*
    if (style == SOLID) {
      circle.setVisualEffect(ZCircle::VE_GRADIENT_FILL);
    }
    */
    circle.display(painter, n, style);
  }

#if 0
  if (n >= 0) {
    if (fabs(dataFocus - m_z) > m_radius) {
      isVisible = false;
    }
  }

  if (isVisible) {
    if (style == NORMAL) {
      style = SOLID;
    }

    if (m_selected == true) {
      painter.setPen(QPen(selectingColor(m_color), 1.5));
    } else {
      painter.setPen(QPen(m_color, .7));
    }

    switch (style) {
    case SOLID: {
      if ((iround(m_z) == dataFocus) || (n == -1)) {
        int half_size = iround(m_radius - 0.5);
        int cx = iround(m_x);
        int cy = iround(m_y);
        painter.drawRect(cx - half_size, cy - half_size,
                         half_size * 2 + 1, half_size * 2 + 1);
      }
      break;
    }
    case SKELETON:
      break;
    case BOUNDARY: {
      double r = m_radius;
      bool visible = false;

      if ((iround(m_z) == iround(dataFocus)) || (n == -1)) {
        visible = true;
      } else if (fabs(m_z - dataFocus) < r) {
        r = sqrt(r * r - (m_z - dataFocus) * (m_z - dataFocus));
        visible = true;
      }

      if (visible) {
        if (m_selected == true) {
          painter.setPen(QPen(selectingColor(m_color), 1.5));
        } else {
          painter.setPen(QPen(m_color, .7));
        }
        painter.drawEllipse(QPointF(m_x, m_y), r, r);
      }
      break;
    }
    default:
      break;
    }
  }
#endif
}
#endif


QList<ZPunctum *> ZPunctum::deepCopyPunctaList(const QList<ZPunctum *> &src)
{
  QList<ZPunctum*> des;
  for (int i=0; i<src.size(); ++i) {
    des.push_back(new ZPunctum(*(src[i])));
  }
  return des;
}

void ZPunctum::setSelected(bool selected)
{
  ZStackObject::setSelected(selected);
}

QColor ZPunctum::highlightingColor(const QColor &color) const
{
  QColor highlight;

  highlight.setRed(imin2(255, color.red() + 96));
  highlight.setGreen(imin2(255, color.green() + 96));
  highlight.setBlue(imin2(255, color.blue() + 96));

  return highlight;
}

QColor ZPunctum::selectingColor(const QColor &color) const
{
  QColor select;

  select.setHsv((color.hue() + 60) % 360,
                /*color.saturation()*/255, 255/*color.value()*/);

  return select;
}

std::string ZPunctum::toString()
{
  std::ostringstream stream;

  stream << "Puncta(" << m_name.toStdString() << "): "
         << "(" << getX() << ", " << getY() << ", " << getZ() << ")";

  return stream.str();
}

ZVaa3dMarker ZPunctum::toVaa3dMarker() const
{
  ZVaa3dMarker marker;
  marker.setCenter(getX(), getY(), getZ());
  marker.setRadius(getRadius());
#if defined(_QT_GUI_USED_)
  marker.setColor(m_color.red(), m_color.green(), m_color.blue());
#endif
  marker.setSource(getSource());
#ifdef _DEBUG_2
  std::cout << marker.source() << std::endl;
#endif
  marker.setName(m_name.toStdString());
  marker.setComment(m_comment.toStdString());

  return marker;
}

void ZPunctum::setFromMarker(const ZVaa3dMarker &marker)
{
  set(marker.x(), marker.y(), marker.z(), marker.radius());
  setColor(marker.colorR(), marker.colorG(), marker.colorB());
  setComment(marker.comment().c_str());
  setName(marker.name().c_str());
//  setSource(QString("%1").arg(marker.type()).toStdString());
  setSource(marker.source());
}

int ZPunctum::getTypeFromSource() const
{
  int type = -1;
  if (getSource() == "unknown") {
    type = 0;
  } else if (getSource() == "tbar") {
    type = 1;
  } else if (getSource() == "psd") {
    type = 2;
  } else if (getSource() == "tbar_multi") {
    type = 3;
  } else if (getSource() == "tbar_conv") {
    type = 4;
  } else if (getSource() == "tbar_multi_conv") {
    type = 5;
  } else {
    type = 6;
  }

  return type;
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZPunctum)
