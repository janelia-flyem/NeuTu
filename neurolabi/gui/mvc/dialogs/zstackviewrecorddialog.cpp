#include "zstackviewrecorddialog.h"
#include "ui_zstackviewrecorddialog.h"

#include <QDir>

#include "zdialogfactory.h"
#include "mvc/zstackviewrecorder.h"

ZStackViewRecordDialog::ZStackViewRecordDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZStackViewRecordDialog)
{
  ui->setupUi(this);

  connect(ui->outputPushButton, SIGNAL(clicked()), this, SLOT(setOutput()));
}

ZStackViewRecordDialog::~ZStackViewRecordDialog()
{
  delete ui;
}

QString ZStackViewRecordDialog::getFullPrefix() const
{
  QString prefix = QDir(ui->outputLineEdit->text()).
      filePath(ui->prefixLineEdit->text());
  if (ui->prefixLineEdit->text().isEmpty()) {
    prefix += "/";
  }

  return prefix;
}

void ZStackViewRecordDialog::setOutput()
{
  QString saveFileDir = ZDialogFactory::GetDirectory(
        "Output", ui->outputLineEdit->text(), this);

  if (!saveFileDir.isEmpty()) {
    setOutputPath(saveFileDir);
  }
}

void ZStackViewRecordDialog::setOutputPath(const QString &path)
{
  ui->outputLineEdit->setText(path);
}

bool ZStackViewRecordDialog::isAuto() const
{
  return ui->autoCheckBox->isChecked();
}

void ZStackViewRecordDialog::configureRecorder(ZStackViewRecorder *recorder)
{
  if (recorder) {
    recorder->setPrefix(getFullPrefix());
    recorder->setMode(isAuto() ? ZStackViewRecorder::EMode::AUTO :
                                 ZStackViewRecorder::EMode::MANUAL);
  }
}
