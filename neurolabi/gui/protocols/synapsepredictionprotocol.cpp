#include "synapsepredictionprotocol.h"
#include "ui_synapsepredictionprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QInputDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QtAlgorithms>

#include "synapsepredictioninputdialog.h"
#include "synapsepredictionbodyinputdialog.h"

#include "dvid/zdvidreader.h"
#include "dvid/zdvidsynapse.h"
#include "logging/zqslog.h"
#include "zjsonarray.h"
#include "zjsonobject.h"
#include "zjsonparser.h"
#include "geometry/zintcuboid.h"
#include "geometry/zintpoint.h"
#include "geometry/zpoint.h"
#include "zjsonfactory.h"
#include "zjsonarray.h"

SynapsePredictionProtocol::SynapsePredictionProtocol(QWidget *parent, std::string variation) :
    ProtocolDialog(parent),
    ui(new Ui::SynapsePredictionProtocol)
{
    m_variation = variation;

    ui->setupUi(this);

    // mode menu
    ui->modeMenu->clear();
    ui->modeMenu->addItem(MODE_SYNAPSE);
    ui->modeMenu->addItem(MODE_TBAR);
    ui->modeMenu->addItem(MODE_PSD);
    ui->modeMenu->setCurrentIndex(0);
    m_currentMode = MODE_SYNAPSE;
    // connect(ui->modeMenu, SIGNAL(currentIndexChanged(QString)), this, SLOT(onModeChanged(QString)));
    connect(ui->modeMenu, SIGNAL(activated(QString)), this, SLOT(onModeChanged(QString)));

    // sites table
    m_sitesModel = new QStandardItemModel(0, 5, ui->sitesTableView);
    setSitesHeaders(m_sitesModel);
    m_sitesProxy = new QSortFilterProxyModel(this);
    m_sitesProxy->setSourceModel(m_sitesModel);
    m_sitesProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    ui->sitesTableView->setModel(m_sitesProxy);

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

    connect(ui->gridCheckBox, SIGNAL(clicked(bool)), this, SLOT(onGridToggled()));

    // misc UI setup
    setupColorList();

    ui->buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

    m_currentPendingIndex = 0;
    m_currentFinishedIndex = 0;
}

const std::string SynapsePredictionProtocol::KEY_VARIATION = "variation";
const std::string SynapsePredictionProtocol::KEY_SUBVARIATION = "subvariation";
const std::string SynapsePredictionProtocol::VARIATION_REGION = "region";
const std::string SynapsePredictionProtocol::VARIATION_BODY = "body";
const std::string SynapsePredictionProtocol::SUBVARIATION_BODY_TBAR = "T-bar";
const std::string SynapsePredictionProtocol::SUBVARIATION_BODY_PSD = "PSD";
const std::string SynapsePredictionProtocol::SUBVARIATION_BODY_BOTH = "both";
const std::string SynapsePredictionProtocol::KEY_VERSION = "version";
const std::string SynapsePredictionProtocol::KEY_PROTOCOL_RANGE = "range";
const std::string SynapsePredictionProtocol::KEY_BODYID= "body ID";
const std::string SynapsePredictionProtocol::KEY_MODE= "mode";
const int SynapsePredictionProtocol::fileVersion = 3;
const QColor SynapsePredictionProtocol::COLOR_DEFAULT = QColor(0, 0, 0);      // no color
const QColor SynapsePredictionProtocol::COLOR_TARGET_BODY = QColor(102, 204, 255);      // medium mild blue
const QString SynapsePredictionProtocol::MODE_SYNAPSE = "whole synapses";
const QString SynapsePredictionProtocol::MODE_TBAR = "T-bars only";
const QString SynapsePredictionProtocol::MODE_PSD = "PSDs only";

/*
 * start the protocol anew; returns success status;
 * initialize is also expected to do the first save
 * of the protocol's data
 */
