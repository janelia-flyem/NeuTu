#include "zstringparameter.h"

#include "common/utilities.h"

#ifdef _QT5_
#include <QtWidgets>
#include <QSignalBlocker>
#else
#include <QtGui>
#endif

ZStringParameter::ZStringParameter(const QString& name, QObject* parent)
  : ZSingleValueParameter<QString>(name, parent)
{
}

ZStringParameter::ZStringParameter(const QString& name, const QString& str, QObject* parent)
  : ZSingleValueParameter<QString>(name, str, parent)
{
}

void ZStringParameter::setContent(const QString& str)
{
  set(str);
}

void ZStringParameter::setContentQuietly(const QString &str)
{
  QSignalBlocker blocker(this);

  set(str);
}

QWidget* ZStringParameter::actualCreateWidget(QWidget* parent)
{
  auto le = new QLineEdit(parent);
  le->setText(m_value);
  connect(le, &QLineEdit::textChanged, this, &ZStringParameter::setContentQuietly);
  connect(this, &ZStringParameter::stringChanged, le, &QLineEdit::setText);
  return le;
}

void ZStringParameter::afterChange(QString& /*unused*/)
{
  emit stringChanged(m_value);
}

void ZStringParameter::setSameAs(const ZParameter& rhs)
{
  CHECK(this->isSameType(rhs));
  const ZStringParameter* src = static_cast<const ZStringParameter*>(&rhs);
  this->set(src->get());
  ZParameter::setSameAs(rhs);
}
