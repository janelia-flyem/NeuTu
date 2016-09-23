#include "focusedpathprotocol.h"
#include "ui_focusedpathprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QMessageBox>

#include "zjsonobject.h"

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

bool FocusedPathProtocol::initialize() {

    // input dialog, get things ready to go
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


        // saveState();



        emit protocolCompleting();
    }
}

void FocusedPathProtocol::loadDataRequested(ZJsonObject data) {

    // deal with data load

}


FocusedPathProtocol::~FocusedPathProtocol()
{
    delete ui;
}
