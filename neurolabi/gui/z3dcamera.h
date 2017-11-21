#ifndef Z3DCAMERA_H
#define Z3DCAMERA_H

#include "z3dgl.h"
#include "zflags.h"
#include "zglmutils.h"
#include "zbbox.h"
#include <map>
#include <vector>

class ZJsonObject;

//Z3DCamera is used for view transformation and projection transformation.

//During the view transformation, the eye, center, and upVector are used to
//generate the view matrix.
//During the projection transformation, the projectionType, nearDist, windowAspectRatio
//farDist, fieldOfView, and aspectRatio are used to generate projection matrix.
//The aspect ratio is the ratio of x (width) to y (height).

//The view matrix is returned by viewMatrix().
//The projection matrix is returned by projectionMatrix().

class Z3DCamera
{
public:
  enum class ProjectionType
  {
    Perspective, Orthographic
  };

  enum class ResetOption
  {
    ResetAll = 0,
    PreserveCenterDistance = 1,
    PreserveViewVector = 1 << 1
  };

  Z3DCamera();

  const glm::vec3& eye() const
  { return m_eye; }

  void setEye(const glm::vec3& eye)
  {
    m_eye = eye;
    updateCamera();
  }

  const glm::vec3& center() const
  { return m_center; }

  void setCenter(const glm::vec3& center)
  {
    m_center = center;
    updateCamera();
  }

  // always return normalized vector
  const glm::vec3& upVector() const
  { return m_upVector; }

  void setUpVector(const glm::vec3& upVector)
  {
    m_upVector = glm::normalize(upVector);
    updateCamera();
  }

  ProjectionType projectionType() const
  { return m_projectionType; }

  void setProjectionType(ProjectionType pt)
  {
    m_projectionType = pt;
    updateFrustum();
  }

  bool isPerspectiveProjection() const
  { return m_projectionType == ProjectionType::Perspective; }

  bool isOrthographicProjection() const
  { return m_projectionType == ProjectionType::Orthographic; }

  float fieldOfView() const
  { return m_fieldOfView; }

  void setFieldOfView(float fov)
  {
    m_fieldOfView = glm::clamp(fov, glm::radians(10.f), glm::radians(170.f));
    updateFrustum();
  }

  float aspectRatio() const
  { return m_aspectRatio; }

  void setAspectRatio(float ar)
  {
    m_aspectRatio = ar;
    updateFrustum();
  }

  float nearDist() const
  { return m_nearDist; }

  void setNearDist(float nd)
  {
    m_nearDist = nd;
    updateFrustum();
  }

  float farDist() const
  { return m_farDist; }

  void setFarDist(float fd)
  {
    m_farDist = fd;
    updateFrustum();
  }

  float windowAspectRatio() const
  { return m_windowAspectRatio; }

  void setWindowAspectRatio(float war)
  {
    m_windowAspectRatio = war;
    updateFrustum();
  }

  glm::vec2 frustumNearPlaneSize() const
  { return glm::vec2(m_right - m_left, m_top - m_bottom); }

  float eyeSeparationAngle() const
  { return m_eyeSeparationAngle; }

  void setEyeSeparationAngle(float angle)
  {
    m_eyeSeparationAngle = glm::clamp(angle, glm::radians(1.f), glm::radians(80.f));
    updateCamera();
    updateFrustum();
  }

  // convinient functions to set many variables at once:

  // setCamera function will set eye, center, upVector and other derived values of camera based on input
  void setCamera(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& upVector);

  // setFrustum function will set fov, ratio, nearDist, farDist and other derived values of frustum based on input
  void setFrustum(float fov, float ratio, float nearDist, float farDist);

  //
  void setTileFrustum(double left = 0.0, double right = 1.0, double bottom = 0.0, double top = 1.0);

  // Automatically set up the camera based on a specified bounding box. Camera will reposition itself so
  // that its focal point is the center of the bounding box, and adjust its
  // position (if PreserveCenterDistance is not set) and frustum to make sure everything inside bounding
  // box is visible. Initial view vector (vector defined from eye to center)
  // will be preserved based on the PreserveViewVector flag. By default it is not preserved and will be
  // reset to (0,0,1) (upVector will then be set to (0,-1,0)).
  // Result depends on current field of view and aspect ratio.
  void resetCamera(const ZBBox<glm::dvec3>& bound, ResetOption options = ResetOption::ResetAll);

  void resetCamera(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax,
                   ResetOption options = ResetOption::ResetAll);

  // Reset the camera near far plane based on the bounding box (see resetCamera).
  // This ensures that nothing is clipped by the near far planes
  void resetCameraNearFarPlane(const ZBBox<glm::dvec3>& bound);

  void resetCameraNearFarPlane(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax);

  const glm::mat4& viewMatrix(Z3DEye eye) const
  { return m_viewMatrices[enumToUnderlyingType(eye)]; }

  const glm::mat4& projectionMatrix(Z3DEye eye) const
  { return m_projectionMatrices[enumToUnderlyingType(eye)]; }

  const glm::mat4& inverseViewMatrix(Z3DEye eye) const
  { return m_inverseViewMatrices[enumToUnderlyingType(eye)]; }

  const glm::mat4& inverseProjectionMatrix(Z3DEye eye) const
  { return m_inverseProjectionMatrices[enumToUnderlyingType(eye)]; }

  const glm::mat3& normalMatrix(Z3DEye eye) const
  { return m_normalMatrices[enumToUnderlyingType(eye)]; }

