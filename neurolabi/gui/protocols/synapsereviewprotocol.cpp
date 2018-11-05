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

/*
 * protocol to review a group of synapses (T-bars)
 *
 * adapted heavily from SynapsePredictionProtocol
 *
 */
SynapseReviewProtocol::SynapseReviewProtocol(QWidget *parent) :
    ProtocolDialog(parent),
    ui(new Ui::SynapseReviewProtocol)
{
    ui->setupUi(this);

    // sites table
    m_sitesModel = new QStandardItemModel(0, 3, ui->sitesTableView);
    setSitesHeaders(m_sitesModel);
    ui->sitesTableView->setModel(m_sitesModel);

    // UI connections
    connect(ui->reviewFirstButton, SIGNAL(clicked(bool)), this, SLOT(onReviewFirstButton()));
    connect(ui->reviewNextButton, SIGNAL(clicked(bool)), this, SLOT(onReviewNextButton()));
    connect(ui->reviewPrevButton, SIGNAL(clicked(bool)), this, SLOT(onReviewPreviousButton()));
    connect(ui->gotoCurrentButton, SIGNAL(clicked(bool)), this, SLOT(onGotoCurrentButton()));
    connect(ui->markReviewedButton, SIGNAL(clicked(bool)), this, SLOT(onMarkReviewedButton()));
    connect(ui->finishedLastButton, SIGNAL(clicked(bool)), this, SLOT(onFinishedLastButton()));
    connect(ui->finishedNextButton, SIGNAL(clicked(bool)), this, SLOT(onFinishedNextButton()));
    connect(ui->finishedPrevButton, SIGNAL(clicked(bool)), this, SLOT(onFinishedPreviousButton()));
    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));

    connect(ui->sitesTableView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(onDoubleClickSitesTable(QModelIndex)));

    // misc UI setup
    ui->buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

    // misc other
    m_currentPendingIndex = -1;
    m_currentFinishedIndex = -1;

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
    ZDvidReader &reader = m_dvidReader;
    ZDvidReader::PauseVerbose pv(&reader);
//    reader.setVerbose(false);
    if (reader.good()) {
        SynapseReviewInputDialog::SynapseReviewInputOptions option = inputDialog.getInputOption();
        if (option == SynapseReviewInputDialog::BY_BODYID) {
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
            synapseList = reader.readSynapse(bodyID, flyem::EDvidAnnotationLoadMode::PARTNER_LOCATION);

        } else if (option == SynapseReviewInputDialog::BY_VOLUME) {
            // I think volume is foolproof given our widgets; I set minimum
            //  width, etc. to 1, so we'll never get an invalid volume
            ZIntCuboid box = inputDialog.getVolume();
            synapseList = reader.readSynapse(box, flyem::EDvidAnnotationLoadMode::PARTNER_LOCATION);
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
        if (synapseList.at(i).getKind() == ZDvidAnnotation::EKind::KIND_PRE_SYN) {
            m_pendingList.append(synapseList.at(i).getPosition());
        }
    }

    // save initial data and get going
    saveState();
    onReviewFirstButton();

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

    // convert old versions to current version here, when it becomes necessary

    m_pendingList.clear();
    m_finishedList.clear();

    ZJsonArray pending(data.value(KEY_PENDING_LIST.c_str()));

    for (size_t i=0; i<pending.size(); i++) {
        m_pendingList.append(ZJsonParser::toIntPoint(pending.at(i)));
    }
    ZJsonArray finished(data.value(KEY_FINISHED_LIST.c_str()));
    for (size_t i=0; i<finished.size(); i++) {
        m_finishedList.append(ZJsonParser::toIntPoint(finished.at(i)));
    }

    // if, in the future, you need to update to a new save version,
    //  remember to do a save here

    // start work
    onReviewFirstButton();
}

void SynapseReviewProtocol::gotoCurrent() {
    if (m_currentPendingIndex >= 0) {
        ZIntPoint currentSite = m_pendingList[m_currentPendingIndex];
        emit requestDisplayPoint(currentSite.getX(), currentSite.getY(), currentSite.getZ());
    }
}

