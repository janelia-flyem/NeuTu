#include "zflyemsplitcommitdialog.h"
#include "ui_zflyemsplitcommitdialog.h"

ZFlyEmSplitCommitDialog::ZFlyEmSplitCommitDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmSplitCommitDialog)
{
  ui->setupUi(this);
  QPalette p = ui->noticeTextEdit->palette();
  p.setColor(QPalette::Base, QColor(128, 128, 128, 0));
  ui->noticeTextEdit->setPalette(p);
}

ZFlyEmSplitCommitDialog::~ZFlyEmSplitCommitDialog()
{
  delete ui;
}

int ZFlyEmSplitCommitDialog::getGroupSize() const
{
  if (!ui->groupCheckBox->isChecked()) {
    return 0;
  }

  return ui->minObjSizeSpinBox->value();
}

bool ZFlyEmSplitCommitDialog::keepingMainSeed() const
{
  return ui->keepingSeedCheckBox->isChecked();
}

bool ZFlyEmSplitCommitDialog::runningCca() const
{
  return ui->ccaCheckBox->isChecked();
}
