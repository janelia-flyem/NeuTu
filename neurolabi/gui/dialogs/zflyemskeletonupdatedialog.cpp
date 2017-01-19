#include "zflyemskeletonupdatedialog.h"
#include "ui_zflyemskeletonupdatedialog.h"

ZFlyEmSkeletonUpdateDialog::ZFlyEmSkeletonUpdateDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmSkeletonUpdateDialog)
{
  ui->setupUi(this);
}

ZFlyEmSkeletonUpdateDialog::~ZFlyEmSkeletonUpdateDialog()
{
  delete ui;
}

void ZFlyEmSkeletonUpdateDialog::setComputingServer(const QString &address)
{
  ui->serviceLabel->setText(address);
}

bool ZFlyEmSkeletonUpdateDialog::isOverwriting() const
{
  return ui->overwriteCheckBox->isChecked();
}
