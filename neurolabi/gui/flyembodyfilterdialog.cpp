#include "flyembodyfilterdialog.h"
#include "ui_flyembodyfilterdialog.h"

FlyEmBodyFilterDialog::FlyEmBodyFilterDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmBodyFilterDialog)
{
  ui->setupUi(this);
}

FlyEmBodyFilterDialog::~FlyEmBodyFilterDialog()
{
  delete ui;
}
