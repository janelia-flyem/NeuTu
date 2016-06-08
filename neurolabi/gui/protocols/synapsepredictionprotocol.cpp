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
    connect(ui->firstButton, SIGNAL(clicked(bool)), this, SLOT(onFirstButton()));
    connect(ui->markButton, SIGNAL(clicked(bool)), this, SLOT(onMarkedButton()));
    connect(ui->skipButton, SIGNAL(clicked(bool)), this, SLOT(onSkipButton()));
    connect(ui->gotoButton, SIGNAL(clicked(bool)), this, SLOT(onGotoButton()));
    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));


    // misc UI setup
    ui->buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

}

// protocol name should not contain hyphens
const std::string SynapsePredictionProtocol::PROTOCOL_NAME = "synapse_prediction";
const std::string SynapsePredictionProtocol::KEY_FINISHED = "finished";
const std::string SynapsePredictionProtocol::KEY_PENDING = "pending";


/*
 * start the protocol anew; returns success status;
 * initialize is also expected to do the first save
 * of the protocol's data
 */
bool SynapsePredictionProtocol::initialize() {


    // testing
    bool ok;
    int n = QInputDialog::getInt(this, "How many things?",
        "How many things", 5, 1, 100, 1, &ok);
    if (!ok) {
        return false;
    }

    // needs a custom dialog, ugh
    // two entry fields for corners of volume
    //  accept comma or whitespace delimited ints
    // entry field for name of RoI
    //  in perfect world, this will be a drop-down and you'll
    //  choose from existing RoIs in DVID



    // generate pending/finished lists from user input

    // considering leaving lists as ZJsonArrays rather than
    //  convert back and forth for each save

    // DVID call exists in ZDvidReader for read synapse given volume
    // will need to filter by RoI; raw DVID call to get "in RoI"
    //  status for list of points exists
    // given synapse list, need to split out pre/post sites
    // might need to filter on something (auto vs user placed?)
    // then arrange list in some appropriate way:
    //  -- pre then post or the other way around?
    //  -- cluster spatially?



    // go to first item
    onFirstButton();
    saveState();

    return true;
}

std::string SynapsePredictionProtocol::getName() {
    return PROTOCOL_NAME;
}

void SynapsePredictionProtocol::onFirstButton() {
    std::cout << "SynapsePredictionProtocol::onFirstButton" << std::endl;
}

void SynapsePredictionProtocol::onMarkedButton() {
    std::cout << "SynapsePredictionProtocol::onMarkedButton" << std::endl;

}

void SynapsePredictionProtocol::onSkipButton() {
    std::cout << "SynapsePredictionProtocol::onSkipButton" << std::endl;

}

void SynapsePredictionProtocol::onGotoButton() {
    std::cout << "SynapsePredictionProtocol::onGotoButton" << std::endl;

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

void SynapsePredictionProtocol::saveState() {
    // json save format: {"pending": [[x, y, z], [x2, y2, z2], ...],
    //                    "finished": similar list}

    ZJsonObject data;


    emit requestSaveProtocol(data);
}

void SynapsePredictionProtocol::loadDataRequested(ZJsonObject data) {

    std::cout << "SynapsePredictionProtocol::loadDataRequested" << std::endl;


}

void SynapsePredictionProtocol::updateLabels() {
    // might want to separate update current label from update progress?
    // if both are fast, don't need to

}

SynapsePredictionProtocol::~SynapsePredictionProtocol()
{
    delete ui;
}
