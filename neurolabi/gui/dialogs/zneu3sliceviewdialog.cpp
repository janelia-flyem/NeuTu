#include "zneu3sliceviewdialog.h"
#include "ui_zneu3sliceviewdialog.h"

ZNeu3SliceViewDialog::ZNeu3SliceViewDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZNeu3SliceViewDialog)
{
  ui->setupUi(this);
#if !defined(_USE_WEBENGINE_)
  ui->neuroglancerRadioButton->hide();
#endif
}

ZNeu3SliceViewDialog::~ZNeu3SliceViewDialog()
{
  delete ui;
}

Neu3Window::EBrowseMode ZNeu3SliceViewDialog::getBrowseMode() const
{
  if (ui->defaultRadioButton->isChecked()) {
    return Neu3Window::EBrowseMode::NATIVE;
  } else if (ui->neuroglancerRadioButton->isChecked()) {
    return Neu3Window::EBrowseMode::NEUROGLANCER;
  } else if (ui->extNeuroglancerRadioButton->isChecked()) {
    return Neu3Window::EBrowseMode::NEUROGLANCER_EXT;
  }

  return Neu3Window::EBrowseMode::NONE;
}
