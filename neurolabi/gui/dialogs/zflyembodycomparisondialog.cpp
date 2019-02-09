#include "zflyembodycomparisondialog.h"
#include "ui_zflyembodycomparisondialog.h"
#include "geometry/zintpoint.h"
#include "zglobal.h"
#include "zstring.h"

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
  connect(ui->pastePushButton, SIGNAL(clicked()),
          this, SLOT(pastePosition()));
}

void ZFlyEmBodyComparisonDialog::updateSegInfo()
{
  QString info;
  if (ui->defaultRadioButton->isChecked()) {
    info = "Use the default segmentaion name specified in DVID.";
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

void ZFlyEmBodyComparisonDialog::clearPosition()
{
  ui->positionLineEdit->clear();
}

void ZFlyEmBodyComparisonDialog::pastePosition()
{
  ZIntPoint pt = ZGlobal::GetInstance().getStackPosition();
  if (pt.isValid()) {
    ui->positionLineEdit->setText(pt.toString().c_str());
  }
}

ZIntPoint ZFlyEmBodyComparisonDialog::getPosition() const
{
  ZIntPoint pt;
  pt.invalidate();

  ZString str = ui->positionLineEdit->text().toStdString();
  std::vector<int> coords = str.toIntegerArray();
  if (coords.size() == 3) {
    pt.set(coords[0], coords[1], coords[2]);
  }

  return pt;
}
