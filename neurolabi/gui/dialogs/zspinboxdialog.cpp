#include "zspinboxdialog.h"
#include <QHBoxLayout>
#include "zwidgetfactory.h"
#include "zbuttonbox.h"
#include "tz_constant.h"

ZSpinBoxDialog::ZSpinBoxDialog(QWidget *parent) :
  QDialog(parent), m_isSkipped(false), m_skippedValue(-1)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  QHBoxLayout *spinBoxLayout = new QHBoxLayout(this);

  m_label = new QLabel("Value");
  spinBoxLayout->addWidget(m_label);

  m_spinBox = new QSpinBox(this);
  m_spinBox->setMinimum(0);
  m_spinBox->setMaximum(MAX_INT32);
  m_spinBox->setValue(0);
  spinBoxLayout->addWidget(m_spinBox);

  QHBoxLayout *buttonLayout = new QHBoxLayout(this);
  buttonLayout->addSpacerItem(ZWidgetFactory::makeHSpacerItem());

  m_buttonBox = ZWidgetFactory::makeButtonBox(
        ZButtonBox::ROLE_YES | ZButtonBox::ROLE_NO | ZButtonBox::ROLE_SKIP,
        this);

  connect(m_buttonBox->getButton(ZButtonBox::ROLE_SKIP), SIGNAL(clicked()),
          this, SLOT(skip()));

  buttonLayout->addWidget(m_buttonBox);

  layout->addLayout(spinBoxLayout);
  layout->addLayout(buttonLayout);
}

int ZSpinBoxDialog::getValue() const
{
  if (m_isSkipped) {
    return m_skippedValue;
  }

  return m_spinBox->value();
}

void ZSpinBoxDialog::setValue(int v)
{
  m_spinBox->setValue(v);
}

void ZSpinBoxDialog::setValueLabel(const QString &label)
{
  m_label->setText(label);
}

void ZSpinBoxDialog::skip()
{
  m_isSkipped = true;
  QDialog::accept();
}

void ZSpinBoxDialog::accept()
{
  m_isSkipped = false;
  QDialog::accept();
}

QPushButton* ZSpinBoxDialog::getButton(ZButtonBox::TRole role)
{
  return m_buttonBox->getButton(role);
}
