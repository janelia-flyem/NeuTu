#include "znumericparameter.h"

#include <iostream>

#include "logging/zlog.h"
#include "zspinboxwithslider.h"
#include "zspinboxwithscrollbar.h"
#include "zspinbox.h"
#include "zclickablelabel.h"
#include "zspanslider.h"
#include "zutils.h"
#include <QBoxLayout>
#include <QGroupBox>
#include <QLabel>

ZIntParameter::ZIntParameter(const QString& name, QObject* parent)
  : ZNumericParameter<int>(name, 0, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), parent)
{
  addStyle("SPINBOX");
  addStyle("SPINBOXWITHSCROLLBAR");
}

ZIntParameter::ZIntParameter(const QString& name, int value, int min, int max, QObject* parent)
  : ZNumericParameter<int>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
  addStyle("SPINBOXWITHSCROLLBAR");
}

void ZIntParameter::setValue(int v)
{
  set(v);
}

void ZIntParameter::beforeChange(int& value)
{
  emit valueWillChange(value);
}

void ZIntParameter::afterChange(int& value)
{
  emit intChanged(value);
}

QWidget* ZIntParameter::actualCreateWidget(QWidget* parent)
{
  if (m_style == "SPINBOX") {
    auto sb = new ZSpinBox(parent);
    sb->setRange(m_min, m_max);
    sb->setValue(m_value);
    sb->setSingleStep(m_step);
    sb->setPrefix(m_prefix);
    sb->setSuffix(m_suffix);
#if __cplusplus > 201103L
    connect(sb, qOverload<int>(&ZSpinBox::valueChanged), this, &ZIntParameter::setValue);
#else
    connect(sb, QOverload<int>::of(&ZSpinBox::valueChanged), this, &ZIntParameter::setValue);
#endif
    connect(this, &ZIntParameter::valueWillChange, sb, &ZSpinBox::setValue);
    connect(this, &ZIntParameter::rangeChanged, sb, &ZSpinBox::setRange);
    return sb;
  } else {
    if (!m_style.isEmpty() && m_style != "SPINBOXWITHSCROLLBAR"
        && m_style != "DEFAULT") {
      ZWARN(neutu::TOPIC_NULL) << "Unknown widget style: " + m_style + ". Fall back to default style.";
    }
    ZSpinBoxWithSlider* sbws = new ZSpinBoxWithSlider(
          m_value, m_min, m_max, m_step, m_tracking, m_prefix, m_suffix, parent);
    connect(sbws, &ZSpinBoxWithSlider::valueChanged, this, &ZIntParameter::setValue);
    connect(this, &ZIntParameter::valueWillChange, sbws, &ZSpinBoxWithSlider::setValue);
    connect(this, &ZIntParameter::rangeChanged, sbws, &ZSpinBoxWithSlider::setDataRange);
    return sbws;
  }
}

void ZIntParameter::changeRange()
{
  emit rangeChanged(m_min, m_max);
}

ZDoubleParameter::ZDoubleParameter(const QString& name, QObject* parent)
  : ZNumericParameter<double>(name, 0.0, std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(),
                              parent)
{
  addStyle("SPINBOX");
}

ZDoubleParameter::ZDoubleParameter(const QString& name, double value, double min, double max, QObject* parent)
  : ZNumericParameter<double>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
}

void ZDoubleParameter::setValue(double v)
{
  set(v);
}

void ZDoubleParameter::beforeChange(double& value)
{
  emit valueWillChange(value);
}

void ZDoubleParameter::afterChange(double& value)
{
  emit doubleChanged(value);
}

QWidget* ZDoubleParameter::actualCreateWidget(QWidget* parent)
{
  if (m_style == "SPINBOX") {
    auto sb = new ZDoubleSpinBox(parent);
    sb->setRange(m_min, m_max);
    sb->setValue(m_value);
    sb->setSingleStep(m_step);
    sb->setDecimals(m_decimal);
    sb->setPrefix(m_prefix);
    sb->setSuffix(m_suffix);
#if __cplusplus > 201103L
    connect(sb, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZDoubleParameter::setValue);
#else
    connect(sb, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZDoubleParameter::setValue);
#endif
    connect(this, &ZDoubleParameter::valueWillChange, sb, &ZDoubleSpinBox::setValue);
    connect(this, &ZDoubleParameter::rangeChanged, sb, &ZDoubleSpinBox::setRange);
    return sb;
  } else {
    ZDoubleSpinBoxWithSlider* sbws = new ZDoubleSpinBoxWithSlider(m_value, m_min, m_max, m_step,
                                                                  m_decimal, m_tracking, m_prefix, m_suffix, parent);
    connect(sbws, &ZDoubleSpinBoxWithSlider::valueChanged, this, &ZDoubleParameter::setValue);
    connect(this, &ZDoubleParameter::valueWillChange, sbws, &ZDoubleSpinBoxWithSlider::setValue);
    connect(this, &ZDoubleParameter::rangeChanged, sbws, &ZDoubleSpinBoxWithSlider::setDataRange);
    return sbws;
  }
}

void ZDoubleParameter::changeRange()
{
  emit rangeChanged(m_min, m_max);
}

ZFloatParameter::ZFloatParameter(const QString& name, QObject* parent)
  : ZNumericParameter<float>(name, 0.f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max(), parent)
{
  addStyle("SPINBOX");
}

ZFloatParameter::ZFloatParameter(const QString& name, float value, float min, float max, QObject* parent)
  : ZNumericParameter<float>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
}

void ZFloatParameter::setValue(double v)
{
  set(static_cast<float>(v));
}

void ZFloatParameter::beforeChange(float& value)
{
  emit valueWillChange(value);
}

void ZFloatParameter::afterChange(float& value)
{
  emit floatChanged(static_cast<double>(value));
}

QWidget* ZFloatParameter::actualCreateWidget(QWidget* parent)
{
  if (m_style == "SPINBOX") {
    auto sb = new ZDoubleSpinBox(parent);
    sb->setRange(m_min, m_max);
    sb->setValue(m_value);
    sb->setSingleStep(m_step);
    sb->setDecimals(m_decimal);
    sb->setPrefix(m_prefix);
    sb->setSuffix(m_suffix);
#if __cplusplus > 201103L
    connect(sb, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZFloatParameter::setValue);
#else
    connect(sb, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZFloatParameter::setValue);
#endif
    connect(this, &ZFloatParameter::valueWillChange, sb, &ZDoubleSpinBox::setValue);
    connect(this, &ZFloatParameter::rangeChanged, sb, &ZDoubleSpinBox::setRange);
    return sb;
  } else {
    ZDoubleSpinBoxWithSlider* sbws = new ZDoubleSpinBoxWithSlider(m_value, m_min, m_max, m_step,
                                                                  m_decimal, m_tracking, m_prefix, m_suffix, parent);
    connect(sbws, &ZDoubleSpinBoxWithSlider::valueChanged, this, &ZFloatParameter::setValue);
    connect(this, &ZFloatParameter::valueWillChange, sbws, &ZDoubleSpinBoxWithSlider::setValue);
    connect(this, &ZFloatParameter::rangeChanged, sbws, &ZDoubleSpinBoxWithSlider::setDataRange);
    return sbws;
  }
}

void ZFloatParameter::changeRange()
{
  emit rangeChanged(m_min, m_max);
}

//---------------------------------------------------------------------------------------------------------------

ZVec2Parameter::ZVec2Parameter(const QString& name, QObject* parent)
  : ZNumericVectorParameter<glm::vec2>(name, glm::vec2(0.0), glm::vec2(std::numeric_limits<float>::lowest()),
                                       glm::vec2(std::numeric_limits<float>::max()), parent)
{
  addStyle("SPINBOX");
}

ZVec2Parameter::ZVec2Parameter(const QString& name, glm::vec2 value, glm::vec2 min, glm::vec2 max, QObject* parent)
  : ZNumericVectorParameter<glm::vec2>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
}

void ZVec2Parameter::setValue1(double v)
{
  set(glm::vec2(static_cast<float>(v), m_value[1]));
}

void ZVec2Parameter::setValue2(double v)
{
  set(glm::vec2(m_value[0], static_cast<float>(v)));
}

void ZVec2Parameter::beforeChange(glm::vec2& value)
{
  if (value[0] != m_value[0])
    emit value1WillChange(value[0]);
  if (value[1] != m_value[1])
    emit value2WillChange(value[1]);
}

