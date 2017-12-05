#include "z3dobjectfilter.h"

Z3DObjectFilter::Z3DObjectFilter(Z3DGlobalParameters &globalParas, QObject *parent)
  : Z3DGeometryFilter(globalParas, parent)
  , m_lineRenderer(m_rendererBase)
{
  setName(QString("objectfilter"));
}

Z3DObjectFilter::~Z3DObjectFilter()
{
  clear();
}

void Z3DObjectFilter::clear()
{
  m_objList.clear();
}

