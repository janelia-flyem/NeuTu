#include "informationdialog.h"

#include <QPushButton>

#include "ui_informationdialog.h"

InformationDialog::InformationDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::InformationDialog)
{
  ui->setupUi(this);
}

InformationDialog::~InformationDialog()
{
  delete ui;
}

void InformationDialog::setText(const char *text)
{
  setText(QString(text));
}

void InformationDialog::setText(const std::string &text)
{
  setText(QString::fromStdString(text));
}

void InformationDialog::setText(const QString &text)
{
  ui->textEdit->setText(text);
}