QWidget* ZVec2Parameter::actualCreateWidget(QWidget* parent)
{
  QWidget* w;
  if (m_widgetOrientation == Qt::Horizontal)
    w = new QWidget(parent);
  else
    w = new QGroupBox(m_groupBoxName, parent);
  QBoxLayout* lo;
  if (m_widgetOrientation == Qt::Horizontal)
    lo = new QHBoxLayout();
  else
    lo = new QVBoxLayout();

  if (m_style == "SPINBOX") {
    {
      auto sb1 = new ZDoubleSpinBox();
      sb1->setRange(m_min[0], m_max[0]);
      sb1->setValue(m_value[0]);
      sb1->setSingleStep(m_step);
      sb1->setDecimals(m_decimal);
      sb1->setPrefix(m_prefix);
      sb1->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb1, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZVec2Parameter::setValue1);
#else
      connect(sb1, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZVec2Parameter::setValue1);
#endif
      connect(this, &ZVec2Parameter::value1WillChange, sb1, &ZDoubleSpinBox::setValue);
      if (m_nameOfEachValue.at(0).isEmpty()) {
        lo->addWidget(sb1);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[0]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb1);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb1);
          lo->addLayout(hlo);
        }
      }
    }
    {
      auto sb2 = new ZDoubleSpinBox();
      sb2->setRange(m_min[1], m_max[1]);
      sb2->setValue(m_value[1]);
      sb2->setSingleStep(m_step);
      sb2->setDecimals(m_decimal);
      sb2->setPrefix(m_prefix);
      sb2->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb2, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZVec2Parameter::setValue2);
#else
      connect(sb2, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZVec2Parameter::setValue2);
#endif
      connect(this, &ZVec2Parameter::value2WillChange, sb2, &ZDoubleSpinBox::setValue);
      if (m_nameOfEachValue.at(1).isEmpty()) {
        lo->addWidget(sb2);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[1]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb2);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb2);
          lo->addLayout(hlo);
        }
      }
    }
  } else {
    {
      ZDoubleSpinBoxWithSlider* sbws1 = new ZDoubleSpinBoxWithSlider(m_value[0], m_min[0], m_max[0], m_step,
                                                                     m_decimal, m_tracking, m_prefix, m_suffix, parent);
      connect(sbws1, &ZDoubleSpinBoxWithSlider::valueChanged, this, &ZVec2Parameter::setValue1);
      connect(this, &ZVec2Parameter::value1WillChange, sbws1, &ZDoubleSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(0).isEmpty()) {
        lo->addWidget(sbws1);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[0]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws1);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws1);
          lo->addLayout(hlo);
        }
      }
    }
    {
      ZDoubleSpinBoxWithSlider* sbws2 = new ZDoubleSpinBoxWithSlider(m_value[1], m_min[1], m_max[1], m_step,
                                                                     m_decimal, m_tracking, m_prefix, m_suffix, parent);
      connect(sbws2, &ZDoubleSpinBoxWithSlider::valueChanged, this, &ZVec2Parameter::setValue2);
      connect(this, &ZVec2Parameter::value2WillChange, sbws2, &ZDoubleSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(1).isEmpty()) {
        lo->addWidget(sbws2);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[1]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws2);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws2);
          lo->addLayout(hlo);
        }
      }
    }
  }

  w->setLayout(lo);
  return w;
}

//---------------------------------------------------------------------------------------------------------------

ZVec3Parameter::ZVec3Parameter(const QString& name, QObject* parent)
  : ZNumericVectorParameter<glm::vec3>(name, glm::vec3(0.0), glm::vec3(std::numeric_limits<float>::lowest()),
                                       glm::vec3(std::numeric_limits<float>::max()), parent)
{
  addStyle("SPINBOX");
  addStyle("COLOR");
}

ZVec3Parameter::ZVec3Parameter(const QString& name, glm::vec3 value, glm::vec3 min, glm::vec3 max, QObject* parent)
  : ZNumericVectorParameter<glm::vec3>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
  addStyle("COLOR");
}

void ZVec3Parameter::setValue1(double v)
{
  set(glm::vec3(static_cast<float>(v), m_value[1], m_value[2]));
}

void ZVec3Parameter::setValue2(double v)
{
  set(glm::vec3(m_value[0], static_cast<float>(v), m_value[2]));
}

void ZVec3Parameter::setValue3(double v)
{
  set(glm::vec3(m_value[0], m_value[1], static_cast<float>(v)));
}

void ZVec3Parameter::beforeChange(glm::vec3& value)
{
  if (value[0] != m_value[0])
    emit value1WillChange(value[0]);
  if (value[1] != m_value[1])
    emit value2WillChange(value[1]);
  if (value[2] != m_value[2])
    emit value3WillChange(value[2]);
}

QWidget* ZVec3Parameter::actualCreateWidget(QWidget* parent)
{
  if (m_style == "COLOR") {
    return new ZClickableColorLabel(this, parent);
  }

  QWidget* w;
  if (m_widgetOrientation == Qt::Horizontal)
    w = new QWidget(parent);
  else
    w = new QGroupBox(m_groupBoxName, parent);
  QBoxLayout* lo;
  if (m_widgetOrientation == Qt::Horizontal)
    lo = new QHBoxLayout();
  else
    lo = new QVBoxLayout();

  if (m_style == "SPINBOX") {
    {
      auto sb1 = new ZDoubleSpinBox();
      sb1->setRange(m_min[0], m_max[0]);
      sb1->setValue(m_value[0]);
      sb1->setSingleStep(m_step);
      sb1->setDecimals(m_decimal);
      sb1->setPrefix(m_prefix);
      sb1->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb1, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZVec3Parameter::setValue1);
#else
      connect(sb1, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZVec3Parameter::setValue1);
#endif
      connect(this, &ZVec3Parameter::value1WillChange, sb1, &ZDoubleSpinBox::setValue);
      if (m_nameOfEachValue.at(0).isEmpty()) {
        lo->addWidget(sb1);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[0]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb1);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb1);
          lo->addLayout(hlo);
        }
      }
    }
    {
      auto sb2 = new ZDoubleSpinBox();
      sb2->setRange(m_min[1], m_max[1]);
      sb2->setValue(m_value[1]);
      sb2->setSingleStep(m_step);
      sb2->setDecimals(m_decimal);
      sb2->setPrefix(m_prefix);
      sb2->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb2, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZVec3Parameter::setValue2);
#else
      connect(sb2, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZVec3Parameter::setValue2);
#endif
      connect(this, &ZVec3Parameter::value2WillChange, sb2, &ZDoubleSpinBox::setValue);
      if (m_nameOfEachValue.at(1).isEmpty()) {
        lo->addWidget(sb2);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[1]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb2);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb2);
          lo->addLayout(hlo);
        }
      }
    }
    {
      auto sb3 = new ZDoubleSpinBox();
      sb3->setRange(m_min[2], m_max[2]);
      sb3->setValue(m_value[2]);
      sb3->setSingleStep(m_step);
      sb3->setDecimals(m_decimal);
      sb3->setPrefix(m_prefix);
      sb3->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb3, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZVec3Parameter::setValue3);
#else
      connect(sb3, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZVec3Parameter::setValue3);
#endif
      connect(this, &ZVec3Parameter::value3WillChange, sb3, &ZDoubleSpinBox::setValue);
      if (m_nameOfEachValue.at(2).isEmpty()) {
        lo->addWidget(sb3);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[2]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb3);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb3);
          lo->addLayout(hlo);
        }
      }
    }
  } else {
    {
      ZDoubleSpinBoxWithSlider* sbws1 = new ZDoubleSpinBoxWithSlider(m_value[0], m_min[0], m_max[0], m_step,
                                                                     m_decimal, m_tracking, m_prefix, m_suffix, parent);
      connect(sbws1, &ZDoubleSpinBoxWithSlider::valueChanged, this, &ZVec3Parameter::setValue1);
      connect(this, &ZVec3Parameter::value1WillChange, sbws1, &ZDoubleSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(0).isEmpty()) {
        lo->addWidget(sbws1);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[0]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws1);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws1);
          lo->addLayout(hlo);
        }
      }
    }
    {
      ZDoubleSpinBoxWithSlider* sbws2 = new ZDoubleSpinBoxWithSlider(m_value[1], m_min[1], m_max[1], m_step,
                                                                     m_decimal, m_tracking, m_prefix, m_suffix, parent);
      connect(sbws2, &ZDoubleSpinBoxWithSlider::valueChanged, this, &ZVec3Parameter::setValue2);
      connect(this, &ZVec3Parameter::value2WillChange, sbws2, &ZDoubleSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(1).isEmpty()) {
        lo->addWidget(sbws2);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[1]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws2);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws2);
          lo->addLayout(hlo);
        }
      }
    }
    {
      ZDoubleSpinBoxWithSlider* sbws3 = new ZDoubleSpinBoxWithSlider(m_value[2], m_min[2], m_max[2], m_step,
                                                                     m_decimal, m_tracking, m_prefix, m_suffix, parent);
      connect(sbws3, &ZDoubleSpinBoxWithSlider::valueChanged, this, &ZVec3Parameter::setValue3);
      connect(this, &ZVec3Parameter::value3WillChange, sbws3, &ZDoubleSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(2).isEmpty()) {
        lo->addWidget(sbws3);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[2]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws3);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws3);
          lo->addLayout(hlo);
        }
      }
    }
  }

  w->setLayout(lo);
  return w;
}

