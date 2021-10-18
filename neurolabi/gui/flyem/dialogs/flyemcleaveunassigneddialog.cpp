#include "flyemcleaveunassigneddialog.h"

#include <QPushButton>

#include "ui_flyemcleaveunassigneddialog.h"


FlyEmCleaveUnassignedDialog::FlyEmCleaveUnassignedDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmCleaveUnassignedDialog)
{
  ui->setupUi(this);

  ui->noneRadioButton->hide();
  connect(ui->mainRadioButton, SIGNAL(clicked(bool)),
          this, SLOT(updateWidget()));
  connect(ui->shatterRadioButton, SIGNAL(clicked(bool)),
          this, SLOT(updateWidget()));

  updateWidget();
}

FlyEmCleaveUnassignedDialog::~FlyEmCleaveUnassignedDialog()
{
  delete ui;
}

FlyEmCleaveUnassignedDialog::EOption FlyEmCleaveUnassignedDialog::getOption() const
{
  if (ui->mainRadioButton->isChecked()) {
    return EOption::MAIN_BODY;
  } else if (ui->shatterRadioButton->isChecked()) {
    return EOption::NEW_BODY;
  }

  return EOption::NONE;
}

void FlyEmCleaveUnassignedDialog::updateWidget()
{
  if (m_unassignedCount == 0) {
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->description->setText(
          "<p><font color='red'>Something wrong happened.</font></p>"
          "<p>You can contact the developer for troubleshooting.</p>");
    ui->mainRadioButton->setEnabled(false);
    ui->shatterRadioButton->setEnabled(false);
  } else {
    ui->mainRadioButton->setEnabled(true);
    ui->shatterRadioButton->setEnabled(true);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
          getOption() != EOption::NONE);
    ui->description->setText(
          QString(
            "<p><font color='#896403'><b>%1</b> %2 NOT assigned (gray color).</font></p>"
            "<p>Please select one of the opitions below to proceed</p>")
          .arg(m_unassignedCount)
          .arg(m_unassignedCount == 1 ? "supervoxel is" : "supervoxels are"));
  }
}

void FlyEmCleaveUnassignedDialog::setUnassignedCount(size_t n)
{
  m_unassignedCount = n;
  updateWidget();
}

void FlyEmCleaveUnassignedDialog::setIndexNotCleavedOff(size_t index)
{
  m_indexNotCleavedOff = index;
  updateWidget();
}

void FlyEmCleaveUnassignedDialog::reset()
{
  setUnassignedCount(0);
  setIndexNotCleavedOff(0);
  ui->noneRadioButton->setChecked(true);
  updateWidget();
}
