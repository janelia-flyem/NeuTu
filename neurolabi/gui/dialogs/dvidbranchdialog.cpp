#include "dvidbranchdialog.h"
#include "ui_dvidbranchdialog.h"

DvidBranchDialog::DvidBranchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DvidBranchDialog)
{
    ui->setupUi(this);


    // UI connections



    // data load connections


    // populate first panel with server data from some static source;
    //  probably a file on disk for now





}

DvidBranchDialog::~DvidBranchDialog()
{
    delete ui;
}
