#include "zspinboxgroupdialog.h"
#include "zbuttonbox.h"
#include "zwidgetfactory.h"

ZSpinBoxGroupDialog::ZSpinBoxGroupDialog(QWidget *parent) :
  QDialog(parent), m_isSkipped(false)
{
  m_mainLayout = new QVBoxLayout(this);
  setLayout(m_mainLayout);

  m_spinBoxLayout = new QVBoxLayout(this);

  ZButtonBox *buttonBox = ZWidgetFactory::makeButtonBox(
        ZButtonBox::ROLE_YES | ZButtonBox::ROLE_NO | ZButtonBox::ROLE_SKIP,
        this);

  connect(buttonBox->getButton(ZButtonBox::ROLE_SKIP), SIGNAL(clicked()),
          this, SLOT(skip()));


  QHBoxLayout *buttonLayout = new QHBoxLayout(this);
  buttonLayout->addSpacerItem(ZWidgetFactory::makeHSpacerItem());
  buttonLayout->addWidget(buttonBox);

  m_mainLayout->addLayout(m_spinBoxLayout);
  m_mainLayout->addLayout(buttonLayout);
}

int ZSpinBoxGroupDialog::getValue(const QString &name) const
{
  int v = 0;
  if (m_spinBoxMap.contains(name)) {
    const ZLabeledSpinBoxWidget *box = m_spinBoxMap[name];
    if (m_isSkipped) {
      v = box->getSkipValue();
    } else {
      v = box->getValue();
    }
  }

  return v;
}

bool ZSpinBoxGroupDialog::addSpinBox(
    const QString &name, int vmin, int vmax, int defaultValue, int skipValue)
{
  if (!m_spinBoxMap.contains(name)) {
    ZLabeledSpinBoxWidget *box = new ZLabeledSpinBoxWidget(this);
    box->addSpacer();
    box->setLabel(name);
    box->setRange(vmin, vmax);
    box->setValue(defaultValue);
    box->setSkipValue(skipValue);
    m_spinBoxMap[name] = box;

    m_spinBoxLayout->addWidget(box);

    return true;
  }

  return false;
}

void ZSpinBoxGroupDialog::skip()
{
  m_isSkipped = true;
  accept();
}
