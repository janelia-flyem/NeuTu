#include "zspinboxdialog.h"
#include <QHBoxLayout>
#include "zwidgetfactory.h"
#include "zbuttonbox.h"

ZSpinBoxDialog::ZSpinBoxDialog(QWidget *parent) :
  QDialog(parent)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  QHBoxLayout *spinBoxLayout = new QHBoxLayout(this);

  m_label = new QLabel("Value");
  spinBoxLayout->addWidget(m_label);

  m_spinBox = new QSpinBox(this);
  m_spinBox->setMinimum(0);
  m_spinBox->setMaximum(99999);
  m_spinBox->setValue(0);
  spinBoxLayout->addWidget(m_spinBox);

  QHBoxLayout *buttonLayout = new QHBoxLayout(this);
  buttonLayout->addSpacerItem(ZWidgetFactory::makeHSpacerItem());

  ZButtonBox *buttonBox = ZWidgetFactory::makeButtonBox(
        ZButtonBox::ROLE_YES | ZButtonBox::ROLE_NO, this);
  buttonLayout->addWidget(buttonBox);

  layout->addLayout(spinBoxLayout);
  layout->addLayout(buttonLayout);
}

int ZSpinBoxDialog::getValue() const
{
  return m_spinBox->value();
}
