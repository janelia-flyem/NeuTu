#include "doNthingsprotocol.h"
#include "ui_doNthingsprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QInputDialog>
#include <QMessageBox>

#include "zjsonarray.h"
#include "zjsonobject.h"
#include "zjsonparser.h"

/*
 * this is a test protocol: "do N things"
 */
DoNThingsProtocol::DoNThingsProtocol(QWidget *parent) : ProtocolDialog(parent),
    ui(new Ui::DoNThingsProtocol)
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

// protocol name should not contain hyphens
const std::string DoNThingsProtocol::KEY_FINISHED = "finished";
const std::string DoNThingsProtocol::KEY_PENDING = "pending";

/*
 * start the protocol anew; returns success status;
 * initialize is also expected to do the first save
 * of the protocol's data
 */
bool DoNThingsProtocol::initialize() {

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

    // go to first item
    onFirstButton();
    saveState();

    return true;
}

void DoNThingsProtocol::onFirstButton() {
    if (m_pendingList.size() > 0) {
        m_currentItem = m_pendingList.first();
    } else {
        m_currentItem = "";
    }

    updateLabels();
}

void DoNThingsProtocol::onSkipButton() {
    gotoNextItem();
    updateLabels();
}

void DoNThingsProtocol::onDoButton() {
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

void DoNThingsProtocol::onCompleteButton() {
    QMessageBox mb;
    mb.setText("Complete protocol");
    mb.setInformativeText("When you complete the protocol, it will save and exit immediately.  You will not be able to reopen it.\n\nComplete protocol now?");
    mb.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    mb.setDefaultButton(QMessageBox::Cancel);
    int ans = mb.exec();

    if (ans == QMessageBox::Ok) {
        saveState();
        emit protocolCompleting();
    }
}

void DoNThingsProtocol::onExitButton() {
    // exit protocol; can be reopened and worked on later
    emit protocolExiting();
}

void DoNThingsProtocol::gotoNextItem() {
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

void DoNThingsProtocol::saveState() {
    // json save format: {"pending": ["thing 1", "thing 3", ...],
    //                    "finished": ["thing 0", ...]}

    ZJsonArray pending;
    for (int i=0; i<m_pendingList.size(); i++) {
        pending.append(m_pendingList.at(i).toStdString());
    }
    ZJsonArray finished;
    for (int i=0; i<m_finishedList.size(); i++) {
        finished.append(m_finishedList.at(i).toStdString());
    }

    ZJsonObject data;
    data.setEntry(KEY_PENDING.c_str(), pending);
    data.setEntry(KEY_FINISHED.c_str(), finished);
    emit requestSaveProtocol(data);
}

void DoNThingsProtocol::loadDataRequested(ZJsonObject data) {
    if (!data.hasKey(KEY_FINISHED.c_str()) || !data.hasKey(KEY_PENDING.c_str())) {
        // how to communicate failure?  overwrite a label?
        ui->progressLabel->setText("Data could not be loaded from DVID!");
        return;
    }

    m_pendingList = QStringList();
    ZJsonArray pending = ZJsonArray(data.value(KEY_PENDING.c_str()));
    for (size_t i=0; i<pending.size(); i++) {
        m_pendingList << QString::fromUtf8(ZJsonParser::stringValue(pending.at(i)));
    }

    m_finishedList = QStringList();
    ZJsonArray finished = ZJsonArray(data.value(KEY_FINISHED.c_str()));
    for (size_t i=0; i<finished.size(); i++) {
        m_finishedList << QString::fromUtf8(ZJsonParser::stringValue(finished.at(i)));
    }

    onFirstButton();
}

void DoNThingsProtocol::updateLabels() {
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

DoNThingsProtocol::~DoNThingsProtocol()
{
    delete ui;
}
