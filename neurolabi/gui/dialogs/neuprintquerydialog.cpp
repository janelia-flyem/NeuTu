#include "neuprintquerydialog.h"
#include "ui_neuprintquerydialog.h"

NeuPrintQueryDialog::NeuPrintQueryDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::NeuPrintQueryDialog)
{
  ui->setupUi(this);
}

NeuPrintQueryDialog::~NeuPrintQueryDialog()
{
  delete ui;
}

QList<QString> NeuPrintQueryDialog::getInputRoi() const
{
  QList<QString> inputRoiList;
  inputRoiList.append(ui->inputRoiLineEdit->text());

  return inputRoiList;
}

QList<QString> NeuPrintQueryDialog::getOutputRoi() const
{
  QList<QString> outputRoiList;
  outputRoiList.append(ui->outputRoiLineEdit->text());

  return outputRoiList;
}
