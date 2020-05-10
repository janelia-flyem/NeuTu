#include "utilities.h"

#include <QtGui>

#include "data3d/zviewplanetransform.h"

QTransform neutu::vis2d::GetPainterTransform(const ZViewPlaneTransform &t)
{
  QTransform transform;
  transform.setMatrix(
        t.getScale(), 0, 0, 0, t.getScale(), 0, t.getTx(), t.getTy(), 1);

  return  transform;
}
