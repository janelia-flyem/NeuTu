#include "zstresstestoptiondialog.h"
#include "ui_zstresstestoptiondialog.h"

ZStressTestOptionDialog::ZStressTestOptionDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZStressTestOptionDialog)
{
  ui->setupUi(this);
}

ZStressTestOptionDialog::~ZStressTestOptionDialog()
{
  delete ui;
}

ZStressTestOptionDialog::EOption ZStressTestOptionDialog::getOption() const
{
  EOption option = OPTION_CUSTOM;

  if (ui->customRadioButton->isChecked()) {
    option = OPTION_CUSTOM;
  } else if (ui->bodySplitRadioButton->isChecked()) {
    option = OPTION_BODY_SPLIT;
  } else if (ui->bodyMergeRadioButton->isChecked()) {
    option = OPTION_BODY_MERGE;
  } else if (ui->bodyVisRadioButton->isChecked()) {
    option = OPTION_BODY_3DVIS;
  } else if (ui->objectManagementRadioButton->isChecked()) {
    option = OPTION_OBJECT_MANAGEMENT;
  }

  return option;
}
