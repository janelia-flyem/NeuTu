#include "zgeo3dscalarfield.h"

ZGeo3dScalarField::ZGeo3dScalarField()
{
}

double* ZGeo3dScalarField::getRawPointArray()
{
  if (m_pointArray.empty()) {
    return NULL;
  }

  return &(m_pointArray[0]);
}

double* ZGeo3dScalarField::getRawWeight()
{
  if (m_weightArray.empty()) {
    return NULL;
  }

  return &(m_weightArray[0]);
}

size_t ZGeo3dScalarField::getPointNumber() const
{
  return m_pointArray.size();
}
