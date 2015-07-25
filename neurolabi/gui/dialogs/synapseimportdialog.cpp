#include "synapseimportdialog.h"
#include "ui_synapseimportdialog.h"

SynapseImportDialog::SynapseImportDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SynapseImportDialog)
{
  ui->setupUi(this);
}

SynapseImportDialog::~SynapseImportDialog()
{
  delete ui;
}

int SynapseImportDialog::getSynapseSelection() const
{
  if (ui->allRadioButton->isChecked()) {
    return 0;
  }

  if (ui->tbarRadioButton->isChecked()) {
    return 1;
  }

  if (ui->psdRadioButton->isChecked()) {
    return 2;
  }

  return -1;
}
