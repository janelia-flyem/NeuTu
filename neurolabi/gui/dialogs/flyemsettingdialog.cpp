#include "flyemsettingdialog.h"
#include "ui_flyemsettingdialog.h"

FlyEmSettingDialog::FlyEmSettingDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmSettingDialog)
{
  ui->setupUi(this);
}

FlyEmSettingDialog::~FlyEmSettingDialog()
{
  delete ui;
}
