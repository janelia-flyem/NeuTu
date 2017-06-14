#include "synapsereviewprotocol.h"
#include "ui_synapsereviewprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QMessageBox>

#include "synapsereviewinputdialog.h"

#include "dvid/zdvidreader.h"
#include "zjsonfactory.h"
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
const std::string SynapseReviewProtocol::KEY_PENDING_LIST= "pending";
const std::string SynapseReviewProtocol::KEY_FINISHED_LIST= "finished";
const int SynapseReviewProtocol::fileVersion = 1;

bool SynapseReviewProtocol::initialize() {

    // input dialog; not sure there are any initial
    //  values that make sense to set
    SynapseReviewInputDialog inputDialog;
    int ans = inputDialog.exec();
    if (ans == QDialog::Rejected) {
        return false;
    }

    // validate the input and generate the initial synapse list
    std::vector<ZDvidSynapse> synapseList;
    ZDvidReader reader;
    reader.setVerbose(false);
    if (reader.open(m_dvidTarget)) {
        SynapseReviewInputDialog::SynapseReviewInputOptions option = inputDialog.getInputOption();
        if (option == SynapseReviewInputDialog::BY_BODYID) {

            std::cout << "body ID input" << std::endl;

            // body ID not blank?  exists in dvid?
            QString bodyIDstring = inputDialog.getBodyID().trimmed();
            if (bodyIDstring.size() == 0) {
                inputErrorDialog("No body ID specified");
                return false;
            }

            bool ok;
            uint64_t bodyID = bodyIDstring.toLong(&ok);
            if (!ok) {
                inputErrorDialog("Couldn't parse body ID!");
                return false;
            }
            if (!reader.hasBody(bodyID)) {
                inputErrorDialog(QString("Body ID %1 doesn't exist!").arg(bodyID));
                return false;
            }

            // get synapses
            synapseList = reader.readSynapse(bodyID, FlyEM::LOAD_PARTNER_LOCATION);


        } else if (option == SynapseReviewInputDialog::BY_VOLUME) {

            std::cout << "volume input" << std::endl;

            // I think volume is foolproof given our widgets; I set minimum
            //  width, etc. to 1, so we'll never get an invalid volume
            ZIntCuboid box = inputDialog.getVolume();
            synapseList = reader.readSynapse(box, FlyEM::LOAD_PARTNER_LOCATION);

        }

    } else {
        inputErrorDialog("Couldn't open DVID!");
        return false;
    }


    // given the list of synapses, extract the list of T-bar locations (which is
    //  all we need)
    if (synapseList.size() == 0) {
        inputErrorDialog("No synapses read!");
        return false;
    }
    m_pendingList.clear();
    m_finishedList.clear();
    for (size_t i=0; i<synapseList.size(); i++) {
        // check if it's a T-bar or PSD, and only keep T-bars
        if (synapseList.at(i).getKind() == ZDvidAnnotation::KIND_PRE_SYN) {
            m_pendingList.append(synapseList.at(i).getPosition());
        }
    }
    std::cout << "# pending sites = " << m_pendingList.size() << std::endl;



    // save initial data
    saveState();


    // poke the UI to get going
    // something something goto first point something something

    return true;
}

void SynapseReviewProtocol::inputErrorDialog(QString message) {
    QMessageBox mb;
    mb.setText("Input error");
    mb.setInformativeText(message);
    mb.setStandardButtons(QMessageBox::Ok);
    mb.setDefaultButton(QMessageBox::Ok);
    mb.exec();
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


    m_pendingList.clear();
    m_finishedList.clear();



    // this statement is the issue, so pick it apart until it works
    // ZJsonArray pending(data.value(KEY_PENDING_LIST.c_str()));
    std::cout << "extracting pending list" << std::endl;
    ZJsonValue temp = data.value(KEY_PENDING_LIST.c_str());
    std::cout << "temp.isarray(): " << temp.isArray() << std::endl;
    ZJsonArray pending(temp);
    // ZJsonArray pending;
    std::cout << "size of temp cast to ZJsonArray: " << ((ZJsonArray) temp).size() << std::endl;


    std::cout << "pending array length = " << pending.size() << std::endl;
    std::cout << "first parsed element = " << ZJsonParser::toIntPoint(pending.at(0)).toString() << std::endl;

    for (size_t i=0; i<pending.size(); i++) {
        m_pendingList.append(ZJsonParser::toIntPoint(pending.at(i)));
    }
    ZJsonArray finished(data.value(KEY_FINISHED_LIST.c_str()));
    for (size_t i=0; i<finished.size(); i++) {
        m_finishedList.append(ZJsonParser::toIntPoint(finished.at(i)));
    }


    std::cout << "loaded pending sites: " << m_pendingList.size() << std::endl;
    std::cout << "loaded finished sites: " << m_finishedList.size() << std::endl;


    // if, in the future, you need to update to a new save version,
    //  remember to do a save here


    // dispatch to whatever is next



}

void SynapseReviewProtocol::saveState() {
    ZJsonObject data;

    // always version your output files!
    data.setEntry(KEY_VERSION.c_str(), fileVersion);

    // save the pending and finished lists:
    ZJsonArray pending;
    foreach (ZIntPoint p, m_pendingList) {
        pending.append(ZJsonFactory::MakeJsonArray(p));
    }
    data.setEntry(KEY_PENDING_LIST.c_str(), pending);

    ZJsonArray finished;
    foreach (ZIntPoint p, m_finishedList) {
        finished.append(ZJsonFactory::MakeJsonArray(p));
    }
    data.setEntry(KEY_FINISHED_LIST.c_str(), finished);

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

