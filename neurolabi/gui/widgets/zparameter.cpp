#include "zparameter.h"

#include <iostream>

#include <QWidget>
#include <QLabel>
#include <QCheckBox>

ZParameter::ZParameter(const QString& name, QObject* parent)
  : QObject(parent)
  , m_name(name)
{
  addStyle("DEFAULT");
#ifdef _DEBUG_2
  std::cout << "Constructing ZParameter " << name.toStdString() << " " << this << std::endl;
#endif
}

ZParameter::~ZParameter()
{
#ifdef _DEBUG_2
  std::cout << "Destroying ZParameter " << m_name.toStdString() << " " << this << std::endl;
#endif
}

void ZParameter::setName(const QString& name)
{
  if (name != m_name) {
    m_name = name;
    emit nameChanged(m_name);
  }
}

QString ZParameter::type() const
{
  QString className = metaObject()->className();
  if (className.startsWith("nim::"))
    className.remove(0, 5);
  return className.remove(0, 1).left(className.length() - 9);
}

void ZParameter::setStyle(const QString& style)
{
  if (m_allStyles.contains(style))
    m_style = style;
  else
    m_style = "DEFAULT";
}

QLabel* ZParameter::createNameLabel(QWidget* parent)
{
  auto label = new QLabel(m_name, parent);
  label->setTextInteractionFlags(Qt::TextSelectableByMouse);
  if (!m_isWidgetsVisible)
    label->setVisible(m_isWidgetsVisible);
  if (!m_isWidgetsEnabled)
    label->setEnabled(m_isWidgetsEnabled);
  connect(this, &ZParameter::setWidgetsEnabled, label, &QLabel::setEnabled);
  connect(this, &ZParameter::setWidgetsVisible, label, &QLabel::setVisible);
  connect(this, &ZParameter::nameChanged, label, &QLabel::setText);

#ifdef __APPLE__
  //QFont fnt = label->font();
  //fnt.setPointSize(11);
  //label->setFont(fnt);
#endif
  return label;
}

QWidget* ZParameter::createWidget(QWidget* parent)
{
  QWidget* widget = actualCreateWidget(parent);
  if (!m_isWidgetsVisible)
    widget->setVisible(m_isWidgetsVisible);
  if (!m_isWidgetsEnabled)
    widget->setEnabled(m_isWidgetsEnabled);
  connect(this, &ZParameter::setWidgetsEnabled, widget, &QWidget::setEnabled);
  connect(this, &ZParameter::setWidgetsVisible, widget, &QWidget::setVisible);
#ifdef __APPLE__
  widget->setAttribute(Qt::WA_LayoutUsesWidgetRect);

  //QFont fnt = widget->font();
  //fnt.setPointSize(11);
  //widget->setFont(fnt);
#endif
  return widget;
}

void ZParameter::setSameAs(const ZParameter& rhs)
{
  m_allStyles = rhs.m_allStyles;
  setName(rhs.m_name);
  setStyle(rhs.m_style);
  setEnabled(rhs.m_isWidgetsEnabled);
  setVisible(rhs.m_isWidgetsVisible);
}

void ZParameter::setVisible(bool s)
{
  if (s != m_isWidgetsVisible) {
    m_isWidgetsVisible = s;
    emit setWidgetsVisible(m_isWidgetsVisible);
  }
}

void ZParameter::setEnabled(bool s)
{
  if (s != m_isWidgetsEnabled) {
    m_isWidgetsEnabled = s;
    emit setWidgetsEnabled(m_isWidgetsEnabled);
  }
}

void ZParameter::updateFromSender()
{
  ZParameter* para = static_cast<ZParameter*>(sender());
  if (isSameType(*para)) {
    setValueSameAs(*para);
  } else {
    LOG(ERROR) << "can not update parameter " << name() << " with type " << type()
               << " from different type parameter " << para->name() << " type " << para->type();
  }
}

ZBoolParameter::ZBoolParameter(const QString& name, QObject* parent)
  : ZSingleValueParameter<bool>(name, parent)
{
}

ZBoolParameter::ZBoolParameter(const QString& name, bool value, QObject* parent)
  : ZSingleValueParameter<bool>(name, value, parent)
{
}

void ZBoolParameter::setValue(bool v)
{
  set(v);
}

void ZBoolParameter::beforeChange(bool& value)
{
  emit valueWillChange(value);
}

void ZBoolParameter::afterChange(bool& /*unused*/)
{
  emit boolChanged(m_value);
}

QWidget* ZBoolParameter::actualCreateWidget(QWidget* parent)
{
  auto cb = new QCheckBox(parent);
  cb->setChecked(m_value);
  connect(cb, &QCheckBox::toggled, this, &ZBoolParameter::setValue);
  connect(this, &ZBoolParameter::valueWillChange, cb, &QCheckBox::setChecked);
  return cb;
}

void ZBoolParameter::setSameAs(const ZParameter& rhs)
{
  CHECK(this->isSameType(rhs));
  set(static_cast<const ZBoolParameter*>(&rhs)->get());
  ZSingleValueParameter<bool>::setSameAs(rhs);
}
