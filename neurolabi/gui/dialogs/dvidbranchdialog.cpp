#include "dvidbranchdialog.h"
#include "ui_dvidbranchdialog.h"

#include <iostream>
#include <stdlib.h>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QModelIndex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardItem>
#include <QStringList>
#include <QStringListModel>
#include <QUrl>

#include "dvid/zdvidtarget.h"
#include "dvid/zdvidnode.h"
#include "dialogs/zdviddialog.h"
#include "neutubeconfig.h"
#include "zqslog.h"

/*
 * this class is a drop-in replacement for ZDvidDialog; this version lets you
 * browse branches in a DVID repo and have default data instance names filled in;
 * in principle, you wouldn't need to know much of anything
 *
 * ZDialogFactory::makeDvidDialog() has a switch for indicating whether you want
 * the original or this one
 *
 * this dialog does delegate to the original dialog for some functions--it's not
 * fully indepedent
 */
DvidBranchDialog::DvidBranchDialog(QWidget *parent) :
    ZDvidTargetProviderDialog(parent),
    ui(new Ui::DvidBranchDialog)
{
    ui->setupUi(this);

    m_reader.setVerbose(false);

    m_networkManager = new QNetworkAccessManager(this);

    // UI configuration
    // placeholder text for various entry fields:
    ui->todoBox->setPlaceholderText("default");
    ui->bodyLabelBox->setPlaceholderText("default");
    ui->ROIBox->setPlaceholderText("none");
    ui->UUIDBox->setPlaceholderText("at least 4 characters");
    ui->grayscaleUUIDBox->setPlaceholderText("at least 4 characters");
    ui->tileUUIDBox->setPlaceholderText("at least 4 characters");

    // UI connections
    connect(ui->repoListView, SIGNAL(clicked(QModelIndex)), this, SLOT(onRepoClicked(QModelIndex)));
    connect(ui->branchListView, SIGNAL(clicked(QModelIndex)), this, SLOT(onBranchClicked(QModelIndex)));
    connect(ui->detailsButton, SIGNAL(clicked(bool)), this, SLOT(toggleDetailsPanel()));
    connect(ui->oldDialogButton, SIGNAL(clicked(bool)), this, SLOT(launchOldDialog()));

    // data load connections
    connect(this, SIGNAL(datasetsFinishedLoading(QJsonObject)), this, SLOT(finishLoadingDatasets(QJsonObject)));

    // models & related
    m_repoModel = new QStringListModel();
    ui->repoListView->setModel(m_repoModel);

    m_branchModel = new QStringListModel();
    ui->branchListView->setModel(m_branchModel);

    // set initial UI state
    hideDetailsPanel();
}

// constants
const QString DvidBranchDialog::KEY_DATASETS = "primary";
const QString DvidBranchDialog::KEY_VERSION = "version";
const QString DvidBranchDialog::KEY_CONFIG = "config";
const int DvidBranchDialog::SUPPORTED_VERSION = 1;
const QString DvidBranchDialog::URL_DATASETS = "http://config.int.janelia.org/config/em_datasets";
const QString DvidBranchDialog::KEY_NAME = "name";
const QString DvidBranchDialog::KEY_SERVER = "server";
const QString DvidBranchDialog::KEY_PORT = "port";
const QString DvidBranchDialog::KEY_UUID = "UUID";
const QString DvidBranchDialog::KEY_DESCRIPTION = "description";
const QString DvidBranchDialog::KEY_DAG = "DAG";
const QString DvidBranchDialog::KEY_NODES = "Nodes";
const QString DvidBranchDialog::KEY_NOTE = "Note";

const QString DvidBranchDialog::INSTANCE_BRANCHES = "branches";
const QString DvidBranchDialog::KEY_MASTER = "master";

const int DvidBranchDialog::DEFAULT_PORT = 8000;

const QString DvidBranchDialog::DEFAULT_MASTER_NAME = "master";
const QString DvidBranchDialog::MESSAGE_LOADING = "Loading...";
const QString DvidBranchDialog::MESSAGE_ERROR = "Error:";

/*
 * when the dialog is shown, load the dataset list; it turns out that
 * multiple copies of this dialog are created but rarely/never used,
 * and we don't want to load data for all of them, so we postpone
 * it until the dialog is shown
 */
void DvidBranchDialog::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (m_repoMap.isEmpty()) {
        loadDatasets();
    }
}