//---------------------------------------------------------------------------------------------------------------

ZVec4Parameter::ZVec4Parameter(const QString& name, QObject* parent)
  : ZNumericVectorParameter<glm::vec4>(name, glm::vec4(0.0), glm::vec4(std::numeric_limits<float>::lowest()),
                                       glm::vec4(std::numeric_limits<float>::max()), parent)
{
  addStyle("SPINBOX");
  addStyle("COLOR");
}

ZVec4Parameter::ZVec4Parameter(const QString& name, glm::vec4 value, glm::vec4 min, glm::vec4 max, QObject* parent)
  : ZNumericVectorParameter<glm::vec4>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
  addStyle("COLOR");
}

void ZVec4Parameter::setValue1(double v)
{
  set(glm::vec4(static_cast<float>(v), m_value[1], m_value[2], m_value[3]));
}

void ZVec4Parameter::setValue2(double v)
{
  set(glm::vec4(m_value[0], static_cast<float>(v), m_value[2], m_value[3]));
}

void ZVec4Parameter::setValue3(double v)
{
  set(glm::vec4(m_value[0], m_value[1], static_cast<float>(v), m_value[3]));
}

void ZVec4Parameter::setValue4(double v)
{
  set(glm::vec4(m_value[0], m_value[1], m_value[2], static_cast<float>(v)));
}

void ZVec4Parameter::beforeChange(glm::vec4& value)
{
  if (value[0] != m_value[0])
    emit value1WillChange(value[0]);
  if (value[1] != m_value[1])
    emit value2WillChange(value[1]);
  if (value[2] != m_value[2])
    emit value3WillChange(value[2]);
  if (value[3] != m_value[3])
    emit value4WillChange(value[3]);
}

QWidget* ZVec4Parameter::actualCreateWidget(QWidget* parent)
{
  if (m_style == "COLOR") {
    ZClickableColorLabel *widget = new ZClickableColorLabel(this, parent);
//    widget->setSyncMutex(m_syncMutex);
    return widget;
//    return new ZClickableColorLabel(this, parent);
  }

  QWidget* w;
  if (m_widgetOrientation == Qt::Horizontal)
    w = new QWidget(parent);
  else
    w = new QGroupBox(m_groupBoxName, parent);
  QBoxLayout* lo;
  if (m_widgetOrientation == Qt::Horizontal)
    lo = new QHBoxLayout();
  else
    lo = new QVBoxLayout();

  if (m_style == "SPINBOX") {
    {
      auto sb1 = new ZDoubleSpinBox();
      sb1->setRange(m_min[0], m_max[0]);
      sb1->setValue(m_value[0]);
      sb1->setSingleStep(m_step);
      sb1->setDecimals(m_decimal);
      sb1->setPrefix(m_prefix);
      sb1->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb1, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZVec4Parameter::setValue1);
#else
      connect(sb1, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZVec4Parameter::setValue1);
#endif
      connect(this, &ZVec4Parameter::value1WillChange, sb1, &ZDoubleSpinBox::setValue);
      if (m_nameOfEachValue.at(0).isEmpty()) {
        lo->addWidget(sb1);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[0]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb1);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb1);
          lo->addLayout(hlo);
        }
      }
    }
    {
      auto sb2 = new ZDoubleSpinBox();
      sb2->setRange(m_min[1], m_max[1]);
      sb2->setValue(m_value[1]);
      sb2->setSingleStep(m_step);
      sb2->setDecimals(m_decimal);
      sb2->setPrefix(m_prefix);
      sb2->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb2, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZVec4Parameter::setValue2);
#else
      connect(sb2, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZVec4Parameter::setValue2);
#endif
      connect(this, &ZVec4Parameter::value2WillChange, sb2, &ZDoubleSpinBox::setValue);
      if (m_nameOfEachValue.at(1).isEmpty()) {
        lo->addWidget(sb2);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[1]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb2);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb2);
          lo->addLayout(hlo);
        }
      }
    }
    {
      auto sb3 = new ZDoubleSpinBox();
      sb3->setRange(m_min[2], m_max[2]);
      sb3->setValue(m_value[2]);
      sb3->setSingleStep(m_step);
      sb3->setDecimals(m_decimal);
      sb3->setPrefix(m_prefix);
      sb3->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb3, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZVec4Parameter::setValue3);
#else
      connect(sb3, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZVec4Parameter::setValue3);
#endif
      connect(this, &ZVec4Parameter::value3WillChange, sb3, &ZDoubleSpinBox::setValue);
      if (m_nameOfEachValue.at(2).isEmpty()) {
        lo->addWidget(sb3);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[2]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb3);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb3);
          lo->addLayout(hlo);
        }
      }
    }
    {
      auto sb4 = new ZDoubleSpinBox();
      sb4->setRange(m_min[3], m_max[3]);
      sb4->setValue(m_value[3]);
      sb4->setSingleStep(m_step);
      sb4->setDecimals(m_decimal);
      sb4->setPrefix(m_prefix);
      sb4->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb4, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZVec4Parameter::setValue4);
#else
      connect(sb4, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZVec4Parameter::setValue4);
#endif
      connect(this, &ZVec4Parameter::value4WillChange, sb4, &ZDoubleSpinBox::setValue);
      if (m_nameOfEachValue.at(3).isEmpty()) {
        lo->addWidget(sb4);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[3]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb4);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb4);
          lo->addLayout(hlo);
        }
      }
    }
  } else {
    {
      ZDoubleSpinBoxWithSlider* sbws1 = new ZDoubleSpinBoxWithSlider(m_value[0], m_min[0], m_max[0], m_step,
                                                                     m_decimal, m_tracking, m_prefix, m_suffix, parent);
      connect(sbws1, &ZDoubleSpinBoxWithSlider::valueChanged, this, &ZVec4Parameter::setValue1);
      connect(this, &ZVec4Parameter::value1WillChange, sbws1, &ZDoubleSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(0).isEmpty()) {
        lo->addWidget(sbws1);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[0]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws1);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws1);
          lo->addLayout(hlo);
        }
      }
    }
    {
      ZDoubleSpinBoxWithSlider* sbws2 = new ZDoubleSpinBoxWithSlider(m_value[1], m_min[1], m_max[1], m_step,
                                                                     m_decimal, m_tracking, m_prefix, m_suffix, parent);
      connect(sbws2, &ZDoubleSpinBoxWithSlider::valueChanged, this, &ZVec4Parameter::setValue2);
      connect(this, &ZVec4Parameter::value2WillChange, sbws2, &ZDoubleSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(1).isEmpty()) {
        lo->addWidget(sbws2);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[1]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws2);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws2);
          lo->addLayout(hlo);
        }
      }
    }
    {
      ZDoubleSpinBoxWithSlider* sbws3 = new ZDoubleSpinBoxWithSlider(m_value[2], m_min[2], m_max[2], m_step,
                                                                     m_decimal, m_tracking, m_prefix, m_suffix, parent);
      connect(sbws3, &ZDoubleSpinBoxWithSlider::valueChanged, this, &ZVec4Parameter::setValue3);
      connect(this, &ZVec4Parameter::value3WillChange, sbws3, &ZDoubleSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(2).isEmpty()) {
        lo->addWidget(sbws3);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[2]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws3);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws3);
          lo->addLayout(hlo);
        }
      }
    }
    {
      ZDoubleSpinBoxWithSlider* sbws4 = new ZDoubleSpinBoxWithSlider(m_value[3], m_min[3], m_max[3], m_step,
                                                                     m_decimal, m_tracking, m_prefix, m_suffix, parent);
      connect(sbws4, &ZDoubleSpinBoxWithSlider::valueChanged, this, &ZVec4Parameter::setValue4);
      connect(this, &ZVec4Parameter::value4WillChange, sbws4, &ZDoubleSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(3).isEmpty()) {
        lo->addWidget(sbws4);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[3]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws4);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws4);
          lo->addLayout(hlo);
        }
      }
    }
  }

  w->setLayout(lo);
  return w;
}


