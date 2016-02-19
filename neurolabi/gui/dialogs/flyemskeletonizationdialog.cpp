#include "flyemskeletonizationdialog.h"
#include "ui_flyemskeletonizationdialog.h"

FlyEmSkeletonizationDialog::FlyEmSkeletonizationDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmSkeletonizationDialog)
{
  ui->setupUi(this);
  connect(ui->connectCheckBox, SIGNAL(stateChanged(int)),
          this, SLOT(enableDistanceThreshold()));
  connect(ui->objSizeCheckBox, SIGNAL(stateChanged(int)),
          this, SLOT(enableSizeThreshold()));
  connect(ui->downsampleCheckBox, SIGNAL(stateChanged(int)),
          this, SLOT(enableDownsample()));
}

FlyEmSkeletonizationDialog::~FlyEmSkeletonizationDialog()
{
  delete ui;
}

int FlyEmSkeletonizationDialog::lengthThreshold() const
{
  return ui->lengthSpinBox->value();
}

int FlyEmSkeletonizationDialog::distanceThreshold() const
{
  return ui->distThreSpinBox->value();
}

int FlyEmSkeletonizationDialog::sizeThreshold() const
{
  return ui->objSizeSpinBox->value();
}

bool FlyEmSkeletonizationDialog::isConnectingAll() const
{
  return ui->connectCheckBox->isChecked();
}

bool FlyEmSkeletonizationDialog::isExcludingSmallObj() const
{
  return ui->objSizeCheckBox->isChecked();
}

bool FlyEmSkeletonizationDialog::isKeepingShortObject() const
{
  return ui->shortObjectCheckBox->isChecked();
}

void FlyEmSkeletonizationDialog::enableDistanceThreshold()
{
  ui->distThreSpinBox->setEnabled(!ui->connectCheckBox->isChecked());
}

void FlyEmSkeletonizationDialog::enableSizeThreshold()
{
  ui->objSizeSpinBox->setEnabled(ui->objSizeCheckBox->isChecked());
}

void FlyEmSkeletonizationDialog::enableDownsample()
{
  ui->dsXSpinBox->setEnabled(ui->downsampleCheckBox->isChecked());
  ui->dsYSpinBox->setEnabled(ui->downsampleCheckBox->isChecked());
  ui->dsZSpinBox->setEnabled(ui->downsampleCheckBox->isChecked());
}

bool FlyEmSkeletonizationDialog::isLevelChecked() const
{
  return ui->greyToBinaryCheckBox->isChecked();
}

int FlyEmSkeletonizationDialog::level() const
{
  return ui->levelSpinBox->value();
}

int FlyEmSkeletonizationDialog::getLevelOp() const
{
  return ui->grayOpComboBox->currentIndex();
}

bool FlyEmSkeletonizationDialog::isDownsampleChecked() const
{
  return ui->downsampleCheckBox->isChecked();
}

int FlyEmSkeletonizationDialog::getXInterval() const
{
  if (!isDownsampleChecked()) {
    return 0;
  }

  return ui->dsXSpinBox->value();
}

int FlyEmSkeletonizationDialog::getYInterval() const
{
  if (!isDownsampleChecked()) {
    return 0;
  }

  return ui->dsYSpinBox->value();
}

int FlyEmSkeletonizationDialog::getZInterval() const
{
  if (!isDownsampleChecked()) {
    return 0;
  }

  return ui->dsZSpinBox->value();
}