/*
 * load stored dataset list into the first panel of the UI
 */
void DvidBranchDialog::loadDatasets() {
    // this functions as a UI reset
    clearNode();

    QStringList datasetNames;
    datasetNames.append(MESSAGE_LOADING);
    m_repoModel->setStringList(datasetNames);

    // this call sends a signal when done to finishLoadingDatasets():

    // testing: from file
    // loadDatasetsFromFile();

    // production: from config server
    loadDatasetsFromConfigServer();
}

/*
 * handle loaded dataset
 */
void DvidBranchDialog::finishLoadingDatasets(QJsonObject jsonData) {
    if (jsonData.isEmpty()) {
        displayDatasetError(MESSAGE_ERROR + " Dataset file did not have any data!");
        return;
    }
    if (!jsonData.contains(KEY_DATASETS) || !jsonData.contains(KEY_VERSION)) {
        displayDatasetError(MESSAGE_ERROR + " Dataset file did not have expected data!");
        return;
    }

    // version check
    if (jsonData[KEY_VERSION].toInt() > SUPPORTED_VERSION) {
        displayDatasetError(MESSAGE_ERROR + " Dataset file is a newer version than we can handle! Please update NeuTu/Neu3!");
        return;
    }

    // we'll keep the repo data as json, but index it a bit
    m_repoMap.clear();
    foreach(QJsonValue value, jsonData[KEY_DATASETS].toArray()) {
        QJsonObject repo = value.toObject();
        m_repoMap[repo[KEY_NAME].toString()] = repo;
    }

    // note that once this is populated, it's never changed or cleared
    QStringList repoNameList = m_repoMap.keys();
    repoNameList.sort();
    m_repoModel->setStringList(repoNameList);

}

/*
 * handle error display from dataset loads
 */
void DvidBranchDialog::displayDatasetError(QString errorMessage) {
    QStringList stringList;
    stringList.append(errorMessage);
    m_repoModel->setStringList(stringList);
}

/*
 * load stored dataset from file into json; for testing only!
 * note that errors are silent (you just get empty data back)
 */
void DvidBranchDialog::loadDatasetsFromFile() {
    QJsonObject empty;

    // testing: hard-coded file location
    QString filepath = "/Users/olbrisd/projects/flyem/misc-repos/DVID-datasets/datasets.json";
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit datasetsFinishedLoading(empty);
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() or !doc.isObject()) {
        emit datasetsFinishedLoading(empty);
    } else {
        LINFO() << "Read DVID repo information json from file" + filepath;
        emit datasetsFinishedLoading(doc.object());
    }
}

/*
 * retrieve the list of datasets from our config server; this method
 * initiates the network call
 */
void DvidBranchDialog::loadDatasetsFromConfigServer() {
    QUrl requestUrl;
    requestUrl.setUrl(URL_DATASETS);
    m_datasetReply = m_networkManager->get(QNetworkRequest(requestUrl));
    connect(m_datasetReply, SIGNAL(finished()), this, SLOT(finishLoadingDatasetsFromConfigServer()));
}

/*
 * finish loading from config server
 */
void DvidBranchDialog::finishLoadingDatasetsFromConfigServer(QNetworkReply::NetworkError error) {
    if (error != QNetworkReply::NoError) {
        displayDatasetError(" Error reading dataset list from config server; status code: " +
            m_datasetReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString());
    } else {
        QString stringReply = (QString) m_datasetReply->readAll();

        // the part we want is in the "config" key:
        QJsonDocument jsonResponse = QJsonDocument::fromJson(stringReply.toUtf8());
        QJsonObject temp = jsonResponse.object();
        if (!temp.contains(KEY_CONFIG)) {
            displayDatasetError("Error parsing dataset list from config server");
            return;
        }
        emit datasetsFinishedLoading(temp[KEY_CONFIG].toObject());
    }
}

void DvidBranchDialog::onRepoClicked(QModelIndex modelIndex) {
    clearNode();
    QString repoName = m_repoModel->data(modelIndex, Qt::DisplayRole).toString();
    if (repoName.startsWith(MESSAGE_ERROR) || repoName == MESSAGE_LOADING) {
        return;
    }
    loadBranches(repoName);
}

/*
 * given a repo name, load all its branches into the UI
 */
