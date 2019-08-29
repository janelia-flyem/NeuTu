#include "z3dcamerautils.h"
#include "logging/zqslog.h"

namespace Z3DCameraUtils
{

  bool eyeNormalToPoints(glm::vec3 point1, glm::vec3 point2, glm::vec3 up, const Z3DCamera &camera,
                         glm::vec3 &eye)
  {
    glm::vec3 p1ToP2 = glm::normalize(point2 - point1);
    glm::vec3 toEye = glm::cross(p1ToP2, up);
    float toEyeLength = glm::length(toEye);

    bool changed = true;
    if (toEyeLength > 1e-5f) {
      toEye /= toEyeLength;
    } else {

      // The vector between the two supervoxel points is parallel to the up vector.
      // So the camera is already giving a good view of the supervoxel points.

      toEye = glm::normalize(camera.eye() - camera.center());
      changed = false;
    }
    eye = camera.center() + toEye;

    return changed;
  }

  void resetCamera(ZPoint pos, double radius, Z3DCamera &camera)
  {
    glm::vec3 center = glm::vec3(pos.x(), pos.y(), pos.z());

    double angle = camera.fieldOfView();
    double aspectRatio = camera.windowAspectRatio() * camera.aspectRatio();
    if (aspectRatio < 1.0) {

      // If the frustum window is taller than it is wide, use the (smaller) horizontal angle.

      angle = 2.0 * std::atan(std::tan(angle * 0.5) * aspectRatio);
    }

    float centerDist = radius / std::sin(angle * 0.5);
    glm::vec3 eye = center - centerDist * glm::normalize(camera.center() - camera.eye());
    camera.setCamera(eye, center, camera.upVector());

    double xMin = pos.x() - radius, xMax = pos.x() + radius;
    double yMin = pos.y() - radius, yMax = pos.y() + radius;
    double zMin = pos.z() - radius, zMax = pos.z() + radius;
    camera.resetCameraNearFarPlane(xMin, xMax, yMin, yMax, zMin, zMax);
  }

  void projectToViewPlane3D(const std::vector<glm::vec3> &vertices,
                            const Z3DCamera &camera,
                            std::vector<glm::vec3> &result)
  {
    glm::vec3 view = camera.center() - camera.eye();
    float viewDist = glm::length(view);
    view /= viewDist;

    // Using a perspective projection for the current camera parameters, project each
    // vertex onto the plane passing through camera.center() that is normal to the
    // view vector (from camera.eye() to camera.center()).

    for (const glm::vec3 &vert : vertices) {
      glm::vec3 eyeToVert = vert - camera.eye();
      glm::vec3 eyeToVertOnView = glm::dot(eyeToVert, view) * view;
      float eyeToVertDist = glm::length(eyeToVert);
      eyeToVert /= eyeToVertDist;

      // By similar triangles, viewDist / glm::length(eyeToVertOnView) = eyeToVertOnPlaneDist / eyeToVertDist

      float eyeToVertOnPlaneDist = viewDist / glm::length(eyeToVertOnView) * eyeToVertDist;
      glm::vec3 eyeToVertOnPlane = camera.eye() + eyeToVert * eyeToVertOnPlaneDist;
      result.push_back(eyeToVertOnPlane);
    }
  }

  void viewPlane3Dto2D(const std::vector<glm::vec3> &vertices,
                       const Z3DCamera &camera,
                       std::vector<glm::vec2> &result)
  {
    // Extend w, the normal to the plane containing the vertices, to a
    // 3D coordinate frame, with u and v lying on the plane.

    glm::vec3 w = glm::normalize(camera.eye() - camera.center());
    glm::vec3 u = glm::normalize(glm::cross(camera.upVector(), w));
    glm::vec3 v = glm::normalize(glm::cross(w, u));

    // Use that frame to generate 2D points on the plane, with camera.center()
    // as the origin.

    for (const glm::vec3 &vert : vertices) {
      glm::vec3 centerToVert = vert - camera.center();
      glm::vec2 p = glm::vec2(glm::dot(centerToVert, u), glm::dot(centerToVert, v));
      result.push_back(p);
    }
  }

