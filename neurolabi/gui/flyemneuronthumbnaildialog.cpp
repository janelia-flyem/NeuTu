#include "flyemneuronthumbnaildialog.h"
#include "ui_flyemneuronthumbnaildialog.h"

#include <QDir>

FlyEmNeuronThumbnailDialog::FlyEmNeuronThumbnailDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmNeuronThumbnailDialog)
{
  ui->setupUi(this);
}

FlyEmNeuronThumbnailDialog::~FlyEmNeuronThumbnailDialog()
{
  delete ui;
}

void FlyEmNeuronThumbnailDialog::setOutputDirectory(
    const QString &path, const QString &subdir)
{
  QFileInfo fileInfo(path);
  ui->outputLineEdit->setText(fileInfo.absoluteDir().absoluteFilePath(subdir));
}

QString FlyEmNeuronThumbnailDialog::getOutputDirectory()
{
  return ui->outputLineEdit->text();
}

int FlyEmNeuronThumbnailDialog::getXIntv()
{
  return ui->xDsSpinBox->value();
}

int FlyEmNeuronThumbnailDialog::getYIntv()
{
  return ui->yDsSpinBox->value();
}

int FlyEmNeuronThumbnailDialog::getZIntv()
{
  return ui->zDsSpinBox->value();
}

bool FlyEmNeuronThumbnailDialog::updatingDataBundle()
{
  return ui->updateCheckBox->isChecked();
}