void DvidBranchDialog::loadBranches(QString repoName) {
    LINFO() << "Loading branches from DVID repo" << repoName.toStdString();

    m_repoName = repoName;

    // clear model; use it as a loading message, too
    QStringList branchNames;
    branchNames.append(MESSAGE_LOADING);
    m_branchModel->setStringList(branchNames);


    // read repo info, in this thread; there's nothing else the user can do
    //  but wait at this point anyway
    ZDvidTarget target(m_repoMap[m_repoName][KEY_SERVER].toString().toStdString(),
        m_repoMap[m_repoName][KEY_UUID].toString().toStdString(),
        m_repoMap[m_repoName][KEY_PORT].toInt());
    if (!m_reader.open(target)) {
        QMessageBox errorBox;
        errorBox.setText("Error connecting to DVID");
        errorBox.setInformativeText("Could not conenct to DVID at " +
            QString::fromStdString(target.getAddressWithPort()) +
            "!  Check server information!");
        errorBox.setStandardButtons(QMessageBox::Ok);
        errorBox.setIcon(QMessageBox::Warning);
        errorBox.exec();
        branchNames.clear();
        m_branchModel->setStringList(branchNames);
        return;
    }
    ZJsonObject info = m_reader.readInfo();
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(info.dumpString()).toUtf8());
    QJsonObject repoJson = doc.object();

    // parse out actual branches and populate the model
    QJsonObject nodeJson = repoJson[KEY_DAG].toObject()[KEY_NODES].toObject();
    QStringList branchUUIDs = findBranches(nodeJson);

    branchNames.clear();
    foreach (QString uuid, branchUUIDs) {
        QJsonObject branchNode = nodeJson[uuid].toObject();
        QString branchName = branchNode["Branch"].toString();
        m_branchMap[branchName] = branchNode;
        branchNames.append(branchName);
    }

    // at this point we have to deal with the branch name "" (the empty string);
    //  that is to be interpreted as being the master branch,
    //  so name it explicitly in the UI (top of list); first,
    //  check that no other branch is already using the name we want to use
    QString masterName = DEFAULT_MASTER_NAME;
    if (branchNames.contains("")) {
        if (branchNames.contains(DEFAULT_MASTER_NAME)) {
            masterName = findMasterName(DEFAULT_MASTER_NAME, branchNames);
        }

        // now rename in local data structures (not in DVID)
        branchNames.removeAll("");
        branchNames.prepend(masterName);

        m_branchMap[masterName] = m_branchMap[""];
        m_branchMap.remove("");
    }

    if (!m_branchMap.contains(DEFAULT_MASTER_NAME)) {
        // if it's not there, make a last-ditch effort to get the
        //  master branch from a key-value we used to store in some
        //  of the older repos:
        if (m_reader.hasData(INSTANCE_BRANCHES.toStdString())) {
            const QByteArray &rawData = m_reader.readKeyValue(INSTANCE_BRANCHES, KEY_MASTER);
            if (!rawData.isEmpty()) {
                QJsonArray UUIDs = QJsonDocument::fromJson(rawData).array();
                if (UUIDs.size() > 0) {
                    QString masterUUID = UUIDs.first().toString();
                    // find it in the list, rename it, and put it at the top; remember that
                    //  it might not be there, if it's on a branch we didn't find the tip for
                    //  (in fact, it's likely this is the case) (but double-check that we
                    //  actually have it!  this list of nodes is hand-curated)

                    // find that UUID in nodeJson; note we have to check one by one because
                    //  the nodeJson keys aren't abbreviated, like the ones in this instance are
                    bool found = false;
                    foreach (QString uuid, nodeJson.keys()) {
                        if (uuid.startsWith(masterUUID)) {
                            found = true;
                            masterUUID = uuid;
                            break;
                        }
                    }
                    if (found) {
                        QString currentName = nodeJson[masterUUID].toObject()["Branch"].toString();
                        if (m_branchMap.contains(currentName)) {
                            m_branchMap[masterName] = m_branchMap[currentName];
                            m_branchMap.remove(currentName);
                        }
                        branchNames.removeAll(currentName);
                        branchNames.prepend(masterName);
                    }
                }
            }
        }
    }

    m_branchModel->setStringList(branchNames);
}

/*
 * find a branch name not in the input list
 */
QString DvidBranchDialog::findMasterName(QString prefix, QStringList names) {
    int n = 2;
    QString temp = QString("%1 %2").arg(prefix, QString::number(n));
    while (names.contains(temp)) {
        n++;
        temp = QString("%1 %2").arg(prefix, QString::number(n));
    }
    return temp;
}

