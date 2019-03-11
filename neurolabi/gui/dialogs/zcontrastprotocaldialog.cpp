#include "zcontrastprotocaldialog.h"
#include "ui_zcontrastprotocaldialog.h"

#include "zjsonobject.h"
#include "zcontrastprotocol.h"

ZContrastProtocalDialog::ZContrastProtocalDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZContrastProtocalDialog)
{
  ui->setupUi(this);

  connect(ui->offsetSpinBox, SIGNAL(valueChanged(double)),
          this, SIGNAL(protocolChanged()));
  connect(ui->scaleSpinBox, SIGNAL(valueChanged(double)),
          this, SIGNAL(protocolChanged()));
//  connect(ui->nonlinearCheckBox, SIGNAL(toggled(bool)),
//          this, SIGNAL(protocolChanged()));
  connect(ui->noneRadioButton, SIGNAL(toggled(bool)),
          this, SIGNAL(protocolChanged()));
  connect(ui->powerRadioButton, SIGNAL(toggled(bool)),
          this, SIGNAL(protocolChanged()));
  connect(ui->sigmoidRadioButton, SIGNAL(toggled(bool)),
          this, SIGNAL(protocolChanged()));

  connect(ui->buttonBox, SIGNAL(accepted()), this, SIGNAL(committing()));
  connect(ui->buttonBox, SIGNAL(rejected()), this, SIGNAL(canceled()));
  connect(ui->offsetStepSlider, SIGNAL(valueChanged(int)),
          this, SLOT(updateOffsetStep()));
  connect(ui->scaleStepSlider, SIGNAL(valueChanged(int)),
          this, SLOT(updateScaleStep()));

  updateOffsetStep();
  updateScaleStep();
}

ZContrastProtocalDialog::~ZContrastProtocalDialog()
{
  delete ui;
}

int ZContrastProtocalDialog::getNonlinearMode() const
{
  if (ui->powerRadioButton->isChecked()) {
    return 1;
  }

  if (ui->sigmoidRadioButton->isChecked()) {
    return 2;
  }

  return 0;
}

ZJsonObject ZContrastProtocalDialog::getContrastProtocal() const
{
  ZJsonObject obj;

  obj.setEntry("offset", ui->offsetSpinBox->value());
  obj.setEntry("scale", ui->scaleSpinBox->value());
  obj.setEntry("nonlinear", getNonlinearMode());

  /*
  int nonlinearIndex = 0;
  switch (getNonlinearMode()) {
  case ZContrastProtocol::ENonlinearMode::NONLINEAR_POWER:
    nonlinearIndex = 1;
    break;
  case ZContrastProtocol::ENonlinearMode::NONLINEAR_SIGMOID:
    nonlinearIndex = 2;
    break;
  default:
    break;
  }
*/
//  obj.setEntry("nonlinear", nonlinearIndex);
//  obj.setEntry("nonlinear", ui->nonlinearCheckBox->isChecked());

  return obj;
}

void ZContrastProtocalDialog::setContrastProtocol(
    const ZJsonObject &protocolJson)
{
  ZContrastProtocol protocal;
  protocal.load(protocolJson);

  ui->offsetSpinBox->setValue(protocal.getOffset());
  ui->scaleSpinBox->setValue(protocal.getScale());

  switch (protocal.getNonlinearMode()) {
  case ZContrastProtocol::ENonlinearMode::POWER:
    ui->powerRadioButton->setChecked(true);
    break;
  case ZContrastProtocol::ENonlinearMode::SIGMOID:
    ui->sigmoidRadioButton->setChecked(true);
    break;
  default:
    ui->noneRadioButton->setChecked(true);
    break;
  }

//  ui->nonlinearCheckBox->setChecked(protocal.isNonlinear());
}

double ZContrastProtocalDialog::getOffsetStep() const
{
  return ui->offsetStepSlider->value() * 0.1;
}

double ZContrastProtocalDialog::getScaleStep() const
{
  return ui->scaleStepSlider->value() * 0.1;
}

void ZContrastProtocalDialog::updateOffsetStep()
{
  ui->offsetStepLabel->setText(QString("%1").arg(getOffsetStep()));
  ui->offsetSpinBox->setSingleStep(getOffsetStep());
}

void ZContrastProtocalDialog::updateScaleStep()
{
  ui->scaleStepLabel->setText(QString("%1").arg(getScaleStep()));
  ui->scaleSpinBox->setSingleStep(getScaleStep());
}















