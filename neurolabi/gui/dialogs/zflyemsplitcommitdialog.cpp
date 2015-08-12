#include "zflyemsplitcommitdialog.h"
#include "ui_zflyemsplitcommitdialog.h"

ZFlyEmSplitCommitDialog::ZFlyEmSplitCommitDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmSplitCommitDialog)
{
  ui->setupUi(this);
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
