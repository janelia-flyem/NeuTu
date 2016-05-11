#include "protocoldialog.h"
#include "ui_protocoldialog.h"

#include <iostream>
#include <stdlib.h>

#include <QInputDialog>


/*
 * this is the base class for all protocols; it's also the
 * "do N things" test protocol
 */
ProtocolDialog::ProtocolDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProtocolDialog)
{
    ui->setupUi(this);

    // UI connections:


    // misc UI setup
    ui->buttonBox->button(QDialogButtonBox::Close)->setDefault(true);
    
}

void ProtocolDialog::initialize() {

    // ask user how many things to do; test proxy for
    //  asking the user for a filename or something

    bool ok;
    int n = QInputDialog::getInt(this, "How many things?",
        "How many things", 5, 1, 100, 1, &ok);
    if (ok) {
        setNThings(n);
    } else {
        setNThings(5);
    }

    std::cout << "prdia: nThings = " << getNThings() << std::endl;

    // generate pending/finished lists
    // save state to dvid
    // set current item; update current item label
    // update progress label


}

void ProtocolDialog::setNThings(int nThings) {
    this->m_nThings = nThings;
}

int ProtocolDialog::getNThings() {
    return m_nThings;
}

ProtocolDialog::~ProtocolDialog()
{
    delete ui;
}
