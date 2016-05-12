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
    connect(ui->firstButton, SIGNAL(clicked(bool)), this, SLOT(onFirstButton()));
    connect(ui->doButton, SIGNAL(clicked(bool)), this, SLOT(onDoButton()));
    connect(ui->skipButton, SIGNAL(clicked(bool)), this, SLOT(onSkipButton()));
    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));
    connect(ui->gotoButton, SIGNAL(clicked(bool)), this, SLOT(onGotoButton()));


    // misc UI setup
    ui->buttonBox->button(QDialogButtonBox::Close)->setDefault(true);
    
}

/*
 * start the protocol anew; returns success status
 */
bool ProtocolDialog::initialize() {

    // ask user how many things to do; test proxy for
    //  asking the user for a filename or something

    bool ok;
    int n = QInputDialog::getInt(this, "How many things?",
        "How many things", 5, 1, 100, 1, &ok);
    if (ok) {
        setNThings(n);
    } else {
        return false;
    }

    std::cout << "prdia: nThings = " << getNThings() << std::endl;

    // generate pending/finished lists
    // set current item
    // remove nThings as attribute (don't need after lists generated)!
    // save state to dvid
    // update current item label
    // update progress label


    return true;

}

void ProtocolDialog::onFirstButton() {
    std::cout << "prdia: first button clicked" << std::endl;
}

void ProtocolDialog::onSkipButton() {
    std::cout << "prdia: skip button clicked" << std::endl;
}

void ProtocolDialog::onDoButton() {
    std::cout << "prdia: do button clicked" << std::endl;
}

void ProtocolDialog::onCompleteButton() {
    // complete = mark protocol as finished; does not
    //  exit, but once exited, can't be reopened

    std::cout << "prdia: complete button clicked" << std::endl;
}

void ProtocolDialog::onExitButton() {
    // exit = save and exit protocol; can be reopened and
    //  worked on later

    // save?  probably not, should already have state saved

    std::cout << "prdia: exit button clicked" << std::endl;

    emit protocolExiting();

}

void ProtocolDialog::onGotoButton() {
    std::cout << "prdia: go to button clicked" << std::endl;
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
