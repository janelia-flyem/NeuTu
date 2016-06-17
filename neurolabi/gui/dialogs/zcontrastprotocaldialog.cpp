#include "zcontrastprotocaldialog.h"
#include "ui_zcontrastprotocaldialog.h"

#include "zjsonobject.h"

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
