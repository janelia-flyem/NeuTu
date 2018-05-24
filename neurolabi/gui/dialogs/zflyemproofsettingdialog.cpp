#include "zflyemproofsettingdialog.h"
#include "ui_zflyemproofsettingdialog.h"
#include "flyem/zflyemproofdoc.h"
#include "neutubeconfig.h"
#include "flyem/flyemdef.h"

ZFlyEmProofSettingDialog::ZFlyEmProofSettingDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmProofSettingDialog)
{
  ui->setupUi(this);

  initGrayscaleCenterCut();
  initSegmentationCenterCut();
}

ZFlyEmProofSettingDialog::~ZFlyEmProofSettingDialog()
{
  delete ui;
}

void ZFlyEmProofSettingDialog::initGrayscaleCenterCut()
{
  std::pair<int,int> centercut =
      GET_FLYEM_CONFIG.getCenterCut(flyem::key::GRAYSCALE);
  ui->centerCutWidthSpinBox->setValue(centercut.first);
  ui->centerCutHeightSpinBox->setValue(centercut.second);
}

void ZFlyEmProofSettingDialog::initSegmentationCenterCut()
{
  std::pair<int,int> centercut =
      GET_FLYEM_CONFIG.getCenterCut(flyem::key::SEGMENTATION);
  ui->segCenterCutWidthSpinBox->setValue(centercut.first);
  ui->segCenterCutHeightSpinBox->setValue(centercut.second);
}

int ZFlyEmProofSettingDialog::getGrayscaleCenterCutWidth() const
{
  return ui->centerCutWidthSpinBox->value();
}

int ZFlyEmProofSettingDialog::getGrayscaleCenterCutHeight() const
{
  return ui->centerCutHeightSpinBox->value();
}

int ZFlyEmProofSettingDialog::getSegmentationCenterCutWidth() const
{
  return ui->segCenterCutWidthSpinBox->value();
}

int ZFlyEmProofSettingDialog::getSegmentationCenterCutHeight() const
{
  return ui->segCenterCutHeightSpinBox->value();
}

bool ZFlyEmProofSettingDialog::showingFullSkeleton() const
{
  return ui->fullSkeletonCheckBox->isChecked();
}

void ZFlyEmProofSettingDialog::applySettings(ZFlyEmProofDoc *doc) const
{
  if (doc != NULL) {
    doc->setGraySliceCenterCut(
          getGrayscaleCenterCutWidth(), getGrayscaleCenterCutHeight());
    doc->setSegmentationCenterCut(getSegmentationCenterCutWidth(),
                                  getSegmentationCenterCutHeight());
//    doc->setSeg
    doc->showSwcFullSkeleton(showingFullSkeleton());
  }
}