bool SynapsePredictionProtocol::initialize() {

    if (m_variation == VARIATION_REGION) {
        ui->gridCheckBox->show();

        SynapsePredictionInputDialog inputDialog;

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

        setRange(volume);
//        m_protocolRange = volume;

    } else if (m_variation == VARIATION_BODY) {

        // grid isn't used in this variation
        ui->gridCheckBox->hide();

        SynapsePredictionBodyInputDialog inputDialog;

        int ans = inputDialog.exec();
        if (ans == QDialog::Rejected) {
            return false;
        }
        if (!inputDialog.hasBodyID()) {
            return false;
        }
        m_subvariation = inputDialog.getMode();
        uint64_t bodyID = inputDialog.getBodyID();
        const ZDvidReader &reader = m_dvidReader;
        if (reader.good()) {
            if (!reader.hasBody(bodyID)) {
                QMessageBox mb;
                mb.setText("Body ID doesn't exist!");
                mb.setInformativeText("The entered body ID " + QString::number(bodyID)  + " doesn't seem to exist!");
                mb.setStandardButtons(QMessageBox::Ok);
                mb.setDefaultButton(QMessageBox::Ok);
                mb.exec();
                return false;
            } else {
                m_bodyID = bodyID;
            }
        } else {
            return false;
        }
    } else {
        variationError(m_variation);
        return false;
    }

    // initial color map is nothing
    enableProtocolColorMap();

    // generate pending/finished lists from user input
    // throw this into a thread?
    loadInitialSynapseList();
    updateSiteListLabel();

    // get started
    onFirstButton();
    saveState();

    return true;
}

ZIntCuboid SynapsePredictionProtocol::getRange() const
{
  return m_protocolRange;
}

void SynapsePredictionProtocol::setRange(const ZIntCuboid &range)
{
  m_protocolRange = range;
  emit rangeChanged(range.getFirstCorner(), range.getLastCorner());
}

void SynapsePredictionProtocol::setRange(const ZJsonArray &rangeJson)
{
  ZIntCuboid range;
  range.loadJson(rangeJson);
  setRange(range);
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
    if (!isFinished(currentPoint)) {
        QMessageBox mb;
        mb.setText("Not all verified!");
        if (m_currentMode == MODE_SYNAPSE) {
            mb.setInformativeText("Not all elements of the current T-bar are verified!  Verify the T-bar and all PSDs before finishing.");
        } else if (m_currentMode == MODE_TBAR) {
            mb.setInformativeText("This T-bar is not verified!  Verify it before finishing.");
        } else if (m_currentMode == MODE_PSD) {
            mb.setInformativeText("Not all PSDs of the current T-bar are verified!  Verify all PSDs before finishing.");
        }
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

        // restore original color map
        disableProtocolColorMap();

        emit protocolCompleting();
    }
}

void SynapsePredictionProtocol::onRefreshButton()
{
    refreshData(true);
}

/*
 * refresh the data in the protocol and update labels; current
 * T-bar is preserved if possible; if unfinishCurrent = true,
 * it will be moved from the finished list back to pending if
 * it moved to finished during the refresh (there are reasons
 * for wanting both behaviors)
 */
void SynapsePredictionProtocol::refreshData(bool unfinishCurrent) {
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
            //  effect of the refresh; if desired, move it back from
            //  the finished list to the pending list, at the same position
            if (unfinishCurrent) {
                m_finishedList.removeOne(savedPoint);
                m_pendingList.insert(savedIndex, savedPoint);
                m_currentPendingIndex = savedIndex;
            }
        }
    }
    updateLabels();
}

void SynapsePredictionProtocol::onExitButton() {
    // restore original color map
    disableProtocolColorMap();

    // exit protocol; can be reopened and worked on later
    emit protocolExiting();
}

void SynapsePredictionProtocol::onModeChanged(QString item) {
    // full refresh is needed to update the pending and finished lists;
    //  maybe an "are you sure?" dialog is useful, since it takes some time?
    //  then save so the change is persisted
    m_currentMode = item;
    refreshData(false);
    saveState();
}

void SynapsePredictionProtocol::onGridToggled() {
    // note that we use emit rangeChanged(), which triggers the visual
    //  grid update, and not setRange(), which also changes our local data;
    //  when we hide the grid, we're not actually changing the protocol range
    if (ui->gridCheckBox->isChecked()) {
        emit rangeChanged(m_protocolRange.getFirstCorner(), m_protocolRange.getLastCorner());
    } else {
        emit rangeChanged(ZIntPoint(0, 0, 0), ZIntPoint(-1, -1, -1));
    }
}

