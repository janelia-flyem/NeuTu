#ifndef Z3DVOXELFILTER_H
#define Z3DVOXELFILTER_H

#include "z3dgeometryfilter.h"

class ZObject3d;
class ZObject3dScan;

class Z3DVoxelFilter : public Z3DGeometryFilter
{
  Q_OBJECT

public:
  Z3DVoxelFilter();

private:
  QList<ZObject3d*> m_objList;
};

#endif // Z3DVOXELFILTER_H
