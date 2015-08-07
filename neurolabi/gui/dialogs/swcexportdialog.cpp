#include "swcexportdialog.h"
#include "ui_swcexportdialog.h"
#include "zdialogfactory.h"

SwcExportDialog::SwcExportDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SwcExportDialog)
{
  ui->setupUi(this);
}

SwcExportDialog::~SwcExportDialog()
{
  delete ui;
}

QString SwcExportDialog::getSavePath() const
{
  return ui->pathLineEdit->text();
}

ZFlyEmCoordinateConverter::ESpace SwcExportDialog::getCoordSpace() const
{
  ZFlyEmCoordinateConverter::ESpace space =
      ZFlyEmCoordinateConverter::IMAGE_SPACE;
  if (ui->physicalSpaceRadioButton->isChecked()) {
    space = ZFlyEmCoordinateConverter::PHYSICAL_SPACE;
  }

  return space;
}

void SwcExportDialog::on_pathPushButton_clicked()
{
  QString saveDir =
      ZDialogFactory::GetDirectory("SWC Export Directory", getSavePath(), this);
  if (!saveDir.isEmpty()) {
    ui->pathLineEdit->setText(saveDir);
  }
}