//---------------------------------------------------------------------------------------------------------------

ZDVec2Parameter::ZDVec2Parameter(const QString& name, QObject* parent)
  : ZNumericVectorParameter<glm::dvec2>(name, glm::dvec2(0.0), glm::dvec2(std::numeric_limits<double>::lowest()),
                                        glm::dvec2(std::numeric_limits<double>::max()), parent)
{
  addStyle("SPINBOX");
}

ZDVec2Parameter::ZDVec2Parameter(const QString& name, glm::dvec2 value, glm::dvec2 min, glm::dvec2 max, QObject* parent)
  : ZNumericVectorParameter<glm::dvec2>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
}

void ZDVec2Parameter::setValue1(double v)
{
  set(glm::dvec2(v, m_value[1]));
}

void ZDVec2Parameter::setValue2(double v)
{
  set(glm::dvec2(m_value[0], v));
}

void ZDVec2Parameter::beforeChange(glm::dvec2& value)
{
  if (value[0] != m_value[0])
    emit value1WillChange(value[0]);
  if (value[1] != m_value[1])
    emit value2WillChange(value[1]);
}

QWidget* ZDVec2Parameter::actualCreateWidget(QWidget* parent)
{
  QWidget* w;
  if (m_widgetOrientation == Qt::Horizontal)
    w = new QWidget(parent);
  else
    w = new QGroupBox(m_groupBoxName, parent);
  QBoxLayout* lo;
  if (m_widgetOrientation == Qt::Horizontal)
    lo = new QHBoxLayout();
  else
    lo = new QVBoxLayout();

  if (m_style == "SPINBOX") {
    {
      auto sb1 = new ZDoubleSpinBox();
      sb1->setRange(m_min[0], m_max[0]);
      sb1->setValue(m_value[0]);
      sb1->setSingleStep(m_step);
      sb1->setDecimals(m_decimal);
      sb1->setPrefix(m_prefix);
      sb1->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb1, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZDVec2Parameter::setValue1);
#else
      connect(sb1, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZDVec2Parameter::setValue1);
#endif

      connect(this, &ZDVec2Parameter::value1WillChange, sb1, &ZDoubleSpinBox::setValue);
      if (m_nameOfEachValue.at(0).isEmpty()) {
        lo->addWidget(sb1);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[0]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb1);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb1);
          lo->addLayout(hlo);
        }
      }
    }
    {
      auto sb2 = new ZDoubleSpinBox();
      sb2->setRange(m_min[1], m_max[1]);
      sb2->setValue(m_value[1]);
      sb2->setSingleStep(m_step);
      sb2->setDecimals(m_decimal);
      sb2->setPrefix(m_prefix);
      sb2->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb2, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZDVec2Parameter::setValue2);
#else
      connect(sb2, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZDVec2Parameter::setValue2);
#endif

      connect(this, &ZDVec2Parameter::value2WillChange, sb2, &ZDoubleSpinBox::setValue);
      if (m_nameOfEachValue.at(1).isEmpty()) {
        lo->addWidget(sb2);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[1]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb2);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb2);
          lo->addLayout(hlo);
        }
      }
    }
  } else {
    {
      ZDoubleSpinBoxWithSlider* sbws1 = new ZDoubleSpinBoxWithSlider(m_value[0], m_min[0], m_max[0], m_step,
                                                                     m_decimal, m_tracking, m_prefix, m_suffix, parent);
      connect(sbws1, &ZDoubleSpinBoxWithSlider::valueChanged, this, &ZDVec2Parameter::setValue1);
      connect(this, &ZDVec2Parameter::value1WillChange, sbws1, &ZDoubleSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(0).isEmpty()) {
        lo->addWidget(sbws1);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[0]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws1);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws1);
          lo->addLayout(hlo);
        }
      }
    }
    {
      ZDoubleSpinBoxWithSlider* sbws2 = new ZDoubleSpinBoxWithSlider(m_value[1], m_min[1], m_max[1], m_step,
                                                                     m_decimal, m_tracking, m_prefix, m_suffix, parent);
      connect(sbws2, &ZDoubleSpinBoxWithSlider::valueChanged, this, &ZDVec2Parameter::setValue2);
      connect(this, &ZDVec2Parameter::value2WillChange, sbws2, &ZDoubleSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(1).isEmpty()) {
        lo->addWidget(sbws2);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[1]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws2);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws2);
          lo->addLayout(hlo);
        }
      }
    }
  }

  w->setLayout(lo);
  return w;
}

//---------------------------------------------------------------------------------------------------------------

ZDVec3Parameter::ZDVec3Parameter(const QString& name, QObject* parent)
  : ZNumericVectorParameter<glm::dvec3>(name, glm::dvec3(0.0), glm::dvec3(std::numeric_limits<double>::lowest()),
                                        glm::dvec3(std::numeric_limits<double>::max()), parent)
{
  addStyle("SPINBOX");
  addStyle("COLOR");
}

ZDVec3Parameter::ZDVec3Parameter(const QString& name, glm::dvec3 value, glm::dvec3 min, glm::dvec3 max, QObject* parent)
  : ZNumericVectorParameter<glm::dvec3>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
  addStyle("COLOR");
}

void ZDVec3Parameter::setValue1(double v)
{
  set(glm::dvec3(v, m_value[1], m_value[2]));
}

void ZDVec3Parameter::setValue2(double v)
{
  set(glm::dvec3(m_value[0], v, m_value[2]));
}

void ZDVec3Parameter::setValue3(double v)
{
  set(glm::dvec3(m_value[0], m_value[1], v));
}

void ZDVec3Parameter::beforeChange(glm::dvec3& value)
{
  if (value[0] != m_value[0])
    emit value1WillChange(value[0]);
  if (value[1] != m_value[1])
    emit value2WillChange(value[1]);
  if (value[2] != m_value[2])
    emit value3WillChange(value[2]);
}

QWidget* ZDVec3Parameter::actualCreateWidget(QWidget* parent)
{
  if (m_style == "COLOR") {
    return new ZClickableColorLabel(this, parent);
  }

  QWidget* w;
  if (m_widgetOrientation == Qt::Horizontal)
    w = new QWidget(parent);
  else
    w = new QGroupBox(m_groupBoxName, parent);
  QBoxLayout* lo;
  if (m_widgetOrientation == Qt::Horizontal)
    lo = new QHBoxLayout();
  else
    lo = new QVBoxLayout();

  if (m_style == "SPINBOX") {
    {
      auto sb1 = new ZDoubleSpinBox();
      sb1->setRange(m_min[0], m_max[0]);
      sb1->setValue(m_value[0]);
      sb1->setSingleStep(m_step);
      sb1->setDecimals(m_decimal);
      sb1->setPrefix(m_prefix);
      sb1->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb1, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZDVec3Parameter::setValue1);
#else
      connect(sb1, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZDVec3Parameter::setValue1);
#endif

      connect(this, &ZDVec3Parameter::value1WillChange, sb1, &ZDoubleSpinBox::setValue);
      if (m_nameOfEachValue.at(0).isEmpty()) {
        lo->addWidget(sb1);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[0]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb1);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb1);
          lo->addLayout(hlo);
        }
      }
    }
    {
      auto sb2 = new ZDoubleSpinBox();
      sb2->setRange(m_min[1], m_max[1]);
      sb2->setValue(m_value[1]);
      sb2->setSingleStep(m_step);
      sb2->setDecimals(m_decimal);
      sb2->setPrefix(m_prefix);
      sb2->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb2, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZDVec3Parameter::setValue2);
#else
      connect(sb2, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZDVec3Parameter::setValue2);
#endif

      connect(this, &ZDVec3Parameter::value2WillChange, sb2, &ZDoubleSpinBox::setValue);
      if (m_nameOfEachValue.at(1).isEmpty()) {
        lo->addWidget(sb2);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[1]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb2);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb2);
          lo->addLayout(hlo);
        }
      }
    }
    {
      auto sb3 = new ZDoubleSpinBox();
      sb3->setRange(m_min[2], m_max[2]);
      sb3->setValue(m_value[2]);
      sb3->setSingleStep(m_step);
      sb3->setDecimals(m_decimal);
      sb3->setPrefix(m_prefix);
      sb3->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb3, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZDVec3Parameter::setValue3);
#else
      connect(sb3, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZDVec3Parameter::setValue3);
