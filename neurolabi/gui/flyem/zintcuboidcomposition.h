#ifndef ZINTCUBOIDCOMPOSITION_H
#define ZINTCUBOIDCOMPOSITION_H

#include "tz_cuboid_i.h"
#include <utility>
#include "common/zsharedpointer.h"

namespace flyem {
class ZIntCuboidComposition
{
public:
  ZIntCuboidComposition();

  enum EOperator {
    AND, OR, XOR, SINGULAR
  };

  bool hitTestF(double x, double y, double z);

  bool hitTest(int x, int y, int z);

  void setComposition(ZSharedPointer<ZIntCuboidComposition> firstComponent,
                      ZSharedPointer<ZIntCuboidComposition> secondComponent,
                      EOperator opr);
  void setSingular(int x, int y, int z, int width, int height, int depth);

private:
  ZSharedPointer<ZIntCuboidComposition> m_firstComponent;
  ZSharedPointer<ZIntCuboidComposition> m_secondComponent;
  Cuboid_I m_cuboid;
  EOperator m_operator;
};
}

#endif // ZINTCUBOIDCOMPOSITION_H
