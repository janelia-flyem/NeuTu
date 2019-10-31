#include "z3dcamera.h"

#include <cmath>

#include "logging/zqslog.h"
#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonparser.h"

Z3DCamera::Z3DCamera()
{
  updateCamera();
  updateFrustum();
}

void Z3DCamera::setCamera(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& upVector)
{
  m_eye = eye;
  m_center = center;
  m_upVector = glm::normalize(upVector);
  updateCamera();
}

void Z3DCamera::setFrustum(float fov, float ratio, float nearDist, float farDist)
{
  m_fieldOfView = fov;
  m_aspectRatio = ratio;
  m_nearDist = nearDist;
  m_farDist = farDist;
  updateFrustum();
}

void Z3DCamera::setTileFrustum(double left, double right, double bottom, double top)
{
  float halfheight = std::tan(0.5f * m_fieldOfView) * m_nearDist;
  float halfwidth = halfheight * m_aspectRatio * m_windowAspectRatio;

  m_left = -halfwidth + 2 * halfwidth * left;
  m_right = -halfwidth + 2 * halfwidth * right;
  m_bottom = -halfheight + 2 * halfheight * bottom;
  m_top = -halfheight + 2 * halfheight * top;

  //LOG(INFO) << m_left << m_right << m_bottom << m_top << halfheight << halfwidth;

  makeProjectionMatrices();
}

void Z3DCamera::resetCamera(const ZBBox<glm::dvec3>& bound, ResetOption options)
{
  glm::vec3 center = glm::vec3((bound.minCorner() + bound.maxCorner()) / 2.0);

  if (!is_flag_set(options, ResetOption::PreserveCenterDistance)) {
    auto boundSize = bound.size();
    float w1 = boundSize.x;
    float w2 = boundSize.y;
    float w3 = boundSize.z;
    w1 *= w1;
    w2 *= w2;
    w3 *= w3;
    float radius = w1 + w2 + w3;
    radius = (radius == 0) ? (1.0) : (radius);

    // compute the radius of the enclosing sphere
    //radius = std::sqrt(radius)*0.5 + m_eyeSeparation/2.f;
    radius = std::sqrt(radius) * 0.5;

    // (from VTK) compute the distance from the intersection of the view frustum with the
    // bounding sphere. Basically in 2D draw a circle representing the bounding
    // sphere in 2D then draw a horizontal line going out from the center of
    // the circle. That is the camera view. Then draw a line from the camera
    // position to the point where it intersects the circle. (it will be tangent
    // to the circle at this point, this is important, only go to the tangent
    // point, do not draw all the way to the view plane). Then draw the radius
    // from the tangent point to the center of the circle. You will note that
    // this forms a right triangle with one side being the radius, another being
    // the target distance for the camera, then just find the target dist using
    // a sin.
    double angle = m_fieldOfView;
    if (m_aspectRatio < 1.0) {  // use horizontal angle to calculate
      angle = 2.0 * std::atan(std::tan(angle * 0.5) * m_aspectRatio);
    }

    m_centerDist = radius / std::sin(angle * 0.5);
  }
  if (!is_flag_set(options, ResetOption::PreserveViewVector)) {
    m_viewVector = glm::vec3(0.f, 0.f, 1.f);
    m_upVector = glm::vec3(0.f, -1.f, 0.f);
  }
  glm::vec3 eye = center - m_centerDist * m_viewVector;
  setCamera(eye, center, m_upVector);

  resetCameraNearFarPlane(bound);
}

void Z3DCamera::resetCamera(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax,
                            ResetOption options)
{
  resetCamera(ZBBox<glm::dvec3>(glm::dvec3(xmin, ymin, zmin), glm::dvec3(xmax, ymax, zmax)), options);
}

