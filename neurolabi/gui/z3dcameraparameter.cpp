#include "z3dcameraparameter.h"

#include <QWidget>
#include <QGroupBox>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

#include "logging/zqslog.h"
#include "zwidgetsgroup.h"
#include "zjsonobject.h"
#include "zjsonparser.h"
#include "zjsonarray.h"

Z3DCameraParameter::Z3DCameraParameter(const QString& name, QObject* parent)
  : ZSingleValueParameter<Z3DCamera>(name, parent)
  , m_projectionType("Projection Type")
  , m_eye("Eye Position", m_value.eye(), glm::vec3(std::numeric_limits<float>::lowest()),
          glm::vec3(std::numeric_limits<float>::max()))
  , m_center("Center Position", m_value.center(), glm::vec3(std::numeric_limits<float>::lowest()),
             glm::vec3(std::numeric_limits<float>::max()))
  , m_upVector("Up Vector", m_value.upVector(), glm::vec3(-1.f), glm::vec3(1.f))
  , m_eyeSeparationAngle("Eye Separation Angle", glm::degrees(m_value.eyeSeparationAngle()), 1.f, 80.f)
  , m_fieldOfView("Field of View", glm::degrees(m_value.fieldOfView()), 10.f, 170.f)
  , m_nearDist("Near Distance", m_value.nearDist(), 1e-10, std::numeric_limits<float>::max())
  , m_farDist("Far Distance", m_value.farDist(), 1e-10, std::numeric_limits<float>::max())
  , m_receiveWidgetSignal(true)
{
  m_projectionType.addOptions("Perspective", "Orthographic");
  if (m_value.isPerspectiveProjection())
    m_projectionType.select("Perspective");
  else
    m_projectionType.select("Orthographic");
  connect(&m_projectionType, &ZStringIntOptionParameter::valueChanged, this, &Z3DCameraParameter::updateProjectionType);

  m_eye.setSingleStep(1e-10);
  m_eye.setDecimal(10);
  //m_eye.setWidgetOrientation(Qt::Horizontal);
  m_eye.setStyle("SPINBOX");
  connect(&m_eye, &ZVec3Parameter::valueChanged, this, &Z3DCameraParameter::updateEye);

  m_center.setSingleStep(1e-10);
  m_center.setDecimal(10);
  //m_center.setWidgetOrientation(Qt::Horizontal);
  m_center.setStyle("SPINBOX");
  connect(&m_center, &ZVec3Parameter::valueChanged, this, &Z3DCameraParameter::updateCenter);

  m_upVector.setSingleStep(1e-10);
  m_upVector.setDecimal(10);
  m_upVector.setStyle("SPINBOX");
  connect(&m_upVector, &ZVec3Parameter::valueChanged, this, &Z3DCameraParameter::updateUpVector);

  m_eyeSeparationAngle.setSingleStep(.1);
  m_eyeSeparationAngle.setDecimal(1);
  connect(&m_eyeSeparationAngle, &ZFloatParameter::valueChanged, this, &Z3DCameraParameter::updateEyeSeparationAngle);

  m_fieldOfView.setSingleStep(.1);
  m_fieldOfView.setDecimal(1);
  connect(&m_fieldOfView, &ZFloatParameter::valueChanged, this, &Z3DCameraParameter::updateFieldOfView);

  m_nearDist.setSingleStep(1e-10);
  m_nearDist.setDecimal(10);
  m_nearDist.setStyle("SPINBOX");
  m_nearDist.setRange(1e-10, m_value.farDist());
  connect(&m_nearDist, &ZFloatParameter::valueChanged, this, &Z3DCameraParameter::updateNearDist);

  m_farDist.setSingleStep(1e-10);
  m_farDist.setDecimal(10);
  m_farDist.setStyle("SPINBOX");
  m_farDist.setRange(m_value.nearDist(), std::numeric_limits<float>::max());
  connect(&m_farDist, &ZFloatParameter::valueChanged, this, &Z3DCameraParameter::updateFarDist);
}

