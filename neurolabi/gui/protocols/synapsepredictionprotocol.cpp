#include "synapsepredictionprotocol.h"
#include "ui_synapsepredictionprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QInputDialog>
#include <QMessageBox>

#include "zjsonarray.h"
#include "zjsonobject.h"
#include "zjsonparser.h"

SynapsePredictionProtocol::SynapsePredictionProtocol(QWidget *parent) :
    ui(new Ui::SynapsePredictionProtocol),
    ProtocolDialog(parent)
{
    ui->setupUi(this);

    // UI connections:
    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));


    // misc UI setup
    ui->buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

}

// protocol name should not contain hyphens
const std::string SynapsePredictionProtocol::PROTOCOL_NAME = "synapse_prediction";

/*
 * start the protocol anew; returns success status;
 * initialize is also expected to do the first save
 * of the protocol's data
 */
bool SynapsePredictionProtocol::initialize() {


    return true;
}

std::string SynapsePredictionProtocol::getName() {
    return PROTOCOL_NAME;
}

void SynapsePredictionProtocol::saveState() {

    // save stuff


    // emit requestSaveProtocol(data);
}

void SynapsePredictionProtocol::loadDataRequested(ZJsonObject data) {

    // do stuff

}

void SynapsePredictionProtocol::onCompleteButton() {
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

void SynapsePredictionProtocol::onExitButton() {
    // exit protocol; can be reopened and worked on later
    emit protocolExiting();
}


void SynapsePredictionProtocol::gotoNextItem() {

    // do stuff
}

void SynapsePredictionProtocol::updateLabels() {
    // might want to separate update current label from update progress?
    // if both are fast, don't need to

}

SynapsePredictionProtocol::~SynapsePredictionProtocol()
{
    delete ui;
}