/*
 * given the DVID-returned node json for a repo, return a list
 * of the UUID keys into that json that correspond to the nodes
 * that are tips of branches
 */
QStringList DvidBranchDialog::findBranches(QJsonObject nodeJson) {

    QStringList UUIDs;

    // branches should have nodes in a parent-child relationship;
    //  the node with a given branch name that has no children with
    //  the branch name is the tip (since it can have children with
    //  different branch names)

    // problem is that we have to be able to handle old repos that don't
    //  enforce the branch convention; eg, tons of unnamed branches that
    //  have no relation to each other

    // first pass: separate by branch name; accumulate parent versions

    // branch name: list of uuids
    QMap<QString, QStringList> branches;
    // branch name: set of parent version IDs
    QMap<QString, QSet<int>> parents;

    foreach (QString uuid, nodeJson.keys()) {
        QJsonObject node = nodeJson[uuid].toObject();
        QString branchName = node["Branch"].toString();
        branches[branchName].append(uuid);
        foreach (QJsonValue parent, node["Parents"].toArray()) {
            parents[branchName].insert(parent.toInt());
        }
    }

    // second pass: for each branch, find the tip; if a node isn't the
    //  parent of any other node in the branch, it's a tip; if there's more than
    //  one, that's a result of older repos that don't respect the rules
    //  we have no way to pick a commit, so eliminate it from the list (for now)
    foreach (QString branchName, branches.keys()) {
        QList<QString> tiplist;
        foreach (QString uuid, branches[branchName]) {
            QJsonObject node = nodeJson[uuid].toObject();
            if (!parents[branchName].contains(node["VersionID"].toInt())) {
                tiplist.append(uuid);
            }
        }
        if (tiplist.size() != 1) {
            LINFO() << "Branch" << branchName.toStdString() << "has indeterminate tip";

            // not sure if I need a dialog here, but it's useful for testing
            showError("Branch error", "Can't find the tip of branch '" + branchName +
                "' (" + QString::number(tiplist.size()) + " candidates); it won't be listed.");
        } else {
            UUIDs.append(tiplist[0]);
        }
    }
    return UUIDs;
}

void DvidBranchDialog::onBranchClicked(QModelIndex modelIndex) {
    loadNode(m_branchModel->data(modelIndex, Qt::DisplayRole).toString());
}

/*
 * given branch name, load the data for the node into the third column of the UI
 */
void DvidBranchDialog::loadNode(QString branchName) {
    // this is kind of ad hoc: filter out the loading message
    if (branchName == MESSAGE_LOADING) {
        return;
    }

    m_branchName = branchName;

    // server and port are from repo:
    QString server = m_repoMap[m_repoName][KEY_SERVER].toString();
    int port = m_repoMap[m_repoName][KEY_PORT].toInt();
    ui->serverBox->setText(server);
    ui->portBox->setValue(port);

    // rest is from the specific node:
    QJsonObject nodeJson = m_branchMap[m_branchName];
    QString uuid = nodeJson[KEY_UUID].toString().left(4);
    ui->UUIDBox->setText(uuid);
    ui->commentBox->setText(nodeJson[KEY_NOTE].toString());
    ui->commentBox->setCursorPosition(0);
    ui->commentBox->setToolTip(nodeJson[KEY_NOTE].toString());

    // check for default settings on the branch node (not root)
    ZJsonObject defaultsJson;
    m_reader.open(server, uuid, port);
    defaultsJson = m_reader.readDefaultDataSetting();
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(defaultsJson.dumpString()).toUtf8());
    QJsonObject defaults = doc.object();

    // populate dialog from default values; note that some
    //  stored default values are ignored, and the user can't 
    //  alter them in the dialog; usually those are generated in
    //  NeuTu internally based on other input values; eg the 
    //  synapse labelsz value is just the synapse value + "_labelsz",
    //  regardless of what is in default_instances

    // should make these keys constants?
    if (defaults.contains("segmentation")) {
        ui->segmentationBox->setText(defaults["segmentation"].toString());
    }
    if (defaults.contains("grayscale")) {
        ui->grayscaleBox->setText(defaults["grayscale"].toString());
    }
    if (defaults.contains("synapses")) {
        ui->synapsesBox->setText(defaults["synapses"].toString());
    }
    if (defaults.contains("todos")) {
        ui->todoBox->setText(defaults["todos"].toString());
    }
    if (defaults.contains("bodies")) {
        ui->bodyLabelBox->setText(defaults["bodies"].toString());
    }

