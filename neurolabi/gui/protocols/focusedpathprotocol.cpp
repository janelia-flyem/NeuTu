#include "focusedpathprotocol.h"
#include "ui_focusedpathprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QtGui>
#include <QInputDialog>
#include <QMessageBox>

#include "dvid/zdvidreader.h"
#include "zjsonobject.h"
#include "zjsonparser.h"

FocusedPathProtocol::FocusedPathProtocol(QWidget *parent, std::string variation) :
    ProtocolDialog(parent),
    ui(new Ui::FocusedPathProtocol)
{
    m_variation = variation;

    ui->setupUi(this);



    // UI connections


    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));


    // misc UI setup
    ui->buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

}

const std::string FocusedPathProtocol::KEY_VERSION = "version";
const std::string FocusedPathProtocol::VARIATION_BODY = "body";
const int FocusedPathProtocol::fileVersion = 1;

bool FocusedPathProtocol::initialize() {

    if (m_variation == VARIATION_BODY) {
        // input body ID as text because we can overflow 32-bit ints
        bool ok;
        uint64_t bodyID;
        QString ans = QInputDialog::getText(this,
            "Choose body", "Do focused proofreading on body with ID:",
            QLineEdit::Normal, "", &ok);
        if (ok && !ans.isEmpty()) {
            // convert to int and check that it exists:
            bodyID = ans.toLong(&ok);
            if (!ok) {
                QMessageBox mb;
                mb.setText("Can't parse body ID");
                mb.setInformativeText("The entered body ID " + ans + " doesn't seem to be an integer!");
                mb.setStandardButtons(QMessageBox::Ok);
                mb.setDefaultButton(QMessageBox::Ok);
                mb.exec();
                return false;
            }
            ZDvidReader reader;
            if (reader.open(m_dvidTarget)) {
                if (!reader.hasBody(bodyID)) {
                    QMessageBox mb;
                    mb.setText("Body ID doesn't exist!");
                    mb.setInformativeText("The entered body ID " +  ans + " doesn't seem to exist!");
                    mb.setStandardButtons(QMessageBox::Ok);
                    mb.setDefaultButton(QMessageBox::Ok);
                    mb.exec();
                    return false;
                }
            }
        } else {
            return false;
        }

        // do something with body ID here


    } else {
        variationError(m_variation);
        return false;
    }





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
    if (m_variation == VARIATION_BODY) {
        // do body load

    } else {
        variationError(m_variation);
    }




}

void FocusedPathProtocol::saveState() {

    ZJsonObject data;

    // always version your output files!
    data.setEntry(KEY_VERSION.c_str(), fileVersion);

    emit requestSaveProtocol(data);
}

void FocusedPathProtocol::variationError(std::string variation) {
    QMessageBox mb;
    mb.setText("Unknown protocol variation!");
    mb.setInformativeText("Unknown protocol variation " + QString::fromStdString(variation) + " was encountered!  Report this error!");
    mb.setStandardButtons(QMessageBox::Ok);
    mb.setDefaultButton(QMessageBox::Ok);
    mb.exec();
}

FocusedPathProtocol::~FocusedPathProtocol()
{
    delete ui;
}
