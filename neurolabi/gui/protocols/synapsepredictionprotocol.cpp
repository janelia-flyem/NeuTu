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
#include "zintpoint.h"
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


    // test data we can pretend came from dialog; this volume
    //  has 12 synaptic elements
    QString point1Input = "3500, 5200,  7300";
    QString point2Input = "3700 5400  7350";
    QString roiInput = "testroi";

    // for now, parrot back the input
    QMessageBox mb;
    mb.setText("Synapse prediction protocol");
    mb.setInformativeText(QString("This will eventually ask for user input.  For testing, using:\npoint1 = %1\npoint2 = %2\nRoI = %3")
        .arg(point1Input).arg(point2Input).arg(roiInput));
    mb.setStandardButtons(QMessageBox::Ok);
    mb.setDefaultButton(QMessageBox::Ok);
    mb.exec();

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
    if (m_pendingList.size() > 0) {
        m_currentPoint = m_pendingList.first();
    } else {

        // still not sure the best way to represent a null;
        m_currentPoint = ZIntPoint();

    }

    gotoCurrent();
    updateLabels();
}

void SynapsePredictionProtocol::onMarkedButton() {

    // using this as our null point, ugh
    if (m_currentPoint.isZero()) {
        return;
    }

    bool done = (m_pendingList.size() == 1);

    // handle the lists
    ZIntPoint nextPoint = getNextPoint(m_currentPoint);
    m_pendingList.removeAll(m_currentPoint);
    m_finishedList.append(m_currentPoint);

    saveState();

    if (done) {
        m_currentPoint = ZIntPoint();

        QMessageBox mb;
        mb.setText("Finished!");
        mb.setInformativeText("All predictions reviewed!  Remember to complete the protocol when you are satisfied with the results.");
        mb.setStandardButtons(QMessageBox::Ok);
        mb.setDefaultButton(QMessageBox::Ok);
        mb.exec();
    } else {
        m_currentPoint = nextPoint;
        gotoCurrent();
    }

    updateLabels();
}

void SynapsePredictionProtocol::onSkipButton() {
    if (m_pendingList.size() > 1) {
        // if pending list has elements, should always be a current point
        m_currentPoint = getNextPoint(m_currentPoint);
        gotoCurrent();
        updateLabels();
    }
}

void SynapsePredictionProtocol::onGotoButton() {
    gotoCurrent();
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


void SynapsePredictionProtocol::gotoCurrent() {
    std::cout << "SynapsePredictionProtocol::gotoCurrent" << std::endl;

    // do stuff
}

ZIntPoint SynapsePredictionProtocol::getNextPoint(ZIntPoint point) {
    if (!m_pendingList.contains(point)) {
        // poor excuse for a null...
        return ZIntPoint();
    } else if (m_pendingList.size() == 0) {
        return ZIntPoint();
    } else {
        int currentIndex = m_pendingList.indexOf(point);
        currentIndex = (currentIndex + 1) % m_pendingList.size();
        return m_pendingList[currentIndex];
    }
}

void SynapsePredictionProtocol::saveState() {
    // json save format: {"pending": [[x, y, z], [x2, y2, z2], ...],
    //                    "finished": similar list}

    ZJsonObject data;

    ZJsonArray pending;
    foreach (ZIntPoint point, m_pendingList) {
        ZJsonArray temp;
        temp.append(point.getX());
        temp.append(point.getY());
        temp.append(point.getZ());
        pending.append(temp);
    }
    data.setEntry(KEY_PENDING.c_str(), pending);

    ZJsonArray finished;
    foreach (ZIntPoint point, m_finishedList) {
        ZJsonArray temp;
        temp.append(point.getX());
        temp.append(point.getY());
        temp.append(point.getZ());
        finished.append(temp);
    }
    data.setEntry(KEY_FINISHED.c_str(), finished);

    emit requestSaveProtocol(data);
}

void SynapsePredictionProtocol::loadDataRequested(ZJsonObject data) {

    std::cout << "SynapsePredictionProtocol::loadDataRequested" << std::endl;

    if (!data.hasKey(KEY_FINISHED.c_str()) || !data.hasKey(KEY_PENDING.c_str())) {
        // how to communicate failure?  overwrite a label?
        ui->progressLabel->setText("Data could not be loaded from DVID!");
        return;
    }

    m_pendingList.clear();
    m_finishedList.clear();

    ZJsonArray pendingJson(data.value(KEY_PENDING.c_str()));
    for (size_t i=0; i<pendingJson.size(); i++) {
        m_pendingList.append(ZJsonParser::toIntPoint(pendingJson.at(i)));
    }

    ZJsonArray finishedJson(data.value(KEY_FINISHED.c_str()));
    for (size_t i=0; i<finishedJson.size(); i++) {
        m_finishedList.append(ZJsonParser::toIntPoint(finishedJson.at(i)));
    }

    onFirstButton();
}

void SynapsePredictionProtocol::updateLabels() {
    // currently update both labels together (which is fine if they are fast)

    // current item:

    // is a zero point good enough for null?
    if (!m_currentPoint.isZero()) {
        ui->currentLabel->setText(QString("Current: %1, %2, %3").arg(m_currentPoint.getX())
            .arg(m_currentPoint.getY()).arg(m_currentPoint.getZ()));
    } else {
        ui->currentLabel->setText(QString("Current: --, --, --"));
    }

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
            m_pendingList.append(iter->getPosition());
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
