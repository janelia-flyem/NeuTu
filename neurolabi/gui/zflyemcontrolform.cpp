#include "zflyemcontrolform.h"
#include "ui_zflyemcontrolform.h"

ZFlyEmControlForm::ZFlyEmControlForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ZFlyEmControlForm)
{
  ui->setupUi(this);
}

ZFlyEmControlForm::~ZFlyEmControlForm()
{
  delete ui;
}
