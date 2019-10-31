#ifndef Z3DCAMERAUTILS_H
#define Z3DCAMERAUTILS_H

#include "geometry/zpoint.h"
#include "z3dcamera.h"

namespace Z3DCameraUtils
{
  bool eyeNormalToPoints(glm::vec3 point1, glm::vec3 point2, glm::vec3 up, const Z3DCamera &camera,
                         glm::vec3 &eye);

  // Set the frustum to just enclose the bounding sphere.  The result is a tighter fit than
  // available with Z3DCamera::resetCamera(const ZBBox<glm::dvec3>& bound, ResetOption options),
  // whose input would be a box around the sphere, and which then would put asphere around the box.

  void resetCamera(ZPoint pos, double radius, Z3DCamera &camera);

  // Zoom the camera so that the view just shows the specified vertices.  Assumes that the camera
  // starts zoomed out so the all the vertices are visible.

  void tightenZoom(Z3DCamera &camera,
                   const std::vector<std::vector<glm::vec3>> &vertices,
                   size_t only = SIZE_MAX);

};

#endif // Z3DCAMERAUTILS_H
