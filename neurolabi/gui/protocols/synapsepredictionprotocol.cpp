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
#include "zjsonfactory.h"
#include "zjsonarray.h"

SynapsePredictionProtocol::SynapsePredictionProtocol(QWidget *parent) :
    ProtocolDialog(parent),
    ui(new Ui::SynapsePredictionProtocol)
{
    ui->setupUi(this);

    // UI connections:
    connect(ui->firstButton, SIGNAL(clicked(bool)), this, SLOT(onFirstButton()));
    connect(ui->prevButton, SIGNAL(clicked(bool)), this, SLOT(onPrevButton()));
    connect(ui->nextButton, SIGNAL(clicked(bool)), this, SLOT(onNextButton()));

    connect(ui->reviewFirstButton, SIGNAL(clicked(bool)),
            this, SLOT(onReviewFirstButton()));
    connect(ui->reviewPrevButton, SIGNAL(clicked(bool)),
            this, SLOT(onReviewPrevButton()));
    connect(ui->reviewNextButton, SIGNAL(clicked(bool)),
            this, SLOT(onReviewNextButton()));

    connect(ui->gotoButton, SIGNAL(clicked(bool)), this, SLOT(onGotoButton()));
    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));
    connect(ui->refreshButton, SIGNAL(clicked(bool)), this, SLOT(onRefreshButton()));


    // misc UI setup
    ui->buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

    m_currentPendingIndex = 0;
    m_currentFinishedIndex = 0;
}

// protocol name should not contain hyphens
const std::string SynapsePredictionProtocol::PROTOCOL_NAME = "synapse_prediction";
const std::string SynapsePredictionProtocol::KEY_FINISHED = "finished";
const std::string SynapsePredictionProtocol::KEY_PENDING = "pending";
const std::string SynapsePredictionProtocol::KEY_VERSION = "version";
const std::string SynapsePredictionProtocol::KEY_PROTOCOL_RANGE = "range";
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

    m_protocolRange = volume;

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
      m_currentPendingIndex = 0;
    } else {
      m_currentPendingIndex = -1;
    }

    gotoCurrent();
    updateLabels();
}

void SynapsePredictionProtocol::onReviewFirstButton()
{
  if (m_finishedList.size() > 0) {
    m_currentPendingIndex = 0;
  } else {
    m_currentPendingIndex = -1;
  }

  gotoCurrentFinished();
}

void SynapsePredictionProtocol::onPrevButton()
{
  if (!m_pendingList.empty()) {
    m_currentPendingIndex--;
    if (m_currentPendingIndex < 0) {
      m_currentPendingIndex = m_pendingList.size() - 1;
    }
    gotoCurrent();
    updateLabels();
  } else {
    m_currentPendingIndex = -1;
  }
}

void SynapsePredictionProtocol::onReviewPrevButton()
{
  if (!m_finishedList.empty()) {
    m_currentFinishedIndex--;
    if (m_currentFinishedIndex < 0) {
      m_currentFinishedIndex = m_finishedList.size() - 1;
    }
    gotoCurrentFinished();
  } else {
    m_currentFinishedIndex = -1;
  }
}

void SynapsePredictionProtocol::onNextButton()
{
  if (!m_pendingList.empty()) {
    m_currentPendingIndex++;
    if (m_currentPendingIndex >= m_pendingList.size()) {
      m_currentPendingIndex = 0;
    }
    gotoCurrent();
    updateLabels();
  } else {
    m_currentPendingIndex = -1;
  }
}