/*
 * is the synapse at the input point finished being reviewed under the current mode?
 */
bool SynapsePredictionProtocol::isFinished(ZIntPoint point) {
    return isFinished(getWholeSynapse(point, m_dvidReader));
}

/* as above, but input is vector of synapse elements returned by getWholeSynapse(),
 * in which T-bar element is first
 */
bool SynapsePredictionProtocol::isFinished(std::vector<ZDvidSynapse> synapseElements) {
    bool tbarVerified = synapseElements[0].isVerified();
    if (m_currentMode == MODE_TBAR) {
        return tbarVerified;
    }

    bool psdVerified = true;
    for (size_t i=1; i<synapseElements.size(); i++) {
        if (!synapseElements[i].isVerified()) {
            psdVerified = false;
            break;
        }
    }

    if (m_currentMode == MODE_PSD) {
        return psdVerified;
    } else if (m_currentMode == MODE_SYNAPSE) {
        return tbarVerified && psdVerified;
    } else {
        // unknown mode!  log it and say no
        LINFO() << "unknown mode found in SynapsePredictionProtocol::isfinished():" << m_currentMode;
        return false;
    }
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
        data.setEntry(KEY_SUBVARIATION.c_str(), m_subvariation);
    } else {
        variationError(m_variation);
        return;
    }

    data.setEntry(KEY_MODE.c_str(), m_currentMode.toStdString().c_str());

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
        // 1 to 2: in v2, we added the "variation" key; in v1, that was always "region"
        if (version == 1) {
            data.setEntry(KEY_VARIATION.c_str(), VARIATION_REGION.c_str());
            version = 2;
        }

        // 2 to 3:
        if (version == 2) {
            // added "mode" key; in previous versions, it was always "Whole synapses"
            data.setEntry(KEY_MODE.c_str(), MODE_SYNAPSE.toStdString().c_str());

            // added subvariation to body variation; old one was always T-bar:
            if (m_variation == VARIATION_BODY) {
                data.setEntry(KEY_SUBVARIATION.c_str(), SUBVARIATION_BODY_TBAR.c_str());
            }

            version = 3;
        }

        updated = true;
    }

    // must get mode before synapse list is retrieved
    m_currentMode = QString::fromUtf8(
          ZJsonParser::stringValue(data[KEY_MODE.c_str()]).c_str());
    ui->modeMenu->setCurrentText(m_currentMode);

    // variation specific loading:
    if (m_variation == VARIATION_REGION) {
        ui->gridCheckBox->show();
        setRange(ZJsonArray(data.value(KEY_PROTOCOL_RANGE.c_str())));
        if (!m_protocolRange.isEmpty()) {
            loadInitialSynapseList();
        } else {
            ui->progressLabel->setText("Invalid protocol range. No data loaded!");
            return;
        }
    } else if (m_variation == VARIATION_BODY) {
        ui->gridCheckBox->hide();
        m_subvariation = ZJsonParser::stringValue(data[KEY_SUBVARIATION.c_str()]);
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

    enableProtocolColorMap();
    updateSiteListLabel();
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
  ZDvidReader &reader = m_dvidReader;
  ZIntPoint targetPoint = pt;
  bool isVerified = true;
  if (reader.good()) {
    ZDvidSynapse synapse =
        reader.readSynapse(pt, dvid::EAnnotationLoadMode::PARTNER_LOCATION);

    if (synapse.getKind() == ZDvidAnnotation::EKind::KIND_PRE_SYN) {
      std::vector<ZIntPoint> psdArray = synapse.getPartners();
      for (std::vector<ZIntPoint>::const_iterator iter = psdArray.begin();
           iter != psdArray.end(); ++iter) {
        const ZIntPoint &pt = *iter;
        ZDvidSynapse synapse =
            reader.readSynapse(pt, dvid::EAnnotationLoadMode::NO_PARTNER);
        if (!synapse.isVerified()) {
          isVerified = false;
          break;
        }
      }
      if (!synapse.isVerified()) {
          isVerified = false;
      }
    } else if (synapse.getKind() == ZDvidAnnotation::EKind::KIND_POST_SYN) {
      std::vector<ZIntPoint> partnerArray = synapse.getPartners();
      if (!partnerArray.empty()) {
        targetPoint = partnerArray.front();
        ZDvidSynapse presyn =
            reader.readSynapse(targetPoint, dvid::EAnnotationLoadMode::PARTNER_LOCATION);

        std::vector<ZIntPoint> psdArray = presyn.getPartners();
        for (std::vector<ZIntPoint>::const_iterator iter = psdArray.begin();
             iter != psdArray.end(); ++iter) {
          const ZIntPoint &pt = *iter;
          ZDvidSynapse synapse =
              reader.readSynapse(pt, dvid::EAnnotationLoadMode::NO_PARTNER);
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
  ZDvidReader &reader = m_dvidReader;
  ZIntPoint targetPoint = pt;

  if (reader.good()) {
    ZDvidSynapse synapse =
        reader.readSynapse(pt, dvid::EAnnotationLoadMode::PARTNER_LOCATION);

    if (synapse.getKind() == ZDvidAnnotation::EKind::KIND_POST_SYN) {
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

void SynapsePredictionProtocol::updateSiteListLabel() {
    QString message = "Site list: ";
    if (m_variation == VARIATION_REGION) {
        message += "T-bars within ";
        message += QString::fromStdString(m_protocolRange.getFirstCorner().toString());
        message += " to ";
        message += QString::fromStdString(m_protocolRange.getLastCorner().toString());
    } else if (m_variation == VARIATION_BODY) {
        if (m_subvariation == SUBVARIATION_BODY_TBAR) {
            message += "T-bars";
        } else if (m_subvariation == SUBVARIATION_BODY_PSD) {
            message += "T-bars with PSDs";
        } else if (m_subvariation == SUBVARIATION_BODY_BOTH) {
            message += "synapses";
        }
        message += " on body";
        // color the body ID; we'll use that color to indicate sites on that body
        ui->siteListLabel2->setStyleSheet(targetBodyStylesheet(COLOR_TARGET_BODY));
        ui->siteListLabel2->setText(QString::number(m_bodyID));
        ui->siteListLabel2->setToolTip("T-bars and PSDs on this body will be highlighted in the same color");
    } else {
        variationError(m_variation);
    }
    ui->siteListLabel->setText(message);
}

void SynapsePredictionProtocol::updateLabels() {
    // currently we update all labels here while calling methods
    //  to do the table and color map

    // current presynaptic sites labels:
    if (m_currentPendingIndex >= 0 && m_currentPendingIndex < m_pendingList.size()) {
        ZIntPoint currentPoint = m_pendingList[m_currentPendingIndex];
        std::vector<ZDvidSynapse> synapse = getWholeSynapse(
              currentPoint, m_dvidReader);

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
        if (m_currentMode != MODE_PSD) {
            ui->preConfLabel->setText(QString("Confidence: %1").arg(synapse[0].getConfidence(), 3, 'f', 1));
            if (synapse[0].isVerified()) {
                ui->preStatusLabel->setText(QString("Verified: yes"));
            } else {
                ui->preStatusLabel->setText(QString("Verified: no"));
            }
        } else {
            // in PSD mode, don't show T-bar info
            ui->preConfLabel->setText(QString("Confidence: n/a"));
            ui->preStatusLabel->setText(QString("Verified: n/a"));
        }

        int nPSDverified = 0;
        for (size_t i=1; i<synapse.size(); i++) {
            if (synapse[i].isVerified()) {
                nPSDverified++;
            }
        }
        if (m_currentMode != MODE_TBAR) {
            ui->postTableLabel->setText(QString("PSDs (%1/%2 verified)").arg(nPSDverified).arg(synapse.size() - 1));
        } else {
            // in T-bar mode, don't show PSD info
            ui->postTableLabel->setText(QString("PSDs"));
        }

        // if we have segmentation, we also color stuff; get body IDs for all locations
        std::vector<uint64_t> bodyList = getBodiesForSynapse(synapse);

        // color labels for T-bar if we're in body variation; remember, the
        //  first synaptic element is the T-bar
        if (m_variation == VARIATION_BODY && bodyList.size() > 0 && m_bodyID == bodyList[0]) {
            ui->preLocationLabel->setStyleSheet(targetBodyStylesheet(COLOR_TARGET_BODY));
        } else {
            ui->preLocationLabel->setStyleSheet("");
        }

        updateSitesTable(synapse, bodyList);

        if (bodyList.size() > 0) {
            updateColorMap(synapse, bodyList);
        }
    } else {
        ui->preLocationLabel->setText(QString("(--, --, --)"));
        ui->preConfLabel->setText(QString("Confidence: --"));
        ui->preStatusLabel->setText(QString("Verified: --"));

        clearSitesTable();
    }

    // progress label is same for all modes:
    int nPending = m_pendingList.size();
    int nFinished = m_finishedList.size();
    int nTotal = nPending + nFinished;
    float percent = (100.0 * nFinished) / nTotal;
    ui->progressLabel->setText(QString("Progress:\n\n %1 / %2 (%3%)").arg(nFinished).arg(nTotal).arg(percent, 4, 'f', 1));
}

/*
 * given a list of synaptic elements, return a list of body IDs at their locations;
 * returns empty list if we have no segmentation
 */
std::vector<uint64_t> SynapsePredictionProtocol::getBodiesForSynapse(std::vector<ZDvidSynapse> synapse) {
    std::vector<ZIntPoint> sites;
    for (size_t i=0; i<synapse.size(); i++) {
        sites.push_back(synapse[i].getPosition());
    }
    ZDvidReader &reader = m_dvidReader;
    std::vector<uint64_t> bodyList;
    if (reader.good()) {
      bodyList = reader.readBodyIdAt(sites);
    }
    return bodyList;
}

void SynapsePredictionProtocol::loadInitialSynapseList()
{
    // I don't *think* there's any way these lists will already be populated, but...
    m_pendingList.clear();
    m_finishedList.clear();
    m_currentPendingIndex = 0;

    ZDvidReader &reader = m_dvidReader;
//    reader.setVerbose(false);
    ZDvidReader::PauseVerbose pv(&reader);

    if (reader.good()) {
        QProgressDialog progressDialog("Loading synapses...", 0, 0, 100, this);
        progressDialog.setWindowModality(Qt::WindowModal);
        progressDialog.setMinimumDuration(1000);
        progressDialog.setValue(20);

        std::vector<ZDvidSynapse> synapseList;
        if (m_variation == VARIATION_REGION) {
            synapseList = reader.readSynapse(m_protocolRange, dvid::EAnnotationLoadMode::PARTNER_LOCATION);
        } else if (m_variation == VARIATION_BODY) {
            synapseList = reader.readSynapse(m_bodyID, dvid::EAnnotationLoadMode::PARTNER_LOCATION);
        } else {
            variationError(m_variation);
        }

        // do the filter by roi here (coming "soon")
        // will need to do raw DVID call to batch ask "is point in RoI?";
        //  that call not in ZDvidReader() yet

        // look at pre-synaptic sites; check whether the appropriate
        //  items have been verified according to the mode we're in; if so, its
        //  position goes in the finished list
        // update progress dialog approximate every 5%; some ad hoc testing indicated
        //  that the initial read (above) is about 20% of the total (note setValue() above),
        //  so scale based on that
        int progressInterval = 1;
        if (synapseList.size() >= 16) {
            progressInterval = synapseList.size() / 16;
        }
        QList<ZDvidSynapse> pendingSynapses;
        for (size_t i=0; i<synapseList.size(); i++) {
            if (i % progressInterval == 0) {
                progressDialog.setValue(20 + 5 * (i / progressInterval));
            }
            ZDvidSynapse &synapse = synapseList[i];
            std::vector<ZDvidSynapse> wholeSynapse = getWholeSynapse(
                  synapse.getPosition(), m_dvidReader);
            if (keepSynapse(synapse)) {
                // if synapse is post, get its pre (T-bar)
                if (synapse.getKind() == ZDvidAnnotation::EKind::KIND_POST_SYN) {
                    synapse = wholeSynapse[0];
                }

                if (isFinished(wholeSynapse)) {
                    m_finishedList.append(synapse.getPosition());
                } else {
                    // collect the pending synapses for later sorting; otherwise,
                    //  we'd just add position to m_pendingList directly
                    pendingSynapses.append(synapse);
                }
            }
        }

        // sort the pending synapse list; DVID can return things in variable order,
        //  and people don't like that; then put ordered positions into pending list
        qSort(pendingSynapses.begin(), pendingSynapses.end(), SynapsePredictionProtocol::compareSynapses);
        for (int i=0; i<pendingSynapses.size(); i++) {
            m_pendingList.append(pendingSynapses[i].getPosition());
        }

        progressDialog.setValue(100);
    }
}

/*
 * is this a synapse we want to look at in given the variation and subvariation?
 */
bool SynapsePredictionProtocol::keepSynapse(ZDvidSynapse synapse) {
    // for region, want T-bars; likewise body/T-bar;
    //  for body/PSD, keep the post, and for body/both, keep all
    if (m_variation == VARIATION_REGION) {
        return synapse.getKind() == ZDvidAnnotation::EKind::KIND_PRE_SYN;
    } else if (m_variation == VARIATION_BODY) {
        if (m_subvariation == SUBVARIATION_BODY_TBAR) {
            return synapse.getKind() == ZDvidAnnotation::EKind::KIND_PRE_SYN;
        } else if (m_subvariation == SUBVARIATION_BODY_PSD) {
            return synapse.getKind() == ZDvidAnnotation::EKind::KIND_POST_SYN;
        } else if (m_subvariation == SUBVARIATION_BODY_BOTH) {
            return true;
        } else {
            variationError("found unexpected subvariation while loading synapses: " + m_subvariation);
            return false;
        }
    } else {
        variationError("found unexpected variation while loading synapses: " + m_variation);
        return false;
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

void SynapsePredictionProtocol::updateSitesTable(std::vector<ZDvidSynapse> synapse,
    std::vector<uint64_t> bodyList) {
    clearSitesTable();

    // don't show PSD info if we're in T-bar mode
    if (m_currentMode == MODE_TBAR) {
        return;
    }

    // currently plan to rebuild from scratch each time
    if (m_currentPendingIndex >= 0 && m_currentPendingIndex < m_pendingList.size()) {

        // save sort order to restore later
        Qt::SortOrder sortOrder = m_sitesProxy->sortOrder();
        int sortColumn = m_sitesProxy->sortColumn();

        // note: post synaptic sites start at index 1, but the
        //  table row still starts at 0
        for (size_t i=1; i<synapse.size(); i++) {
            ZDvidSynapse site = synapse[i];

            // need to exclude other things that could be linked;
            //  eg, other T-bars (in a multi- or convergent configuration)
            if (site.getKind() != ZDvidAnnotation::EKind::KIND_POST_SYN) {
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
            if (m_variation == VARIATION_BODY && bodyList.size() > 0 && m_bodyID == bodyList[i]) {
                // color the PSDs on the body of interest
                xItem->setData(COLOR_TARGET_BODY, Qt::BackgroundRole);
                yItem->setData(COLOR_TARGET_BODY, Qt::BackgroundRole);
                zItem->setData(COLOR_TARGET_BODY, Qt::BackgroundRole);
            }
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

        // restore sort order
        ui->sitesTableView->horizontalHeader()->setSortIndicator(sortColumn, sortOrder);
    }
}

void SynapsePredictionProtocol::onDoubleClickSitesTable(QModelIndex tableIndex) {
    QModelIndex modelIndex = m_sitesProxy->mapToSource(tableIndex);

    QStandardItem *itemX = m_sitesModel->item(modelIndex.row(), SITES_X_COLUMN);
    int x = itemX->data(Qt::DisplayRole).toInt();

    QStandardItem *itemY = m_sitesModel->item(modelIndex.row(), SITES_Y_COLUMN);
    int y = itemY->data(Qt::DisplayRole).toInt();

    QStandardItem *itemZ = m_sitesModel->item(modelIndex.row(), SITES_Z_COLUMN);
    int z = itemZ->data(Qt::DisplayRole).toInt();

    emit requestDisplayPoint(x, y, z);
}

// input: point (pre- or post-synaptic site)
// output: array with first pre-synaptic site then all post-synaptic sites
//  for the synapse; returns empty list on errors
std::vector<ZDvidSynapse> SynapsePredictionProtocol::getWholeSynapse(
    ZIntPoint point, const ZDvidReader &reader) {

    std::vector<ZDvidSynapse> result;

    if (reader.good()) {
        ZDvidSynapse synapse = reader.readSynapse(point, dvid::EAnnotationLoadMode::PARTNER_LOCATION);

        // find the presynaptic site
        if (!(synapse.getKind() == ZDvidAnnotation::EKind::KIND_PRE_SYN)) {
            if (synapse.getPartners().size() > 0) {
                ZIntPoint preLocation = synapse.getPartners().front();
                synapse = reader.readSynapse(preLocation, dvid::EAnnotationLoadMode::PARTNER_LOCATION);
            } else {
                // can't find presynaptic site, so give up
                return result;
            }
        }
        result.push_back(synapse);

        // get all the post-synaptic sites
        std::vector<ZIntPoint> psdArray = synapse.getPartners();
        for (size_t i=0; i<psdArray.size(); i++) {
            ZDvidSynapse post = reader.readSynapse(psdArray[i], dvid::EAnnotationLoadMode::NO_PARTNER);
            // we've been seeing some blank lines in the PSD table; I think
            //  they might be due to unlinked PSDs, and this might catch them:
            if (post.isValid()) {
                result.push_back(post);
            } else {
                LINFO() << "found invalid PSD at " << psdArray[i].toString();
            }
        }
    }
    return result;
}

/*
 * returns stylesheet string for a color for label background color
 */
QString SynapsePredictionProtocol::targetBodyStylesheet(QColor color) {
    return QString("QLabel { background-color : rgb(%1, %2, %3); }").arg(color.red()).arg(color.green()).arg(color.blue());
}

void SynapsePredictionProtocol::setupColorList() {
    // these are the colors I used in Raveler for its PSD protocol; they're
    //  chosen to be fairly bright
    m_postColorList.clear();
    m_postColorList << QColor(0, 255, 0)     // green
                    << QColor(255, 0, 0)     // red
                    << QColor(0, 128, 255)   // blue
                    << QColor(128, 0, 255)   // purple
                    << QColor(255, 128, 0)   // orange
                    << QColor(0, 255, 128)   // blue-green
                    << QColor(0, 0, 255)     // dark blue
                    << QColor(255, 0, 128)   // purple-red
                    << QColor(255, 255, 0)   // yellow
                    << QColor(255, 0, 255)  // pink-purple
                    << QColor(128, 255, 0)  // yellow-green
                    << QColor(0, 255, 255)  // light blue
                    ;

}

/*
 * return a color out of the table for a given index
 */
QColor SynapsePredictionProtocol::getColor(int index) {
    return m_postColorList[index % m_postColorList.size()];
}

/*
 * update the color map so bodies with verified PSDs are colored;
 * input: list of synaptic elements (T-bar first) and list
 *      of body ID at each element position
 *
 * this method isn't called if there is no segmentation
 */
void SynapsePredictionProtocol::updateColorMap(std::vector<ZDvidSynapse> synapses,
    std::vector<uint64_t> bodyList) {
    // remember, the first element of the incoming vector is the pre-synaptic
    //  element (the T-bar)
    if (synapses.size() < 2) {
        // no post-synaptic elements
        return;
    }

    m_colorScheme.clear();

    // in T-bar mode, no coloring:
    if (m_currentMode == MODE_TBAR) {
        emit requestColorMapChange(m_colorScheme);
        return;
    }

    // color the T-bar (i=0)
    m_colorScheme.setBodyColor(bodyList[0], getColor(0));

    // color the verified post-synaptic sites; note that
    //  by using the overall synapse index, we keep the colors
    // constant as the PSDs are verified
    for (size_t i=0; i<synapses.size(); i++) {
        if (synapses[i].isVerified()) {
            m_colorScheme.setBodyColor(bodyList[i], getColor(i));
        }
    }
    m_colorScheme.buildColorTable();
    emit requestColorMapChange(m_colorScheme);
}

/*
 * when you enter the protocol, switch to the protocol color map
 */
void SynapsePredictionProtocol::enableProtocolColorMap() {
    m_colorScheme.clear();
    m_colorScheme.setDefaultColor(COLOR_DEFAULT);
    emit requestColorMapChange(m_colorScheme);
    emit requestActivateColorMap();
}

void SynapsePredictionProtocol::disableProtocolColorMap() {
    emit requestDeactivateColorMap();
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
