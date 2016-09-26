#include "focusedpathprotocol.h"
#include "ui_focusedpathprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QMessageBox>

#include "zjsonobject.h"
#include "zjsonparser.h"

FocusedPathProtocol::FocusedPathProtocol(QWidget *parent) :
    ProtocolDialog(parent),
    ui(new Ui::FocusedPathProtocol)
{
    ui->setupUi(this);



    // UI connections


    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));


    // misc UI setup
    ui->buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

}

const std::string FocusedPathProtocol::KEY_VERSION = "version";
const int FocusedPathProtocol::fileVersion = 1;

bool FocusedPathProtocol::initialize() {

    // input dialog, get things ready to go





    // everything OK; save and return
    saveState();
    return true;

}

void FocusedPathProtocol::onExitButton() {
    // exit protocol; can be reopened and worked on later
    emit protocolExiting();
}

void FocusedPathProtocol::onCompleteButton() {
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

void FocusedPathProtocol::loadDataRequested(ZJsonObject data) {

    // check version of saved data here, once we have a second version
    if (!data.hasKey(KEY_VERSION.c_str())) {

        // display error in UI; for now, print it
        std::cout << "No version info in saved data; data not loaded!" << std::endl;
        return;
    }
    int version = ZJsonParser::integerValue(data[KEY_VERSION.c_str()]);
    if (version > fileVersion) {

        // likewise...
        std::cout << "Saved data is from a newer version of NeuTu; update NeuTu and try again!" << std::endl;



        return;
    }


    // convert to newer version here if needed



    // do actual load


}

void FocusedPathProtocol::saveState() {

    ZJsonObject data;

    // always version your output files!
    data.setEntry(KEY_VERSION.c_str(), fileVersion);

    emit requestSaveProtocol(data);
}

FocusedPathProtocol::~FocusedPathProtocol()
{
    delete ui;
}
