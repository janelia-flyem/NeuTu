#include "flyembodyfilterdialog.h"
#include "ui_flyembodyfilterdialog.h"
#include "zstring.h"

FlyEmBodyFilterDialog::FlyEmBodyFilterDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmBodyFilterDialog)
{
  ui->setupUi(this);
  connect(ui->maxBodySizeCheckBox, SIGNAL(toggled(bool)),
          ui->maxSizeSpinBox, SLOT(setEnabled(bool)));
  connect(ui->minBodySizeCheckBox, SIGNAL(toggled(bool)),
          ui->minSizeSpinBox, SLOT(setEnabled(bool)));
}

FlyEmBodyFilterDialog::~FlyEmBodyFilterDialog()
{
  delete ui;
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

ZDvidFilter FlyEmBodyFilterDialog::getDvidFilter() const
{
  ZDvidFilter filter;
  filter.setMinBodySize(getMinBodySize());
  filter.setMaxBodySize(getMaxBodySize());
  filter.setUpperBodySizeEnabled(hasUpperBodySize());
  filter.exclude(getExcludedBodies());

  return filter;
}
