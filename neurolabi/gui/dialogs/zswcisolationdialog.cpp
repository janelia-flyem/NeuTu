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
