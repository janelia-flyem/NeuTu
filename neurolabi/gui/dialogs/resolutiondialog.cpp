#include "resolutiondialog.h"
#include "ui_resolutiondialog.h"

ResolutionDialog::ResolutionDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ResolutionDialog)
{
  ui->setupUi(this);

  connect(ui->xyCheckBox, SIGNAL(toggled(bool)), this, SLOT(linkXYRes(bool)));
  if (ui->xyCheckBox->isChecked()) {
    linkXYRes(true);
  }
}

ResolutionDialog::~ResolutionDialog()
{
  delete ui;
}

double ResolutionDialog::getXScale() const
{
  return ui->xDoubleSpinBox->value();
}

double ResolutionDialog::getYScale() const
{
  return ui->yDoubleSpinBox->value();
}

double ResolutionDialog::getZScale() const
{
  return ui->zDoubleSpinBox->value();
}

void ResolutionDialog::setXScaleQuitely(double v)
{
  ui->xDoubleSpinBox->blockSignals(true);
  ui->xDoubleSpinBox->setValue(v);
  ui->xDoubleSpinBox->blockSignals(false);
}

void ResolutionDialog::setYScaleQuitely(double v)
{
  ui->yDoubleSpinBox->blockSignals(true);
  ui->yDoubleSpinBox->setValue(v);
  ui->yDoubleSpinBox->blockSignals(false);
}

void ResolutionDialog::linkXYRes(bool linked)
{
  if (linked) {
    connect(ui->xDoubleSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(setYScaleQuitely(double)));
    connect(ui->yDoubleSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(setXScaleQuitely(double)));
  } else {
    disconnect(ui->xDoubleSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(setYScaleQuitely(double)));
    disconnect(ui->yDoubleSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(setXScaleQuitely(double)));
  }
}