#if defined(_FLYEM_)
    ui->librarianCheckBox->setChecked(true);
    ui->librarianBox->setText(QString::fromStdString(GET_FLYEM_CONFIG.getDefaultLibrarian()));
#endif

}

ZDvidTarget &DvidBranchDialog::getDvidTarget() {
    // later inspection reveals m_dvidTarget could be a local
    //  variable; don't want to mess with it now while I'm
    //  fixing something else, though
    m_dvidTarget.clear();

    m_dvidTarget.setServer(ui->serverBox->text().toStdString());
    m_dvidTarget.setUuid(ui->UUIDBox->text().toStdString());
    m_dvidTarget.setPort(ui->portBox->text().toInt());

    m_dvidTarget.setComment(ui->commentBox->text().toStdString());

    m_dvidTarget.setSegmentationName(ui->segmentationBox->text().toStdString());
    m_dvidTarget.setGrayScaleName(ui->grayscaleBox->text().toStdString());
    m_dvidTarget.setSynapseName(ui->synapsesBox->text().toStdString());

    m_dvidTarget.setMultiscale2dName(ui->tilesBox->text().toStdString());
    m_dvidTarget.configTile(ui->tilesBox->text().toStdString(), ui->tilesCheckBox->isChecked());
    m_dvidTarget.setRoiName(ui->ROIBox->text().toStdString());

    m_dvidTarget.setTodoListName(ui->todoBox->text().toStdString());
    m_dvidTarget.setBodyLabelName(ui->bodyLabelBox->text().toStdString());

    if (!ui->grayscaleSourceCheckBox->isChecked()) {
        ZDvidNode grayscaleNode;
        grayscaleNode.setServer(ui->grayscaleServerBox->text().toStdString());
        grayscaleNode.setPort(ui->grayscalePortBox->text().toInt());
        grayscaleNode.setUuid(ui->grayscaleUUIDBox->text().toStdString());
        m_dvidTarget.setGrayScaleSource(grayscaleNode);
    }

    if (!ui->tileSourceCheckBox->isChecked()) {
        ZDvidNode tileNode;
        tileNode.setServer(ui->tileServerBox->text().toStdString());
        tileNode.setPort(ui->tilePortBox->text().toInt());
        tileNode.setUuid(ui->tileUUIDBox->text().toStdString());
        m_dvidTarget.setTileSource(tileNode);
    }

    if (ui->librarianCheckBox->isChecked()) {
        m_dvidTarget.setSupervisorServer(ui->librarianBox->text().toStdString());
    }

    if (ui->readOnlyCheckBox->isChecked()) {
        m_dvidTarget.setReadOnly(true);
    }

    return m_dvidTarget;
}

/*
 * this method gets a predefined constant target from the old dialog
 */
const ZDvidTarget& DvidBranchDialog::getDvidTarget(const std::string &name) const {
    ZDvidDialog * dialog = new ZDvidDialog(NULL);
    const ZDvidTarget &target = dialog->getDvidTarget(name);
    delete dialog;
    return target;
}

/*
 * clear the info in the third column of the UI
 */
void DvidBranchDialog::clearNode() {
    ui->serverBox->clear();
    ui->portBox->setValue(DEFAULT_PORT);
    ui->UUIDBox->clear();

    ui->commentBox->clear();

    ui->segmentationBox->clear();
    ui->grayscaleBox->clear();
    ui->synapsesBox->clear();

    ui->ROIBox->clear();
    ui->tilesBox->clear();
    ui->tilesCheckBox->setChecked(false);
    ui->todoBox->clear();
    ui->bodyLabelBox->clear();

    ui->grayscaleSourceCheckBox->setChecked(true);
    ui->grayscaleServerBox->clear();
    ui->grayscalePortBox->setValue(DEFAULT_PORT);
    ui->grayscaleUUIDBox->clear();

    ui->tileSourceCheckBox->setChecked(true);
    ui->tileServerBox->clear();
    ui->tilePortBox->setValue(DEFAULT_PORT);
    ui->tileUUIDBox->clear();

    ui->librarianBox->clear();
    ui->librarianCheckBox->setChecked(false);
}