void Z3DCamera::resetCameraNearFarPlane(const ZBBox<glm::dvec3>& bound)
{
  double a = m_viewVector[0];
  double b = m_viewVector[1];
  double c = m_viewVector[2];
  double d = -(a * m_eye[0] + b * m_eye[1] + c * m_eye[2]);

  double bd[6];
  bd[0] = bound.minCorner().x;
  bd[1] = bound.maxCorner().x;
  bd[2] = bound.minCorner().y;
  bd[3] = bound.maxCorner().y;
  bd[4] = bound.minCorner().z;
  bd[5] = bound.maxCorner().z;

  // Set the max near clipping plane and the min far clipping plane
  double range[2];
  range[0] = std::numeric_limits<double>::max();
  range[1] = 1e-18;

  // Find the closest / farthest bounding box vertex
  for (int k = 0; k < 2; ++k) {
    for (int j = 0; j < 2; ++j) {
      for (int i = 0; i < 2; ++i) {
        double dist = a * bd[i] + b * bd[2 + j] + c * bd[4 + k] + d;
        range[0] = std::min(dist, range[0]);
        range[1] = std::max(dist, range[1]);
      }
    }
  }

  // Do not let the range behind the camera throw off the calculation.
  if (range[0] < 0.0) {
    range[0] = 0.0;
  }

  // Give ourselves a little breathing room
  range[0] = 0.99 * range[0]; // - (range[1] - range[0])*0.5;
  range[1] = 1.01 * range[1]; // + (range[1] - range[0])*0.5;

  // Make sure near is not bigger than far
  //range[0] = (range[0] >= range[1])?(0.01*range[1]):(range[0]);

  // Make sure near is at least some fraction of far - this prevents near
  // from being behind the camera or too close in front.
  double nearClippingPlaneTolerance = 0.001;

  // make sure the front clipping range is not too far from the far clippnig
  // range, this is to make sure that the zbuffer resolution is effectively
  // used
  if (range[0] < nearClippingPlaneTolerance * range[1]) {
    range[0] = nearClippingPlaneTolerance * range[1];
  }

  m_nearDist = range[0];
  m_farDist = range[1];
  updateFrustum();
}

void Z3DCamera::resetCameraNearFarPlane(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax)
{
  resetCameraNearFarPlane(ZBBox<glm::dvec3>(glm::dvec3(xmin, ymin, zmin), glm::dvec3(xmax, ymax, zmax)));
}

bool Z3DCamera::operator==(const Z3DCamera& rhs) const
{
  return (m_eye == rhs.m_eye) &&
         (m_center == rhs.m_center) &&
         (m_upVector == rhs.m_upVector) &&
         (m_projectionType == rhs.m_projectionType) &&
         (m_fieldOfView == rhs.m_fieldOfView) &&
         (m_aspectRatio == rhs.m_aspectRatio) &&
         (m_nearDist == rhs.m_nearDist) &&
         (m_farDist == rhs.m_farDist) &&
         (m_windowAspectRatio == rhs.m_windowAspectRatio) &&
         (m_eyeSeparationAngle == rhs.m_eyeSeparationAngle);
}

bool Z3DCamera::operator!=(const Z3DCamera& rhs) const
{
  return !(*this == rhs);
}

void Z3DCamera::dolly(float value)
{
  if (value <= 0.f || (m_centerDist < 0.01f && value > 1.f))
    return;
  glm::vec3 pos = m_center - m_viewVector * (m_centerDist / value);
  float maxV = 1e15;
  if (std::abs(pos.x) < maxV && std::abs(pos.y) < maxV && std::abs(pos.z) < maxV)
    setEye(pos);
}

void Z3DCamera::dollyToCenterDistance(float centerDist)
{
  centerDist = std::max(0.01f, std::min(m_centerDist * 100.f, centerDist));
  glm::vec3 pos = m_center - m_viewVector * centerDist;
  float maxV = 1e15;
  if (std::abs(pos.x) < maxV && std::abs(pos.y) < maxV && std::abs(pos.z) < maxV)
    setEye(pos);
}

void Z3DCamera::roll(float angle)
{
  glm::vec3 up = glm::rotate(glm::angleAxis(angle, m_viewVector), m_upVector);
  setUpVector(up);
}

void Z3DCamera::azimuth(float angle)
{
  glm::vec3 eye = m_eye - m_center;
  eye = glm::rotate(glm::angleAxis(angle, m_upVector), eye);
  eye += m_center;
  setEye(eye);
}

void Z3DCamera::yaw(float angle)
{
  glm::vec3 center = m_center - m_eye;
  center = glm::rotate(glm::angleAxis(angle, m_upVector), center);
  center += m_eye;
  setCenter(center);
}

void Z3DCamera::elevation(float angle)
{
  rotate(angle, -m_strafeVector);
}

void Z3DCamera::pitch(float angle)
{
  rotate(angle, m_strafeVector, m_eye);
}

void Z3DCamera::zoom(float factor)
{
  if (factor <= 0.f)
    return;
  setFieldOfView(m_fieldOfView / factor);
}

void Z3DCamera::rotate(float angle, const glm::vec3& axis, const glm::vec3& point)
{
  rotate(glm::angleAxis(angle, glm::normalize(axis)), point);
}

