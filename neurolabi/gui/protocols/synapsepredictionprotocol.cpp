#include "synapsepredictionprotocol.h"
#include "ui_synapsepredictionprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QtGui>
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

    // sites table
    // currently not putting in the confidence column:
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
    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));
    connect(ui->refreshButton, SIGNAL(clicked(bool)), this, SLOT(onRefreshButton()));

    connect(ui->sitesTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickSitesTable(QModelIndex)));

    // misc UI setup
    ui->buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

    m_currentPendingIndex = 0;
    m_currentFinishedIndex = 0;
}

// protocol name should not contain hyphens
const std::string SynapsePredictionProtocol::PROTOCOL_NAME = "synapse_prediction";
const std::string SynapsePredictionProtocol::KEY_VERSION = "version";
const std::string SynapsePredictionProtocol::KEY_PROTOCOL_RANGE = "range";
const int SynapsePredictionProtocol::fileVersion = 1;


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

    ZJsonArray rangeJson = ZJsonFactory::MakeJsonArray(m_protocolRange);
    data.setEntry(KEY_PROTOCOL_RANGE.c_str(), rangeJson);

    // always version your output files!
    data.setEntry(KEY_VERSION.c_str(), fileVersion);

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

    // current item label:
    if (m_currentPendingIndex >= 0 && m_currentPendingIndex < m_pendingList.size()) {
      ZIntPoint currentPoint = m_pendingList[m_currentPendingIndex];
        ui->currentLabel->setText(QString("Current: %1, %2, %3").arg(currentPoint.getX())
            .arg(currentPoint.getY()).arg(currentPoint.getZ()));
    } else {
        ui->currentLabel->setText(QString("Current: --, --, --"));
    }

    // current item table:
    updateSitesTable();

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

void SynapsePredictionProtocol::updateSitesTable() {
    m_sitesModel->clear();

    // currently plan to rebuild from scratch each time
    if (m_currentPendingIndex >= 0 && m_currentPendingIndex < m_pendingList.size()) {

        ZIntPoint location = m_pendingList.at(m_currentPendingIndex);
        std::vector<ZDvidSynapse> synapse = getWholeSynapse(location);

        // testing
        std::cout << "synpre::updateTable: got synapse" << std::endl;
        std::cout << "pre-syn site at " << synapse.front().getPosition().toString() << std::endl;
        std::cout << "# post-syn sites = " << synapse.size() - 1 << std::endl;

        // for indicating current table

        for (size_t i=0; i<synapse.size(); i++) {
            ZDvidSynapse site = synapse[i];

            // we will need to style the current item differently
            bool isCurrent = (site.getPosition() == location);
            QFont boldFont;
            boldFont.setBold(true);

            if (site.getKind() == ZDvidAnnotation::KIND_PRE_SYN) {
                // this is just a marker in the "pre" column
                QStandardItem * preItem = new QStandardItem();
                preItem->setData(QVariant(QString("•")), Qt::DisplayRole);
                if (isCurrent) {
                    preItem->setData(boldFont, Qt::FontRole);
                }
                m_sitesModel->setItem(i, SITES_PRE_COLUMN, preItem);
            }

            QStandardItem * xItem = new QStandardItem();
            QStandardItem * yItem = new QStandardItem();
            QStandardItem * zItem = new QStandardItem();
            xItem->setData(QVariant(site.getX()), Qt::DisplayRole);
            yItem->setData(QVariant(site.getX()), Qt::DisplayRole);
            zItem->setData(QVariant(site.getX()), Qt::DisplayRole);
            if (isCurrent) {
                xItem->setData(boldFont, Qt::FontRole);
                yItem->setData(boldFont, Qt::FontRole);
                zItem->setData(boldFont, Qt::FontRole);
            }
            m_sitesModel->setItem(i, SITES_X_COLUMN, xItem);
            m_sitesModel->setItem(i, SITES_Y_COLUMN, yItem);
            m_sitesModel->setItem(i, SITES_Z_COLUMN, zItem);


            if (site.isVerified()) {
                // text marker in "status" column
                QStandardItem * statusItem = new QStandardItem();
                statusItem->setData(QVariant(QString("•")), Qt::DisplayRole);
                if (isCurrent) {
                    statusItem->setData(boldFont, Qt::FontRole);
                }
                m_sitesModel->setItem(i, SITES_STATUS_COLUMN, statusItem);
            }
        }
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
        ZDvidSynapse synapse = reader.readSynapse(point, NeuTube::FlyEM::LOAD_PARTNER_LOCATION);

        // find the presynaptic site
        if (!(synapse.getKind() == ZDvidAnnotation::KIND_PRE_SYN)) {
            if (synapse.getPartners().size() > 0) {
                ZIntPoint preLocation = synapse.getPartners().front();
                synapse = reader.readSynapse(preLocation, NeuTube::FlyEM::LOAD_PARTNER_LOCATION);
            } else {
                // can't find presynaptic site, so give up
                return result;
            }
        }
        result.push_back(synapse);

        // get all the post-synaptic sites
        std::vector<ZIntPoint> psdArray = synapse.getPartners();
        for (size_t i=0; i<psdArray.size(); i++) {
            ZDvidSynapse post = reader.readSynapse(psdArray[i], NeuTube::FlyEM::LOAD_NO_PARTNER);
            result.push_back(post);
        }
    }
    return result;
}

void SynapsePredictionProtocol::setSitesHeaders(QStandardItemModel * model) {
    model->setHorizontalHeaderItem(SITES_PRE_COLUMN, new QStandardItem(QString("Pre")));
    model->setHorizontalHeaderItem(SITES_X_COLUMN, new QStandardItem(QString("x")));
    model->setHorizontalHeaderItem(SITES_Y_COLUMN, new QStandardItem(QString("y")));
    model->setHorizontalHeaderItem(SITES_Z_COLUMN, new QStandardItem(QString("z")));
    model->setHorizontalHeaderItem(SITES_STATUS_COLUMN, new QStandardItem(QString("V")));
    // model->setHorizontalHeaderItem(SITES_CONFIDENCE_COLUMN, new QStandardItem(QString("Conf")));
}

SynapsePredictionProtocol::~SynapsePredictionProtocol()
{
    delete ui;
}
