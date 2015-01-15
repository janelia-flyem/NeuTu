#include "zautotracedialog.h"

#include <QCheckBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

ZAutoTraceDialog::ZAutoTraceDialog(QWidget *parent, Qt::WindowFlags f)
  : QDialog(parent, f)
{
  QVBoxLayout *alllayout = new QVBoxLayout;
  m_resampleCheckbox = new QCheckBox("Resample SWC after Tracing");
  m_resampleCheckbox->setCheckState(Qt::Unchecked);
  alllayout->addWidget(m_resampleCheckbox);

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                   | QDialogButtonBox::Cancel);

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  alllayout->addWidget(buttonBox);

  setLayout(alllayout);
}

ZAutoTraceDialog::~ZAutoTraceDialog()
{
}

bool ZAutoTraceDialog::getDoResample() const
{
  return m_resampleCheckbox->isChecked();
}

