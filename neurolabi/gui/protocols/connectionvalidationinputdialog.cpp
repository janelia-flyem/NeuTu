#include "connectionvalidationinputdialog.h"
#include "ui_connectionvalidationinputdialog.h"

ConnectionValidationInputDialog::ConnectionValidationInputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionValidationInputDialog)
{
    ui->setupUi(this);
}

ConnectionValidationInputDialog::~ConnectionValidationInputDialog()
{
    delete ui;
}
