#include "todoreviewprotocol.h"
#include "ui_todoreviewprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QMessageBox>


#include "todoreviewinputdialog.h"

#include "todosearcher.h"

#include "dvid/zdvidreader.h"
#include "zjsonobject.h"
#include "zjsonparser.h"


ToDoReviewProtocol::ToDoReviewProtocol(QWidget *parent) :
    ProtocolDialog(parent),
    ui(new Ui::ToDoReviewProtocol)
{
    ui->setupUi(this);

    // sites table
    m_sitesModel = new QStandardItemModel(0, 3, ui->todoTable);



    // UI connections
    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));





}

// constants
const std::string ToDoReviewProtocol::KEY_VERSION = "version";
const std::string ToDoReviewProtocol::KEY_PARAMETERS = "parameters";
const int ToDoReviewProtocol::fileVersion = 1;

bool ToDoReviewProtocol::initialize() {
    // input dialog; not sure there are any initial
    //  values that make sense to set
    ToDoReviewInputDialog inputDialog;
    int ans = inputDialog.exec();
    if (ans == QDialog::Rejected) {
        return false;
    }

    // transfer input parameters into the searcher
    ZDvidReader &reader = m_dvidReader;
    if (reader.good()) {
        // body ID not blank?  exists in dvid?
        QString bodyIDstring = inputDialog.getBodyID().trimmed();
        if (bodyIDstring.size() == 0) {
            inputErrorDialog("No body ID specified");
            return false;
        }

        bool ok;
        uint64_t bodyID = bodyIDstring.toULongLong(&ok);
        if (!ok) {
            inputErrorDialog("Couldn't parse body ID!");
            return false;
        }
        if (!reader.hasBody(bodyID)) {
            inputErrorDialog(QString("Body ID %1 doesn't exist!").arg(bodyID));
            return false;
        }

        m_searcher.setBodyID(bodyID);

    } else {
        inputErrorDialog("Couldn't open DVID!");
        return false;
    }


    // get to do list to review; if we're here, we've already checked that the reader is good

    // if searcher type != none:
    //  list of things = searcher.search( reader )


    // set up internal pending/done lists













    // save initial data and get going
    saveState();

    return true;
}


void ToDoReviewProtocol::saveState() {
    ZJsonObject data;

    // always version your output files!
    data.setEntry(KEY_VERSION.c_str(), fileVersion);



    // search parameters:
    ZJsonObject searchParams = m_searcher.toJson();
    data.setEntry(KEY_PARAMETERS.c_str(), searchParams);




    // progress:





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
    m_searcher.fromJson(data.value(KEY_PARAMETERS.c_str()));





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

void ToDoReviewProtocol::inputErrorDialog(QString message) {
    QMessageBox mb;
    mb.setText("Input error");
    mb.setInformativeText(message);
    mb.setStandardButtons(QMessageBox::Ok);
    mb.setDefaultButton(QMessageBox::Ok);
    mb.exec();
}

ToDoReviewProtocol::~ToDoReviewProtocol()
{
    delete ui;
}