/*
 * launch the old dialog and forward its value to this dialog
 */
void DvidBranchDialog::launchOldDialog() {

    // pop up the old dialog and transfer values to the UI, if user clicks OK

    ZDvidDialog * dialog = new ZDvidDialog(NULL);
    if (dialog->exec()) {
        clearNode();
        ZDvidTarget target = dialog->getDvidTarget();

        ui->serverBox->setText(QString::fromStdString(target.getAddress()));
        ui->portBox->setValue(target.getPort());
        ui->UUIDBox->setText(QString::fromStdString(target.getUuid()));

        ui->commentBox->setText(QString::fromStdString(target.getComment()));

        ui->segmentationBox->setText(QString::fromStdString(target.getSegmentationName()));
        ui->grayscaleBox->setText(QString::fromStdString(target.getGrayScaleName()));
        ui->synapsesBox->setText(QString::fromStdString(target.getSynapseName()));

        ui->ROIBox->setText(QString::fromStdString(target.getRoiName()));
        ui->tilesBox->setText(QString::fromStdString(target.getMultiscale2dName()));
        ui->tilesCheckBox->setChecked(target.isLowQualityTile(target.getMultiscale2dName()));
        ui->todoBox->setText(QString::fromStdString(target.getTodoListName()));
        ui->bodyLabelBox->setText(QString::fromStdString(target.getBodyLabelName()));

        // note that for each of the optional "use main source", we have to check
        //  explicitly whether they match in order to set the check box state

        if (target.getGrayScaleSource().isValid()) {
            ui->grayscaleSourceCheckBox->setChecked(true);
            ui->grayscaleServerBox->setText(QString::fromStdString(target.getGrayScaleSource().getAddress()));
            ui->grayscalePortBox->setValue(target.getGrayScaleSource().getPort());
            ui->grayscaleUUIDBox->setText(QString::fromStdString(target.getGrayScaleSource().getUuid()));

            if (target.getAddress() != target.getGrayScaleSource().getAddress() ||
                target.getPort() != target.getGrayScaleSource().getPort() ||
                target.getUuid() != target.getGrayScaleSource().getUuid()) {
                ui->grayscaleSourceCheckBox->setChecked(false);
            }
        }

        if (target.getTileSource().isValid()) {
            ui->tileSourceCheckBox->setChecked(true);
            ui->tileServerBox->setText(QString::fromStdString(target.getTileSource().getAddress()));
            ui->tilePortBox->setValue(target.getTileSource().getPort());
            ui->tileUUIDBox->setText(QString::fromStdString(target.getTileSource().getUuid()));

            if (target.getAddress() != target.getTileSource().getAddress() ||
                target.getPort() != target.getTileSource().getPort() ||
                target.getUuid() != target.getTileSource().getUuid()) {
                ui->tileSourceCheckBox->setChecked(false);
            }
        }

        if (target.isSupervised()) {
            ui->librarianCheckBox->setChecked(true);
            ui->librarianBox->setText(QString::fromStdString(target.getSupervisor()));
        } else {
            ui->librarianCheckBox->setChecked(false);
        }

    }
    delete dialog;

}

/*
 * toggle the details panel open and closed
 */
void DvidBranchDialog::toggleDetailsPanel() {
    if (m_detailsVisible) {
        hideDetailsPanel();
    } else {
        showDetailsPanel();
    }
}

void DvidBranchDialog::showDetailsPanel() {
    // here and in hide, below, I want the dialog width to adjust
    //  for adding and removing a third panel of same width as the other two
    resize(3 * width() / 2, height());
    ui->detailsWidget->show();
    ui->detailsButton->setText("<< Details");
    m_detailsVisible = true;
}

void DvidBranchDialog::hideDetailsPanel() {
    resize(2 * width() / 3, height());
    ui->detailsWidget->hide();
    ui->detailsButton->setText("Details >>");
    m_detailsVisible = false;
}

/*
 * input: title and message for error dialog
 * effect: shows error dialog (convenience function)
 */
void DvidBranchDialog::showError(QString title, QString message) {
    QMessageBox errorBox;
    errorBox.setText(title);
    errorBox.setInformativeText(message);
    errorBox.setStandardButtons(QMessageBox::Ok);
    errorBox.setIcon(QMessageBox::Warning);
    errorBox.exec();
}

DvidBranchDialog::~DvidBranchDialog()
{
    delete ui;
}
