#include "neuroglancerlinkdialog.h"
#include "ui_neuroglancerlinkdialog.h"

#include "dvid/zdvidenv.h"

NeuroglancerLinkDialog::NeuroglancerLinkDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::NeuroglancerLinkDialog)
{
  ui->setupUi(this);
}

NeuroglancerLinkDialog::~NeuroglancerLinkDialog()
{
  delete ui;
}

void NeuroglancerLinkDialog::initCheckbox(QCheckBox *box, bool enabled)
{
  box->setChecked(enabled);
  box->setEnabled(enabled);
}

void NeuroglancerLinkDialog::init(const ZDvidEnv &env)
{
  initCheckbox(ui->grayscaleCheckBox, env.hasData(ZDvidEnv::ERole::GRAYSCALE));
  initCheckbox(ui->segmentationCheckBox, env.hasData(ZDvidEnv::ERole::SEGMENTATION));
  initCheckbox(ui->synapseCheckBox, env.hasData(ZDvidEnv::ERole::SYNAPSES));
  initCheckbox(ui->bookmarkCheckBox, env.hasData(ZDvidEnv::ERole::BOOKMARKS));
}

