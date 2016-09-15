#include "synapsepredictionprotocol.h"
#include "ui_synapsepredictionprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QtGui>
#include <QInputDialog>
#include <QMessageBox>
#include <QtAlgorithms>

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

SynapsePredictionProtocol::SynapsePredictionProtocol(QWidget *parent, std::string variation) :
    ProtocolDialog(parent),
    ui(new Ui::SynapsePredictionProtocol)
{
    m_variation = variation;

    ui->setupUi(this);

    // sites table
    m_sitesModel = new QStandardItemModel(0, 5, ui->sitesTableView);
    setSitesHeaders(m_sitesModel);
    ui->sitesTableView->setModel(m_sitesModel);


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
    connect(ui->finishCurrentButton, SIGNAL(clicked(bool)), this, SLOT(onFinishCurrentButton()));
    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));
    connect(ui->refreshButton, SIGNAL(clicked(bool)), this, SLOT(onRefreshButton()));
    connect(ui->lastVerifiedButton, SIGNAL(clicked(bool)), this, SLOT(onLastVerifiedButton()));

    connect(ui->sitesTableView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(onDoubleClickSitesTable(QModelIndex)));

    // misc UI setup
    ui->buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

    m_currentPendingIndex = 0;
    m_currentFinishedIndex = 0;
}

const std::string SynapsePredictionProtocol::KEY_VARIATION = "variation";
const std::string SynapsePredictionProtocol::VARIATION_REGION = "region";
const std::string SynapsePredictionProtocol::VARIATION_BODY = "body";
const std::string SynapsePredictionProtocol::KEY_VERSION = "version";
const std::string SynapsePredictionProtocol::KEY_PROTOCOL_RANGE = "range";
const std::string SynapsePredictionProtocol::KEY_BODYID= "body ID";
const int SynapsePredictionProtocol::fileVersion = 2;


/*
 * start the protocol anew; returns success status;
 * initialize is also expected to do the first save
 * of the protocol's data
 */