  bool resetCameraForViewPlane2D(const ZBBox<glm::vec2> &bbox,
                                 glm::vec3 &eyeValid,
                                 Z3DCamera& camera)
  {
    if ((camera.right() - std::abs(camera.left()) > 1e-6) ||
        (camera.top() - std::abs(camera.bottom()) > 1e-6)) {

      // The code is not designed to handle a skewed view frustum (but it is unlikely anyway).

      return false;
    }

    // Move eye, the tip of the frustum, so the frustum most tightly fits bbox, the 2D bounding box
    // of the vertices projected onto the plane through camera.center().  First consider the vertical
    // dimension of bbox.  Use similar triangles to determine the new distance from camera.center()
    // to eye.

    float height = std::max(bbox.maxCorner().y, std::abs(bbox.minCorner().y));
    float viewDistY = camera.nearDist() * height / camera.top();

    // Repeat for the horizontal dimension of bbox.

    float width = std::max(bbox.maxCorner().x, std::abs(bbox.minCorner().x));
    float viewDistX = camera.nearDist() * width / camera.right();

    // Use the larger of the two distances.

    float viewDist = std::max(viewDistX, viewDistY);

    glm::vec3 viewOld = camera.center() - camera.eye();
    float viewDistOld = glm::length(viewOld);
    viewOld /= viewDistOld;
    glm::vec3 eye;

    if (std::abs(viewDist - viewDistOld) < 1) {

      // Stop the iteration (repeatedly calling this routine) if the new eye has barely moved.

      return false;
    } else if (viewDist < viewDistOld) {

      // If eye has moved closer to camera.center() then we want to use it as camera.eye() for
      // the next iteration.  And the current camera.eye(), which was used for the projection and
      // computation of the new eye also is valid, and should be saved in case the next iteration goes
      // too far and has to be backed out.

      eye = camera.center() - viewDist * viewOld;
      eyeValid = camera.eye();
    } else {

      // If eye has moved further from camera.center() then camera.eye() was too close and is not valid.
      // Back out halfway to the previous valid eye for the next iteration.

      glm::vec3 eyeToEyeValid = eyeValid - camera.eye();
      float eyeToEyeValidDist = glm::length(eyeToEyeValid);
      eyeToEyeValid /= eyeToEyeValidDist;
      eye = camera.eye() + eyeToEyeValidDist / 2 * eyeToEyeValid;
    }

    camera.setCamera(eye, camera.center(), camera.upVector());

    return true;
  }

  void tightenZoom(Z3DCamera &camera,
                   const std::vector<std::vector<glm::vec3>> &vertices,
                   size_t only)
  {
    float closestDist = std::numeric_limits<float>::max();
    size_t iClosestMesh = 0;
    size_t iClosestVertex = 0;
    bool found = false;

    // The algorithm iterqtively moves the eye point, and at each iteration it must
    // make sure that no vertex is in front of the near clipping plane.  Precompute
    // which vertex is closest to the eye, as it won't change during the iteration.

    glm::vec3 view = glm::normalize(camera.center() - camera.eye());
    for (size_t i = 0; i < vertices.size(); i++) {
      if ((only != SIZE_T_MAX) && (i != only)) {
          continue;
      }

      for (size_t j = 0; j < vertices[i].size(); j++) {
        glm::vec3 eyeToVert = vertices[i][j] - camera.eye();
        float dist = glm::dot(eyeToVert, view);
        if (dist < closestDist) {
          closestDist = dist;
          iClosestMesh = i;
          iClosestVertex = j;
          found = true;
        }
      }
    }

    if (!found) {
      LWARN() << "closest not found, tightening aborted";
      return;
    }

    // Iteratively adjust the eye point.

    glm::vec3 eyeValid = camera.eye();
    for (size_t iter = 0; iter < 10; iter++) {
      ZBBox<glm::vec2> bbox;

      for (size_t i = 0; i < vertices.size(); i++) {
        if ((only != SIZE_T_MAX) && (i != only)) {
            continue;
        }

        std::vector<glm::vec3> onViewPlane3D;
        projectToViewPlane3D(vertices[i], camera, onViewPlane3D);

        std::vector<glm::vec2> onViewPlane2D;
        viewPlane3Dto2D(onViewPlane3D, camera, onViewPlane2D);

        for (glm::vec2 point : onViewPlane2D) {
          bbox.expand(point);
        }
      }

      if (!resetCameraForViewPlane2D(bbox, eyeValid, camera)) {
        break;
      }

      // Check the closest vertex against the near clipping plane for the moved eye point.

      glm::vec3 view = glm::normalize(camera.center() - camera.eye());
      glm::vec3 eyeToVert = vertices[iClosestMesh][iClosestVertex] - camera.eye();
      float near = glm::dot(eyeToVert, view);

      if (near < camera.nearDist()) {

        // If adjusting the near clipping plane would make it too small relative to the
        // far clipping plane, then stop the iteration.  The definition of "too small"
        // matches that in Z3DCamera.

        float nearClippingPlaneTolerance = 0.001f;
        float minNear = nearClippingPlaneTolerance * camera.farDist();
        if (near < minNear) {
          glm::vec3 eye = camera.eye() + near * view;
          eye -= minNear * view;
          camera.setEye(eye);
          camera.setNearDist(minNear);
          break;
        }
        camera.setNearDist(near);
      }
    }
  }

}