void Z3DCamera::rotate(const glm::quat& quat, const glm::vec3& point)
{
  glm::vec3 eye = m_eye - point;
  eye = glm::rotate(quat, eye);
  eye += point;

  glm::vec3 center = m_center - point;
  center = glm::rotate(quat, center);
  center += point;

  glm::vec3 upVector = glm::rotate(quat, m_upVector);

  setCamera(eye, center, upVector);
}

void Z3DCamera::rotate(float angle, const glm::vec3& axis)
{
  rotate(glm::angleAxis(angle, glm::normalize(axis)));
}

void Z3DCamera::rotate(const glm::quat& quat)
{
  glm::vec3 eye = m_eye - m_center;
  eye = glm::rotate(quat, eye);
  eye += m_center;

  glm::vec3 upVector = glm::rotate(quat, m_upVector);

  setCamera(eye, m_center, upVector);
}

glm::vec3 Z3DCamera::vectorEyeToWorld(const glm::vec3& vec, Z3DEye eye)
{
  return glm::inverse(glm::mat3(viewMatrix(eye))) * vec;
}

glm::vec3 Z3DCamera::vectorWorldToEye(const glm::vec3& vec, Z3DEye eye)
{
  return glm::mat3(viewMatrix(eye)) * vec;
}

glm::vec3 Z3DCamera::pointEyeToWorld(const glm::vec3& pt, Z3DEye eye)
{
  return glm::applyMatrix(glm::inverse(viewMatrix(eye)), pt);
}

glm::vec3 Z3DCamera::pointWorldToEye(const glm::vec3& pt, Z3DEye eye)
{
  return glm::applyMatrix(viewMatrix(eye), pt);
}

glm::vec3 Z3DCamera::worldToScreen(const glm::vec3& wpt, const glm::ivec4& viewport, Z3DEye eye)
{
//  glm::vec4 clipSpacePos = projectionMatrix(eye) * viewMatrix(eye) * glm::vec4(wpt, 1.f);
//  if (clipSpacePos.w == 0.f)
//    return glm::vec3(-1.f, -1.f, -1.f);
//  glm::vec3 ndcSpacePos = glm::vec3(clipSpacePos.xyz()) / clipSpacePos.w;
//  return ((ndcSpacePos + 1.f) / 2.f) * glm::vec3(viewport.z, viewport.w, 1.f)
//         + glm::vec3(viewport.x, viewport.y, 0.f);
  return glm::project(wpt, viewMatrix(eye), projectionMatrix(eye), viewport);
}

glm::vec3 Z3DCamera::screenToWorld(const glm::vec3& spt, const glm::ivec4& viewport, Z3DEye eye)
{
  return glm::unProject(spt, viewMatrix(eye), projectionMatrix(eye), viewport);
}

void Z3DCamera::updateCamera()
{
  m_viewVector = glm::normalize(m_center - m_eye);
  m_centerDist = glm::length(m_center - m_eye);
  // make sure upVector is not parallel to viewVector
  if (std::abs(glm::dot(m_upVector, m_viewVector)) >= 0.9) {
    LOG(WARNING) << "Resetting view up since view plane normal is parallel";
    m_upVector = glm::cross(m_viewVector, glm::vec3(1.f, 0.f, 0.f));
    if (glm::dot(m_upVector, m_upVector) < 0.001)
      m_upVector = glm::cross(m_viewVector, glm::vec3(0.f, 1.f, 0.f));
    m_upVector = glm::normalize(m_upVector);
  }
  m_strafeVector = glm::cross(m_viewVector, m_upVector);
  m_eyeSeparation = 2.f * m_farDist * std::tan(m_eyeSeparationAngle / 2.f);
  m_focusDistance = std::min((m_farDist - m_nearDist) * 0.75f + m_nearDist,
                             m_nearDist * 2.f);
  m_eyeSeparation = m_focusDistance / 30.f;

  makeViewMatrices();
}

void Z3DCamera::updateFrustum()
{
  float halfheight = std::tan(0.5f * m_fieldOfView) * m_nearDist;
  m_top = halfheight;
  m_bottom = -halfheight;
  float halfwidth = halfheight * m_aspectRatio * m_windowAspectRatio;
  m_left = -halfwidth;
  m_right = halfwidth;

  makeProjectionMatrices();
}

