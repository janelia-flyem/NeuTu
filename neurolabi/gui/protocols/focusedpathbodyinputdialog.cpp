#include "focusedpathbodyinputdialog.h"
#include "ui_focusedpathbodyinputdialog.h"

FocusedPathBodyInputDialog::FocusedPathBodyInputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FocusedPathBodyInputDialog)
{
    ui->setupUi(this);
}

uint64_t FocusedPathBodyInputDialog::getBodyID() {
    bool ok;
    uint64_t bodyID = ui->bodyIDInput->text().toLong(&ok);
    if (!ok) {
        // do something
    }
    return bodyID;
}

std::string FocusedPathBodyInputDialog::getEdgeInstance() {
    return ui->edgeInstanceInput->text().toStdString();
}

FocusedPathBodyInputDialog::~FocusedPathBodyInputDialog()
{
    delete ui;
}
