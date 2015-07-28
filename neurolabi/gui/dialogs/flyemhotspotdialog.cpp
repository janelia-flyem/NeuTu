#include "flyemhotspotdialog.h"
#include "ui_flyemhotspotdialog.h"
#include "zstring.h"

FlyEmHotSpotDialog::FlyEmHotSpotDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmHotSpotDialog)
{
  ui->setupUi(this);
}

FlyEmHotSpotDialog::~FlyEmHotSpotDialog()
{
  delete ui;
}

int FlyEmHotSpotDialog::getBodyId() const
{
  return ZString::firstInteger(ui->idLineEdit->text().toStdString());
}

FlyEmHotSpotDialog::EHotSpotType FlyEmHotSpotDialog::getType() const
{
  if (ui->typeComboBox->currentIndex() == 0) {
    return FALSE_MERGE;
  }

  return FALSE_SPLIT;
}