void Z3DCamera::makeViewMatrices()
{
  glm::vec3 adjust = m_strafeVector * -m_eyeSeparation / 2.f;
  m_viewMatrices[enumToUnderlyingType(Z3DEye::Left)] = glm::lookAt(m_eye + adjust, m_center + adjust, m_upVector);
  m_viewMatrices[enumToUnderlyingType(Z3DEye::Mono)] = glm::lookAt(m_eye, m_center, m_upVector);
  adjust = m_strafeVector * m_eyeSeparation / 2.f;
  m_viewMatrices[enumToUnderlyingType(Z3DEye::Right)] = glm::lookAt(m_eye + adjust, m_center + adjust, m_upVector);

  m_inverseViewMatrices[enumToUnderlyingType(Z3DEye::Left)] = glm::inverse(
    m_viewMatrices[enumToUnderlyingType(Z3DEye::Left)]);
  m_inverseViewMatrices[enumToUnderlyingType(Z3DEye::Mono)] = glm::inverse(
    m_viewMatrices[enumToUnderlyingType(Z3DEye::Mono)]);
  m_inverseViewMatrices[enumToUnderlyingType(Z3DEye::Right)] = glm::inverse(
    m_viewMatrices[enumToUnderlyingType(Z3DEye::Right)]);

  m_projectionViewMatrices[enumToUnderlyingType(Z3DEye::Left)] =
    m_projectionMatrices[enumToUnderlyingType(Z3DEye::Left)] * m_viewMatrices[enumToUnderlyingType(Z3DEye::Left)];
  m_projectionViewMatrices[enumToUnderlyingType(Z3DEye::Mono)] =
    m_projectionMatrices[enumToUnderlyingType(Z3DEye::Mono)] * m_viewMatrices[enumToUnderlyingType(Z3DEye::Mono)];
  m_projectionViewMatrices[enumToUnderlyingType(Z3DEye::Right)] =
    m_projectionMatrices[enumToUnderlyingType(Z3DEye::Right)] * m_viewMatrices[enumToUnderlyingType(Z3DEye::Right)];

  m_normalMatrices[enumToUnderlyingType(Z3DEye::Left)] =
    glm::transpose(glm::inverse(glm::mat3(m_viewMatrices[enumToUnderlyingType(Z3DEye::Left)])));
  m_normalMatrices[enumToUnderlyingType(Z3DEye::Mono)] =
    glm::transpose(glm::inverse(glm::mat3(m_viewMatrices[enumToUnderlyingType(Z3DEye::Mono)])));
  m_normalMatrices[enumToUnderlyingType(Z3DEye::Right)] =
    glm::transpose(glm::inverse(glm::mat3(m_viewMatrices[enumToUnderlyingType(Z3DEye::Right)])));
}

void Z3DCamera::makeProjectionMatrices()
{
  if (m_projectionType == ProjectionType::Orthographic) {
    glm::mat4 pmat = glm::ortho(m_left, m_right, m_bottom, m_top, m_nearDist, m_farDist);
    m_projectionMatrices[enumToUnderlyingType(Z3DEye::Left)] = pmat;
    m_projectionMatrices[enumToUnderlyingType(Z3DEye::Mono)] = pmat;
    m_projectionMatrices[enumToUnderlyingType(Z3DEye::Right)] = pmat;
    pmat = glm::inverse(pmat);
    m_inverseProjectionMatrices[enumToUnderlyingType(Z3DEye::Left)] = pmat;
    m_inverseProjectionMatrices[enumToUnderlyingType(Z3DEye::Mono)] = pmat;
    m_inverseProjectionMatrices[enumToUnderlyingType(Z3DEye::Right)] = pmat;
  } else {
    //LOG(INFO) << m_left << m_right << m_bottom << m_top;
    m_projectionMatrices[enumToUnderlyingType(Z3DEye::Mono)] = glm::frustum(m_left, m_right, m_bottom, m_top,
                                                                            m_nearDist, m_farDist);
    float frustumShift = 0.5f * m_eyeSeparation * m_nearDist / m_focusDistance;
    m_projectionMatrices[enumToUnderlyingType(Z3DEye::Left)] = glm::frustum(m_left + frustumShift,
                                                                            m_right + frustumShift,
                                                                            m_bottom, m_top, m_nearDist, m_farDist);
    m_projectionMatrices[enumToUnderlyingType(Z3DEye::Right)] = glm::frustum(m_left - frustumShift,
                                                                             m_right - frustumShift,
                                                                             m_bottom, m_top, m_nearDist, m_farDist);

    m_inverseProjectionMatrices[enumToUnderlyingType(Z3DEye::Left)] = glm::inverse(
      m_projectionMatrices[enumToUnderlyingType(Z3DEye::Left)]);
    m_inverseProjectionMatrices[enumToUnderlyingType(Z3DEye::Mono)] = glm::inverse(
      m_projectionMatrices[enumToUnderlyingType(Z3DEye::Mono)]);
    m_inverseProjectionMatrices[enumToUnderlyingType(Z3DEye::Right)] = glm::inverse(
      m_projectionMatrices[enumToUnderlyingType(Z3DEye::Right)]);
  }

  m_projectionViewMatrices[enumToUnderlyingType(Z3DEye::Left)] =
    m_projectionMatrices[enumToUnderlyingType(Z3DEye::Left)] * m_viewMatrices[enumToUnderlyingType(Z3DEye::Left)];
  m_projectionViewMatrices[enumToUnderlyingType(Z3DEye::Mono)] =
    m_projectionMatrices[enumToUnderlyingType(Z3DEye::Mono)] * m_viewMatrices[enumToUnderlyingType(Z3DEye::Mono)];
  m_projectionViewMatrices[enumToUnderlyingType(Z3DEye::Right)] =
    m_projectionMatrices[enumToUnderlyingType(Z3DEye::Right)] * m_viewMatrices[enumToUnderlyingType(Z3DEye::Right)];
}