bool SynapsePredictionProtocol::initialize() {

    if (m_variation == VARIATION_REGION) {
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

    } else if (m_variation == VARIATION_BODY) {
        // using text dialog because getInt() variation is 32-bit;
        //  our body IDs get bigger than that
        // note for future improvement: would be nice to let the user
        //  enter a body name, but currently we store body names in
        //  key-value, which is not indexed and not synced to body IDs
        bool ok;
        uint64_t bodyID;
        QString ans = QInputDialog::getText(this,
            "Choose body", "Review synapses on body with ID:",
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
        m_bodyID = bodyID;
    } else {
        variationError(m_variation);
        return false;
    }

    // generate pending/finished lists from user input
    // throw this into a thread?
    loadInitialSynapseList();

    // get started
    onFirstButton();
    saveState();

    return true;
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
  } else {
    m_currentPendingIndex = -1;
  }
  updateLabels();
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

void SynapsePredictionProtocol::onLastVerifiedButton() {
    // note that this only really works while a session is active; if
    //  you reload the data, the finished list may be in a different
    //  order; while we're active, we know that we always append the
    //  most recently verified synapse at the end of the list
    if (m_finishedList.size() > 0) {
        ZIntPoint pt = m_finishedList[m_finishedList.size() - 1];
        emit requestDisplayPoint(pt.getX(), pt.getY(), pt.getZ());
    }
}

void SynapsePredictionProtocol::onGotoButton() {
    gotoCurrent();
}

void SynapsePredictionProtocol::onFinishCurrentButton() {
    if (m_currentPendingIndex < 0 || m_currentPendingIndex >= m_pendingList.size()) {
        return;
    }

    // ensure the thing is 100% verified
    ZIntPoint currentPoint = m_pendingList[m_currentPendingIndex];
    std::vector<ZDvidSynapse> synapseElements = getWholeSynapse(currentPoint);
    bool verified = true;
    for (size_t i=0; i<synapseElements.size(); i++) {
        if (!synapseElements[i].isVerified()) {
            verified = false;
            break;
        }
    }
    if (!verified) {
        QMessageBox mb;
        mb.setText("Not all verified!");
        mb.setInformativeText("Not all elements of the current T-bar are verified!  Verify the T-bar and all PSDs before finishing.");
        mb.setStandardButtons(QMessageBox::Ok);
        mb.setDefaultButton(QMessageBox::Ok);
        mb.exec();
        return;
    }

    // advance protocol and go to next point
    m_pendingList.removeOne(currentPoint);
    m_finishedList.append(currentPoint);
    if (!m_pendingList.empty()) {
        // having removed the point at the current index, the current
        //  index now points to the next point or past the end of
        //  the list; if the latter, loop back to the top
        if (m_currentPendingIndex >= m_pendingList.size()) {
            m_currentPendingIndex = 0;
        }
    } else {
        m_currentPendingIndex = -1;
    }
    gotoCurrent();
    updateLabels();
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
    // try to keep the same current T-bar across the refresh
    ZIntPoint savedPoint;
    int savedIndex = -1;
    bool saved = false;
    if (m_currentPendingIndex >= 0) {
        savedPoint = m_pendingList[m_currentPendingIndex];
        savedIndex = m_currentPendingIndex;
        saved = true;
    }
    loadInitialSynapseList();
    if (saved) {
        int index = m_pendingList.indexOf(savedPoint);
        if (index >= 0) {
            m_currentPendingIndex = index;
        } else {
            // if we make it here, that means the current synapse was
            //  all verified but not yet finished when the list was
            //  refreshed, and it got finished because that's a side
            //  effect of the refresh; move it back from the finished list to
            //  the pending list, at the same position
            m_finishedList.removeOne(savedPoint);
            m_pendingList.insert(savedIndex, savedPoint);
            m_currentPendingIndex = savedIndex;
        }
    }
    updateLabels();
}

void SynapsePredictionProtocol::onExitButton() {
    // exit protocol; can be reopened and worked on later
    emit protocolExiting();
}

void SynapsePredictionProtocol::gotoCurrent() {
    if (m_currentPendingIndex >= 0 && m_currentPendingIndex < m_pendingList.size()) {
      ZIntPoint pt = m_pendingList[m_currentPendingIndex];
      emit requestDisplayPoint(pt.getX(), pt.getY(), pt.getZ());
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
    // json save format: {"range": [x1, y1, z1, x2, y2, z2]},
    //      which are the corners of cube we're looking at;
    //      note that the examined/not status of the synapses
    //      is stored directly in DVID

    ZJsonObject data;

    data.setEntry(KEY_VARIATION.c_str(), m_variation.c_str());

    if (m_variation == VARIATION_REGION) {
        ZJsonArray rangeJson = ZJsonFactory::MakeJsonArray(m_protocolRange);
        data.setEntry(KEY_PROTOCOL_RANGE.c_str(), rangeJson);
    } else if (m_variation == VARIATION_BODY) {
        data.setEntry(KEY_BODYID.c_str(), m_bodyID);
    } else {
        variationError(m_variation);
        return;
    }

    // always version your output files!
    data.setEntry(KEY_VERSION.c_str(), fileVersion);

    emit requestSaveProtocol(data);
}

void SynapsePredictionProtocol::loadDataRequested(ZJsonObject data) {

    // check version of saved data here, once we have a second version
    if (!data.hasKey(KEY_VERSION.c_str())) {
        ui->progressLabel->setText("No version info in saved data; data not loaded!");
        return;
    }
    int version = ZJsonParser::integerValue(data[KEY_VERSION.c_str()]);
    if (version > fileVersion) {
        ui->progressLabel->setText("Saved data is from a newer version of NeuTu; update NeuTu and try again!");
        return;
    }

    // convert old versions; do in sequential order: 1 to 2, 2 to 3, etc;
    //  worked well with Raveler, so do it here; can break these out to
    //  separate methods if they get wordy
    bool updated = false;
    if (version != fileVersion) {
        // 1 to 2:
        if (version == 1) {
            // in v2, we added the "variation" key; in v1, that was always "region"
            data.setEntry(KEY_VARIATION.c_str(), VARIATION_REGION.c_str());
            version = 2;
        }


        updated = true;
    }

    // variation specific loading:
    if (m_variation == VARIATION_REGION) {
        m_protocolRange.loadJson(ZJsonArray(data.value(KEY_PROTOCOL_RANGE.c_str())));
        if (!m_protocolRange.isEmpty()) {
            loadInitialSynapseList();
        } else {
            ui->progressLabel->setText("Invalid protocol range. No data loaded!");
            return;
        }
    } else if (m_variation == VARIATION_BODY) {
        m_bodyID = ZJsonParser::integerValue(data[KEY_BODYID.c_str()]);
        loadInitialSynapseList();
    } else {
        variationError(m_variation);
    }

    // at end of updates, save, because not all variations save frequently;
    //  has to be done here, after everything is loaded
    if (updated) {
        saveState();
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
  int oldPendingListSize = m_pendingList.size();

  if (verified) {
    verifySynapse(ZIntPoint(x, y, z));
  } else {
    unverifySynapse(ZIntPoint(x, y, z));
  }

  // if pending list has fewer items, that means the whole
  //    synapse got verified, and it's time to move to the next one
  if (m_pendingList.size() < oldPendingListSize) {
      onNextButton();
  } else {
      updateLabels();
  }
}

void SynapsePredictionProtocol::verifySynapse(const ZIntPoint &pt)
{
  ZDvidReader reader;
  ZIntPoint targetPoint = pt;
  bool isVerified = true;
  if (reader.open(m_dvidTarget)) {
    ZDvidSynapse synapse =
        reader.readSynapse(pt, FlyEM::LOAD_PARTNER_LOCATION);

    if (synapse.getKind() == ZDvidAnnotation::KIND_PRE_SYN) {
      std::vector<ZIntPoint> psdArray = synapse.getPartners();
      for (std::vector<ZIntPoint>::const_iterator iter = psdArray.begin();
           iter != psdArray.end(); ++iter) {
        const ZIntPoint &pt = *iter;
        ZDvidSynapse synapse =
            reader.readSynapse(pt, FlyEM::LOAD_NO_PARTNER);
        if (!synapse.isVerified()) {
          isVerified = false;
          break;
        }
      }
      if (!synapse.isVerified()) {
          isVerified = false;
      }
    } else if (synapse.getKind() == ZDvidAnnotation::KIND_POST_SYN) {
      std::vector<ZIntPoint> partnerArray = synapse.getPartners();
      if (!partnerArray.empty()) {
        targetPoint = partnerArray.front();
        ZDvidSynapse presyn =
            reader.readSynapse(targetPoint, FlyEM::LOAD_PARTNER_LOCATION);

        std::vector<ZIntPoint> psdArray = presyn.getPartners();
        for (std::vector<ZIntPoint>::const_iterator iter = psdArray.begin();
             iter != psdArray.end(); ++iter) {
          const ZIntPoint &pt = *iter;
          ZDvidSynapse synapse =
              reader.readSynapse(pt, FlyEM::LOAD_NO_PARTNER);
          if (!synapse.isVerified()) {
            isVerified = false;
            break;
          }
        }
        if (!presyn.isVerified()) {
            isVerified = false;
        }
      }
    }
  }

}

void SynapsePredictionProtocol::unverifySynapse(const ZIntPoint &pt)
{
  ZDvidReader reader;
  ZIntPoint targetPoint = pt;

  if (reader.open(m_dvidTarget)) {
    ZDvidSynapse synapse =
        reader.readSynapse(pt, FlyEM::LOAD_PARTNER_LOCATION);

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
    // currently we update all labels and the PSD table at once


    // current presynaptic sites labels:
    if (m_currentPendingIndex >= 0 && m_currentPendingIndex < m_pendingList.size()) {
        ZIntPoint currentPoint = m_pendingList[m_currentPendingIndex];
        std::vector<ZDvidSynapse> synapse = getWholeSynapse(currentPoint);

        if (synapse.size() == 0) {
            QMessageBox mb;
            mb.setText("Can't find T-bar");
            mb.setInformativeText("The current T-bar appears to have been moved or deleted; refreshing data...");
            mb.setStandardButtons(QMessageBox::Ok);
            mb.setDefaultButton(QMessageBox::Ok);
            mb.exec();

            loadInitialSynapseList();
            onFirstButton();
            return;
        }

        // first item in that list is the pre-synaptic element
        ui->preLocationLabel->setText(QString::fromStdString(currentPoint.toString()));
        ui->preConfLabel->setText(QString("Confidence: %1").arg(synapse[0].getConfidence(), 3, 'f', 1));
        if (synapse[0].isVerified()) {
            ui->preStatusLabel->setText(QString("Verified: yes"));
        } else {
            ui->preStatusLabel->setText(QString("Verified: no"));
        }

        int nPSDverified = 0;
        for (size_t i=1; i<synapse.size(); i++) {
            if (synapse[i].isVerified()) {
                nPSDverified++;
            }
        }
        ui->postTableLabel->setText(QString("PSDs (%1/%2 verified)").arg(nPSDverified).arg(synapse.size() - 1));

        updateSitesTable(synapse);
    } else {
        ui->preLocationLabel->setText(QString("(--, --, --)"));
        ui->preConfLabel->setText(QString("Confidence: --"));
        ui->preStatusLabel->setText(QString("Verified: --"));

        clearSitesTable();
    }

    // progress label:
    int nPending = m_pendingList.size();
    int nFinished = m_finishedList.size();
    int nTotal = nPending + nFinished;
    float percent = (100.0 * nFinished) / nTotal;
    ui->progressLabel->setText(QString("Progress:\n\n %1 / %2 (%3%)").arg(nFinished).arg(nTotal).arg(percent, 4, 'f', 1));
}

void SynapsePredictionProtocol::loadInitialSynapseList()
{
    // I don't *think* there's any way these lists will already be populated, but...
    m_pendingList.clear();
    m_finishedList.clear();
    m_currentPendingIndex = 0;

    ZDvidReader reader;
    reader.setVerbose(false);
    if (reader.open(m_dvidTarget)) {
        std::vector<ZDvidSynapse> synapseList;
        if (m_variation == VARIATION_REGION) {
            synapseList = reader.readSynapse(m_protocolRange, FlyEM::LOAD_PARTNER_LOCATION);

            // filter by roi (coming "soon")
            // will need to do raw DVID call to batch ask "is point in RoI?";
            //  that call not in ZDvidReader() yet

        } else if (m_variation == VARIATION_BODY) {
            synapseList = reader.readSynapse(m_bodyID, FlyEM::LOAD_PARTNER_LOCATION);
        } else {
            variationError(m_variation);
        }

        // build the lists of pre-synaptic sites; if a site is
        //  already verified, then it is "finished"; if not,
        //  add to the pending synapse list that we will then sort;
        //  only after that transfer the positions to the pending position list
        QList<ZDvidSynapse> pendingSynapses;
        for (size_t i=0; i<synapseList.size(); i++) {
            ZDvidSynapse &synapse = synapseList[i];
            if (synapse.getKind() == ZDvidAnnotation::KIND_PRE_SYN) {
                if (synapse.isProtocolVerified(m_dvidTarget)) {
                    m_finishedList.append(synapse.getPosition());
                } else {
                    pendingSynapses.append(synapse);
                }
            }
        }

        // sort the pending synapse list; DVID can return things in variable order,
        //  and people don't like that
        qSort(pendingSynapses.begin(), pendingSynapses.end(), SynapsePredictionProtocol::compareSynapses);
        for (int i=0; i<pendingSynapses.size(); i++) {
            m_pendingList.append(pendingSynapses[i].getPosition());
        }
    }
}

bool SynapsePredictionProtocol::compareSynapses(const ZDvidSynapse &synapse1, const ZDvidSynapse &synapse2) {
    // how to sort?  proofreaders like to do synapses together with other
    //  nearby synapses (especially helps when untangling multi-T-bars);
    //  it would be a pain (and more calculation) to do a proper distance clustering,
    //  so instead start by sorting on body ID, which we expect will almost always
    //  be available; that already spatially clusters the T-bars

    // hmm...that helps, but doesn't help enough; if you then sort by x,y, etc.,
    //  you're still going to jump around, just within the body; maybe that's enough;
    //  the volumes we use are small, so maybe there aren't a lot of T-bars in
    //  a small volume on each body unless they are multis, which is the case
    //  we want to catch

    // but in the long run, I'm starting to think I may need to calculate
    //  the distance matrix and do some clustering; at least if we group
    //  first by body ID, that would cut down on the amount of calculation

    // August 2016: we now store a "GroupedWith" relationship in DVID,
    //  expected to be used for noting T-bars that are part of a multi-T-bar;
    //  we should at some point group them in the list, too

    // but for now, it's sort by body ID, then x, then y (ignore z)
    if (synapse1.getBodyId() < synapse2.getBodyId()) {
        return true;
    } else if (synapse1.getBodyId() > synapse2.getBodyId()) {
        return false;
    } else {
        if (synapse1.getPosition().getX() < synapse2.getPosition().getX()) {
            return true;
        } else if (synapse1.getPosition().getX() > synapse2.getPosition().getX()) {
            return false;
        } else {
            return ((synapse1.getPosition().getY() < synapse2.getPosition().getY()));
        }
    }
}

void SynapsePredictionProtocol::clearSitesTable() {
    m_sitesModel->clear();
    setSitesHeaders(m_sitesModel);
}

void SynapsePredictionProtocol::updateSitesTable(std::vector<ZDvidSynapse> synapse) {
    clearSitesTable();

    // currently plan to rebuild from scratch each time
    if (m_currentPendingIndex >= 0 && m_currentPendingIndex < m_pendingList.size()) {

        // note: post synaptic sites start at index 1, but the
        //  table row still starts at 0
        for (size_t i=1; i<synapse.size(); i++) {
            ZDvidSynapse site = synapse[i];

            // need to exclude other things that could be linked;
            //  eg, other T-bars (in a multi- or convergent configuration)
            if (site.getKind() != ZDvidAnnotation::KIND_POST_SYN) {
                continue;
            }

            if (site.isVerified()) {
                // text marker in "status" column
                QStandardItem * statusItem = new QStandardItem();
                statusItem->setData(QVariant(QString::fromUtf8("\u2714")), Qt::DisplayRole);
                m_sitesModel->setItem(i - 1, SITES_STATUS_COLUMN, statusItem);
            }

            QStandardItem * confItem = new QStandardItem();
            confItem->setData(QVariant(site.getConfidence()), Qt::DisplayRole);
            m_sitesModel->setItem(i - 1, SITES_CONFIDENCE_COLUMN, confItem);

            QStandardItem * xItem = new QStandardItem();
            QStandardItem * yItem = new QStandardItem();
            QStandardItem * zItem = new QStandardItem();
            xItem->setData(QVariant(site.getX()), Qt::DisplayRole);
            yItem->setData(QVariant(site.getY()), Qt::DisplayRole);
            zItem->setData(QVariant(site.getZ()), Qt::DisplayRole);
            m_sitesModel->setItem(i - 1, SITES_X_COLUMN, xItem);
            m_sitesModel->setItem(i - 1, SITES_Y_COLUMN, yItem);
            m_sitesModel->setItem(i - 1, SITES_Z_COLUMN, zItem);
        }
#if QT_VERSION >= 0x050000
        ui->sitesTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->sitesTableView->horizontalHeader()->setSectionResizeMode(SITES_CONFIDENCE_COLUMN, QHeaderView::Stretch);
#else
        ui->sitesTableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
        ui->sitesTableView->horizontalHeader()->setResizeMode(SITES_CONFIDENCE_COLUMN, QHeaderView::Stretch);
#endif
    }
}

void SynapsePredictionProtocol::onDoubleClickSitesTable(QModelIndex index) {
    QStandardItem *itemX = m_sitesModel->item(index.row(), SITES_X_COLUMN);
    int x = itemX->data(Qt::DisplayRole).toInt();

    QStandardItem *itemY = m_sitesModel->item(index.row(), SITES_Y_COLUMN);
    int y = itemY->data(Qt::DisplayRole).toInt();

    QStandardItem *itemZ = m_sitesModel->item(index.row(), SITES_Z_COLUMN);
    int z = itemZ->data(Qt::DisplayRole).toInt();

    emit requestDisplayPoint(x, y, z);
}

// input: point (pre- or post-synaptic site)
// output: array with first pre-synaptic site then all post-synaptic sites
//  for the synapse; returns empty list on errors
std::vector<ZDvidSynapse> SynapsePredictionProtocol::getWholeSynapse(ZIntPoint point) {

    std::vector<ZDvidSynapse> result;

    ZDvidReader reader;
    if (reader.open(m_dvidTarget)) {
        ZDvidSynapse synapse = reader.readSynapse(point, FlyEM::LOAD_PARTNER_LOCATION);

        // find the presynaptic site
        if (!(synapse.getKind() == ZDvidAnnotation::KIND_PRE_SYN)) {
            if (synapse.getPartners().size() > 0) {
                ZIntPoint preLocation = synapse.getPartners().front();
                synapse = reader.readSynapse(preLocation, FlyEM::LOAD_PARTNER_LOCATION);
            } else {
                // can't find presynaptic site, so give up
                return result;
            }
        }
        result.push_back(synapse);

        // get all the post-synaptic sites
        std::vector<ZIntPoint> psdArray = synapse.getPartners();
        for (size_t i=0; i<psdArray.size(); i++) {
            ZDvidSynapse post = reader.readSynapse(psdArray[i], FlyEM::LOAD_NO_PARTNER);
            result.push_back(post);
        }
    }
    return result;
}

void SynapsePredictionProtocol::setSitesHeaders(QStandardItemModel * model) {
    model->setHorizontalHeaderItem(SITES_STATUS_COLUMN, new QStandardItem(QString("V")));
    model->setHorizontalHeaderItem(SITES_CONFIDENCE_COLUMN, new QStandardItem(QString("Conf")));
    model->setHorizontalHeaderItem(SITES_X_COLUMN, new QStandardItem(QString("x")));
    model->setHorizontalHeaderItem(SITES_Y_COLUMN, new QStandardItem(QString("y")));
    model->setHorizontalHeaderItem(SITES_Z_COLUMN, new QStandardItem(QString("z")));
}

void SynapsePredictionProtocol::variationError(std::string variation) {
    QMessageBox mb;
    mb.setText("Unknown protocol variation!");
    mb.setInformativeText("Unknown protocol variation " + QString::fromStdString(variation) + " was encountered!  Report this error!");
    mb.setStandardButtons(QMessageBox::Ok);
    mb.setDefaultButton(QMessageBox::Ok);
    mb.exec();
}

SynapsePredictionProtocol::~SynapsePredictionProtocol()
{
    delete ui;
}
