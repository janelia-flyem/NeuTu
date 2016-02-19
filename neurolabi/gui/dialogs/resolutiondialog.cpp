#include "resolutiondialog.h"
#include "ui_resolutiondialog.h"

ResolutionDialog::ResolutionDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ResolutionDialog)
{
  ui->setupUi(this);
}

ResolutionDialog::~ResolutionDialog()
{
  delete ui;
}

double ResolutionDialog::getXYScale() const
{
  return ui->xyDoubleSpinBox->value();
}

double ResolutionDialog::getZScale() const
{
  return ui->zDoubleSpinBox->value();
}
