#include "zdviddialog.h"
#include "ui_zdviddialog.h"

ZDvidDialog::ZDvidDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZDvidDialog)
{
  ui->setupUi(this);
}

ZDvidDialog::~ZDvidDialog()
{
  delete ui;
}

int ZDvidDialog::getPort() const
{
  return ui->portSpinBox->value();
}

QString ZDvidDialog::getAddress() const
{
  return ui->addressLineEdit->text();
}

QString ZDvidDialog::getUuid() const
{
  return ui->uuidLineEdit->text();
}
