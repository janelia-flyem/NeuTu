#include "zlinesegmentobject.h"

#include <QPen>
#include <cmath>

#include "tz_utilities.h"
#include "zpainter.h"

ZLineSegmentObject::ZLineSegmentObject() : m_width(1.0), m_label(0)
{
  m_type = TYPE_LINE_SEGMENT;
  m_focusColor.setAlpha(0);
}

double ZLineSegmentObject::getLowerX() const
{
  return m_segment.getLowerX();
}

double ZLineSegmentObject::getUpperX() const
{
  return m_segment.getUpperX();
}

double ZLineSegmentObject::getLowerY() const
{
  return m_segment.getLowerY();
}

double ZLineSegmentObject::getUpperY() const
{
  return m_segment.getUpperY();
}

double ZLineSegmentObject::getLowerZ() const
{
  return m_segment.getLowerZ();
}

double ZLineSegmentObject::getUpperZ() const
{
  return m_segment.getUpperZ();
}

QPointF ZLineSegmentObject::getStartXY() const
{
  return QPointF(m_segment.getStartPoint().x(), m_segment.getStartPoint().y());
}

QPointF ZLineSegmentObject::getEndXY() const
{
  return QPointF(m_segment.getEndPoint().x(), m_segment.getEndPoint().y());
}

void ZLineSegmentObject::display(
    ZPainter &painter, int slice, EDisplayStyle /*option*/,
    NeuTube::EAxis sliceAxis) const
{
  /*
  if (sliceAxis != NeuTube::Z_AXIS) {
    return;
  }
  */

  int z = painter.getZ(slice);

  bool isProj = false;
  if (slice < 0) {
    isProj = true;
  }

  QColor focusColor = m_focusColor;
  if (focusColor.alpha() == 0) {
    focusColor = getColor();
  }

  if (isSliceVisible(z, sliceAxis)) {
    QPen pen = painter.getPen();
    pen.setWidthF(m_width);

    ZLineSegment segment = m_segment;
    segment.shiftSliceAxis(sliceAxis);

    double dz1 = segment.getLowerZ() - z;
    double dz2 = segment.getUpperZ() - z;

    bool fullLinePainting =
        (hasVisualEffect(NeuTube::Display::Line::VE_LINE_PROJ) ||
         hasVisualEffect(NeuTube::Display::Line::VE_LINE_FADING_PROJ) ||
         isProj || (dz1 * dz2 <= 0));
    bool intersectPainting = !isProj && (dz1 * dz2 <= 0);
    if (intersectPainting) {
      pen.setStyle(Qt::SolidLine);
    }

    //Draw full line
    if (fullLinePainting) {
      if (isProj) {
        pen.setColor(focusColor);
      } else {
        QColor lineColor = getColor();
        lineColor.setHsvF(lineColor.hueF(), lineColor.saturationF() * 0.5,
                          lineColor.valueF(), lineColor.alphaF() * 0.5);
        //Whole line is out of focus
        if (dz1 * dz2 > 0) {
          if (hasVisualEffect(NeuTube::Display::Line::VE_LINE_FADING_PROJ)) {
            double deltaZ = std::min(fabs(dz1), fabs(dz2));
            double alphaRatio = 1.0 / deltaZ;
            if (alphaRatio >= 0.1) {
              double alpha = lineColor.alphaF() * alphaRatio;
              if (alpha > 1.0) {
                alpha = 1.0;
              }
              lineColor.setAlphaF(alpha);
            } else {
              lineColor.setAlpha(0);
            }
          }
        }

        pen.setColor(lineColor);
        if (lineColor.alpha() > 0) {
          if (!intersectPainting &&
              hasVisualEffect(NeuTube::Display::Line::VE_LINE_PROJ)) {
            pen.setStyle(Qt::DotLine);
          }
        }
      }

      if (pen.color().alpha() > 0) {
        painter.setPen(pen);
        painter.drawLine(
              QPointF(segment.getStartPoint().x(), segment.getStartPoint().y()),
              QPointF(segment.getEndPoint().x(), segment.getEndPoint().y()));
      }
    }

    if (intersectPainting) {
      pen.setStyle(Qt::SolidLine);
      pen.setColor(focusColor);
      painter.setPen(pen);

      bool visible = false;

      QPointF lineStart, lineEnd;
      computePlaneInersection(lineStart, lineEnd, visible, z, sliceAxis);

      if (visible) {
        painter.drawLine(lineStart, lineEnd);
      }
    }
  }
}