Z3DCameraParameter::Z3DCameraParameter(const QString& name, const Z3DCamera& value, QObject* parent)
  : ZSingleValueParameter<Z3DCamera>(name, value, parent)
  , m_projectionType("Projection Type")
  , m_eye("Eye Position", m_value.eye(), glm::vec3(std::numeric_limits<float>::lowest()),
          glm::vec3(std::numeric_limits<float>::max()))
  , m_center("Center Position", m_value.center(), glm::vec3(std::numeric_limits<float>::lowest()),
             glm::vec3(std::numeric_limits<float>::max()))
  , m_upVector("Up Vector", m_value.upVector(), glm::vec3(-1.f), glm::vec3(1.f))
  , m_eyeSeparationAngle("Eye Separation Angle", glm::degrees(m_value.eyeSeparationAngle()), 1.f, 80.f)
  , m_fieldOfView("Field of View", glm::degrees(m_value.fieldOfView()), 10.f, 170.f)
  , m_nearDist("Near Distance", m_value.nearDist(), 1e-10, std::numeric_limits<float>::max())
  , m_farDist("Far Distance", m_value.farDist(), 1e-10, std::numeric_limits<float>::max())
  , m_receiveWidgetSignal(true)
{
  m_projectionType.addOptions("Perspective", "Orthographic");
  if (m_value.isPerspectiveProjection())
    m_projectionType.select("Perspective");
  else
    m_projectionType.select("Orthographic");
  connect(&m_projectionType, &ZStringIntOptionParameter::valueChanged, this, &Z3DCameraParameter::updateProjectionType);

  m_eye.setSingleStep(1e-10);
  m_eye.setDecimal(10);
  //m_eye.setWidgetOrientation(Qt::Horizontal);
  m_eye.setStyle("SPINBOX");
  connect(&m_eye, &ZVec3Parameter::valueChanged, this, &Z3DCameraParameter::updateEye);

  m_center.setSingleStep(1e-10);
  m_center.setDecimal(10);
  //m_center.setWidgetOrientation(Qt::Horizontal);
  m_center.setStyle("SPINBOX");
  connect(&m_center, &ZVec3Parameter::valueChanged, this, &Z3DCameraParameter::updateCenter);

  m_upVector.setSingleStep(1e-10);
  m_upVector.setDecimal(10);
  m_upVector.setStyle("SPINBOX");
  connect(&m_upVector, &ZVec3Parameter::valueChanged, this, &Z3DCameraParameter::updateUpVector);

  m_eyeSeparationAngle.setSingleStep(.1);
  m_eyeSeparationAngle.setDecimal(1);
  connect(&m_eyeSeparationAngle, &ZFloatParameter::valueChanged, this, &Z3DCameraParameter::updateEyeSeparationAngle);

  m_fieldOfView.setSingleStep(.1);
  m_fieldOfView.setDecimal(1);
  connect(&m_fieldOfView, &ZFloatParameter::valueChanged, this, &Z3DCameraParameter::updateFieldOfView);

  m_nearDist.setSingleStep(1e-10);
  m_nearDist.setDecimal(10);
  m_nearDist.setStyle("SPINBOX");
  m_nearDist.setRange(1e-10, m_value.farDist());
  connect(&m_nearDist, &ZFloatParameter::valueChanged, this, &Z3DCameraParameter::updateNearDist);

  m_farDist.setSingleStep(1e-10);
  m_farDist.setDecimal(10);
  m_farDist.setStyle("SPINBOX");
  m_farDist.setRange(m_value.nearDist(), std::numeric_limits<float>::max());
  connect(&m_farDist, &ZFloatParameter::valueChanged, this, &Z3DCameraParameter::updateFarDist);
}

void Z3DCameraParameter::flipViewDirection()
{
  glm::vec3 referenceCenter = m_value.center();
  glm::vec3 eyePosition = m_value.eye();

  glm::vec3 viewVector = eyePosition - referenceCenter;
  setEye(referenceCenter - viewVector);
}

void Z3DCameraParameter::rotate90X()
{
//  azimuth(glm::radians(90.f));
  rotate(glm::radians(90.f), glm::vec3(1, 0, 0));
}

