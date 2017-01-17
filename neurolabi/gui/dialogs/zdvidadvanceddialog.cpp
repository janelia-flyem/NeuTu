#include "zdvidadvanceddialog.h"
#include "ui_zdvidadvanceddialog.h"

ZDvidAdvancedDialog::ZDvidAdvancedDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZDvidAdvancedDialog)
{
  ui->setupUi(this);
}

ZDvidAdvancedDialog::~ZDvidAdvancedDialog()
{
  delete ui;
}

void ZDvidAdvancedDialog::setDvidServer(const QString &str)
{
  ui->serverLabel->setText(str);
}
