#include "z3dtransformparameter.h"

#include "widgets/zwidgetsgroup.h"
#include <QWidget>
#include <QGroupBox>
#include <QPushButton>


Z3DTransformParameter::Z3DTransformParameter(const QString& name, QObject* parent)
  : ZSingleValueParameter<glm::mat4>(name, glm::mat4(1.f), parent)
  , m_scale("Scale", glm::vec3(1.f), glm::vec3(std::numeric_limits<float>::lowest()),
            glm::vec3(std::numeric_limits<float>::max()))
  , m_translation("Translation", glm::vec3(0.f), glm::vec3(std::numeric_limits<float>::lowest()),
                  glm::vec3(std::numeric_limits<float>::max()))
  , m_rotation("Rotation", glm::vec4(0, 0, 1, 0), glm::vec4(std::numeric_limits<float>::lowest()),
               glm::vec4(std::numeric_limits<float>::max()))
  , m_center("Rotation Center", glm::vec3(0.f), glm::vec3(std::numeric_limits<float>::lowest()),
             glm::vec3(std::numeric_limits<float>::max()))
  , m_receiveWidgetSignal(true)
{
  updateWidget(m_value);

  QStringList names;
  names << "x:" << "y:" << "z:";
  m_scale.setNameForEachValue(names);
  m_translation.setNameForEachValue(names);
  m_center.setNameForEachValue(names);

  m_scale.setSingleStep(.1);
  m_scale.setDecimal(6);
  m_scale.setStyle("SPINBOX");
  connect(&m_scale, &ZVec3Parameter::valueChanged, this, &Z3DTransformParameter::updateMatrix);

  m_translation.setSingleStep(10);
  m_translation.setDecimal(6);
  m_translation.setStyle("SPINBOX");
  connect(&m_translation, &ZVec3Parameter::valueChanged, this, &Z3DTransformParameter::updateMatrix);

  names.clear();
  names << "angle:" << "x:" << "y:" << "z:";
  m_rotation.setNameForEachValue(names);
  m_rotation.setSingleStep(1);
  m_rotation.setDecimal(6);
  m_rotation.setStyle("SPINBOX");
  connect(&m_rotation, &ZVec4Parameter::valueChanged, this, &Z3DTransformParameter::updateMatrix);

  m_center.setSingleStep(10);
  m_center.setDecimal(6);
  m_center.setStyle("SPINBOX");
  connect(&m_center, &ZVec3Parameter::valueChanged, this, &Z3DTransformParameter::updateMatrix);
}

Z3DTransformParameter::Z3DTransformParameter(const QString& name, const glm::mat4& value, QObject* parent)
  : ZSingleValueParameter<glm::mat4>(name, value, parent)
  , m_scale("Scale", glm::vec3(1.f), glm::vec3(std::numeric_limits<float>::lowest()),
            glm::vec3(std::numeric_limits<float>::max()))
  , m_translation("Translation", glm::vec3(0.f), glm::vec3(std::numeric_limits<float>::lowest()),
                  glm::vec3(std::numeric_limits<float>::max()))
  , m_rotation("Rotation", glm::vec4(0, 0, 1, 0), glm::vec4(std::numeric_limits<float>::lowest()),
               glm::vec4(std::numeric_limits<float>::max()))
  , m_center("Rotation Center", glm::vec3(0.f), glm::vec3(std::numeric_limits<float>::lowest()),
             glm::vec3(std::numeric_limits<float>::max()))
  , m_receiveWidgetSignal(true)
{
  updateWidget(m_value);

  QStringList names;
  names << "x:" << "y:" << "z:";
  m_scale.setNameForEachValue(names);
  m_translation.setNameForEachValue(names);
  m_center.setNameForEachValue(names);

  m_scale.setSingleStep(.1);
  m_scale.setDecimal(6);
  m_scale.setStyle("SPINBOX");
  connect(&m_scale, &ZVec3Parameter::valueChanged, this, &Z3DTransformParameter::updateMatrix);

  m_translation.setSingleStep(10);
  m_translation.setDecimal(6);
  m_translation.setStyle("SPINBOX");
  connect(&m_translation, &ZVec3Parameter::valueChanged, this, &Z3DTransformParameter::updateMatrix);

  names.clear();
  names << "angle:" << "x:" << "y:" << "z:";
  m_rotation.setNameForEachValue(names);
  m_rotation.setSingleStep(1);
  m_rotation.setDecimal(6);
  m_rotation.setStyle("SPINBOX");
  connect(&m_rotation, &ZVec4Parameter::valueChanged, this, &Z3DTransformParameter::updateMatrix);

  m_center.setSingleStep(10);
  m_center.setDecimal(6);
  m_center.setStyle("SPINBOX");
  connect(&m_center, &ZVec3Parameter::valueChanged, this, &Z3DTransformParameter::updateMatrix);
}

glm::quat Z3DTransformParameter::rotation() const
{
  if (glm::length(m_rotation.get().yzw()) > 0)
    return glm::angleAxis(glm::radians(m_rotation.get().x), glm::normalize(m_rotation.get().yzw()));
  else
    return glm::quat();
}