#endif

      connect(this, &ZDVec3Parameter::value3WillChange, sb3, &ZDoubleSpinBox::setValue);
      if (m_nameOfEachValue.at(2).isEmpty()) {
        lo->addWidget(sb3);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[2]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb3);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb3);
          lo->addLayout(hlo);
        }
      }
    }
  } else {
    {
      ZDoubleSpinBoxWithSlider* sbws1 = new ZDoubleSpinBoxWithSlider(m_value[0], m_min[0], m_max[0], m_step,
                                                                     m_decimal, m_tracking, m_prefix, m_suffix, parent);
      connect(sbws1, &ZDoubleSpinBoxWithSlider::valueChanged, this, &ZDVec3Parameter::setValue1);
      connect(this, &ZDVec3Parameter::value1WillChange, sbws1, &ZDoubleSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(0).isEmpty()) {
        lo->addWidget(sbws1);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[0]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws1);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws1);
          lo->addLayout(hlo);
        }
      }
    }
    {
      ZDoubleSpinBoxWithSlider* sbws2 = new ZDoubleSpinBoxWithSlider(m_value[1], m_min[1], m_max[1], m_step,
                                                                     m_decimal, m_tracking, m_prefix, m_suffix, parent);
      connect(sbws2, &ZDoubleSpinBoxWithSlider::valueChanged, this, &ZDVec3Parameter::setValue2);
      connect(this, &ZDVec3Parameter::value2WillChange, sbws2, &ZDoubleSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(1).isEmpty()) {
        lo->addWidget(sbws2);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[1]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws2);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws2);
          lo->addLayout(hlo);
        }
      }
    }
    {
      ZDoubleSpinBoxWithSlider* sbws3 = new ZDoubleSpinBoxWithSlider(m_value[2], m_min[2], m_max[2], m_step,
                                                                     m_decimal, m_tracking, m_prefix, m_suffix, parent);
      connect(sbws3, &ZDoubleSpinBoxWithSlider::valueChanged, this, &ZDVec3Parameter::setValue3);
      connect(this, &ZDVec3Parameter::value3WillChange, sbws3, &ZDoubleSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(2).isEmpty()) {
        lo->addWidget(sbws3);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[2]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws3);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws3);
          lo->addLayout(hlo);
        }
      }
    }
  }

  w->setLayout(lo);
  return w;
}

//---------------------------------------------------------------------------------------------------------------

ZDVec4Parameter::ZDVec4Parameter(const QString& name, QObject* parent)
  : ZNumericVectorParameter<glm::dvec4>(name, glm::dvec4(0.0), glm::dvec4(std::numeric_limits<double>::lowest()),
                                        glm::dvec4(std::numeric_limits<double>::max()), parent)
{
  addStyle("SPINBOX");
  addStyle("COLOR");
}

ZDVec4Parameter::ZDVec4Parameter(const QString& name, glm::dvec4 value, glm::dvec4 min, glm::dvec4 max, QObject* parent)
  : ZNumericVectorParameter<glm::dvec4>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
  addStyle("COLOR");
}

void ZDVec4Parameter::setValue1(double v)
{
  set(glm::dvec4(v, m_value[1], m_value[2], m_value[3]));
}

void ZDVec4Parameter::setValue2(double v)
{
  set(glm::dvec4(m_value[0], v, m_value[2], m_value[3]));
}

void ZDVec4Parameter::setValue3(double v)
{
  set(glm::dvec4(m_value[0], m_value[1], v, m_value[3]));
}

void ZDVec4Parameter::setValue4(double v)
{
  set(glm::dvec4(m_value[0], m_value[1], m_value[2], v));
}

void ZDVec4Parameter::beforeChange(glm::dvec4& value)
{
  if (value[0] != m_value[0])
    emit value1WillChange(value[0]);
  if (value[1] != m_value[1])
    emit value2WillChange(value[1]);
  if (value[2] != m_value[2])
    emit value3WillChange(value[2]);
  if (value[3] != m_value[3])
    emit value4WillChange(value[3]);
}

QWidget* ZDVec4Parameter::actualCreateWidget(QWidget* parent)
{
  if (m_style == "COLOR") {
    return new ZClickableColorLabel(this, parent);
  }

  QWidget* w;
  if (m_widgetOrientation == Qt::Horizontal)
    w = new QWidget(parent);
  else
    w = new QGroupBox(m_groupBoxName, parent);
  QBoxLayout* lo;
  if (m_widgetOrientation == Qt::Horizontal)
    lo = new QHBoxLayout();
  else
    lo = new QVBoxLayout();

  if (m_style == "SPINBOX") {
    {
      auto sb1 = new ZDoubleSpinBox();
      sb1->setRange(m_min[0], m_max[0]);
      sb1->setValue(m_value[0]);
      sb1->setSingleStep(m_step);
      sb1->setDecimals(m_decimal);
      sb1->setPrefix(m_prefix);
      sb1->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb1, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZDVec4Parameter::setValue1);
#else
      connect(sb1, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZDVec4Parameter::setValue1);
#endif

      connect(this, &ZDVec4Parameter::value1WillChange, sb1, &ZDoubleSpinBox::setValue);
      if (m_nameOfEachValue.at(0).isEmpty()) {
        lo->addWidget(sb1);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[0]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb1);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb1);
          lo->addLayout(hlo);
        }
      }
    }
    {
      auto sb2 = new ZDoubleSpinBox();
      sb2->setRange(m_min[1], m_max[1]);
      sb2->setValue(m_value[1]);
      sb2->setSingleStep(m_step);
      sb2->setDecimals(m_decimal);
      sb2->setPrefix(m_prefix);
      sb2->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb2, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZDVec4Parameter::setValue2);
#else
      connect(sb2, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZDVec4Parameter::setValue2);
#endif

      connect(this, &ZDVec4Parameter::value2WillChange, sb2, &ZDoubleSpinBox::setValue);
      if (m_nameOfEachValue.at(1).isEmpty()) {
        lo->addWidget(sb2);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[1]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb2);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb2);
          lo->addLayout(hlo);
        }
      }
    }
    {
      auto sb3 = new ZDoubleSpinBox();
      sb3->setRange(m_min[2], m_max[2]);
      sb3->setValue(m_value[2]);
      sb3->setSingleStep(m_step);
      sb3->setDecimals(m_decimal);
      sb3->setPrefix(m_prefix);
      sb3->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb3, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZDVec4Parameter::setValue3);
#else
      connect(sb3, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZDVec4Parameter::setValue3);
#endif

      connect(this, &ZDVec4Parameter::value3WillChange, sb3, &ZDoubleSpinBox::setValue);
      if (m_nameOfEachValue.at(2).isEmpty()) {
        lo->addWidget(sb3);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[2]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb3);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb3);
          lo->addLayout(hlo);
        }
      }
    }
    {
      auto sb4 = new ZDoubleSpinBox();
      sb4->setRange(m_min[3], m_max[3]);
      sb4->setValue(m_value[3]);
      sb4->setSingleStep(m_step);
      sb4->setDecimals(m_decimal);
      sb4->setPrefix(m_prefix);
      sb4->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb4, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZDVec4Parameter::setValue4);
#else
      connect(sb4, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZDVec4Parameter::setValue4);