bool ZLineSegmentObject::isSliceVisible(int z, NeuTube::EAxis sliceAxis) const
{
  if (isVisible()) {
    if (hasVisualEffect(NeuTube::Display::Line::VE_LINE_PROJ) ||
        hasVisualEffect(NeuTube::Display::Line::VE_LINE_FADING_PROJ)) {
      return true;
    }

    switch (sliceAxis) {
    case NeuTube::X_AXIS:
      return getLowerX() <= z && getUpperX() >= z;
    case NeuTube::Y_AXIS:
      return getLowerY() <= z && getUpperY() >= z;
    case NeuTube::Z_AXIS:
      return getLowerZ() <= z && getUpperZ() >= z;
    }
  }

  return false;
}

void ZLineSegmentObject::computePlaneInersection(
    QPointF &lineStart, QPointF &lineEnd, bool &visible, int dataFocus,
    NeuTube::EAxis sliceAxis) const
{
  double upperZ = dataFocus + 0.5;
  double lowerZ = dataFocus - 0.5;

  ZLineSegment segment = m_segment;
  segment.shiftSliceAxis(sliceAxis);

  ZPoint lowerEnd = segment.getStartPoint();
  ZPoint upperEnd = segment.getEndPoint();

  if (lowerEnd.z() > upperEnd.z()) {
    ZPoint tmp = lowerEnd;
    lowerEnd = upperEnd;
    upperEnd = tmp;
  }

  double lowerLineZ = lowerEnd.z();
  double upperLineZ = upperEnd.z();

  if ((IS_IN_OPEN_RANGE(lowerLineZ, lowerZ, upperZ) &&
       IS_IN_OPEN_RANGE(upperLineZ, lowerZ, upperZ))) {
    visible = true;
    lineStart.setX(segment.getStartPoint().x());
    lineStart.setY(segment.getStartPoint().y());
    lineEnd.setX(segment.getEndPoint().x());
    lineEnd.setY(segment.getEndPoint().y());
  } else {
    if (lowerLineZ < upperZ && upperLineZ > lowerZ) {
      visible = true;
      double dz = upperLineZ - lowerLineZ;
      double lambda1 = (upperLineZ - dataFocus - 0.5) / dz;
      double lambda2 = lambda1 + 1.0 / dz;
      if (lambda1 < 0.0) {
        lambda1 = 0.0;
      }
      if (lambda2 > 1.0) {
        lambda2 = 1.0;
      }

      lineStart.setX(lowerEnd.x() * lambda1 + upperEnd.x() * (1.0 - lambda1));
      lineStart.setY(lowerEnd.y() * lambda1 + upperEnd.y() * (1.0 - lambda1));

      lineEnd.setX(lowerEnd.x() * lambda2 + upperEnd.x() * (1.0 - lambda2));
      lineEnd.setY(lowerEnd.y() * lambda2 + upperEnd.y() * (1.0 - lambda2));
    }
  }
}

void ZLineSegmentObject::setStartPoint(double x, double y, double z)
{
  m_segment.setStartPoint(x, y, z);
}

void ZLineSegmentObject::setEndPoint(double x, double y, double z)
{
  m_segment.setEndPoint(x, y, z);
}

void ZLineSegmentObject::setStartPoint(const ZIntPoint &pt)
{
  setStartPoint(pt.getX(), pt.getY(), pt.getZ());
}

void ZLineSegmentObject::setEndPoint(const ZIntPoint &pt)
{
  setEndPoint(pt.getX(), pt.getY(), pt.getZ());
}

void ZLineSegmentObject::setFocusColor(const QColor &color)
{
  m_focusColor = color;
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZLineSegmentObject)