void Z3DTransformParameter::rotate(const glm::vec3& axis, float ang)
{
  glm::quat quat = rotation() * glm::angleAxis(ang, glm::normalize(axis));
  setRotation(glm::normalize(quat));
}

void Z3DTransformParameter::rotate(const glm::vec3& axis, float ang, const glm::vec3& center)
{
  m_receiveWidgetSignal = false;
  glm::vec3 scale = m_scale.get();
  glm::mat4 currValue = m_value;
  if (scale.x == 0 || scale.y == 0 || scale.z == 0) {
    scale.x = std::max(scale.x, 0.000001f);
    scale.y = std::max(scale.y, 0.000001f);
    scale.z = std::max(scale.z, 0.000001f);
    glm::mat4 trans1 = glm::translate(glm::mat4(1.f), -m_center.get() * scale);
    glm::mat4 trans = glm::translate(glm::mat4(1.f), m_translation.get() + m_center.get() * scale);
    glm::mat4 scales = glm::scale(glm::mat4(1.f), scale);
    glm::mat4 rot = glm::mat4_cast(rotation());
    currValue = trans * rot * trans1 * scales;
  }
  glm::mat4 trans1 = glm::translate(glm::mat4(1.f), -center);
  glm::mat4 rotation = glm::mat4_cast(glm::angleAxis(ang, glm::normalize(axis)));
  glm::mat4 trans2 = glm::translate(glm::mat4(1.f), center);
  currValue = trans2 * rotation * trans1 * currValue;
  glm::mat3 rotationMat(currValue);
  rotationMat[0] /= scale.x;
  rotationMat[1] /= scale.y;
  rotationMat[2] /= scale.z;
  glm::quat quat = glm::normalize(glm::quat_cast(rotationMat));
  setRotation(quat);
  m_translation.set(currValue[3].xyz());
  m_center.set(glm::vec3(0, 0, 0));
  m_receiveWidgetSignal = true;
  updateMatrix();
}

void Z3DTransformParameter::setValueSameAs(const ZParameter& rhs)
{
  CHECK(this->isSameType(rhs));
  const Z3DTransformParameter& src = static_cast<const Z3DTransformParameter&>(rhs);
  m_receiveWidgetSignal = false;
  m_scale.set(src.m_scale.get());
  m_translation.set(src.m_translation.get());
  m_rotation.set(src.m_rotation.get());
  m_center.set(src.m_center.get());
  m_receiveWidgetSignal = true;
  updateMatrix();
}

void Z3DTransformParameter::updateMatrix()
{
  if (m_receiveWidgetSignal) {
    glm::mat4 trans1 = glm::translate(glm::mat4(1.f), -m_center.get() * m_scale.get());
    glm::mat4 trans = glm::translate(glm::mat4(1.f), m_translation.get() + m_center.get() * m_scale.get());
    glm::mat4 scale = glm::scale(glm::mat4(1.f), m_scale.get());
    glm::mat4 rot = glm::mat4_cast(rotation());

    m_value = trans * rot * trans1 * scale;

    emit valueChanged();
  }
}

void Z3DTransformParameter::showTransformMatrix()
{
  LOG(INFO) << "Transform: " << m_value;
  LOG(INFO) << "Inverse Transform: " << glm::affineInverse(m_value);
  LOG(INFO) << "";
}

QWidget* Z3DTransformParameter::actualCreateWidget(QWidget* parent)
{
  ZWidgetsGroup transform("Transform", 1);
  transform.addChild(m_scale, 1);
  transform.addChild(m_rotation, 1);
  transform.addChild(m_translation, 1);
  transform.addChild(m_center, 1);

  QPushButton* pb = new QPushButton("Show Transform Matrix");
  connect(pb, &QPushButton::clicked, this, &Z3DTransformParameter::showTransformMatrix);
  transform.addChild(*pb, 2);

  QLayout* lw = transform.createLayout(false);
  //QWidget *widget = new QWidget();
  //widget->setLayout(lw);
  QGroupBox* groupBox = new QGroupBox("Transform Parameters", parent);
  groupBox->setLayout(lw);

  //widget->setParent(parent);
  //return widget;
  return groupBox;
}

void Z3DTransformParameter::beforeChange(glm::mat4& value)
{
  updateWidget(value);
}

void Z3DTransformParameter::updateWidget(const glm::mat4& value)
{
  Q_UNUSED(value)
  m_receiveWidgetSignal = false;
  //  m_eye.set(value.getEye());
  //  m_center.set(value.getCenter());
  //  m_upVector.set(value.getUpVector());
  //  if (value.isPerspectiveProjection())
  //    m_projectionType.select("Perspective");
  //  else
  //    m_projectionType.select("Orthographic");
  //  m_eyeSeparationAngle.set(value.getEyeSeparationAngle());
  //  m_fieldOfView.set(value.getFieldOfView());
  //  m_nearDist.set(value.getNearDist());
  //  m_farDist.set(value.getFarDist());
  //  m_nearDist.setRange(1e-10, value.getFarDist());
  //  m_farDist.setRange(value.getNearDist(), std::numeric_limits<float>::max());
  m_receiveWidgetSignal = true;
}

void Z3DTransformParameter::setSameAs(const ZParameter& rhs)
{
  setValueSameAs(rhs);
  ZParameter::setSameAs(rhs);
}
