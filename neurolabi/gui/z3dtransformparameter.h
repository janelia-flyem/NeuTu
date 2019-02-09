#ifndef Z3DTRANSFORMPARAMETER_H
#define Z3DTRANSFORMPARAMETER_H

#include "zglmutils.h"
#include "widgets/znumericparameter.h"
#include "widgets/zparameter.h"

class Z3DTransformParameter : public ZSingleValueParameter<glm::mat4>
{
Q_OBJECT
public:
  explicit Z3DTransformParameter(const QString& name, QObject* parent = nullptr);

  Z3DTransformParameter(const QString& name, const glm::mat4& value, QObject* parent = nullptr);

  inline void setScale(const glm::vec3& v)
  { m_scale.set(v); }

  inline void setTranslation(const glm::vec3& v)
  { m_translation.set(v); }

  inline void setRotationCenter(const glm::vec3& c)
  { m_center.set(c); }

  // angle in degree and axis
  inline void setRotation(const glm::vec4& v)
  { m_rotation.set(v); }

  inline void setRotation(const glm::quat& v)
  { m_rotation.set(glm::vec4(glm::degrees(glm::angle(v)), glm::axis(v))); }

  inline void setCenter(const glm::vec3& v)
  { return m_center.set(v); }

  inline glm::vec3 scale() const
  { return m_scale.get(); }

  inline glm::vec3 translation() const
  { return m_translation.get(); }

  glm::quat rotation() const;

  inline void setXScale(float s)
  { m_scale.set(glm::vec3(s, m_scale.get().y, m_scale.get().z)); }

  inline void setYScale(float s)
  { m_scale.set(glm::vec3(m_scale.get().x, s, m_scale.get().z)); }

  inline void setZScale(float s)
  { m_scale.set(glm::vec3(m_scale.get().x, m_scale.get().y, s)); }

  inline void translate(float x, float y, float z)
  { m_translation.set(glm::vec3(x, y, z) + m_translation.get()); }

  inline void translate(const glm::vec3& t)
  { m_translation.set(t + m_translation.get()); }

  void rotate(const glm::vec3& axis, float ang);

  void rotate(const glm::vec3& axis, float ang, const glm::vec3& center);

  virtual void setValueSameAs(const ZParameter& rhs) override;

  // ZParameter interface
public:
  virtual void setSameAs(const ZParameter& rhs) override;

protected:
  void updateMatrix();

  void showTransformMatrix();

  virtual QWidget* actualCreateWidget(QWidget* parent) override;

  virtual void beforeChange(glm::mat4& value) override;

  void updateWidget(const glm::mat4& value);

private:
  ZVec3Parameter m_scale;
  ZVec3Parameter m_translation;
  ZVec4Parameter m_rotation;  // angle in degree and axis
  ZVec3Parameter m_center;
  bool m_receiveWidgetSignal;
};

#endif // Z3DTRANSFORMPARAMETER_H
