#include "synapsepredictionprotocol.h"
#include "ui_synapsepredictionprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QInputDialog>
#include <QMessageBox>

#include "dvid/zdvidreader.h"
#include "dvid/zdvidsynapse.h"
#include "zjsonarray.h"
#include "zjsonobject.h"
#include "zjsonparser.h"
#include "zintcuboid.h"
#include "zpoint.h"

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


    // needs a custom dialog, ugh
    // two entry fields for corners of volume
    //  accept comma or space delimited floats
    // entry field for name of RoI
    //  in perfect world, this will be a drop-down and you'll
    //  choose from existing RoIs in DVID


    // test data we can pretend came from dialog:
    QString point1Input = "3500, 5200,  7300";
    QString point2Input = "4000 5700  7350";
    QString roiInput = "testroi";

    // for now, parrot back the input
    QMessageBox mb;
    mb.setText("Synapse prediction protocol");
    mb.setInformativeText(QString("This will eventually ask for user input.  For testing, using:\npoint1 = %1\npoint2 = %2\nRoI = %3")
        .arg(point1Input).arg(point2Input).arg(roiInput));
    mb.setStandardButtons(QMessageBox::Ok);
    mb.setDefaultButton(QMessageBox::Ok);
    int ans = mb.exec();

    // convert input point strings to a ZCuboid; parse routine wants
    //  size coordinates in order (x1, y1, z1, x2, y2, z2), delimited
    //  by commas or spaces
    ZIntCuboid volume = parseVolumeString(point1Input + " " + point2Input);
    if (volume.isEmpty()) {
        return false;
    }

    std::cout << "synpre: volume corners: " << volume.getCorner(0).toString() <<
              ", " << volume.getCorner(1).toString() << std::endl;


    // generate pending/finished lists from user input
    // throw this into a thread?

    loadInitialSynapseList(volume, roiInput);

    // testing
    std::cout << "snpre: pending list length: " << m_pendingList.size() << std::endl;
    std::cout << "snpre: finished list length: " << m_finishedList.size() << std::endl;


    // arrange list in some appropriate way:
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

    updateLabels();
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

    // this is easy because we're keeping our data in json
    //  form
    ZJsonObject data;
    data.setEntry(KEY_PENDING.c_str(), m_pendingList);
    data.setEntry(KEY_FINISHED.c_str(), m_finishedList);
    emit requestSaveProtocol(data);
}

void SynapsePredictionProtocol::loadDataRequested(ZJsonObject data) {

    std::cout << "SynapsePredictionProtocol::loadDataRequested" << std::endl;

    if (!data.hasKey(KEY_FINISHED.c_str()) || !data.hasKey(KEY_PENDING.c_str())) {
        // how to communicate failure?  overwrite a label?
        ui->progressLabel->setText("Data could not be loaded from DVID!");
        return;
    }

    m_pendingList = ZJsonArray(data.value(KEY_PENDING.c_str()));
    m_finishedList = ZJsonArray(data.value(KEY_FINISHED.c_str()));

    onFirstButton();
}

void SynapsePredictionProtocol::updateLabels() {
    // currently update both labels together (which is fine if they are fast)

    // current item:
    // ui->currentItemLabel->setText(QString("Current: %1, %2, %3").arg(m_currentItem));

    // progress, in form: "Progress:  #/# (#%)"
    int nFinished = m_finishedList.size();
    int nTotal = m_pendingList.size() + nFinished;
    float percent = (100.0 * nFinished) / nTotal;
    ui->progressLabel->setText(QString("Progress: %1 / %2 (%3%)").arg(nFinished).arg(nTotal).arg(percent, 4, 'f', 1));
}

/*
 * retrieve synapses from the input volume that are in the input RoI;
 * load into arrays
 */
void SynapsePredictionProtocol::loadInitialSynapseList(ZIntCuboid volume, QString roi) {

    // I don't *think* there's any way these lists will be populated, but...
    m_pendingList.clear();
    m_finishedList.clear();

    ZDvidReader reader;
    reader.setVerbose(false);
    if (reader.open(m_dvidTarget)) {
        std::vector<ZDvidSynapse> synapseList = reader.readSynapse(volume, NeuTube::FlyEM::LOAD_PARTNER_RELJSON);

        // filter by roi (coming soon)
        // for now: need to do raw DVID call to batch ask "is point in RoI"?

        // filter to only auto?


        // put each pre/post site into list
        for(std::vector<ZDvidSynapse>::iterator iter = synapseList.begin(); iter != synapseList.end(); ++iter) {
            ZJsonArray point;
            point.append(iter->getX());
            point.append(iter->getY());
            point.append(iter->getZ());
            m_pendingList.append(point);
         }

        // order somehow?  here or earlier?

    }
}

/*
 * parse a string into a cuboid; raises a dialog if parsing fails
 *
 * input = string of six ints, space and/or comma delimited
 * output = cuboid; if parsing fails returns default volume, which
 *      tests as empty
 */
ZIntCuboid SynapsePredictionProtocol::parseVolumeString(QString input) {
    ZIntCuboid volume;

    bool success = true;

    QString input2 = input.replace(",", " ");
    QStringList items = input2.split(" ", QString::SkipEmptyParts);
    if (items.size() != 6) {
        success = false;
    } else {
        bool statusx, statusy, statusz;
        int tempx, tempy, tempz;

        tempx = items.at(0).toInt(&statusx);
        tempy = items.at(1).toInt(&statusy);
        tempz = items.at(2).toInt(&statusz);
        if (statusx && statusy && statusz) {
            volume.setFirstCorner(tempx, tempy, tempz);
        } else {
            success = false;
        }
        if (success) {
            tempx = items.at(3).toInt(&statusx);
            tempy = items.at(4).toInt(&statusy);
            tempz = items.at(5).toInt(&statusz);
            if (statusx && statusy && statusz) {
                volume.setLastCorner(tempx, tempy, tempz);
            } else {
                success = false;
            }
        }
    }

    if (!success) {
        QMessageBox mb;
        mb.setText("Parsing error");
        mb.setInformativeText("Could not parse input volume strings: " + input);
        mb.setStandardButtons(QMessageBox::Ok);
        mb.setDefaultButton(QMessageBox::Ok);
        int ans = mb.exec();

        volume.reset();
    }
    return volume;
}

SynapsePredictionProtocol::~SynapsePredictionProtocol()
{
    delete ui;
}