#endif

      connect(this, &ZDVec4Parameter::value4WillChange, sb4, &ZDoubleSpinBox::setValue);
      if (m_nameOfEachValue.at(3).isEmpty()) {
        lo->addWidget(sb4);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[3]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb4);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb4);
          lo->addLayout(hlo);
        }
      }
    }
  } else {
    {
      ZDoubleSpinBoxWithSlider* sbws1 = new ZDoubleSpinBoxWithSlider(m_value[0], m_min[0], m_max[0], m_step,
                                                                     m_decimal, m_tracking, m_prefix, m_suffix, parent);
      connect(sbws1, &ZDoubleSpinBoxWithSlider::valueChanged, this, &ZDVec4Parameter::setValue1);
      connect(this, &ZDVec4Parameter::value1WillChange, sbws1, &ZDoubleSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(0).isEmpty()) {
        lo->addWidget(sbws1);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[0]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws1);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws1);
          lo->addLayout(hlo);
        }
      }
    }
    {
      ZDoubleSpinBoxWithSlider* sbws2 = new ZDoubleSpinBoxWithSlider(m_value[1], m_min[1], m_max[1], m_step,
                                                                     m_decimal, m_tracking, m_prefix, m_suffix, parent);
      connect(sbws2, &ZDoubleSpinBoxWithSlider::valueChanged, this, &ZDVec4Parameter::setValue2);
      connect(this, &ZDVec4Parameter::value2WillChange, sbws2, &ZDoubleSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(1).isEmpty()) {
        lo->addWidget(sbws2);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[1]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws2);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws2);
          lo->addLayout(hlo);
        }
      }
    }
    {
      ZDoubleSpinBoxWithSlider* sbws3 = new ZDoubleSpinBoxWithSlider(m_value[2], m_min[2], m_max[2], m_step,
                                                                     m_decimal, m_tracking, m_prefix, m_suffix, parent);
      connect(sbws3, &ZDoubleSpinBoxWithSlider::valueChanged, this, &ZDVec4Parameter::setValue3);
      connect(this, &ZDVec4Parameter::value3WillChange, sbws3, &ZDoubleSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(2).isEmpty()) {
        lo->addWidget(sbws3);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[2]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws3);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws3);
          lo->addLayout(hlo);
        }
      }
    }
    {
      ZDoubleSpinBoxWithSlider* sbws4 = new ZDoubleSpinBoxWithSlider(m_value[3], m_min[3], m_max[3], m_step,
                                                                     m_decimal, m_tracking, m_prefix, m_suffix, parent);
      connect(sbws4, &ZDoubleSpinBoxWithSlider::valueChanged, this, &ZDVec4Parameter::setValue4);
      connect(this, &ZDVec4Parameter::value4WillChange, sbws4, &ZDoubleSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(3).isEmpty()) {
        lo->addWidget(sbws4);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[3]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws4);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws4);
          lo->addLayout(hlo);
        }
      }
    }
  }

  w->setLayout(lo);
  return w;
}

//-------------------------------------------------------------------------------------------------

ZIVec2Parameter::ZIVec2Parameter(const QString& name, QObject* parent)
  : ZNumericVectorParameter<glm::ivec2>(name, glm::ivec2(0), glm::ivec2(std::numeric_limits<int>::min()),
                                        glm::ivec2(std::numeric_limits<int>::max()), parent)
{
  addStyle("SPINBOX");
}

ZIVec2Parameter::ZIVec2Parameter(const QString& name, glm::ivec2 value, glm::ivec2 min, glm::ivec2 max, QObject* parent)
  : ZNumericVectorParameter<glm::ivec2>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
}

void ZIVec2Parameter::setValue1(int v)
{
  set(glm::ivec2(v, m_value[1]));
}

void ZIVec2Parameter::setValue2(int v)
{
  set(glm::ivec2(m_value[0], v));
}

void ZIVec2Parameter::beforeChange(glm::ivec2& value)
{
  if (value[0] != m_value[0])
    emit value1WillChange(value[0]);
  if (value[1] != m_value[1])
    emit value2WillChange(value[1]);
}

QWidget* ZIVec2Parameter::actualCreateWidget(QWidget* parent)
{
  QWidget* w;
  if (m_widgetOrientation == Qt::Horizontal)
    w = new QWidget(parent);
  else
    w = new QGroupBox(m_groupBoxName, parent);
  QBoxLayout* lo;
  if (m_widgetOrientation == Qt::Horizontal)
    lo = new QHBoxLayout();
  else
    lo = new QVBoxLayout();

  if (m_style == "SPINBOX") {
    {
      auto sb1 = new ZSpinBox();
      sb1->setRange(m_min[0], m_max[0]);
      sb1->setValue(m_value[0]);
      sb1->setSingleStep(m_step);
      sb1->setPrefix(m_prefix);
      sb1->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb1, qOverload<int>(&ZSpinBox::valueChanged), this, &ZIVec2Parameter::setValue1);
#else
      connect(sb1, QOverload<int>::of(&ZSpinBox::valueChanged), this, &ZIVec2Parameter::setValue1);
#endif

      connect(this, &ZIVec2Parameter::value1WillChange, sb1, &ZSpinBox::setValue);
      if (m_nameOfEachValue.at(0).isEmpty()) {
        lo->addWidget(sb1);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[0]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb1);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb1);
          lo->addLayout(hlo);
        }
      }
    }
    {
      auto sb2 = new ZSpinBox();
      sb2->setRange(m_min[1], m_max[1]);
      sb2->setValue(m_value[1]);
      sb2->setSingleStep(m_step);
      sb2->setPrefix(m_prefix);
      sb2->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb2, qOverload<int>(&ZSpinBox::valueChanged), this, &ZIVec2Parameter::setValue2);
#else
      connect(sb2, QOverload<int>::of(&ZSpinBox::valueChanged), this, &ZIVec2Parameter::setValue2);
#endif

      connect(this, &ZIVec2Parameter::value2WillChange, sb2, &ZSpinBox::setValue);
      if (m_nameOfEachValue.at(1).isEmpty()) {
        lo->addWidget(sb2);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[1]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb2);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb2);
          lo->addLayout(hlo);
        }
      }
    }
  } else {
    {
      ZSpinBoxWithSlider* sbws1 = new ZSpinBoxWithSlider(m_value[0], m_min[0], m_max[0], m_step,
                                                         m_tracking, m_prefix, m_suffix, parent);
      connect(sbws1, &ZSpinBoxWithSlider::valueChanged, this, &ZIVec2Parameter::setValue1);
      connect(this, &ZIVec2Parameter::value1WillChange, sbws1, &ZSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(0).isEmpty()) {
        lo->addWidget(sbws1);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[0]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws1);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws1);
          lo->addLayout(hlo);
        }
      }
    }
    {
      ZSpinBoxWithSlider* sbws2 = new ZSpinBoxWithSlider(m_value[1], m_min[1], m_max[1], m_step,
                                                         m_tracking, m_prefix, m_suffix, parent);
      connect(sbws2, &ZSpinBoxWithSlider::valueChanged, this, &ZIVec2Parameter::setValue2);
      connect(this, &ZIVec2Parameter::value2WillChange, sbws2, &ZSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(1).isEmpty()) {
        lo->addWidget(sbws2);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[1]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws2);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws2);
          lo->addLayout(hlo);
        }
      }
    }
  }

  w->setLayout(lo);
  return w;
}

//-------------------------------------------------------------------------------------------------

ZIVec3Parameter::ZIVec3Parameter(const QString& name, QObject* parent)
  : ZNumericVectorParameter<glm::ivec3>(name, glm::ivec3(0), glm::ivec3(std::numeric_limits<int>::min()),
                                        glm::ivec3(std::numeric_limits<int>::max()), parent)
{
  addStyle("SPINBOX");
}

ZIVec3Parameter::ZIVec3Parameter(const QString& name, glm::ivec3 value, glm::ivec3 min, glm::ivec3 max, QObject* parent)
  : ZNumericVectorParameter<glm::ivec3>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
}

void ZIVec3Parameter::setValue1(int v)
{
  set(glm::ivec3(v, m_value[1], m_value[2]));
}

void ZIVec3Parameter::setValue2(int v)
{
  set(glm::ivec3(m_value[0], v, m_value[2]));
}

void ZIVec3Parameter::setValue3(int v)
{
  set(glm::ivec3(m_value[0], m_value[1], v));
}

void ZIVec3Parameter::beforeChange(glm::ivec3& value)
{
  if (value[0] != m_value[0])
    emit value1WillChange(value[0]);
  if (value[1] != m_value[1])
    emit value2WillChange(value[1]);
  if (value[2] != m_value[2])
    emit value3WillChange(value[2]);
}

QWidget* ZIVec3Parameter::actualCreateWidget(QWidget* parent)
{
  QWidget* w;
  if (m_widgetOrientation == Qt::Horizontal)
    w = new QWidget(parent);
  else
    w = new QGroupBox(m_groupBoxName, parent);
  QBoxLayout* lo;
  if (m_widgetOrientation == Qt::Horizontal)
    lo = new QHBoxLayout();
  else
    lo = new QVBoxLayout();

  if (m_style == "SPINBOX") {
    {
      auto sb1 = new ZSpinBox();
      sb1->setRange(m_min[0], m_max[0]);
      sb1->setValue(m_value[0]);
      sb1->setSingleStep(m_step);
      sb1->setPrefix(m_prefix);
      sb1->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb1, qOverload<int>(&ZSpinBox::valueChanged), this, &ZIVec3Parameter::setValue1);
#else
      connect(sb1, QOverload<int>::of(&ZSpinBox::valueChanged), this, &ZIVec3Parameter::setValue1);
#endif

      connect(this, &ZIVec3Parameter::value1WillChange, sb1, &ZSpinBox::setValue);
      if (m_nameOfEachValue.at(0).isEmpty()) {
        lo->addWidget(sb1);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[0]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb1);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb1);
          lo->addLayout(hlo);
        }
      }
    }
    {
      auto sb2 = new ZSpinBox();
      sb2->setRange(m_min[1], m_max[1]);
      sb2->setValue(m_value[1]);
      sb2->setSingleStep(m_step);
      sb2->setPrefix(m_prefix);
      sb2->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb2, qOverload<int>(&ZSpinBox::valueChanged), this, &ZIVec3Parameter::setValue2);
