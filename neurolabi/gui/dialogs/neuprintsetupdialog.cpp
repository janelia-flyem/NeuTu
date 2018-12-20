#include "neuprintsetupdialog.h"
#include "ui_neuprintsetupdialog.h"

NeuprintSetupDialog::NeuprintSetupDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::NeuprintSetupDialog)
{
  ui->setupUi(this);
}

NeuprintSetupDialog::~NeuprintSetupDialog()
{
  delete ui;
}
