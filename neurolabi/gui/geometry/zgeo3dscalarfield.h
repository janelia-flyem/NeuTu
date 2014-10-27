#ifndef ZGEO3DSCALARFIELD_H
#define ZGEO3DSCALARFIELD_H

#include "zdoublevector.h"

class ZGeo3dScalarField
{
public:
  ZGeo3dScalarField();

  double* getRawPointArray();
  double* getRawWeight();
  size_t getPointNumber() const;

private:
  ZDoubleVector m_pointArray;
  ZDoubleVector m_weightArray;

};

#endif // ZGEO3DSCALARFIELD_H
