#ifndef ZLINESEGMENTOBJECT_H
#define ZLINESEGMENTOBJECT_H

#include "geometry/zlinesegment.h"

#include <QPointF>
#include <QColor>

#include "zstackobject.h"

class ZLineSegmentObject : public ZStackObject
{
public:
  ZLineSegmentObject();
  virtual bool display(
      QPainter *painter, const DisplayConfig &config) const {}


  bool isSliceVisible(int z, neutu::EAxis sliceAxis) const;


  double getLowerX() const;
  double getUpperX() const;
  double getLowerY() const;
  double getUpperY() const;
  double getLowerZ() const;
  double getUpperZ() const;

  QPointF getStartXY() const;
  QPointF getEndXY() const;

  void setStartPoint(double x, double y, double z);
  void setEndPoint(double x, double y, double z);

  void setStartPoint(const ZIntPoint &pt);
  void setEndPoint(const ZIntPoint &pt);

  void setFocusColor(const QColor &color);

//  virtual const std::string& className() const;

private:
  void computePlaneInersection(QPointF &lineStart, QPointF &lineEnd,
                               bool &visible, int ataFocus,
                               neutu::EAxis sliceAxis) const;

private:
  ZLineSegment m_segment;
  double m_width;
//  int m_label;
  QColor m_focusColor;
};

#endif // ZLINESEGMENTOBJECT_H