void SynapsePredictionProtocol::onReviewNextButton()
{
  if (!m_finishedList.empty()) {
    m_currentFinishedIndex++;
    if (m_currentFinishedIndex >= m_finishedList.size()) {
      m_currentFinishedIndex = 0;
    }

    gotoCurrentFinished();
  } else {
    m_currentFinishedIndex = -1;
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

void SynapsePredictionProtocol::onRefreshButton()
{
  loadInitialSynapseList();
  updateLabels();
}

void SynapsePredictionProtocol::onExitButton() {
    // exit protocol; can be reopened and worked on later
    emit protocolExiting();
}

void SynapsePredictionProtocol::gotoCurrent() {
    // the dubious null check appears again...
    if (m_currentPendingIndex >= 0 && m_currentPendingIndex < m_pendingList.size()) {
      ZIntPoint pt = m_pendingList[m_currentPendingIndex];
      emit requestDisplayPoint(pt.getX(), pt.getY(), pt.getZ());
      /*
        emit requestDisplayPoint(m_currentPoint.getX(),
            m_currentPoint.getY(), m_currentPoint.getZ());
            */
    }
}

void SynapsePredictionProtocol::gotoCurrentFinished()
{
  if (m_currentFinishedIndex >= 0 &&
      m_currentFinishedIndex < m_finishedList.size()) {
    ZIntPoint pt = m_finishedList[m_currentFinishedIndex];
    emit requestDisplayPoint(pt.getX(), pt.getY(), pt.getZ());
  }
}

void SynapsePredictionProtocol::saveState() {
    // json save format: {"pending": [[x, y, z], [x2, y2, z2], ...],
    //                    "finished": similar list}

    ZJsonObject data;

    ZJsonArray rangeJson = ZJsonFactory::MakeJsonArray(m_protocolRange);
    data.setEntry(KEY_PROTOCOL_RANGE.c_str(), rangeJson);

    // always version your output files!
    data.setEntry(KEY_VERSION.c_str(), fileVversion);

    emit requestSaveProtocol(data);
}

void SynapsePredictionProtocol::loadDataRequested(ZJsonObject data) {

    // check version of saved data here, once we have a second version

  m_protocolRange.loadJson(
        ZJsonArray(data.value(KEY_PROTOCOL_RANGE.c_str())));
  if (!m_protocolRange.isEmpty()) {
    loadInitialSynapseList();
  } else {
    ui->progressLabel->setText(
          "Invalid protocol range. No data loaded.");
    return;
  }

    onFirstButton();
}

void SynapsePredictionProtocol::processSynapseMoving(
    const ZIntPoint &from, const ZIntPoint &to)
{
  moveSynapse(from, to);
  updateLabels();
}

void SynapsePredictionProtocol::processSynapseVerification(
    int x, int y, int z, bool verified)
{
  if (verified) {
    verifySynapse(ZIntPoint(x, y, z));
  } else {
    unverifySynapse(ZIntPoint(x, y, z));
  }

  updateLabels();
}

void SynapsePredictionProtocol::verifySynapse(const ZIntPoint &pt)
{
  ZDvidReader reader;
  ZIntPoint targetPoint = pt;
  bool isVerified = true;
  if (reader.open(m_dvidTarget)) {
    ZDvidSynapse synapse =
        reader.readSynapse(pt, NeuTube::FlyEM::LOAD_PARTNER_LOCATION);

    if (synapse.getKind() == ZDvidAnnotation::KIND_PRE_SYN) {
      std::vector<ZIntPoint> psdArray = synapse.getPartners();
      for (std::vector<ZIntPoint>::const_iterator iter = psdArray.begin();
           iter != psdArray.end(); ++iter) {
        const ZIntPoint &pt = *iter;
        ZDvidSynapse synapse =
            reader.readSynapse(pt, NeuTube::FlyEM::LOAD_NO_PARTNER);
        if (!synapse.isVerified()) {
          isVerified = false;
          break;
        }
      }
    } else if (synapse.getKind() == ZDvidAnnotation::KIND_POST_SYN) {
      std::vector<ZIntPoint> partnerArray = synapse.getPartners();
      if (!partnerArray.empty()) {
        targetPoint = partnerArray.front();
        ZDvidSynapse presyn =
            reader.readSynapse(targetPoint, NeuTube::FlyEM::LOAD_PARTNER_LOCATION);

        std::vector<ZIntPoint> psdArray = presyn.getPartners();
        for (std::vector<ZIntPoint>::const_iterator iter = psdArray.begin();
             iter != psdArray.end(); ++iter) {
          const ZIntPoint &pt = *iter;
          ZDvidSynapse synapse =
              reader.readSynapse(pt, NeuTube::FlyEM::LOAD_NO_PARTNER);
          if (!synapse.isVerified()) {
            isVerified = false;
            break;
          }
        }
      }
    }
  }

  if (isVerified) {
    if (m_pendingList.removeOne(targetPoint)) {
      if (m_currentPendingIndex >= m_pendingList.size()) {
        m_currentPendingIndex = m_pendingList.size() - 1;
      }
      m_finishedList.append(targetPoint);
    }
  }
}

void SynapsePredictionProtocol::unverifySynapse(const ZIntPoint &pt)
{
  ZDvidReader reader;
  ZIntPoint targetPoint = pt;

  if (reader.open(m_dvidTarget)) {
    ZDvidSynapse synapse =
        reader.readSynapse(pt, NeuTube::FlyEM::LOAD_PARTNER_LOCATION);

    if (synapse.getKind() == ZDvidAnnotation::KIND_POST_SYN) {
      std::vector<ZIntPoint> partnerArray = synapse.getPartners();
      if (!partnerArray.empty()) {
        targetPoint = partnerArray.front();
      }
    }
  }

  if (m_finishedList.removeOne(targetPoint)) {
    m_pendingList.append(targetPoint);
  }
}

void SynapsePredictionProtocol::processSynapseVerification(
    const ZIntPoint &pt, bool verified)
{
  processSynapseVerification(pt.getX(), pt.getY(), pt.getZ(), verified);
}


void SynapsePredictionProtocol::moveSynapse(
    const ZIntPoint &src, const ZIntPoint &dst)
{
  int index = m_pendingList.indexOf(src);
  if (index >= 0) { //A pending synapse
    m_pendingList[index] = dst;
    processSynapseVerification(dst, true);
  } else {
    index = m_finishedList.indexOf(src);
    if (index >= 0) {
      m_finishedList[index] = dst;
    } else { //could be psd
      processSynapseVerification(dst, true);
    }
  }
}

void SynapsePredictionProtocol::updateLabels() {
    // currently update both labels together (which is fine if they are fast)

    // current item:

    // is a zero point good enough for null?
    if (m_currentPendingIndex >= 0 && m_currentPendingIndex < m_pendingList.size()) {
      ZIntPoint currentPoint = m_pendingList[m_currentPendingIndex];
        ui->currentLabel->setText(QString("Current: %1, %2, %3").arg(currentPoint.getX())
            .arg(currentPoint.getY()).arg(currentPoint.getZ()));
    } else {
        ui->currentLabel->setText(QString("Current: --, --, --"));
    }

    // progress, in form: "Progress:  #/# (#%)"
    int nFinished = m_finishedList.size();
    int nTotal = m_pendingList.size() + nFinished;
    float percent = (100.0 * nFinished) / nTotal;
    ui->progressLabel->setText(QString("Progress: %1 / %2 (%3%)").arg(nFinished).arg(nTotal).arg(percent, 4, 'f', 1));
}

void SynapsePredictionProtocol::loadInitialSynapseList()
{
  loadInitialSynapseList(m_protocolRange, "");
}

/*
 * retrieve synapses from the input volume that are in the input RoI;
 * load into arrays
 */
void SynapsePredictionProtocol::loadInitialSynapseList(ZIntCuboid volume, QString /*roi*/) {

    // I don't *think* there's any way these lists will be populated, but...
    m_pendingList.clear();
    m_finishedList.clear();
    m_currentPendingIndex = 0;

    ZDvidReader reader;
    reader.setVerbose(false);
    if (reader.open(m_dvidTarget)) {
      std::vector<ZDvidSynapse> synapseList = reader.readSynapse(
            volume, NeuTube::FlyEM::LOAD_PARTNER_LOCATION);

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
          ZDvidSynapse &synapse = synapseList[i];
          if (synapse.getKind() == ZDvidAnnotation::KIND_PRE_SYN) {
            if (synapse.isProtocolVerified(m_dvidTarget)) {
              m_finishedList.append(synapse.getPosition());
            } else {
              m_pendingList.append(synapse.getPosition());
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
