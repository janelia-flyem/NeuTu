#ifndef ZINTCUBOIDCOMPOSITION_H
#define ZINTCUBOIDCOMPOSITION_H

#include "tz_cuboid_i.h"
#include <utility>
#ifdef __GLIBCXX__
#include <tr1/memory>
using namespace std::tr1;
#else
#include <memory>
using namespace std;
#endif

namespace FlyEm {
class ZIntCuboidComposition
{
public:
  ZIntCuboidComposition();

  enum EOperator {
    AND, OR, XOR, SINGULAR
  };

  bool hitTestF(double x, double y, double z);

  bool hitTest(int x, int y, int z);

  void setComposition(shared_ptr<ZIntCuboidComposition> firstComponent,
                      shared_ptr<ZIntCuboidComposition> secondComponent,
                      EOperator opr);
  void setSingular(int x, int y, int z, int width, int height, int depth);

private:
  shared_ptr<ZIntCuboidComposition> m_firstComponent;
  shared_ptr<ZIntCuboidComposition> m_secondComponent;
  Cuboid_I m_cuboid;
  EOperator m_operator;
};
}

#endif // ZINTCUBOIDCOMPOSITION_H
