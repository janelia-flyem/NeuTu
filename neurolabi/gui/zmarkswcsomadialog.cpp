#include "zmarkswcsomadialog.h"
#include "ui_zmarkswcsomadialog.h"

ZMarkSwcSomaDialog::ZMarkSwcSomaDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZMarkSwcSomaDialog)
{
  ui->setupUi(this);
}

ZMarkSwcSomaDialog::~ZMarkSwcSomaDialog()
{
  delete ui;
}

double ZMarkSwcSomaDialog::getRadiusThre() const
{
  return ui->radiusThresholdDoubleSpinBox->value() / ui->voxelSizeXYDoubleSpinBox->value();
}

int ZMarkSwcSomaDialog::getSomaType() const
{
  return ui->somaNodesTypeSpinBox->value();
}

int ZMarkSwcSomaDialog::getOtherType() const
{
  return ui->otherNodesTypeSpinBox->value();
}
