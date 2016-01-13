#ifndef Z3DGRAPHFACTORY_H
#define Z3DGRAPHFACTORY_H

#include <QColor>
#include "z3dgraph.h"

class ZCuboid;
class ZIntCuboid;
class ZRect2d;

class Z3DGraphFactory
{
public:
  Z3DGraphFactory();

  static Z3DGraph* MakeBox(const ZCuboid &box, double radius = 1.0);
  static Z3DGraph* MakeGrid(const ZRect2d &rect, int ntick, double lineWidth);

  static Z3DGraph* MakeBox(const ZIntCuboid &box, double radius = 1.0);

  Z3DGraph* makeBox(const ZIntCuboid &box);
  Z3DGraph* makeBox(const ZCuboid &box);

  Z3DGraph* makeFaceGraph(
      const ZCuboid &box, const std::vector<int> &faceArray);
  Z3DGraph* makeFaceGraph(
      const ZIntCuboid &box, const std::vector<int> &faceArray);

  void setShapeHint(EGraphShape shape) { m_shapeHint = shape; }
  void setNodeRadiusHint(double r) { m_nodeRadiusHint = r; }
  void setEdgeWidthHint(double w) { m_edgeWidthHint = w; }
  void setNodeColorHint(const QColor &color) { m_nodeColorHint = color; }
  void setEdgeColorHint(const QColor &color) { m_edgeColorHint = color; }


private:
  void init();

private:
  EGraphShape m_shapeHint;
  double m_nodeRadiusHint;
  double m_edgeWidthHint;
  QColor m_nodeColorHint;
  QColor m_edgeColorHint;
};

#endif // Z3DGRAPHFACTORY_H
