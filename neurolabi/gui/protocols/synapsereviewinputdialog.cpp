#include "synapsereviewinputdialog.h"
#include "ui_synapsereviewinputdialog.h"

SynapseReviewInputDialog::SynapseReviewInputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SynapseReviewInputDialog)
{
    ui->setupUi(this);
}

SynapseReviewInputDialog::~SynapseReviewInputDialog()
{
    delete ui;
}