#else
      connect(sb2, QOverload<int>::of(&ZSpinBox::valueChanged), this, &ZIVec3Parameter::setValue2);
#endif

      connect(this, &ZIVec3Parameter::value2WillChange, sb2, &ZSpinBox::setValue);
      if (m_nameOfEachValue.at(1).isEmpty()) {
        lo->addWidget(sb2);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[1]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb2);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb2);
          lo->addLayout(hlo);
        }
      }
    }
    {
      auto sb3 = new ZSpinBox();
      sb3->setRange(m_min[2], m_max[2]);
      sb3->setValue(m_value[2]);
      sb3->setSingleStep(m_step);
      sb3->setPrefix(m_prefix);
      sb3->setSuffix(m_suffix);
#if __cplusplus > 201103L
      connect(sb3, qOverload<int>(&ZSpinBox::valueChanged), this, &ZIVec3Parameter::setValue3);
#else
      connect(sb3, QOverload<int>::of(&ZSpinBox::valueChanged), this, &ZIVec3Parameter::setValue3);
#endif

      connect(this, &ZIVec3Parameter::value3WillChange, sb3, &ZSpinBox::setValue);
      if (m_nameOfEachValue.at(2).isEmpty()) {
        lo->addWidget(sb3);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[2]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sb3);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sb3);
          lo->addLayout(hlo);
        }
      }
    }
  } else {
    {
      ZSpinBoxWithSlider* sbws1 = new ZSpinBoxWithSlider(m_value[0], m_min[0], m_max[0], m_step,
                                                         m_tracking, m_prefix, m_suffix, parent);
      connect(sbws1, &ZSpinBoxWithSlider::valueChanged, this, &ZIVec3Parameter::setValue1);
      connect(this, &ZIVec3Parameter::value1WillChange, sbws1, &ZSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(0).isEmpty()) {
        lo->addWidget(sbws1);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[0]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws1);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws1);
          lo->addLayout(hlo);
        }
      }
    }
    {
      ZSpinBoxWithSlider* sbws2 = new ZSpinBoxWithSlider(m_value[1], m_min[1], m_max[1], m_step,
                                                         m_tracking, m_prefix, m_suffix, parent);
      connect(sbws2, &ZSpinBoxWithSlider::valueChanged, this, &ZIVec3Parameter::setValue2);
      connect(this, &ZIVec3Parameter::value2WillChange, sbws2, &ZSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(1).isEmpty()) {
        lo->addWidget(sbws2);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[1]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws2);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws2);
          lo->addLayout(hlo);
        }
      }
    }
    {
      ZSpinBoxWithSlider* sbws3 = new ZSpinBoxWithSlider(m_value[2], m_min[2], m_max[2], m_step,
                                                         m_tracking, m_prefix, m_suffix, parent);
      connect(sbws3, &ZSpinBoxWithSlider::valueChanged, this, &ZIVec3Parameter::setValue3);
      connect(this, &ZIVec3Parameter::value3WillChange, sbws3, &ZSpinBoxWithSlider::setValue);
      if (m_nameOfEachValue.at(2).isEmpty()) {
        lo->addWidget(sbws3);
      } else {
        QLabel* lb = new QLabel(m_nameOfEachValue[2]);
        lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (m_widgetOrientation == Qt::Horizontal) {
          lo->addWidget(lb);
          lo->addWidget(sbws3);
        } else {
          auto hlo = new QHBoxLayout();
          hlo->addWidget(lb);
          hlo->addWidget(sbws3);
          lo->addLayout(hlo);
        }
      }
    }
  }

  w->setLayout(lo);
  return w;
}

//-------------------------------------------------------------------------------------------------

ZIntSpanParameter::ZIntSpanParameter(const QString& name, QObject* parent)
  : ZNumericSpanParameter<glm::ivec2>(name, glm::ivec2(0, 0), std::numeric_limits<int>::min(),
                                      std::numeric_limits<int>::max(), parent)
{
  addStyle("SPINBOX");
}

ZIntSpanParameter::ZIntSpanParameter(const QString& name, glm::ivec2 value, int min, int max, QObject* parent)
  : ZNumericSpanParameter<glm::ivec2>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
}

void ZIntSpanParameter::setLowerValue(int v)
{
  if (v <= m_value[1])
    set(glm::ivec2(v, m_value[1]));
}

void ZIntSpanParameter::setUpperValue(int v)
{
  if (v >= m_value[0])
    set(glm::ivec2(m_value[0], v));
}

void ZIntSpanParameter::beforeChange(glm::ivec2& value)
{
  if (value[0] != m_value[0])
    emit lowerValueWillChange(value[0]);
  if (value[1] != m_value[1])
    emit upperValueWillChange(value[1]);
}

QWidget* ZIntSpanParameter::actualCreateWidget(QWidget* parent)
{
  if (m_style == "SPINBOX") {
    QWidget* w;
    if (m_widgetOrientation == Qt::Horizontal)
      w = new QWidget(parent);
    else
      w = new QGroupBox(m_groupBoxName, parent);
    QBoxLayout* lo;
    if (m_widgetOrientation == Qt::Horizontal)
      lo = new QHBoxLayout();
    else
      lo = new QVBoxLayout();

    auto sb1 = new ZSpinBox();
    sb1->setRange(m_min, m_value[1]);
    sb1->setValue(m_value[0]);
    sb1->setSingleStep(m_step);
    sb1->setPrefix(m_prefix);
    sb1->setSuffix(m_suffix);
#if __cplusplus > 201103L
    connect(sb1, qOverload<int>(&ZSpinBox::valueChanged), this, &ZIntSpanParameter::setLowerValue);
#else
    connect(sb1, QOverload<int>::of(&ZSpinBox::valueChanged), this, &ZIntSpanParameter::setLowerValue);
#endif

    connect(this, &ZIntSpanParameter::lowerValueWillChange, sb1, &ZSpinBox::setValue);
    if (m_nameOfEachValue.at(0).isEmpty()) {
      lo->addWidget(sb1);
    } else {
      QLabel* lb = new QLabel(m_nameOfEachValue[0]);
      lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb1);
      } else {
        auto hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb1);
        lo->addLayout(hlo);
      }
    }
    auto sb2 = new ZSpinBox();
    sb2->setRange(m_value[0], m_max);
    sb2->setValue(m_value[1]);
    sb2->setSingleStep(m_step);
    sb2->setPrefix(m_prefix);
    sb2->setSuffix(m_suffix);
#if __cplusplus > 201103L
    connect(sb2, qOverload<int>(&ZSpinBox::valueChanged), this, &ZIntSpanParameter::setUpperValue);
#else
    connect(sb2, QOverload<int>::of(&ZSpinBox::valueChanged), this, &ZIntSpanParameter::setUpperValue);
#endif

    connect(this, &ZIntSpanParameter::upperValueWillChange, sb2, &ZSpinBox::setValue);
    if (m_nameOfEachValue.at(1).isEmpty()) {
      lo->addWidget(sb2);
    } else {
      QLabel* lb = new QLabel(m_nameOfEachValue[1]);
      lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb2);
      } else {
        auto hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb2);
        lo->addLayout(hlo);
      }
    }
#if __cplusplus > 201103L
    connect(sb1, qOverload<int>(&ZSpinBox::valueChanged), sb2, &ZSpinBox::setMinimum);
    connect(sb2, qOverload<int>(&ZSpinBox::valueChanged), sb1, &ZSpinBox::setMaximum);
#else
    connect(sb1, QOverload<int>::of(&ZSpinBox::valueChanged), sb2, &ZSpinBox::setMinimum);
    connect(sb2, QOverload<int>::of(&ZSpinBox::valueChanged), sb1, &ZSpinBox::setMaximum);
#endif
    lo->setMargin(0);
    w->setLayout(lo);
    return w;
  } else {
    ZSpanSliderWithSpinBox* spanSlider = new ZSpanSliderWithSpinBox(m_value[0], m_value[1], m_min, m_max, m_step,
                                                                    m_tracking, parent);
    connect(spanSlider, &ZSpanSliderWithSpinBox::lowerValueChanged, this, &ZIntSpanParameter::setLowerValue);
    connect(spanSlider, &ZSpanSliderWithSpinBox::upperValueChanged, this, &ZIntSpanParameter::setUpperValue);
    connect(this, &ZIntSpanParameter::lowerValueWillChange, spanSlider, &ZSpanSliderWithSpinBox::setLowerValue);
    connect(this, &ZIntSpanParameter::upperValueWillChange, spanSlider, &ZSpanSliderWithSpinBox::setUpperValue);
    connect(this, &ZIntSpanParameter::rangeChanged, spanSlider, &ZSpanSliderWithSpinBox::setDataRange);
    return spanSlider;
  }
}

