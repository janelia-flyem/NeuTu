#include "zflyembodysplitdialog.h"
#include "ui_zflyembodysplitdialog.h"
#include "zstring.h"

ZFlyEmBodySplitDialog::ZFlyEmBodySplitDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmBodySplitDialog)
{
  ui->setupUi(this);
}

ZFlyEmBodySplitDialog::~ZFlyEmBodySplitDialog()
{
  delete ui;
}

void ZFlyEmBodySplitDialog::setBodyId(uint64_t bodyId)
{
  ui->bodyLineEdit->setText(QString("%1").arg(bodyId));
}

bool ZFlyEmBodySplitDialog::isOfflineSplit() const
{
  return ui->offlineCheckBox->isChecked();
}

uint64_t ZFlyEmBodySplitDialog::getBodyId() const
{
  return ZString(ui->bodyLineEdit->text().toStdString()).firstUint64();
}
