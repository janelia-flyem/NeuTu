#include "dvidimagedialog.h"
#include "ui_dvidimagedialog.h"
#include "neutubeconfig.h"

DvidImageDialog::DvidImageDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DvidImageDialog)
{
  ui->setupUi(this);
#if defined(_FLYEM_)
  setAddress( NeutubeConfig::getInstance().getFlyEmConfig().
              getDvidTarget().getSourceString(false).c_str());
#endif
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
#if 0
QString DvidImageDialog::getAddress() const
{
  return ui->addressWidget->text();
}
#endif
void DvidImageDialog::setAddress(const QString address)
{
  ui->addressWidget->setText(address);
}


