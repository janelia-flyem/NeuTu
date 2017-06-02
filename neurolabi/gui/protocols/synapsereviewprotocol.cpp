#include "synapsereviewprotocol.h"
#include "ui_synapsereviewprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QMessageBox>

#include "synapsereviewinputdialog.h"

#include "zjsonobject.h"
#include "zjsonparser.h"

SynapseReviewProtocol::SynapseReviewProtocol(QWidget *parent) :
    ProtocolDialog(parent),
    ui(new Ui::SynapseReviewProtocol)
{
    ui->setupUi(this);


    // UI connections
    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));

    // misc UI setup
    ui->buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

}

// constants
const std::string SynapseReviewProtocol::KEY_VERSION = "version";
const int SynapseReviewProtocol::fileVersion = 1;

bool SynapseReviewProtocol::initialize() {

    // input dialog; not sure there are any initial
    //  values that make sense to set
    SynapseReviewInputDialog inputDialog;
    int ans = inputDialog.exec();
    if (ans == QDialog::Rejected) {
        return false;
    }



    // get the value and get started



    // save initial data
    saveState();

    return true;
}

void SynapseReviewProtocol::loadDataRequested(ZJsonObject data) {

    // check version of saved data here
    if (!data.hasKey(KEY_VERSION.c_str())) {
        ui->progressLabel->setText("No version info in saved data; data not loaded!");
        return;
    }
    int version = ZJsonParser::integerValue(data[KEY_VERSION.c_str()]);
    if (version > fileVersion) {
        ui->progressLabel->setText("Saved data is from a newer version of NeuTu; update NeuTu and try again!");
        return;
    }

    // convert old versions when it becomes necessary



    // test:
    std::cout << "in loadDataRequested()" << std::endl;
    std::cout << "json received: " << data.dumpString() << std::endl;

    // check version

    // read in data

    // do stuff

}

void SynapseReviewProtocol::saveState() {
    std::cout << "in saveState()" << std::endl;

    // testing:
    ZJsonObject data;

    data.setEntry("test", "value");

    // always version your output files!
    data.setEntry(KEY_VERSION.c_str(), fileVersion);

    emit requestSaveProtocol(data);

}

void SynapseReviewProtocol::onExitButton() {
    // exit protocol; can be reopened and worked on later
    emit protocolExiting();
}

void SynapseReviewProtocol::onCompleteButton() {
    QMessageBox mb;
    mb.setText("Complete protocol");
    mb.setInformativeText("When you complete the protocol, it will save and exit immediately.  You will not be able to reopen it.\n\nComplete protocol now?");
    mb.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    mb.setDefaultButton(QMessageBox::Cancel);
    int ans = mb.exec();

    if (ans == QMessageBox::Ok) {
        // save here if needed
        // saveState();
        emit protocolCompleting();
    }
}

SynapseReviewProtocol::~SynapseReviewProtocol()
{
    delete ui;
}

