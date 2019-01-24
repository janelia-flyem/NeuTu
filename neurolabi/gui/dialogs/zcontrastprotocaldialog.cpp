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
  connect(ui->nonlinearCheckBox, SIGNAL(toggled(bool)),
          this, SIGNAL(protocolChanged()));
  connect(ui->buttonBox, SIGNAL(accepted()), this, SIGNAL(committing()));
  connect(ui->buttonBox, SIGNAL(rejected()), this, SIGNAL(canceled()));
}

ZContrastProtocalDialog::~ZContrastProtocalDialog()
{
  delete ui;
}

ZJsonObject ZContrastProtocalDialog::getContrastProtocal() const
{
  ZJsonObject obj;

  obj.setEntry("offset", ui->offsetSpinBox->value());
  obj.setEntry("scale", ui->scaleSpinBox->value());
  obj.setEntry("nonlinear", ui->nonlinearCheckBox->isChecked());

  return obj;
}

void ZContrastProtocalDialog::setContrastProtocol(
    const ZJsonObject &protocolJson)
{
  ZContrastProtocol protocal;
  protocal.load(protocolJson);

  ui->offsetSpinBox->setValue(protocal.getOffset());
  ui->scaleSpinBox->setValue(protocal.getScale());
  ui->nonlinearCheckBox->setChecked(protocal.isNonlinear());
}