void Z3DCameraParameter::rotate90XZ()
{
  rotate(glm::radians(90.f), glm::vec3(1, 0, 0));
  rotate(glm::radians(90.f), glm::vec3(0, 0, 1));
}

void Z3DCameraParameter::viewportChanged(const glm::uvec2& viewport)
{
  m_value.setWindowAspectRatio(static_cast<float>(viewport.x) / viewport.y);
  emit windowsAspectRatioChanged(static_cast<float>(viewport.x) / viewport.y);
  emit valueChanged();
}

void Z3DCameraParameter::setWindowsAspectRatio(float r)
{
  m_value.setWindowAspectRatio(r);
  emit valueChanged();
}

void Z3DCameraParameter::updateProjectionType()
{
  if (m_receiveWidgetSignal) {
    if (m_projectionType.isSelected("Perspective"))
      m_value.setProjectionType(Z3DCamera::ProjectionType::Perspective);
    else
      m_value.setProjectionType(Z3DCamera::ProjectionType::Orthographic);
    emit valueChanged();
  }
}

void Z3DCameraParameter::updateEye()
{
  if (m_receiveWidgetSignal) {
    m_value.setEye(m_eye.get());
    emit valueChanged();
  }
}

void Z3DCameraParameter::updateCenter()
{
  if (m_receiveWidgetSignal) {
    m_value.setCenter(m_center.get());
    emit valueChanged();
  }
}

void Z3DCameraParameter::updateUpVector()
{
  if (m_receiveWidgetSignal) {
    m_value.setUpVector(m_upVector.get());
    emit valueChanged();
  }
}

void Z3DCameraParameter::updateEyeSeparationAngle()
{
  if (m_receiveWidgetSignal) {
    m_value.setEyeSeparationAngle(glm::radians(m_eyeSeparationAngle.get()));
    emit valueChanged();
  }
}

void Z3DCameraParameter::updateFieldOfView()
{
  if (m_receiveWidgetSignal) {
    m_value.setFieldOfView(glm::radians(m_fieldOfView.get()));
    emit valueChanged();
  }
}

void Z3DCameraParameter::updateNearDist()
{
  if (m_receiveWidgetSignal) {
    m_value.setNearDist(m_nearDist.get());
    m_farDist.setRange(m_value.nearDist(), std::numeric_limits<float>::max());
    emit valueChanged();
  }
}

void Z3DCameraParameter::updateFarDist()
{
  if (m_receiveWidgetSignal) {
    m_value.setFarDist(m_farDist.get());
    m_nearDist.setRange(1e-10, m_value.farDist());
    emit valueChanged();
  }
}

QWidget* Z3DCameraParameter::actualCreateWidget(QWidget* parent)
{
  ZWidgetsGroup camera("Camera", 1);
  camera.addChild(m_projectionType, 1);
  camera.addChild(m_eye, 1);
  camera.addChild(m_center, 1);
  camera.addChild(m_upVector, 1);
  camera.addChild(m_eyeSeparationAngle, 1);
  camera.addChild(m_fieldOfView, 1);
  camera.addChild(m_nearDist, 1);
  camera.addChild(m_farDist, 1);

  QLayout* lw = camera.createLayout(false);
  //QWidget *widget = new QWidget();
  //widget->setLayout(lw);
  QGroupBox* groupBox = new QGroupBox("Camera Parameters", parent);
  groupBox->setLayout(lw);

  //widget->setParent(parent);
  //return widget;
  return groupBox;
}

void Z3DCameraParameter::beforeChange(Z3DCamera& value)
{
  updateWidget(value);
}

void Z3DCameraParameter::updateWidget(Z3DCamera& value)
{
  m_receiveWidgetSignal = false;
  m_eye.set(value.eye());
  m_center.set(value.center());
  m_upVector.set(value.upVector());
  if (value.isPerspectiveProjection())
    m_projectionType.select("Perspective");
  else
    m_projectionType.select("Orthographic");
  m_eyeSeparationAngle.set(glm::degrees(value.eyeSeparationAngle()));
  m_fieldOfView.set(glm::degrees(value.fieldOfView()));
  m_nearDist.set(value.nearDist());
  m_farDist.set(value.farDist());
  m_nearDist.setRange(1e-10, value.farDist());
  m_farDist.setRange(value.nearDist(), std::numeric_limits<float>::max());
  m_receiveWidgetSignal = true;
}