ZJsonObject Z3DCamera::toJsonObject() const
{
  ZJsonObject cameraJson;

  ZJsonArray eyeJson;
  eyeJson.append(m_eye[0]);
  eyeJson.append(m_eye[1]);
  eyeJson.append(m_eye[2]);
  cameraJson.setEntry("eye", eyeJson);

  ZJsonArray centerJson;
  centerJson.append(m_center[0]);
  centerJson.append(m_center[1]);
  centerJson.append(m_center[2]);
  cameraJson.setEntry("center", centerJson);

  ZJsonArray upVectorJson;
  upVectorJson.append(m_upVector[0]);
  upVectorJson.append(m_upVector[1]);
  upVectorJson.append(m_upVector[2]);
  cameraJson.setEntry("up_vector", upVectorJson);

  switch (m_projectionType) {
  case ProjectionType::Perspective:
    cameraJson.setEntry("projection", std::string("Perspective"));
    break;
  case ProjectionType::Orthographic:
    cameraJson.setEntry("projection", std::string("Orthographic"));
    break;
  }

  cameraJson.setEntry("field_of_view", m_fieldOfView);
  cameraJson.setEntry("aspect_ratio", m_aspectRatio);
  cameraJson.setEntry("near_dist", m_nearDist);
  cameraJson.setEntry("far_dist", m_farDist);

  return cameraJson;
}

void Z3DCamera::set(const ZJsonObject &cameraJson)
{
  if (cameraJson.hasKey("eye")) {
    for (int i = 0; i < 3; ++i) {
      m_eye[i] = ZJsonParser::numberValue(cameraJson["eye"], i);
    }
  }

  if (cameraJson.hasKey("center")) {
    for (int i = 0; i < 3; ++i) {
      m_center[i] = ZJsonParser::numberValue(cameraJson["center"], i);
    }
  }

  if (cameraJson.hasKey("up_vector")) {
    for (int i = 0; i < 3; ++i) {
      m_upVector[i] = ZJsonParser::numberValue(cameraJson["up_vector"], i);
    }
  }

  if (cameraJson.hasKey("projection")) {
    if (ZJsonParser::stringValue(cameraJson["projection"]) ==
               "Perspective") {
      m_projectionType = ProjectionType::Perspective;
    } else if (ZJsonParser::stringValue(cameraJson["projection"]) ==
                      "Orthographic") {
      m_projectionType = ProjectionType::Orthographic;
    }
  }

  if (cameraJson.hasKey("field_of_view")) {
    m_fieldOfView = ZJsonParser::numberValue(cameraJson["field_of_view"]);
  }

  if (cameraJson.hasKey("aspect_ratio")) {
    m_aspectRatio = ZJsonParser::numberValue(cameraJson["aspect_ratio"]);
  }

  if (cameraJson.hasKey("near_dist")) {
    m_nearDist = ZJsonParser::numberValue(cameraJson["near_dist"]);
  }

  if (cameraJson.hasKey("far_dist")) {
    m_farDist = ZJsonParser::numberValue(cameraJson["far_dist"]);
  }

  updateCamera();
  updateFrustum();
}
