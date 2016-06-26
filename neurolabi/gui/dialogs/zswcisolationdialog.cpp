#include "zswcisolationdialog.h"
#include "ui_zswcisolationdialog.h"

ZSwcIsolationDialog::ZSwcIsolationDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZSwcIsolationDialog)
{
  ui->setupUi(this);
}

ZSwcIsolationDialog::~ZSwcIsolationDialog()
{
  delete ui;
}

double ZSwcIsolationDialog::getLengthThreshold() const
{
  return ui->lengthDoubleSpinBox->value();
}

double ZSwcIsolationDialog::getDistanceThreshold() const
{
  return ui->distDoubleSpinBox->value();
}

double ZSwcIsolationDialog::getXScale() const
{
  return ui->xSpinBox->value();
}

double ZSwcIsolationDialog::getYScale() const
{
  return ui->ySpinBox->value();
}

double ZSwcIsolationDialog::getZScale() const
{
  return ui->zSpinBox->value();
}

void ZSwcIsolationDialog::setScale(double xs, double ys, double zs)
{
  ui->xSpinBox->setValue(xs);
  ui->ySpinBox->setValue(ys);
  ui->zSpinBox->setValue(zs);
}