void Z3DCameraParameter::setSameAs(const ZParameter& rhs)
{
  CHECK(this->isSameType(rhs));
  const Z3DCameraParameter* src = static_cast<const Z3DCameraParameter*>(&rhs);
  m_value = src->get();
  updatePara();
  ZParameter::setSameAs(rhs);
}

void Z3DCameraParameter::setValueSameAs(const ZParameter& rhs)
{
  CHECK(this->isSameType(rhs));
  const Z3DCameraParameter* src = static_cast<const Z3DCameraParameter*>(&rhs);
  m_value.setProjectionType(src->get().projectionType());
  m_value.setCamera(src->get().eye(), src->get().center(), src->get().upVector());
  //m_value.setFrustum(src->get().fieldOfView(), m_value.aspectRatio(),
  //                   src->get().nearDist(), src->get().farDist());
  m_value.setEyeSeparationAngle(src->get().eyeSeparationAngle());
  updatePara();
}

glm::quat Z3DCameraParameter::getNeuroglancerRotation() const
{
//  glm::vec3 orig(0, 0, 1);
//  glm::vec3 dest = glm::normalize(m_eye.get() - m_center.get());

  glm::vec3 upVector = -m_upVector.get();
//  upVector.x = -upVector.x;
//  upVector.y = -upVector.y;
//  upVector.z = -upVector.z;

  return glm::conjugate(glm::toQuat(
        glm::lookAt(m_center.get(), m_eye.get(), upVector)));
//  return glm::rotation(orig, dest);
}

std::pair<glm::vec3, glm::vec3> Z3DCameraParameter::getLowtisVec() const
{
  std::pair<glm::vec3, glm::vec3> ort;
  ort.second = -m_upVector.get();
  glm::vec3 look = glm::normalize(m_center.get() - m_eye.get());
  ort.first = glm::cross(ort.second, look);

  return ort;
}

void Z3DCameraParameter::set(const ZJsonObject &cameraJson)
{
  if (cameraJson.hasKey("eye")) {
    for (int i = 0; i < 3; ++i) {
      m_eye.set(glm::vec3(ZJsonParser::numberValue(cameraJson["eye"], 0),
                ZJsonParser::numberValue(cameraJson["eye"], 1),
          ZJsonParser::numberValue(cameraJson["eye"], 2)));
    }
  }

  if (cameraJson.hasKey("center")) {
    for (int i = 0; i < 3; ++i) {
      m_center.set(glm::vec3(ZJsonParser::numberValue(cameraJson["center"], 0),
                ZJsonParser::numberValue(cameraJson["center"], 1),
          ZJsonParser::numberValue(cameraJson["center"], 2)));
    }
  }

  if (cameraJson.hasKey("up_vector")) {
    for (int i = 0; i < 3; ++i) {
      m_upVector.set(glm::vec3(ZJsonParser::numberValue(cameraJson["up_vector"], 0),
                ZJsonParser::numberValue(cameraJson["up_vector"], 1),
          ZJsonParser::numberValue(cameraJson["up_vector"], 2)));
    }
  }

  if (cameraJson.hasKey("projection")) {
    m_projectionType.select(
          ZJsonParser::stringValue(cameraJson["projection"]).c_str());
  }

  if (cameraJson.hasKey("field_of_view")) {
    m_fieldOfView.set(glm::degrees(ZJsonParser::numberValue(
                        cameraJson["field_of_view"])));
  }

  if (cameraJson.hasKey("near_dist")) {
    m_nearDist.set(ZJsonParser::numberValue(cameraJson["near_dist"]));
  }

  if (cameraJson.hasKey("far_dist")) {
    m_farDist.set(ZJsonParser::numberValue(cameraJson["far_dist"]));
  }

  m_nearDist.setRange(1e-10, m_farDist.get());
  m_farDist.setRange(m_nearDist.get(), std::numeric_limits<float>::max());
}
