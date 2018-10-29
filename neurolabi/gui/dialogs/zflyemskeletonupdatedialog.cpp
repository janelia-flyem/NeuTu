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

void ZFlyEmSkeletonUpdateDialog::setMode(EMode mode)
{
  m_mode = mode;
  updateWidget();
}

void ZFlyEmSkeletonUpdateDialog::updateWidget()
{
  switch(m_mode) {
  case EMode::SELECTED:
    ui->topCountLabel->hide();
    ui->topCountSpinBox->hide();
    break;
  case EMode::TOP:
    ui->topCountLabel->setVisible(true);
    ui->topCountSpinBox->setVisible(true);
    break;
  }
}

int ZFlyEmSkeletonUpdateDialog::getTopCount() const
{
  return ui->topCountSpinBox->value();
}
