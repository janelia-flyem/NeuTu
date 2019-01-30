#ifndef ZPOSITIONMAPPER_H
#define ZPOSITIONMAPPER_H

#include "common/neutube_def.h"

#include "geometry/zaffineplane.h"

class ZPoint;
class ZViewProj;
class QPointF;
class ZIntPoint;

class ZPositionMapper
{
public:
  ZPositionMapper();

public:
  ZPoint mapDataToStack(const ZPoint &pt);

public:
  static QPointF WidgetToRawStack(double x, double y, const ZViewProj &vp);
  static QPointF WidgetToRawStack(const QPointF &pt, const ZViewProj &vp);
  static ZPoint WidgetToRawStack(
      const ZIntPoint &pt, const ZViewProj &viewProj);
  static ZPoint WidgetToRawStack(
      const ZPoint &pt, const ZViewProj &viewProj);

  static QPointF WidgetToStack(double x, double y, const ZViewProj &vp);
  static QPointF WidgetToStack(const QPointF &pt, const ZViewProj &vp);
  static ZPoint WidgetToStack(double x, double y, double z,
                              const ZViewProj &vp, double z0);
  static ZPoint WidgetToStack(double x, double y,
                              const ZViewProj &vp, double z0);
  static ZPoint WidgetToStack(const ZPoint &pt,
                              const ZViewProj &vp, double z0);
  static ZPoint WidgetToStack(
      const ZIntPoint &pt, const ZViewProj &viewProj, double z0);

  static ZPoint StackToData(const ZPoint &pt, neutube::EAxis axis);
  static ZPoint StackToData(
      const ZPoint &pt, const ZPoint &stackAnchor, const ZAffinePlane &ap);
  //The anchor points have the same coordinates
  static ZPoint StackToData(const ZPoint &pt, const ZAffinePlane &ap);
  static ZPoint StackToData(double x, double y, const ZAffinePlane &ap);
  static ZPoint StackToData(const QPointF &pt, const ZAffinePlane &ap);

  static ZPoint DataToStack(const ZPoint &pt, neutube::EAxis axis);
  static ZPoint DataToStack(
      const ZPoint &pt, const ZPoint &stackAnchor, const ZAffinePlane &ap);
  //The anchor points have the same coordinates
  static ZPoint DataToStack(const ZPoint &pt, const ZAffinePlane &ap);

private:
  neutube::EAxis m_sliceAxis = neutube::EAxis::Z;
  ZAffinePlane m_ap;
};

#endif // ZPOSITIONMAPPER_H
