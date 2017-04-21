#include "zflyembodycomparisondialog.h"
#include "ui_zflyembodycomparisondialog.h"

ZFlyEmBodyComparisonDialog::ZFlyEmBodyComparisonDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmBodyComparisonDialog)
{
  ui->setupUi(this);
  updateSegInfo();
  connectSignalSlot();
}

ZFlyEmBodyComparisonDialog::~ZFlyEmBodyComparisonDialog()
{
  delete ui;
}

void ZFlyEmBodyComparisonDialog::connectSignalSlot()
{
  connect(ui->defaultRadioButton, SIGNAL(toggled(bool)),
          this, SLOT(updateSegInfo()));
  connect(ui->sameRadioButton, SIGNAL(toggled(bool)),
          this, SLOT(updateSegInfo()));
  connect(ui->customRadioButton, SIGNAL(toggled(bool)),
          this, SLOT(updateSegInfo()));
}

void ZFlyEmBodyComparisonDialog::updateSegInfo()
{
  QString info;
  if (ui->defaultRadioButton->isChecked()) {
    info = "Use default segmentaion name.";
  } else if (ui->sameRadioButton->isChecked()) {
    info = "Use the same segmentation name.";
  } else if (ui->customRadioButton) {
    info = "Input custom segmenation name below:";
  }

  ui->segHintLabel->setText(info);
  ui->segLineEdit->setVisible(ui->customRadioButton->isChecked());
}

std::string ZFlyEmBodyComparisonDialog::getUuid() const
{
  return ui->uuidWidget->getText().toStdString();
}

std::string ZFlyEmBodyComparisonDialog::getSegmentation() const
{
  return ui->segLineEdit->text().toStdString();
}

void ZFlyEmBodyComparisonDialog::setUuidList(const QStringList &stringList)
{
  ui->uuidWidget->setStringList(stringList);
}

void ZFlyEmBodyComparisonDialog::setUuidList(const std::vector<std::string> &stringList)
{
  ui->uuidWidget->setStringList(stringList);
}

void ZFlyEmBodyComparisonDialog::setCurrentUuidIndex(int index)
{
  ui->uuidWidget->setCurrentIndex(index);
}

bool ZFlyEmBodyComparisonDialog::usingCustomSegmentation() const
{
  return ui->customRadioButton->isChecked();
}

bool ZFlyEmBodyComparisonDialog::usingDefaultSegmentation() const
{
  return ui->defaultRadioButton->isChecked();
}

bool ZFlyEmBodyComparisonDialog::usingSameSegmentation() const
{
  return ui->sameRadioButton->isChecked();
}
