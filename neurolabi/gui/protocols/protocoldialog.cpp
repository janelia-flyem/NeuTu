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

    m_protocolStatus = PROTOCOL_INCOMPLETE;

    // generate pending/finished lists
    m_finishedList = QStringList();
    m_pendingList = QStringList();
    for (int i=0; i<n; i++) {
        m_pendingList << QString("Thing %1").arg(i);
    }

    // go to first item
    onFirstButton();
    saveState();

    return true;
}

void ProtocolDialog::onFirstButton() {
    if (m_pendingList.size() > 0) {
        m_currentItem = m_pendingList.first();
    } else {
        m_currentItem = "";
    }

    updateLabels();
}

void ProtocolDialog::onSkipButton() {
    gotoNextItem();
    updateLabels();
}

void ProtocolDialog::onDoButton() {
    if (m_currentItem != "") {
        QString finishedItem = QString(m_currentItem);

        // execute go to next; happens before list
        //  manipulation so we can keep order straight;
        //  this updates m_currentItem, too
        gotoNextItem();

        m_pendingList.removeAt(m_pendingList.indexOf(finishedItem));
        m_finishedList.append(finishedItem);

        if (m_pendingList.size() == 0) {
            m_currentItem = "";
        }

        saveState();

        updateLabels();
    }
}

void ProtocolDialog::onCompleteButton() {
    // complete = mark protocol as finished; does not
    //  exit, but once exited, can't be reopened

    // change status
    // save completed data
    // remove save incomplete (currently I plan diff.
    //  keys in dvid for complete and incomplete
    // confirmation dialog?  "you're done, go home?"

    std::cout << "prdia: complete button clicked" << std::endl;
}

void ProtocolDialog::onExitButton() {
    // exit protocol; can be reopened and worked on later
    emit protocolExiting();
}

void ProtocolDialog::gotoNextItem() {
    std::cout << "prdia: go to next item" << std::endl;

    if (m_pendingList.size() == 0) {
        m_currentItem = "";
    } else if (m_currentItem == "") {
        m_currentItem = m_pendingList.first();
    } else if (m_currentItem == m_pendingList.last()) {
        m_currentItem = m_pendingList.first();
    } else {
        m_currentItem = m_pendingList.at(m_pendingList.indexOf(m_currentItem) + 1);
    }

    // do not update labels here!  calling routine
    //  may still be messing with internal lists
}

void ProtocolDialog::saveState() {
    // save state of protocol to DVID

    std::cout << "prdia: pretending to save state to dvid" << std::endl;

}

void ProtocolDialog::updateLabels() {
    // unified update of labels because for this protocol, they are
    //  both very fast

    // current item
    ui->currentItemLabel->setText(QString("Current: %1").arg(m_currentItem));

    // progress, in form: "Progress:  #/# (#%)"
    int nFinished = m_finishedList.size();
    int nTotal = m_pendingList.size() + nFinished;
    float percent = (100.0 * nFinished) / nTotal;
    ui->progressLabel->setText(QString("Progress: %1 / %2 (%3%)").arg(nFinished).arg(nTotal).arg(percent, 4, 'f', 1));
}

ProtocolDialog::~ProtocolDialog()
{
    delete ui;
}
