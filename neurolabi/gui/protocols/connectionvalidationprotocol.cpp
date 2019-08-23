#include "connectionvalidationprotocol.h"
#include "ui_connectionvalidationprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QFileDialog>
#include <QMessageBox>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>


#include "zjsonobject.h"
#include "zjsonparser.h"

ConnectionValidationProtocol::ConnectionValidationProtocol(QWidget *parent) :
    ProtocolDialog(parent),
    ui(new Ui::ConnectionValidationProtocol)
{
    ui->setupUi(this);

    // sites table
    m_sitesModel = new QStandardItemModel(0, 5, ui->sitesTableView);
    setSitesHeaders(m_sitesModel);
    ui->sitesTableView->setModel(m_sitesModel);

    // UI connections
    connect(ui->firstButton, SIGNAL(clicked(bool)), this, SLOT(onFirstButton()));
    connect(ui->nextButton, SIGNAL(clicked(bool)), this, SLOT(onNextButton()));
    connect(ui->markAndNextButton, SIGNAL(clicked(bool)), this, SLOT(onMarkAndNextButton()));
    connect(ui->gotoButton, SIGNAL(clicked(bool)), this, SLOT(onGotoButton()));

    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));

    // note that connecting clicked() on check boxes will only signal on user clicks,
    //  not programmatic toggles, which is what I want
    connect(ui->reviewedBox, SIGNAL(clicked(bool)), this, SLOT(onReviewedChanged()));
    connect(ui->tbarCheckBox, SIGNAL(clicked(bool)), this, SLOT(onTbarGoodChanged()));
    connect(ui->tbarSegCheckBox, SIGNAL(clicked(bool)), this, SLOT(onTbarSegGoodChanged()));
    connect(ui->psdCheckBox, SIGNAL(clicked(bool)), this, SLOT(onPSDGoodCanged()));
    connect(ui->psdSegCheckBox, SIGNAL(clicked(bool)), this, SLOT(onPSDSegGoodChanged()));

    connect(ui->sitesTableView, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedTable(QModelIndex)));

    m_currentIndex = -1;
}

// constants
const std::string ConnectionValidationProtocol::KEY_VERSION = "version";
const int ConnectionValidationProtocol::fileVersion = 1;

