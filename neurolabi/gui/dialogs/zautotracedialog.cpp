#include "zautotracedialog.h"

#include <QCheckBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#include "zlabeledspinboxwidget.h"

ZAutoTraceDialog::ZAutoTraceDialog(QWidget *parent, Qt::WindowFlags f)
  : QDialog(parent, f)
{
  QVBoxLayout *alllayout = new QVBoxLayout;
  m_resampleCheckbox = new QCheckBox("Resample structure after tracing");
  m_resampleCheckbox->setToolTip(
        "The final structure will be sparser when this option is on (recommended).");
  m_resampleCheckbox->setCheckState(Qt::Checked);
  alllayout->addWidget(m_resampleCheckbox);

  m_levelSpinBox = new ZLabeledSpinBoxWidget;
  m_levelSpinBox->setLabel("Level (0-6)");
  m_levelSpinBox->setToolTip(
        "Tracing level: higher value means longer tracing"
        "time to produce better result (hopefully). "
        "Level 0 means default parameters.");
  m_levelSpinBox->setRange(0, 6);
  alllayout->addWidget(m_levelSpinBox);
  m_levelSpinBox->hide();

  this->setWindowTitle(tr("Automatic Tracing"));

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

int ZAutoTraceDialog::getTraceLevel() const
{
  return m_levelSpinBox->getValue();
}
