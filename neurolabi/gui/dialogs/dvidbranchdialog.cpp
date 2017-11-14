#include "dvidbranchdialog.h"
#include "ui_dvidbranchdialog.h"

#include <iostream>
#include <stdlib.h>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QModelIndex>
#include <QStandardItem>
#include <QStringList>
#include <QStringListModel>
#include <QsLog.h>

#include "dvid/zdvidtarget.h"

DvidBranchDialog::DvidBranchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DvidBranchDialog)
{
    ui->setupUi(this);

    m_reader.setVerbose(false);

    // UI connections
    connect(ui->repoListView, SIGNAL(clicked(QModelIndex)), this, SLOT(onRepoClicked(QModelIndex)));
    connect(ui->branchListView, SIGNAL(clicked(QModelIndex)), this, SLOT(onBranchClicked(QModelIndex)));


    // data load connections


    // models & related
    m_repoModel = new QStringListModel();
    ui->repoListView->setModel(m_repoModel);

    m_branchModel = new QStringListModel();
    ui->branchListView->setModel(m_branchModel);



    // populate first panel with server data
    loadDatasets();











}

// constants
const QString DvidBranchDialog::KEY_REPOS = "repos";
const QString DvidBranchDialog::KEY_NAME = "name";
const QString DvidBranchDialog::KEY_SERVER = "server";
const QString DvidBranchDialog::KEY_PORT = "port";
const QString DvidBranchDialog::KEY_UUID = "UUID";
const QString DvidBranchDialog::KEY_DESCRIPTION = "description";
const QString DvidBranchDialog::KEY_DAG = "DAG";
const QString DvidBranchDialog::KEY_NODES = "Nodes";

const QString DvidBranchDialog::MESSAGE_LOADING = "Loading...";

/*
 * load stored dataset list into the first panel of the UI
 */
void DvidBranchDialog::loadDatasets() {
    // for now, load from file; probably should be from
    //  DVID, some other db, or a Fly EM service of some kind
    QJsonObject jsonData = loadDatasetsFromFile();
    if (jsonData.isEmpty()) {
        // need some message in UI?
        return;
    }
    if (!jsonData.contains(KEY_REPOS)) {
        // message?
        return;
    }

    // we'll keep the repo data as json, but index it a bit
    m_repoMap.clear();
    foreach(QJsonValue value, jsonData[KEY_REPOS].toArray()) {
        QJsonObject repo = value.toObject();
        m_repoMap[repo[KEY_NAME].toString()] = repo;
    }

    // note that once this is populated, it's never changed or cleared
    QStringList repoNameList = m_repoMap.keys();
    repoNameList.sort();
    m_repoModel->setStringList(repoNameList);
}

/*
 * load stored dataset from file into json
 */
QJsonObject DvidBranchDialog::loadDatasetsFromFile() {
    QJsonObject empty;

    // testing: hard-coded file location
    QString filepath = "/Users/olbrisd/projects/flyem/NeuTu/testing/test-repos.json";
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        showError("Error loading datasets", "Couldn't open dataset file " + filepath + "!");
        return empty;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() or !doc.isObject()) {
        showError("Error parsing file", "Couldn't parse file " + filepath + "!");
        return empty;
    } else {
        LINFO() << "Read DVID repo information json from file" + filepath;
        return doc.object();
    }
}

void DvidBranchDialog::onRepoClicked(QModelIndex modelIndex) {
    loadBranches(m_repoModel->data(modelIndex, Qt::DisplayRole).toString());
}

/*
 * given a repo name, load all its branches into the UI
 */
void DvidBranchDialog::loadBranches(QString repoName) {
    LINFO() << "Loading branches from DVID repo" << repoName.toStdString();

    // clear model; use it as a loading message, too
    QStringList branchNames;
    branchNames.append(MESSAGE_LOADING);
    m_branchModel->setStringList(branchNames);


    // read repo info, in this thread; there's nothing else the user can do
    //  but wait at this point anyway
    ZDvidTarget target(m_repoMap[repoName][KEY_SERVER].toString().toStdString(),
        m_repoMap[repoName][KEY_UUID].toString().toStdString(),
        m_repoMap[repoName][KEY_PORT].toInt());
    if (!m_reader.open(target)) {
        QMessageBox errorBox;
        errorBox.setText("Error connecting to DVID");
        errorBox.setInformativeText("Could not open DVID!  Check server information!");
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

    m_branchModel->setStringList(branchNames);
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
        } else {
            UUIDs.append(tiplist[0]);
        }
    }
    return UUIDs;
}

void DvidBranchDialog::onBranchClicked(QModelIndex modelIndex) {

    QString branchName = m_branchModel->data(modelIndex, Qt::DisplayRole).toString();

    // this is kind of ad hoc:
    if (branchName != MESSAGE_LOADING) {
        LINFO() << "branch clicked:" << branchName;
    }

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
