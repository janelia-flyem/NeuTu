#include "zcontrastprotocaldialog.h"
#include "ui_zcontrastprotocaldialog.h"

#include "zjsonobject.h"
#include "zcontrastprotocol.h"

ZContrastProtocalDialog::ZContrastProtocalDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZContrastProtocalDialog)
{
  ui->setupUi(this);
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
