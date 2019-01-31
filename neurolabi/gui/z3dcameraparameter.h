#ifndef ZCAMERAPARAMETER_H
#define ZCAMERAPARAMETER_H

#include "z3dcamera.h"
#include "widgets/znumericparameter.h"
#include "widgets/zoptionparameter.h"
#include "widgets/zparameter.h"

class ZJsonObject;

/*!
 * 
 * \brief The class of camera parameters
 *
 * It wrapps all parameters for the camera, including:
 *   Projection type: which can be orthogonal or projective
 *   Eye center: center of the eye
 *   World center: center of the world, which is also the rotating center
 *   Up vector: picture direction
 *   Field of view: 
 *   Near distance:
 *   Far distance
 */
class Z3DCameraParameter : public ZSingleValueParameter<Z3DCamera>
{
  Q_OBJECT
public:
  explicit Z3DCameraParameter(const QString& name, QObject* parent = nullptr);

  Z3DCameraParameter(const QString& name, const Z3DCamera& value, QObject* parent = nullptr);

  inline void setEye(const glm::vec3& pos)
  {
    m_value.setEye(pos);
    updatePara();
  }

  inline void setCenter(const glm::vec3& focus)
  {
    m_value.setCenter(focus);
    updatePara();
  }

  inline void setUpVector(const glm::vec3& up)
  {
    m_value.setUpVector(up);
    updatePara();
  }

  inline void setEyeSeparationAngle(float angle)
  {
    m_value.setEyeSeparationAngle(angle);
    updatePara();
  }

  inline void setFrustum(float fov, float ratio, float ndist, float fdist)
  {
    m_value.setFrustum(fov, ratio, ndist, fdist);
    updatePara();
  }

  inline void setNearDist(float nd)
  {
    m_value.setNearDist(nd);
    updatePara();
  }

  inline void setFarDist(float fd)
  {
    m_value.setFarDist(fd);
    updatePara();
  }

  inline void setCamera(const glm::vec3& pos, const glm::vec3& focus, const glm::vec3& up)
  {
    m_value.setCamera(pos, focus, up);
    updatePara();
  }

  inline void setCamera(const glm::vec3& pos, const glm::vec3& focus)
  {
    m_value.setCamera(pos, focus, m_value.upVector());
    updatePara();
  }

  inline void setProjectionType(Z3DCamera::ProjectionType pt)
  { m_projectionType.select(pt == Z3DCamera::ProjectionType::Perspective ? "Perspective" : "Orthographic"); }

  void setTileFrustum(double left = 0.0, double right = 1.0, double bottom = 0.0, double top = 1.0)
  { m_value.setTileFrustum(left, right, bottom, top); emit valueChanged(); }

  void set(const ZJsonObject &cameraJson);

  void flipViewDirection();
  void rotate90X();
  void rotate90XZ();

  inline void resetCamera(const ZBBox<glm::dvec3>& bound,
                          Z3DCamera::ResetOption options = Z3DCamera::ResetOption::ResetAll)
  {
    m_value.resetCamera(bound, options);
    updatePara();
  }

  inline void resetCamera(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax,
                          Z3DCamera::ResetOption options = Z3DCamera::ResetOption::ResetAll)
  {
    m_value.resetCamera(xmin, xmax, ymin, ymax, zmin, zmax, options);
    updatePara();
  }

  inline void resetCameraNearFarPlane(const ZBBox<glm::dvec3>& bound)
  {
    m_value.resetCameraNearFarPlane(bound);
    updatePara();
  }

  inline void resetCameraNearFarPlane(double xmin, double xmax, double ymin, double ymax,
                                      double zmin, double zmax)
  {
    m_value.resetCameraNearFarPlane(xmin, xmax, ymin, ymax, zmin, zmax);
    updatePara();
  }

  inline void dolly(float value)
  {
    m_value.dolly(value);
    updatePara();
  }

  inline void dollyToCenterDistance(float cd)
  {
    m_value.dollyToCenterDistance(cd);
    updatePara();
  }

  inline void roll(float angle)
  {
    m_value.roll(angle);
    updatePara();
  }

  inline void azimuth(float angle)
  {
    m_value.azimuth(angle);
    updatePara();
  }

  inline void yaw(float angle)
  {
    m_value.yaw(angle);
    updatePara();
  }

  inline void elevation(float angle)
  {
    m_value.elevation(angle);
    updatePara();
  }

  inline void pitch(float angle)
  {
    m_value.pitch(angle);
    updatePara();
  }

  inline void zoom(float factor)
  {
    m_value.zoom(factor);
    updatePara();
  }

  inline void rotate(float angle, const glm::vec3& axis, const glm::vec3& point)
  {
    m_value.rotate(angle, axis, point);
    updatePara();
  }

  inline void rotate(const glm::quat& quat, const glm::vec3& point)
  {
    m_value.rotate(quat, point);
    updatePara();
  }

  inline void rotate(float angle, const glm::vec3& axis)
  {
    m_value.rotate(angle, axis);
    updatePara();
  }

  inline void rotate(const glm::quat& quat)
  {
    m_value.rotate(quat);
    updatePara();
  }

  glm::quat getNeuroglancerRotation() const;
  std::pair<glm::vec3, glm::vec3> getLowtisVec() const;

  void viewportChanged(const glm::uvec2& viewport);

  // ZParameter interface
public:
  virtual void setSameAs(const ZParameter& rhs) override;

  virtual void setValueSameAs(const ZParameter& rhs) override;

signals:
  void windowsAspectRatioChanged(float r);

protected:
  void setWindowsAspectRatio(float r);

  void updateProjectionType();

  void updateEye();

  void updateCenter();

  void updateUpVector();

  void updateEyeSeparationAngle();

  void updateFieldOfView();

  void updateNearDist();

  void updateFarDist();

  virtual QWidget* actualCreateWidget(QWidget* parent) override;

  virtual void beforeChange(Z3DCamera& value) override;

  void updateWidget(Z3DCamera& value);

  inline void updatePara()
  { updateWidget(m_value); emit valueChanged(); }

private:
  ZStringIntOptionParameter m_projectionType;
  ZVec3Parameter m_eye;
  ZVec3Parameter m_center;
  ZVec3Parameter m_upVector;
  ZFloatParameter m_eyeSeparationAngle;
  ZFloatParameter m_fieldOfView;
  ZFloatParameter m_nearDist;
  ZFloatParameter m_farDist;
  bool m_receiveWidgetSignal;
};

#endif // ZCAMERAPARAMETER_H
