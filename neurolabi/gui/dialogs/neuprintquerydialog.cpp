#include "neuprintquerydialog.h"
#include "ui_neuprintquerydialog.h"

NeuPrintQueryDialog::NeuPrintQueryDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::NeuPrintQueryDialog)
{
  ui->setupUi(this);
  ui->inputRoiWidget->setName("Input ROIs");
  ui->outputRoiWidget->setName("Output ROIs");
}

NeuPrintQueryDialog::~NeuPrintQueryDialog()
{
  delete ui;
}

void NeuPrintQueryDialog::setRoiList(const QStringList roiList)
{
  ui->inputRoiWidget->setOptionList(roiList);
  ui->outputRoiWidget->setOptionList(roiList);
}

QList<QString> NeuPrintQueryDialog::getInputRoi() const
{
  return ui->inputRoiWidget->getSelectedOptionList();
}

QList<QString> NeuPrintQueryDialog::getOutputRoi() const
{
  return ui->outputRoiWidget->getSelectedOptionList();
}
