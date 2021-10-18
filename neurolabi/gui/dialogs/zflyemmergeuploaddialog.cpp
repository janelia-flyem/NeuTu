#include "zflyemmergeuploaddialog.h"
#include "ui_zflyemmergeuploaddialog.h"

ZFlyEmMergeUploadDialog::ZFlyEmMergeUploadDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmMergeUploadDialog)
{
  ui->setupUi(this);
}

ZFlyEmMergeUploadDialog::~ZFlyEmMergeUploadDialog()
{
  delete ui;
}

void ZFlyEmMergeUploadDialog::setMessage(const QString &msg)
{
  ui->infoTextEdit->setText(msg);
}

bool ZFlyEmMergeUploadDialog::mergingToLargest() const
{
  return ui->mergeCheckBox->isChecked();
}

bool ZFlyEmMergeUploadDialog::mergingToHighestStatus() const
{
  return ui->statusCheckBox->isChecked();
}
