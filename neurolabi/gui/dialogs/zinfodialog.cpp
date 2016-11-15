#include "zinfodialog.h"
#include "ui_zinfodialog.h"

ZInfoDialog::ZInfoDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZInfoDialog)
{
  ui->setupUi(this);
  ui->infoWidget->setReadOnly(true);
}

ZInfoDialog::~ZInfoDialog()
{
  delete ui;
}

void ZInfoDialog::setText(const QString &text)
{
  ui->infoWidget->setPlainText(text);
}
