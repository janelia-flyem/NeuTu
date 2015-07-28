#include "flyembodyiddialog.h"
#include "ui_flyembodyiddialog.h"
#include "zstring.h"

FlyEmBodyIdDialog::FlyEmBodyIdDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmBodyIdDialog)
{
  ui->setupUi(this);
}

FlyEmBodyIdDialog::~FlyEmBodyIdDialog()
{
  delete ui;
}

int FlyEmBodyIdDialog::getBodyId() const
{
  ZString str(ui->bodyIdLineEdit->text().toStdString());

  return str.firstInteger();
}

std::vector<int> FlyEmBodyIdDialog::getBodyIdArray() const
{
  ZString str(ui->bodyIdLineEdit->text().toStdString());

  return str.toIntegerArray();
}

std::vector<int> FlyEmBodyIdDialog::getDownsampleInterval() const
{
  std::vector<int> ds(3);
  ds[0] = ui->xDsSpinBox->value();
  ds[1] = ui->yDsSpinBox->value();
  ds[2] = ui->zDsSpinBox->value();

  return ds;
}
