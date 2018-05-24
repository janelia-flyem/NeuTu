#include "zintcuboidcomposition.h"

#include <math.h>

flyem::ZIntCuboidComposition::ZIntCuboidComposition() : m_operator(SINGULAR)
{
  Cuboid_I_Set_S(&m_cuboid, 0, 0, 0, 0, 0, 0);
}

bool flyem::ZIntCuboidComposition::hitTestF(double x, double y, double z)
{
  int ix = floor(x);
  int iy = floor(y);
  int iz = floor(z);

  return hitTest(ix, iy, iz);
}

void flyem::ZIntCuboidComposition::setComposition(
    ZSharedPointer<ZIntCuboidComposition> firstComponent,
    ZSharedPointer<ZIntCuboidComposition> secondComponent,
    EOperator opr)
{
  m_firstComponent = firstComponent;
  m_secondComponent = secondComponent;
  m_operator = opr;
}

bool flyem::ZIntCuboidComposition::hitTest(int x, int y, int z)
{
  if (m_operator == SINGULAR) {
    return Cuboid_I_Hit(&m_cuboid, x, y, z);
  } else {
    if (!m_firstComponent || !m_secondComponent) {
      if (m_firstComponent) {
        return m_firstComponent->hitTest(x, y, z);
      } else if (m_secondComponent) {
        return m_secondComponent->hitTest(x, y, z);
      } else {
        return false;
      }
    }

    switch (m_operator) {
    case OR:
      return m_firstComponent->hitTest(x, y, z) ||
          m_secondComponent->hitTest(x, y, z);
    case AND:
      return m_firstComponent->hitTest(x, y, z) &&
          m_secondComponent->hitTest(x, y, z);
    case XOR:
      return (m_firstComponent->hitTest(x, y, z) ||
              m_secondComponent->hitTest(x, y, z)) &&
          !(m_firstComponent->hitTest(x, y, z) &&
            m_secondComponent->hitTest(x, y, z));
    default:
      break;
    }
  }

  return false;
}

void flyem::ZIntCuboidComposition::setSingular(
    int x, int y, int z, int width, int height, int depth)
{
  m_operator = SINGULAR;
  Cuboid_I_Set_S(&m_cuboid, x, y, z, width, height, depth);
}
