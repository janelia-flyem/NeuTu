#ifndef Z3DGRAPHFACTORY_H
#define Z3DGRAPHFACTORY_H

#include <QColor>
#include <QRect>
#include "z3dgraph.h"

class ZCuboid;
class ZIntCuboid;
class ZRect2d;
class ZSwcTree;

class Z3DGraphFactory
{
public:
  Z3DGraphFactory();

  static Z3DGraph* MakeBox(
      const ZCuboid &box, double radius, Z3DGraph *graph = nullptr);
  static Z3DGraph* MakeGrid(const ZRect2d &rect, int ntick, double lineWidth);

  static Z3DGraph* MakeBox(const ZIntCuboid &box, double radius = 1.0);
  static Z3DGraph* MakeQuadDiag(const ZPoint &pt1, const ZPoint &pt2,
      const ZPoint &pt3, const ZPoint &pt4,
      Z3DGraph *graph = nullptr);
  static Z3DGraph* MakeQuadCross(const ZPoint &pt1, const ZPoint &pt2,
                                 const ZPoint &pt3, const ZPoint &pt4);

  Z3DGraph* makeBox(const ZIntCuboid &box, Z3DGraph *graph = nullptr);
  Z3DGraph* makeBox(const ZCuboid &box, Z3DGraph *graph = nullptr);
  Z3DGraph* makeBoundingBox(const ZIntCuboid &box,
                            const std::vector<int> &faceArray);

  Z3DGraph* makeFaceGraph(
      const ZCuboid &box, const std::vector<int> &faceArray);
  Z3DGraph* makeFaceGraph(
      const ZIntCuboid &box, const std::vector<int> &faceArray);

  Z3DGraph* makeQuadrilateral(const ZPoint &pt1, const ZPoint &pt2,
                              const ZPoint &pt3, const ZPoint &pt4,
                              Z3DGraph *graph = nullptr);

  void setShapeHint(EGraphShape shape) { m_shapeHint = shape; }
  void setNodeRadiusHint(double r) { m_nodeRadiusHint = r; }
  void setEdgeWidthHint(double w) { m_edgeWidthHint = w; }
  void setNodeColorHint(const QColor &color) { m_nodeColorHint = color; }
  void setEdgeColorHint(const QColor &color) { m_edgeColorHint = color; }

  static Z3DGraph MakeSwcGraph(const ZSwcTree &tree, double edgeWidth);
  static Z3DGraph MakeSwcFeatureGraph(const ZSwcTree &tree);


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
