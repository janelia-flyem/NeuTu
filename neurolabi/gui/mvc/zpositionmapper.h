#ifndef ZPOSITIONMAPPER_H
#define ZPOSITIONMAPPER_H

class ZPoint;
class ZViewProj;
class QPointF;
class ZAffinePlane;

class ZPositionMapper
{
public:
  ZPositionMapper();

  static QPointF WidgetToStack(double x, double y, const ZViewProj &vp);
  static QPointF WidgetToStack(const QPointF &pt, const ZViewProj &vp);

  static QPointF WidgetToRawStack(double x, double y, const ZViewProj &vp);
  static QPointF WidgetToRawStack(const QPointF &pt, const ZViewProj &vp);

  static ZPoint WidgetToStack(double x, double y, double z,
                              const ZViewProj &vp, double z0);
  static ZPoint WidgetToStack(double x, double y,
                              const ZViewProj &vp, double z0);

  static ZPoint StackToData(
      const ZPoint &pt, int xvc, int yvc, const ZAffinePlane &ap);
};

#endif // ZPOSITIONMAPPER_H