void SynapseReviewProtocol::gotoCurrentFinished() {
    if (m_currentFinishedIndex >= 0) {
        ZIntPoint currentSite = m_finishedList[m_currentFinishedIndex];
        emit requestDisplayPoint(currentSite.getX(), currentSite.getY(), currentSite.getZ());
    }
}

void SynapseReviewProtocol::updateUI() {
    updatePSDTable();
    updateLabels();
}

void SynapseReviewProtocol::updatePSDTable() {
    clearSitesTable();

    if (m_currentPendingIndex >= 0) {
        // check if T-bar at site; this list will be empty if not,
        //  will have only one element (T-bar) if no PSDs found for T-bar
        std::vector<ZDvidSynapse> synapse = getWholeSynapse(m_pendingList[m_currentPendingIndex]);
        if (synapse.size() > 1) {
            populatePSDTable(synapse);
        }
    }
}

void SynapseReviewProtocol::populatePSDTable(std::vector<ZDvidSynapse> synapse) {
    // note: post synaptic sites start at index 1, but the
    //  table row still starts at 0
    for (size_t i=1; i<synapse.size(); i++) {
        ZDvidSynapse site = synapse[i];

        // need to exclude other things that could be linked;
        //  eg, other T-bars (in a multi- or convergent configuration)
        if (site.getKind() != ZDvidAnnotation::EKind::KIND_POST_SYN) {
            continue;
        }

        QStandardItem * xItem = new QStandardItem();
        QStandardItem * yItem = new QStandardItem();
        QStandardItem * zItem = new QStandardItem();
        xItem->setData(QVariant(site.getX()), Qt::DisplayRole);
        yItem->setData(QVariant(site.getY()), Qt::DisplayRole);
        zItem->setData(QVariant(site.getZ()), Qt::DisplayRole);
        m_sitesModel->setItem(i - 1, SITES_X_COLUMN, xItem);
        m_sitesModel->setItem(i - 1, SITES_Y_COLUMN, yItem);
        m_sitesModel->setItem(i - 1, SITES_Z_COLUMN, zItem);
#if QT_VERSION >= 0x050000
        ui->sitesTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#else
        ui->sitesTableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#endif
    }
}

// input: point
// output: array with first pre-synaptic site then all post-synaptic sites
//  for the synapse; returns empty list on errors or if no T-bar at site
std::vector<ZDvidSynapse> SynapseReviewProtocol::getWholeSynapse(ZIntPoint point) {
    std::vector<ZDvidSynapse> result;
    ZDvidReader &reader = m_dvidReader;
    if (reader.good()) {
        ZDvidSynapse synapse = reader.readSynapse(point, flyem::EDvidAnnotationLoadMode::PARTNER_LOCATION);

        if (!synapse.isValid()) {
            return result;
        }

        if (synapse.getKind() != ZDvidAnnotation::EKind::KIND_PRE_SYN) {
            return result;
        }
        result.push_back(synapse);

        // get all the post-synaptic sites
        std::vector<ZIntPoint> psdArray = synapse.getPartners();
        for (size_t i=0; i<psdArray.size(); i++) {
            ZDvidSynapse post = reader.readSynapse(psdArray[i], flyem::EDvidAnnotationLoadMode::NO_PARTNER);
            result.push_back(post);
        }
    }
    return result;
}

void SynapseReviewProtocol::onDoubleClickSitesTable(QModelIndex index) {
    QStandardItem *itemX = m_sitesModel->item(index.row(), SITES_X_COLUMN);
    int x = itemX->data(Qt::DisplayRole).toInt();

    QStandardItem *itemY = m_sitesModel->item(index.row(), SITES_Y_COLUMN);
    int y = itemY->data(Qt::DisplayRole).toInt();

    QStandardItem *itemZ = m_sitesModel->item(index.row(), SITES_Z_COLUMN);
    int z = itemZ->data(Qt::DisplayRole).toInt();

    emit requestDisplayPoint(x, y, z);
}

void SynapseReviewProtocol::setSitesHeaders(QStandardItemModel * model) {
    model->setHorizontalHeaderItem(SITES_X_COLUMN, new QStandardItem(QString("x")));
    model->setHorizontalHeaderItem(SITES_Y_COLUMN, new QStandardItem(QString("y")));
    model->setHorizontalHeaderItem(SITES_Z_COLUMN, new QStandardItem(QString("z")));
}

