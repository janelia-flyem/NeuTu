#include "zflyembodyscreenshotdialog.h"

#include <QFileDialog>

#include "ui_zflyembodyscreenshotdialog.h"
#include "zflyemutilities.h"

ZFlyEmBodyScreenshotDialog::ZFlyEmBodyScreenshotDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmBodyScreenshotDialog)
{
  ui->setupUi(this);
  connectSignalSlot();
}

ZFlyEmBodyScreenshotDialog::~ZFlyEmBodyScreenshotDialog()
{
  delete ui;
}

void ZFlyEmBodyScreenshotDialog::connectSignalSlot()
{
  connect(ui->inputPushButton, SIGNAL(clicked()), this, SLOT(setInput()));
  connect(ui->outputPushButton, SIGNAL(clicked()), this, SLOT(setOutput()));
}

int ZFlyEmBodyScreenshotDialog::getFrameWidth()
{
  return ui->widthSpinBox->value();
}

int ZFlyEmBodyScreenshotDialog::getFrameHeight()
{
  return ui->heightSpinBox->value();
}

void ZFlyEmBodyScreenshotDialog::setFrameWidth(int width)
{
  ui->widthSpinBox->setValue(width);
}

void ZFlyEmBodyScreenshotDialog::setFrameHeight(int height)
{
  ui->heightSpinBox->setValue(height);
}

QString ZFlyEmBodyScreenshotDialog::getInputPath() const
{
  return ui->inputLineEdit->text();
}

QString ZFlyEmBodyScreenshotDialog::getOutputPath() const
{
  return ui->outputLineEdit->text();
}

void ZFlyEmBodyScreenshotDialog::setInputPath(const QString &path)
{
  ui->inputLineEdit->setText(path);
}

void ZFlyEmBodyScreenshotDialog::setOutputPath(const QString &path)
{
  ui->outputLineEdit->setText(path);
}

void ZFlyEmBodyScreenshotDialog::setInput()
{
  QString fileName =
      QFileDialog::getOpenFileName(this, tr("Load Body List"),
                                   getInputPath(),
                                   tr("Body files (*.txt)"),
                                   NULL/*, QFileDialog::DontUseNativeDialog*/);
  if (!fileName.isEmpty()) {
    setInputPath(fileName);
  }
}

void ZFlyEmBodyScreenshotDialog::setOutput()
{
  QString saveFileDir =
      QFileDialog::getExistingDirectory(this, tr("Output"),
                                        getOutputPath(),
                                        QFileDialog::ShowDirsOnly/* |
                                        QFileDialog::DontUseNativeDialog*/);

  if (!saveFileDir.isEmpty()) {
    setOutputPath(saveFileDir);
  }
}

std::vector<uint64_t> ZFlyEmBodyScreenshotDialog::getBodyIdArray() const
{
  std::vector<uint64_t> bodyArray;
  std::set<uint64_t> bodySet = FlyEm::LoadBodySet(getInputPath().toStdString());
  bodyArray.insert(bodyArray.end(), bodySet.begin(), bodySet.end());

  return bodyArray;
}
