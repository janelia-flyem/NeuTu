#ifndef ZGEOMETRY_H
#define ZGEOMETRY_H

#include "zgeo3dtransform.h"
#include "zgeo3dscalarfield.h"

namespace ZGeometry
{
/*!
 * \brief Transform a 3D field
 */
void transform(ZGeo3dScalarField *field, const ZGeo3dTransform &transform);

}

#endif // ZGEOMETRY_H
