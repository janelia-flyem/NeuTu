#include "neuprintdatasetdialog.h"
#include "ui_neuprintdatasetdialog.h"

NeuprintDatasetDialog::NeuprintDatasetDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::NeuprintDatasetDialog)
{
  ui->setupUi(this);
}

NeuprintDatasetDialog::~NeuprintDatasetDialog()
{
  delete ui;
}

void NeuprintDatasetDialog::setDatasetList(const QStringList &datasets)
{
  ui->datasetComboBox->clear();
  ui->datasetComboBox->addItems(datasets);
}

QString NeuprintDatasetDialog::getDataset() const
{
  return ui->datasetComboBox->currentText();
}
