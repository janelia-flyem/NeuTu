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
    if (!ok) {
        return false;
    }


    // generate pending/finished lists
    m_finishedList = QStringList();
    m_pendingList = QStringList();
    for (int i=0; i<n; i++) {
        m_pendingList << QString("Thing %1").arg(i);
    }


    // set current item
    // save state to dvid
    // update current item label

    // update progress label
    updateProgressLabel();

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

    // change status
    // save completed data
    // remove save incomplete (currently I plan diff.
    //  keys in dvid for complete and incomplete

    std::cout << "prdia: complete button clicked" << std::endl;
}

void ProtocolDialog::onExitButton() {
    // exit = save and exit protocol; can be reopened and
    //  worked on later
    emit protocolExiting();
}

void ProtocolDialog::onGotoButton() {
    std::cout << "prdia: go to button clicked" << std::endl;
}

void ProtocolDialog::updateProgressLabel() {
    // Progress:  #/# (#%)
    int nFinished = m_finishedList.size();
    int nTotal = m_pendingList.size() + nFinished;
    float percent = (float) nFinished / nTotal;
    ui->progressLabel->setText(QString("Progress: %1 / %2 (%3%)").arg(nFinished).arg(nTotal).arg(percent));
}

ProtocolDialog::~ProtocolDialog()
{
    delete ui;
}