bool ConnectionValidationProtocol::initialize() {

    QString filepath = QFileDialog::getOpenFileName(this, "Choose point file");
    if (filepath.size() == 0) {
        return false;
    }

    // read the file
    QFile file;
    file.setFileName(filepath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString fileData = file.readAll();
    file.close();
    QJsonDocument doc = QJsonDocument::fromJson(fileData.toUtf8());
    if (doc.isNull()) {
        // error in parsing
        showError("Point file error", "The point file should be a json file!");
        return false;
    }

    // check file type and version
    QJsonObject data = doc.object();
    if (!data.contains("file type") || data["file type"].toString() != "connection validation") {
        showError("File type error", "Input point file should have type 'connection validation'.");
        return false;
    }
    if (!data.contains("file version") || data["file version"].toInt() != 1) {
        showError("File version error", "Input point file should be version 1");
        return false;
    }

    // after this last check, we assume format is right let it crash if not
    if (!data.contains("points")) {
        showError("No points!", "Input point file does not contain 'points' key!");
        return false;
    }

    // get point data into internal data structures
    QJsonArray jsonArray = data["points"].toArray();
    loadPoints(jsonArray);

    updateTable();
    updateLabels();

    // save initial data and get going
    saveState();
    onFirstButton();

    return true;
}

int ConnectionValidationProtocol::findFirstUnreviewed() {
    int found = -1;
    for (int i=0; i<m_points.size(); i++) {
        if (!m_pointData[m_points[i]].reviewed) {
            found = i;
            break;
        }
    }
    return found;
}

int ConnectionValidationProtocol::findNextUnreviewed() {
    // finding a next element in a list including restarting
    //  from the top is a pain and I've found to be error-prone;
    //  let's try this variation

    QList<int> indices;
    for (int i=m_currentIndex+1; i<m_points.size(); i++) {
        indices << i;
    }
    for (int i=0; i<=m_currentIndex; i++) {
        indices << i;
    }

    int found = -1;
    for (int i: indices) {
        if (!m_pointData[m_points[i]].reviewed) {
            found = i;
            break;
        }
    }
    return found;
}

void ConnectionValidationProtocol::setCurrentReviewed() {
    ui->reviewedBox->setChecked(true);
    onReviewedChanged();
}

void ConnectionValidationProtocol::onFirstButton() {
    int first = findFirstUnreviewed();
    if (first >= 0) {
        setSelectGotoCurrentPoint(first);
    } else {
        showMessage("Done!", "No unreviewed connections! You may complete this protocol.");
    }
}

void ConnectionValidationProtocol::onNextButton() {
    int next = findNextUnreviewed();
    if (next >= 0) {
        setSelectGotoCurrentPoint(next);
    } else {
        showMessage("Done!", "No unreviewed connections! You may complete this protocol.");
    }
}

void ConnectionValidationProtocol::onMarkAndNextButton() {
    setCurrentReviewed();
    onNextButton();
}

void ConnectionValidationProtocol::onGotoButton() {
    gotoCurrentPoint();
}

void ConnectionValidationProtocol::onReviewedChanged() {
    m_pointData[m_points[m_currentIndex]].reviewed = ui->reviewedBox->isChecked();
    saveState();
    updateTable();
    // reviewed change affects progress
    updateProgressLabel();
}

void ConnectionValidationProtocol::onTbarGoodChanged() {
    m_pointData[m_points[m_currentIndex]].tbarGood = ui->tbarCheckBox->isChecked();
    saveState();
    updateTable();
}

void ConnectionValidationProtocol::onTbarSegGoodChanged() {
    m_pointData[m_points[m_currentIndex]].tbarSegGood = ui->tbarSegCheckBox->isChecked();
    saveState();
    updateTable();
}

void ConnectionValidationProtocol::onPSDGoodCanged() {
    m_pointData[m_points[m_currentIndex]].psdGood = ui->psdCheckBox->isChecked();
    saveState();
    updateTable();
}

void ConnectionValidationProtocol::onPSDSegGoodChanged() {
    m_pointData[m_points[m_currentIndex]].psdSegGood = ui->psdSegCheckBox->isChecked();
    saveState();
    updateTable();
}

void ConnectionValidationProtocol::onClickedTable(QModelIndex tableIndex) {
    // we don't have a sort proxy, so the table index = model index at this point
    setCurrentPoint(tableIndex.row());
}

void ConnectionValidationProtocol::setCurrentPoint(int index) {
    m_currentIndex = index;
    updateCurrentLabel();
    updateCheckBoxes();
}

void ConnectionValidationProtocol::selectCurrentRow() {
    QModelIndex index = m_sitesModel->index(m_currentIndex, 0);
    ui->sitesTableView->setCurrentIndex(index);
    ui->sitesTableView->scrollTo(index);
}

void ConnectionValidationProtocol::gotoCurrentPoint() {
    if (m_currentIndex >= 0) {
        ZIntPoint p = m_points[m_currentIndex];
        emit requestDisplayPoint(p.getX(), p.getY(), p.getZ());
    }
}

void ConnectionValidationProtocol::setSelectGotoCurrentPoint(int index) {
    setCurrentPoint(index);
    selectCurrentRow();
    gotoCurrentPoint();
}

void ConnectionValidationProtocol::updateLabels() {
    updateCurrentLabel();
    updateProgressLabel();
}

void ConnectionValidationProtocol::updateCurrentLabel() {
    if (m_currentIndex < 0) {
        // extra spaces keep the layout from shifting as much when the label changes
        ui->currentLabel->setText("(n/a)                    ");
    } else {
        ui->currentLabel->setText(QString::fromStdString(m_points[m_currentIndex].toString()));
    }
}

void ConnectionValidationProtocol::updateProgressLabel() {
    if (m_currentIndex < 0) {
        ui->progressLabel->setText("0/0 (0%)");
    } else {
        int nReviewed = 0;
        for (ZIntPoint p: m_points) {
            if (m_pointData[p].reviewed) {
                nReviewed++;
            }
        }
        float percent = float(nReviewed) / m_points.size();
        ui->progressLabel->setText(QString("%1/%2 (%3%)").arg(nReviewed).arg(m_points.size()).arg(percent, 1, 'f', 1));
    }
}

void ConnectionValidationProtocol::updateCheckBoxes() {
    PointData pd = m_pointData[m_points[m_currentIndex]];
    if (pd.reviewed) {
        ui->reviewedBox->setCheckState(Qt::CheckState::Checked);
    } else {
        ui->reviewedBox->setCheckState(Qt::CheckState::Unchecked);
    }
    if (pd.tbarGood) {
        ui->tbarCheckBox->setCheckState(Qt::CheckState::Checked);
    } else {
        ui->tbarCheckBox->setCheckState(Qt::CheckState::Unchecked);
    }
    if (pd.tbarSegGood) {
        ui->tbarSegCheckBox->setCheckState(Qt::CheckState::Checked);
    } else {
        ui->tbarSegCheckBox->setCheckState(Qt::CheckState::Unchecked);
    }
    if (pd.psdGood) {
        ui->psdCheckBox->setCheckState(Qt::CheckState::Checked);
    } else {
        ui->psdCheckBox->setCheckState(Qt::CheckState::Unchecked);
    }
    if (pd.psdSegGood) {
        ui->psdSegCheckBox->setCheckState(Qt::CheckState::Checked);
    } else {
        ui->psdSegCheckBox->setCheckState(Qt::CheckState::Unchecked);
    }
}

void ConnectionValidationProtocol::updateTable() {
    clearSitesTable();

    int row = 0;
    for (ZIntPoint p: m_points) {
        QStandardItem * pointItem = new QStandardItem();
        pointItem->setData(QVariant(QString::fromStdString(p.toString())), Qt::DisplayRole);
        m_sitesModel->setItem(row, POINT_COLUMN, pointItem);

        PointData pd = m_pointData[p];

        // later I'd like these to be graphics/icons; check mark for reviewed,
        //  and white plus on green circle/white minus on red circle for the others
        QStandardItem * revItem = new QStandardItem();
        QStandardItem * tbarGoodItem = new QStandardItem();
        QStandardItem * tbarSegGoodItem = new QStandardItem();
        QStandardItem * psdGoodItem = new QStandardItem();
        QStandardItem * psdSegGoodItem = new QStandardItem();
        revItem->setData(QVariant(pd.reviewed), Qt::DisplayRole);
        tbarGoodItem->setData(QVariant(pd.tbarGood), Qt::DisplayRole);
        tbarSegGoodItem->setData(QVariant(pd.tbarSegGood), Qt::DisplayRole);
        psdGoodItem->setData(QVariant(pd.psdGood), Qt::DisplayRole);
        psdSegGoodItem->setData(QVariant(pd.psdSegGood), Qt::DisplayRole);
        m_sitesModel->setItem(row, REVIEWED_COLUMN, revItem);
        m_sitesModel->setItem(row, TBAR_GOOD_COLUMN, tbarGoodItem);
        m_sitesModel->setItem(row, TBAR_SEG_GOOD_COLUMN, tbarSegGoodItem);
        m_sitesModel->setItem(row, PSD_GOOD_COLUMN, psdGoodItem);
        m_sitesModel->setItem(row, PSD_SEG_GOOD_COLUMN, psdSegGoodItem);

        row++;

    }
#if QT_VERSION >= 0x050000
    ui->sitesTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->sitesTableView->horizontalHeader()->setSectionResizeMode(POINT_COLUMN, QHeaderView::Stretch);
#else
    ui->sitesTableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->sitesTableView->horizontalHeader()->setResizeMode(POINT_COLUMN, QHeaderView::Stretch);
#endif

    // reselect current point:
    selectCurrentRow();
}

void ConnectionValidationProtocol::loadPoints(QJsonArray array) { // get point data into internal data structures
    m_points.clear();
    m_pointData.clear();
    for (QJsonValue val: array) {
        QJsonArray pointArray = val.toArray();
        ZIntPoint point;
        point.setX(pointArray[0].toInt());
        point.setY(pointArray[1].toInt());
        point.setZ(pointArray[2].toInt());
        PointData pd;
        m_points << point;
        m_pointData[point] = pd;
    }
}

void ConnectionValidationProtocol::saveState() {
    ZJsonObject data;

    // always version your output files!
    data.setEntry(KEY_VERSION.c_str(), fileVersion);

    // save data; all the keys are hard-coded here for now as I'm
    //  in a hurry; make them constants later

    // describe the arrays we'll be adding:
    ZJsonArray fields;
    fields.append("PSD location x");
    fields.append("PSD location y");
    fields.append("PSD location z");
    fields.append("reviewed");
    fields.append("T-bar good");
    fields.append("T-bar segmentation good");
    fields.append("PSD good");
    fields.append("PSD segmentation good");
    data.setEntry("fields", fields);

    // and the items themselves:
    ZJsonArray connections;
    for (ZIntPoint p: m_points) {
        PointData pd = m_pointData[p];
        ZJsonArray c;
        c.append(p.getX());
        c.append(p.getY());
        c.append(p.getZ());
        c.append(pd.reviewed);
        c.append(pd.tbarGood);
        c.append(pd.tbarSegGood);
        c.append(pd.psdGood);
        c.append(pd.psdSegGood);
        connections.append(c);
    }
    data.setEntry("connections", connections);

    emit requestSaveProtocol(data);
}

void ConnectionValidationProtocol::loadDataRequested(ZJsonObject data) {
    // check version of saved data here
    if (!data.hasKey(KEY_VERSION.c_str())) {
        showError("Can't find version!", "Can't find saved data version key!");
        return;
    }
    int version = ZJsonParser::integerValue(data[KEY_VERSION.c_str()]);
    if (version > fileVersion) {
        showError("Wrong version!", "Can't load this data version; you probably need to update NeuTu!");
        return;
    }

    // convert old versions to current version here, when it becomes necessary


    // load data here

    // we ignore the "fields" entry; that's for human use only; as long as the
    //  file version is correct, we know the order of items in the arrays

    // and unfortunately we can't use loadPoints(), because that's QJson, not ZJson

    m_points.clear();
    m_pointData.clear();
    ZJsonArray connections(data.value("connections"));
    for (size_t i=0; i<connections.size(); i++) {
        ZJsonArray pointData(connections.value(i));
        ZIntPoint p;
        PointData pd;
        p.setX(ZJsonParser::integerValue(pointData.at(0)));
        p.setY(ZJsonParser::integerValue(pointData.at(1)));
        p.setZ(ZJsonParser::integerValue(pointData.at(2)));
        pd.reviewed = ZJsonParser::booleanValue(pointData.at(3));
        pd.tbarGood= ZJsonParser::booleanValue(pointData.at(4));
        pd.tbarSegGood  = ZJsonParser::booleanValue(pointData.at(5));
        pd.psdGood = ZJsonParser::booleanValue(pointData.at(6));
        pd.psdSegGood = ZJsonParser::booleanValue(pointData.at(7));
        m_points << p;
        m_pointData[p] = pd;
    }


    // if, in the future, you need to update to a new save version,
    //  remember to do a saveState() here


    updateTable();
    updateLabels();

    // start work
    onFirstButton();
}

void ConnectionValidationProtocol::onExitButton() {
    // exit protocol; can be reopened and worked on later
    emit protocolExiting();
}

void ConnectionValidationProtocol::onCompleteButton() {
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

void ConnectionValidationProtocol::setSitesHeaders(QStandardItemModel * model) {
    model->setHorizontalHeaderItem(POINT_COLUMN, new QStandardItem(QString("point")));
    model->setHorizontalHeaderItem(REVIEWED_COLUMN, new QStandardItem(QString("rev")));
    model->setHorizontalHeaderItem(TBAR_GOOD_COLUMN, new QStandardItem(QString("T-bar")));
    model->setHorizontalHeaderItem(TBAR_SEG_GOOD_COLUMN, new QStandardItem(QString("T-seg")));
    model->setHorizontalHeaderItem(PSD_GOOD_COLUMN, new QStandardItem(QString("PSD")));
    model->setHorizontalHeaderItem(PSD_SEG_GOOD_COLUMN, new QStandardItem(QString("P-seg")));
}

void ConnectionValidationProtocol::clearSitesTable() {
    m_sitesModel->clear();
    setSitesHeaders(m_sitesModel);
}

void ConnectionValidationProtocol::showError(QString title, QString message) {
    QMessageBox mb;
    mb.setText(title);
    mb.setIcon(QMessageBox::Warning);
    mb.setInformativeText(message);
    mb.setStandardButtons(QMessageBox::Ok);
    mb.setDefaultButton(QMessageBox::Ok);
    mb.exec();
}

void ConnectionValidationProtocol::showMessage(QString title, QString message) {
    QMessageBox mb;
    mb.setText(title);
    mb.setInformativeText(message);
    mb.setStandardButtons(QMessageBox::Ok);
    mb.setDefaultButton(QMessageBox::Ok);
    mb.exec();
}

ConnectionValidationProtocol::~ConnectionValidationProtocol()
{
    delete ui;
}
