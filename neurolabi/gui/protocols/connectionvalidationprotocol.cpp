#include "connectionvalidationprotocol.h"
#include "ui_connectionvalidationprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QMessageBox>

#include "connectionvalidationinputdialog.h"

#include "zjsonobject.h"
#include "zjsonparser.h"

ConnectionValidationProtocol::ConnectionValidationProtocol(QWidget *parent) :
    ProtocolDialog(parent),
    ui(new Ui::ConnectionValidationProtocol)
{
    ui->setupUi(this);

    // UI connections
    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));



}

// constants
const std::string ConnectionValidationProtocol::KEY_VERSION = "version";
const int ConnectionValidationProtocol::fileVersion = 1;

bool ConnectionValidationProtocol::initialize() {
    ConnectionValidationInputDialog dialog;
    int ans = dialog.exec();
    if (ans == QDialog::Rejected) {
        return false;
    }


    // read the file




    // set up internal pending/done lists













    // save initial data and get going
    saveState();

    return true;
}

void ConnectionValidationProtocol::saveState() {
    ZJsonObject data;

    // always version your output files!
    data.setEntry(KEY_VERSION.c_str(), fileVersion);



    // save data



    // progress:





    emit requestSaveProtocol(data);
}

void ConnectionValidationProtocol::loadDataRequested(ZJsonObject data) {
    // check version of saved data here
    if (!data.hasKey(KEY_VERSION.c_str())) {

        // report error

        return;
    }
    int version = ZJsonParser::integerValue(data[KEY_VERSION.c_str()]);
    if (version > fileVersion) {


        // report error


        return;
    }

    // convert old versions to current version here, when it becomes necessary


    // load data here






    // if, in the future, you need to update to a new save version,
    //  remember to do a save here


    // start work

}

void ConnectionValidationProtocol::onExitButton() {
    // exit protocol; can be reopened and worked on later
    emit protocolExiting();
}

void ConnectionValidationProtocol::onCompleteButton() {
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

ConnectionValidationProtocol::~ConnectionValidationProtocol()
{
    delete ui;
}
