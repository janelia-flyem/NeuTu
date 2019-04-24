#include "todoreviewprotocol.h"
#include "ui_todoreviewprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QMessageBox>


#include "todoreviewinputdialog.h"


#include "dvid/zdvidreader.h"
#include "zjsonobject.h"
#include "zjsonparser.h"


ToDoReviewProtocol::ToDoReviewProtocol(QWidget *parent) :
    ProtocolDialog(parent),
    ui(new Ui::ToDoReviewProtocol)
{
    ui->setupUi(this);


    // UI connections
    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));





}

// constants
const std::string ToDoReviewProtocol::KEY_VERSION = "version";
const int ToDoReviewProtocol::fileVersion = 1;

bool ToDoReviewProtocol::initialize() {
    // input dialog; not sure there are any initial
    //  values that make sense to set
    ToDoReviewInputDialog inputDialog;
    int ans = inputDialog.exec();
    if (ans == QDialog::Rejected) {
        return false;
    }

    // do stuff to set up protocol


    // save initial data and get going
    saveState();

    return true;
}


void ToDoReviewProtocol::saveState() {
    ZJsonObject data;

    // always version your output files!
    data.setEntry(KEY_VERSION.c_str(), fileVersion);


    // save stuff


    emit requestSaveProtocol(data);
}

void ToDoReviewProtocol::loadDataRequested(ZJsonObject data) {
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


void ToDoReviewProtocol::onExitButton() {
    // exit protocol; can be reopened and worked on later
    emit protocolExiting();
}

void ToDoReviewProtocol::onCompleteButton() {
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

ToDoReviewProtocol::~ToDoReviewProtocol()
{
    delete ui;
}