  const glm::mat4& projectionViewMatrix(Z3DEye eye) const
  { return m_projectionViewMatrices[enumToUnderlyingType(eye)]; }

  // dist from eye to center
  float centerDist() const
  { return m_centerDist; }

  // normalized vector from eye to center
  const glm::vec3& viewVector() const
  { return m_viewVector; }

  // normalized vector represents right direction relative to the viewer's current orientation
  const glm::vec3& strafeVector() const
  { return m_strafeVector; }

  // frustum
  float left() const
  { return m_left; }

  float right() const
  { return m_right; }

  float bottom() const
  { return m_bottom; }

  float top() const
  { return m_top; }

  // other matrix
  inline glm::mat3 rotateMatrix(Z3DEye eye = Z3DEye::Mono)
  { return glm::mat3(viewMatrix(eye)); }

  bool operator==(const Z3DCamera& rhs) const;

  bool operator!=(const Z3DCamera& rhs) const;

  // Divide the camera's distance from the focal point by the given
  // dolly value.  Use a value greater than one to dolly-in toward
  // the focal point, and use a value less than one to dolly-out away
  // from the focal point.
  void dolly(float value);

  void dollyToCenterDistance(float centerDist);

  // Rotate the camera about the view vector.  This will
  // spin the camera about its axis.
  void roll(float angle);

  // Rotate the camera about the view up vector centered at the focal point.
  // Note that the view up vector is whatever was set via setUpVector, and is
  // not necessarily perpendicular to the direction of projection.  The
  // result is a horizontal rotation of the camera.
  void azimuth(float angle);

  // Description:
  // Rotate the focal point about the view up vector, using the camera's
  // position as the center of rotation. Note that the view up vector is
  // whatever was set via setUpVector, and is not necessarily perpendicular
  // to the view vector.  The result is a horizontal rotation
  // of the scene.
  void yaw(float angle);

  inline void pan(float angle)
  { yaw(angle); }

  // Description:
  // Rotate the camera about the cross product of the view up vector and
  // the view vector (point at left in screen), using the focal point as
  // the center of rotation. The result is a vertical rotation of the scene.
  void elevation(float angle);

  // Description:
  // Rotate the focal point about the cross product of the view vector
  // and the view up vector (point right in screen), using the camera's position
  // as the center of rotation.  The result is a vertical rotation of the camera.
  void pitch(float angle);

  inline void tilt(float angle)
  { pitch(angle); }

  // In perspective mode, decrease the view angle by the specified factor.
  // A value greater than 1 is a zoom-in, a value less than 1 is a zoom-out.
  void zoom(float factor);

  // rotate around a point
  // axis and point in worldspace, axis should be normalized
  void rotate(float angle, const glm::vec3& axis, const glm::vec3& point);

  void rotate(const glm::quat& quat, const glm::vec3& point);

  // rotate around center (focus point)
  void rotate(float angle, const glm::vec3& axis);

  void rotate(const glm::quat& quat);

  // convert between eye space and world space
  glm::vec3 vectorEyeToWorld(const glm::vec3& vec, Z3DEye eye = Z3DEye::Mono);

  glm::vec3 vectorWorldToEye(const glm::vec3& vec, Z3DEye eye = Z3DEye::Mono);

  glm::vec3 pointEyeToWorld(const glm::vec3& pt, Z3DEye eye = Z3DEye::Mono);

  glm::vec3 pointWorldToEye(const glm::vec3& pt, Z3DEye eye = Z3DEye::Mono);

  // world to screen, if point is clipped, its screen coord will be (-1,-1,-1)
  glm::vec3 worldToScreen(const glm::vec3& wpt, const glm::ivec4& viewport, Z3DEye eye = Z3DEye::Mono);

  glm::vec3 screenToWorld(const glm::vec3& spt, const glm::ivec4& viewport, Z3DEye eye = Z3DEye::Mono);

  ZJsonObject toJsonObject() const;
  void set(const ZJsonObject &cameraJson);

protected:
  void updateCamera();

  void updateFrustum();

  void makeViewMatrices();

  void makeProjectionMatrices();

private:
  glm::vec3 m_eye{0.f, 0.f, 0.f};
  glm::vec3 m_center{0.f, 0.f, -1.f};
  glm::vec3 m_upVector{0.f, 1.f, 0.f};  // normalized
  ProjectionType m_projectionType = ProjectionType::Perspective;
  float m_fieldOfView = glm::radians(45.f);
  float m_aspectRatio = 1.f;
  float m_nearDist = .1f;
  float m_farDist = 50.f;
  float m_windowAspectRatio = 1.f;
  float m_eyeSeparationAngle = glm::radians(8.f);  // angle between two eyes to focus point

  // derived camera variables
  float m_eyeSeparation;  // dist from left eye to right eye
  float m_focusDistance;
  glm::vec3 m_viewVector;  // normalized vector from eye to center (center - eye)
  float m_centerDist; // distance from eye to center
  glm::vec3 m_strafeVector; // normalized vector point at right in eye space
  // derived frustum variables
  float m_left;
  float m_right;
  float m_bottom;
  float m_top;

  // derived matrices
  glm::mat4 m_viewMatrices[3];
  glm::mat4 m_projectionMatrices[3];
  glm::mat4 m_inverseViewMatrices[3];
  glm::mat4 m_inverseProjectionMatrices[3];
  glm::mat3 m_normalMatrices[3];
  glm::mat4 m_projectionViewMatrices[3];
};

DECLARE_OPERATORS_FOR_ENUM(Z3DCamera::ResetOption)

#endif // Z3DCAMERA_H
