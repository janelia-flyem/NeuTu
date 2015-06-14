#ifndef Z3DGRAPHFACTORY_H
#define Z3DGRAPHFACTORY_H

class Z3DGraph;
class ZCuboid;
class ZIntCuboid;
class ZRect2d;

class Z3DGraphFactory
{
public:
  Z3DGraphFactory();

  static Z3DGraph* MakeBox(const ZCuboid &box, double radius = 1.0);
  static Z3DGraph* MakeGrid(const ZRect2d &rect, int ntick, double lineWidth);

//  static Z3DGraph* MakeBox(const ZIntCuboid &box, double radius = 1.0);
};

#endif // Z3DGRAPHFACTORY_H
