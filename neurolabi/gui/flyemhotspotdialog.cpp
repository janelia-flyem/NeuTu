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

int FlyEmHotSpotDialog::getId() const
{
  return ZString::firstInteger(ui->idLineEdit->text().toStdString());
}
