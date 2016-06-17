#include "synapsepredictionprotocol.h"
#include "ui_synapsepredictionprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QInputDialog>
#include <QMessageBox>

#include "synapsepredictioninputdialog.h"

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
const std::string SynapsePredictionProtocol::KEY_VERSION = "version";
const int SynapsePredictionProtocol::fileVversion = 1;


/*
 * start the protocol anew; returns success status;
 * initialize is also expected to do the first save
 * of the protocol's data
 */
bool SynapsePredictionProtocol::initialize() {

    SynapsePredictionInputDialog inputDialog;

    // set initial volume here
    // small volume for testing (has only a handful of synapses):
    // inputDialog.setVolume(ZIntCuboid(3500, 5200, 7300, 3700, 5400, 7350));

    // larger generic volume as default starting point:
    inputDialog.setVolume(ZIntCuboid(3000, 3000, 3000, 4000, 4000, 4000));

    inputDialog.setRoI("(RoI is ignored for now)");

    int ans = inputDialog.exec();
    if (ans == QDialog::Rejected) {
        return false;
    }
    ZIntCuboid volume = inputDialog.getVolume();
    if (volume.isEmpty()) {
        return false;
    }
    QString roiInput = inputDialog.getRoI();

    // generate pending/finished lists from user input
    // throw this into a thread?
    loadInitialSynapseList(volume, roiInput);


    // arrange list in some appropriate way:
    //  -- here or in above call?
    //  -- pre then post or the other way around?
    //  -- cluster spatially?



    // get started
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
    // the dubious null check appears again...
    if (!m_currentPoint.isZero()) {
        emit requestDisplayPoint(m_currentPoint.getX(),
            m_currentPoint.getY(), m_currentPoint.getZ());
    }
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

    // always version your output files!
    data.setEntry(KEY_VERSION.c_str(), fileVversion);

    emit requestSaveProtocol(data);
}

void SynapsePredictionProtocol::loadDataRequested(ZJsonObject data) {

    // check version of saved data here, once we have a second version

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


        // this list is mixed pre- and post- sites; relations are in there, but the list
        //  doesn't show them in any way as-is


        // cache that list of synapses for later use?
        // would have to rebuild cache when loading from save


        // filter by roi (coming soon)
        // will need to do raw DVID call to batch ask "is point in RoI?";
        //  that call not in ZDvidReader() yet


        // filter to only auto?  (not human-placed)



        // put each pre/post site into list
        // for now: find the pre-synaptic sites; put each one on the list; then,
        //  put all its post-synaptic partners on the list immediately after it,
        //  whether it's in the volume or not
        for (size_t i=0; i<synapseList.size(); i++) {
            if (synapseList[i].getKind() == ZDvidAnnotation::KIND_PRE_SYN) {
                m_pendingList.append(synapseList[i].getPosition());
                for (size_t j=0; j<synapseList[i].getRelationJson().size(); j++) {
                    ZIntPoint point = ZJsonParser::toIntPoint(ZJsonObject(synapseList[i].getRelationJson().value(j))["To"]);
                    m_pendingList.append(point);
                }
            }
        }


        // order somehow?  here or earlier?
        // in a perfect world, I'd sort the pre-synaptic sites spatially, but
        //  for now, it's just the order DVID returns

    }
}

SynapsePredictionProtocol::~SynapsePredictionProtocol()
{
    delete ui;
}