void SynapseReviewProtocol::clearSitesTable() {
    m_sitesModel->clear();
    setSitesHeaders(m_sitesModel);
}

void SynapseReviewProtocol::updateLabels() {
    // current location
    if (m_currentPendingIndex >= 0) {
        ui->currentLocLabel->setText(QString::fromStdString(m_pendingList[m_currentPendingIndex].toString()));
    } else {
        ui->currentLocLabel->setText(QString("(--, --, --)"));
    }

    // progress
    int nPending = m_pendingList.size();
    int nFinished = m_finishedList.size();
    int nTotal = nPending + nFinished;
    float percent = (100.0 * nFinished) / nTotal;
    ui->progressLabel->setText(QString("Progress:\n\n %1 / %2 (%3%)").arg(nFinished).arg(nTotal).arg(percent, 4, 'f', 1));
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

void SynapseReviewProtocol::onReviewFirstButton() {
    if (!m_pendingList.isEmpty()) {
        m_currentPendingIndex = 0;
    } else {
        m_currentPendingIndex = -1;
    }
    gotoCurrent();
    updateUI();
}

void SynapseReviewProtocol::onReviewNextButton() {
    if (!m_pendingList.empty()) {
        m_currentPendingIndex++;
        if (m_currentPendingIndex >= m_pendingList.size()) {
            m_currentPendingIndex = 0;
        }
    } else {
        m_currentPendingIndex = -1;
    }
    gotoCurrent();
    updateUI();
}

void SynapseReviewProtocol::onReviewPreviousButton() {
    if (!m_pendingList.empty()) {
        m_currentPendingIndex--;
        if (m_currentPendingIndex < 0) {
            m_currentPendingIndex = m_pendingList.size() - 1;
        }
    } else {
        m_currentPendingIndex = -1;
    }
    gotoCurrent();
    updateUI();
}

void SynapseReviewProtocol::onGotoCurrentButton() {
    gotoCurrent();
}

void SynapseReviewProtocol::onMarkReviewedButton() {
    if (m_currentPendingIndex < 0) {
        return;
    }

    // move site to finished; save state
    ZIntPoint currentPoint = m_pendingList[m_currentPendingIndex];
    m_pendingList.removeOne(currentPoint);
    m_finishedList.append(currentPoint);
    saveState();

    // advance protocol and go to next point
    if (m_pendingList.empty()) {
        // we're done; dialog
        m_currentPendingIndex = -1;

        QMessageBox mb;
        mb.setText("No more sites");
        mb.setInformativeText("All sites have been reviewed. You may complete the protocol now.");
        mb.setStandardButtons(QMessageBox::Ok);
        mb.setDefaultButton(QMessageBox::Ok);
        mb.exec();
    } else {
        // having removed the point at the current index, the current
        //  index now points to the next point or past the end of
        //  the list; if the latter, loop back to the top
        if (m_currentPendingIndex >= m_pendingList.size()) {
            m_currentPendingIndex = 0;
        }
    }
    gotoCurrent();
    updateLabels();
}

void SynapseReviewProtocol::onFinishedLastButton() {
    if (!m_finishedList.isEmpty()) {
        m_currentFinishedIndex = m_finishedList.size() - 1;
    } else {
        m_currentFinishedIndex = -1;
    }
    gotoCurrentFinished();
}

void SynapseReviewProtocol::onFinishedNextButton() {
    if (!m_finishedList.empty()) {
        m_currentFinishedIndex++;
        if (m_currentFinishedIndex >= m_finishedList.size()) {
            m_currentFinishedIndex = 0;
        }
    } else {
        m_currentFinishedIndex = -1;
    }
    gotoCurrentFinished();
}

void SynapseReviewProtocol::onFinishedPreviousButton() {
    if (!m_finishedList.empty()) {
        m_currentFinishedIndex--;
        if (m_currentFinishedIndex < 0) {
            m_currentFinishedIndex = m_finishedList.size() - 1;
        }
    } else {
        m_currentFinishedIndex = -1;
    }
    gotoCurrentFinished();
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

