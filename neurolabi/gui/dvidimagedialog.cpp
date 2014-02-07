#include "dvidimagedialog.h"
#include "ui_dvidimagedialog.h"

DvidImageDialog::DvidImageDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DvidImageDialog)
{
  ui->setupUi(this);
}

DvidImageDialog::~DvidImageDialog()
{
  delete ui;
}

int DvidImageDialog::getX() const
{
  return ui->xSpinBox->value();
}

int DvidImageDialog::getY() const
{
  return ui->ySpinBox->value();
}

int DvidImageDialog::getZ() const
{
  return ui->zSpinBox->value();
}

int DvidImageDialog::getWidth() const
{
  return ui->widthSpinBox->value();
}

int DvidImageDialog::getHeight() const
{
  return ui->heightSpinBox->value();
}

int DvidImageDialog::getDepth() const
{
  return ui->depthSpinBox->value();
}

QString DvidImageDialog::getAddress() const
{
  return ui->addressWidget->text();
}

void DvidImageDialog::setAddress(const QString address)
{
  ui->addressWidget->setText(address);
}

