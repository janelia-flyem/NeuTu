#include "zcomboeditdialog.h"
#include "ui_zcomboeditdialog.h"

ZComboEditDialog::ZComboEditDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZComboEditDialog)
{
  ui->setupUi(this);
}

ZComboEditDialog::~ZComboEditDialog()
{
  delete ui;
}

QString ZComboEditDialog::getText() const
{
  return ui->editWidget->getText();
}

void ZComboEditDialog::setStringList(const QStringList &stringList)
{
  ui->editWidget->setStringList(stringList);
}

void ZComboEditDialog::setStringList(const std::vector<std::string> &stringList)
{
  ui->editWidget->setStringList(stringList);
}

void ZComboEditDialog::setCurrentIndex(int index)
{
  ui->editWidget->setCurrentIndex(index);
}
