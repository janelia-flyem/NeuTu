#include "zflyemproofsettingdialog.h"
#include "ui_zflyemproofsettingdialog.h"
#include "flyem/zflyemproofdoc.h"

ZFlyEmProofSettingDialog::ZFlyEmProofSettingDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmProofSettingDialog)
{
  ui->setupUi(this);
}

ZFlyEmProofSettingDialog::~ZFlyEmProofSettingDialog()
{
  delete ui;
}

int ZFlyEmProofSettingDialog::getCenterCutWidth() const
{
  return ui->centerCutWidthSpinBox->value();
}

int ZFlyEmProofSettingDialog::getCenterCutHeight() const
{
  return ui->centerCutHeightSpinBox->value();
}

bool ZFlyEmProofSettingDialog::showingFullSkeleton() const
{
  return ui->fullSkeletonCheckBox->isChecked();
}

void ZFlyEmProofSettingDialog::applySettings(ZFlyEmProofDoc *doc) const
{
  if (doc != NULL) {
    doc->setGraySliceCenterCut(getCenterCutWidth(), getCenterCutHeight());
    doc->showSwcFullSkeleton(showingFullSkeleton());
  }
}