void ZIntSpanParameter::changeRange()
{
  emit rangeChanged(m_min, m_max);
}

ZFloatSpanParameter::ZFloatSpanParameter(const QString& name, QObject* parent)
  : ZNumericSpanParameter<glm::vec2>(name, glm::vec2(0.f, 0.f), std::numeric_limits<float>::lowest(),
                                     std::numeric_limits<float>::max(), parent)
{
  addStyle("SPINBOX");
}

ZFloatSpanParameter::ZFloatSpanParameter(const QString& name, glm::vec2 value, float min, float max, QObject* parent)
  : ZNumericSpanParameter<glm::vec2>(name, value, min, max, parent)
{
  addStyle("SPINBOX");
}

void ZFloatSpanParameter::setLowerValue(double v)
{
  if (static_cast<float>(v) <= m_value[1])
    set(glm::vec2(static_cast<float>(v), m_value[1]));
}

void ZFloatSpanParameter::setUpperValue(double v)
{
  if (static_cast<float>(v) >= m_value[0])
    set(glm::vec2(m_value[0], static_cast<float>(v)));
}

void ZFloatSpanParameter::beforeChange(glm::vec2& value)
{
  if (value[0] != m_value[0])
    emit lowerValueWillChange(value[0]);
  if (value[1] != m_value[1])
    emit upperValueWillChange(value[1]);
}

QWidget* ZFloatSpanParameter::actualCreateWidget(QWidget* parent)
{
  if (m_style == "SPINBOX") {
    QWidget* w;
    if (m_widgetOrientation == Qt::Horizontal)
      w = new QWidget(parent);
    else
      w = new QGroupBox(m_groupBoxName, parent);
    QBoxLayout* lo;
    if (m_widgetOrientation == Qt::Horizontal)
      lo = new QHBoxLayout();
    else
      lo = new QVBoxLayout();

    auto sb1 = new ZDoubleSpinBox();
    sb1->setRange(m_min, m_value[1]);
    sb1->setValue(m_value[0]);
    sb1->setSingleStep(m_step);
    sb1->setDecimals(m_decimal);
    sb1->setPrefix(m_prefix);
    sb1->setSuffix(m_suffix);
#if __cplusplus > 201103L
    connect(sb1, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZFloatSpanParameter::setLowerValue);
#else
    connect(sb1, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZFloatSpanParameter::setLowerValue);
#endif

    connect(this, &ZFloatSpanParameter::lowerValueWillChange, sb1, &ZDoubleSpinBox::setValue);
    if (m_nameOfEachValue.at(0).isEmpty()) {
      lo->addWidget(sb1);
    } else {
      QLabel* lb = new QLabel(m_nameOfEachValue[0]);
      lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb1);
      } else {
        auto hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb1);
        lo->addLayout(hlo);
      }
    }
    auto sb2 = new ZDoubleSpinBox();
    sb2->setRange(m_value[0], m_max);
    sb2->setValue(m_value[1]);
    sb2->setSingleStep(m_step);
    sb2->setDecimals(m_decimal);
    sb2->setPrefix(m_prefix);
    sb2->setSuffix(m_suffix);
#if __cplusplus > 201103L
    connect(sb2, qOverload<double>(&ZDoubleSpinBox::valueChanged), this, &ZFloatSpanParameter::setUpperValue);
#else
    connect(sb2, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this, &ZFloatSpanParameter::setUpperValue);
#endif

    connect(this, &ZFloatSpanParameter::upperValueWillChange, sb2, &ZDoubleSpinBox::setValue);
    if (m_nameOfEachValue.at(1).isEmpty()) {
      lo->addWidget(sb2);
    } else {
      QLabel* lb = new QLabel(m_nameOfEachValue[1]);
      lb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
      lb->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      if (m_widgetOrientation == Qt::Horizontal) {
        lo->addWidget(lb);
        lo->addWidget(sb2);
      } else {
        auto hlo = new QHBoxLayout();
        hlo->addWidget(lb);
        hlo->addWidget(sb2);
        lo->addLayout(hlo);
      }
    }
#if __cplusplus > 201103L
    connect(sb1, qOverload<double>(&ZDoubleSpinBox::valueChanged), sb2, &ZDoubleSpinBox::setMinimum);
    connect(sb2, qOverload<double>(&ZDoubleSpinBox::valueChanged), sb1, &ZDoubleSpinBox::setMaximum);
#else
    connect(sb1, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), sb2, &ZDoubleSpinBox::setMinimum);
    connect(sb2, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), sb1, &ZDoubleSpinBox::setMaximum);
#endif

    lo->setMargin(0);
    w->setLayout(lo);
    return w;
  } else {
    ZDoubleSpanSliderWithSpinBox* spanSlider = new ZDoubleSpanSliderWithSpinBox(m_value[0], m_value[1], m_min, m_max,
                                                                                m_step, m_decimal, m_tracking, parent);
    connect(spanSlider, &ZDoubleSpanSliderWithSpinBox::lowerValueChanged, this, &ZFloatSpanParameter::setLowerValue);
    connect(spanSlider, &ZDoubleSpanSliderWithSpinBox::upperValueChanged, this, &ZFloatSpanParameter::setUpperValue);
    connect(this, &ZFloatSpanParameter::lowerValueWillChange, spanSlider, &ZDoubleSpanSliderWithSpinBox::setLowerValue);
    connect(this, &ZFloatSpanParameter::upperValueWillChange, spanSlider, &ZDoubleSpanSliderWithSpinBox::setUpperValue);
    connect(this, &ZFloatSpanParameter::rangeChanged, spanSlider, &ZDoubleSpanSliderWithSpinBox::setDataRange);
    return spanSlider;
  }
}

void ZFloatSpanParameter::changeRange()
{
  emit rangeChanged(m_min, m_max);
}


ZDoubleSpanParameter::ZDoubleSpanParameter(const QString& name, QObject* parent)
  : ZNumericSpanParameter<glm::dvec2>(name, glm::dvec2(0.0, 0.0), std::numeric_limits<double>::lowest(),
                                      std::numeric_limits<double>::max(), parent)
{
}

ZDoubleSpanParameter::ZDoubleSpanParameter(const QString& name, glm::dvec2 value, double min, double max,
                                           QObject* parent)
  : ZNumericSpanParameter<glm::dvec2>(name, value, min, max, parent)
{
}

void ZDoubleSpanParameter::setLowerValue(double v)
{
  if (v <= m_value[1])
    set(glm::dvec2(v, m_value[1]));
}

void ZDoubleSpanParameter::setUpperValue(double v)
{
  if (v >= m_value[0])
    set(glm::dvec2(m_value[0], v));
}

void ZDoubleSpanParameter::beforeChange(glm::dvec2& value)
{
  if (value[0] != m_value[0])
    emit lowerValueWillChange(value[0]);
  if (value[1] != m_value[1])
    emit upperValueWillChange(value[1]);
}

QWidget* ZDoubleSpanParameter::actualCreateWidget(QWidget* parent)
{
  ZDoubleSpanSliderWithSpinBox* spanSlider = new ZDoubleSpanSliderWithSpinBox(m_value[0], m_value[1], m_min, m_max,
                                                                              m_step, m_decimal, m_tracking, parent);
  connect(spanSlider, &ZDoubleSpanSliderWithSpinBox::lowerValueChanged, this, &ZDoubleSpanParameter::setLowerValue);
  connect(spanSlider, &ZDoubleSpanSliderWithSpinBox::upperValueChanged, this, &ZDoubleSpanParameter::setUpperValue);
  connect(this, &ZDoubleSpanParameter::lowerValueWillChange, spanSlider, &ZDoubleSpanSliderWithSpinBox::setLowerValue);
  connect(this, &ZDoubleSpanParameter::upperValueWillChange, spanSlider, &ZDoubleSpanSliderWithSpinBox::setUpperValue);
  connect(this, &ZDoubleSpanParameter::rangeChanged, spanSlider, &ZDoubleSpanSliderWithSpinBox::setDataRange);
  return spanSlider;
}

void ZDoubleSpanParameter::changeRange()
{
  emit rangeChanged(m_min, m_max);
}
