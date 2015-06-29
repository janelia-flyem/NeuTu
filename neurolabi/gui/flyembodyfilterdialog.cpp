#include "flyembodyfilterdialog.h"

#include "ui_flyembodyfilterdialog.h"
#include "zstring.h"
#include "zdialogfactory.h"

FlyEmBodyFilterDialog::FlyEmBodyFilterDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmBodyFilterDialog)
{
  ui->setupUi(this);
  connect(ui->maxBodySizeCheckBox, SIGNAL(toggled(bool)),
          ui->maxSizeSpinBox, SLOT(setEnabled(bool)));
  connect(ui->minBodySizeCheckBox, SIGNAL(toggled(bool)),
          ui->minSizeSpinBox, SLOT(setEnabled(bool)));
  connect(ui->bodyFilePushButton, SIGNAL(clicked()),
          this, SLOT(setBodyListFile()));
}

FlyEmBodyFilterDialog::~FlyEmBodyFilterDialog()
{
  delete ui;
}

void FlyEmBodyFilterDialog::setBodyListFile(const QString path)
{
  ui->bodyFileLineEdit->setText(path);
}

void FlyEmBodyFilterDialog::setBodyListFile()
{
  QString fileName = ZDialogFactory::GetOpenFileName("Load Body List", "", this);
  if (!fileName.isEmpty()) {
    setBodyListFile(fileName);
  }
}

size_t FlyEmBodyFilterDialog::getMinBodySize() const
{
  if (ui->minSizeSpinBox->value() < 0 ||
      !ui->minBodySizeCheckBox->isChecked()) {
    return 0;
  }

  return ui->minSizeSpinBox->value();
}

size_t FlyEmBodyFilterDialog::getMaxBodySize() const
{
  if (ui->maxSizeSpinBox->value() < 0) {
    return 0;
  }

  return ui->maxSizeSpinBox->value();
}

bool FlyEmBodyFilterDialog::hasUpperBodySize() const
{
  if (ui->maxSizeSpinBox->value() < 0) {
    return false;
  }

  return ui->maxBodySizeCheckBox->isChecked();
}

std::vector<int> FlyEmBodyFilterDialog::getExcludedBodies() const
{
  ZString str =  ui->excludedBodyLineEdit->text().toStdString();
  return str.toIntegerArray();
}

std::set<int> FlyEmBodyFilterDialog::getExcludedBodySet() const
{
  std::vector<int> bodyArray = getExcludedBodies();
  std::set<int> bodySet;
  bodySet.insert(bodyArray.begin(), bodyArray.end());

  return bodySet;
}

QString FlyEmBodyFilterDialog::getBodyListFile() const
{
  return ui->bodyFileLineEdit->text();
}

ZDvidFilter FlyEmBodyFilterDialog::getDvidFilter() const
{
  ZDvidFilter filter;
  filter.setMinBodySize(getMinBodySize());
  filter.setMaxBodySize(getMaxBodySize());
  filter.setUpperBodySizeEnabled(hasUpperBodySize());
  filter.exclude(getExcludedBodies());
  filter.setNamedBodyOnly(namedBodyOnly());
  filter.setBodyListFile(getBodyListFile().toStdString());

  return filter;
}

bool FlyEmBodyFilterDialog::namedBodyOnly() const
{
  return ui->namedBodyCheckBox->isChecked();
}
