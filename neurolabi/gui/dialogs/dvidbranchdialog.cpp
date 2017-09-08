#include "dvidbranchdialog.h"
#include "ui_dvidbranchdialog.h"

#include <iostream>
#include <stdlib.h>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QModelIndex>
#include <QStringList>
#include <QStringListModel>
#include <QsLog.h>


DvidBranchDialog::DvidBranchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DvidBranchDialog)
{
    ui->setupUi(this);


    // UI connections
    connect(ui->repoListView, SIGNAL(clicked(QModelIndex)), this, SLOT(onRepoClicked(QModelIndex)));


    // data load connections


    // models & related
    m_repoModel = new QStringListModel();
    ui->repoListView->setModel(m_repoModel);



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
        LINFO() << "Task protocol: json loaded from file" + filepath;
        return doc.object();
    }
}

void DvidBranchDialog::onRepoClicked(QModelIndex modelIndex) {

    std::cout << "in onRepoClicked()" << std::endl;

    // QStandardItem *item = m_repoModel->itemData(modelIndex);


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
