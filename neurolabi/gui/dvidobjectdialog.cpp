#include "dvidobjectdialog.h"
#include "ui_dvidobjectdialog.h"
#include "zstring.h"

DvidObjectDialog::DvidObjectDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DvidObjectDialog)
{
  ui->setupUi(this);
}

DvidObjectDialog::~DvidObjectDialog()
{
  delete ui;
}

void DvidObjectDialog::setAddress(const QString &address)
{
  ui->addressLineEdit->setText(address);
}

QString DvidObjectDialog::getAddress() const
{
  return ui->addressLineEdit->text();
}

std::vector<int> DvidObjectDialog::getBodyId() const
{
  ZString str = ui->bodyLineEdit->text().toStdString();

  return str.toIntegerArray();
}

bool DvidObjectDialog::retrievingSkeleton() const
{
  return ui->retrieveSkeletonCheckBox->isChecked();
}

bool DvidObjectDialog::retrievingObject() const
{
  return ui->retrieveObjectCheckBox->isChecked();
}
