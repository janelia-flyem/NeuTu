#include "connectionvalidationprotocol.h"
#include "ui_connectionvalidationprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QFileDialog>
#include <QMessageBox>

#include <QTableWidgetItem>
#include <QIcon>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include "neutube.h"

#include "zjsonobject.h"
#include "zjsonparser.h"

ConnectionValidationProtocol::ConnectionValidationProtocol(QWidget *parent) :
    ProtocolDialog(parent),
    ui(new Ui::ConnectionValidationProtocol)
{
    ui->setupUi(this);

    // sites table
    m_sitesModel = new QStandardItemModel(0, 7, ui->sitesTableView);
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
    connect(ui->notSureBox, SIGNAL(clicked(bool)), this, SLOT(onNotSureChanged()));

    connect(ui->setCommentButton, SIGNAL(clicked(bool)), this, SLOT(onSetComment()));

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

    // we carry along an optional assignment ID:
    if (data.contains("assignment ID")) {
        m_assignmentID = data["assignment ID"].toString();
    }

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

void ConnectionValidationProtocol::onNotSureChanged() {
    m_pointData[m_points[m_currentIndex]].notSure = ui->notSureBox->isChecked();
    saveState();
    updateTable();
}

void ConnectionValidationProtocol::onSetComment() {
    m_pointData[m_points[m_currentIndex]].comment = ui->commentEdit->text();
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
    updateComment();
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
        float percent = 100 * float(nReviewed) / m_points.size();
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
    if (pd.notSure) {
        ui->notSureBox->setCheckState(Qt::CheckState::Checked);
    } else {
        ui->notSureBox->setCheckState(Qt::CheckState::Unchecked);
    }
}

void ConnectionValidationProtocol::updateComment() {
    PointData pd = m_pointData[m_points[m_currentIndex]];
    ui->commentEdit->setText(pd.comment);
}

void ConnectionValidationProtocol::updateTable() {
    clearSitesTable();

    int row = 0;
    for (ZIntPoint p: m_points) {
        QStandardItem * pointItem = new QStandardItem();
        pointItem->setData(QVariant(QString::fromStdString(p.toString())), Qt::DisplayRole);
        m_sitesModel->setItem(row, POINT_COLUMN, pointItem);

        PointData pd = m_pointData[p];

        // for "reviewed", "has comment", use a text check mark
        if (pd.reviewed) {
            QStandardItem * revItem = new QStandardItem();
            revItem->setData(QVariant(QString::fromUtf8("\u2714")), Qt::DisplayRole);
            m_sitesModel->setItem(row, REVIEWED_COLUMN, revItem);
        }
        if (!pd.comment.isEmpty()) {
            QStandardItem * commentItem = new QStandardItem();
            commentItem->setData(QVariant(QString::fromUtf8("\u2714")), Qt::DisplayRole);
            m_sitesModel->setItem(row, HAS_COMMENT_COLUMN, commentItem);
        }

        // for the various statuses, use icons; note that the code for
        //  for using text instead is still present but commented out;
        //  interestingly, you can use both (icon ends up to left of text)
        QIcon goodIcon(":/images/verify.png");      // green check mark
        QIcon badIcon(":/images/delete.png");       // red X
        QIcon notsureIcon(":/images/help2.png");    // question mark

        QStandardItem * tbarGoodItem = new QStandardItem();
        // tbarGoodItem->setData(QVariant(pd.tbarGood), Qt::DisplayRole);
        if (pd.tbarGood) {
            tbarGoodItem->setIcon(goodIcon);
        } else {
            tbarGoodItem->setIcon(badIcon);
        }
        m_sitesModel->setItem(row, TBAR_GOOD_COLUMN, tbarGoodItem);

        QStandardItem * tbarSegGoodItem = new QStandardItem();
        // tbarSegGoodItem->setData(QVariant(pd.tbarSegGood), Qt::DisplayRole);
        if (pd.tbarSegGood) {
            tbarSegGoodItem->setIcon(goodIcon);
        } else {
            tbarSegGoodItem->setIcon(badIcon);
        }
        m_sitesModel->setItem(row, TBAR_SEG_GOOD_COLUMN, tbarSegGoodItem);

        QStandardItem * psdGoodItem = new QStandardItem();
        // psdGoodItem->setData(QVariant(pd.psdGood), Qt::DisplayRole);
        if (pd.psdGood) {
            psdGoodItem->setIcon(goodIcon);
        } else {
            psdGoodItem->setIcon(badIcon);
        }
        m_sitesModel->setItem(row, PSD_GOOD_COLUMN, psdGoodItem);

        QStandardItem * psdSegGoodItem = new QStandardItem();
        // psdSegGoodItem->setData(QVariant(pd.psdSegGood), Qt::DisplayRole);
        if (pd.psdSegGood) {
            psdSegGoodItem->setIcon(goodIcon);
        } else {
            psdSegGoodItem->setIcon(badIcon);
        }
        m_sitesModel->setItem(row, PSD_SEG_GOOD_COLUMN, psdSegGoodItem);

        QStandardItem * notSureItem = new QStandardItem();
        // notSureItem->setData(QVariant(pd.notSure), Qt::DisplayRole);
        if (pd.notSure) {
            notSureItem->setIcon(notsureIcon);
        } else {
            // no icon if we are sure
        }
        m_sitesModel->setItem(row, NOT_SURE_COLUMN, notSureItem);

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

    // metadata, mostly for assignment tracking purposes:
    data.setEntry("assignment ID", m_assignmentID.toStdString());
    data.setEntry("username", neutu::GetCurrentUserName());

    // and the items themselves:
    ZJsonArray connections;
    for (ZIntPoint p: m_points) {
        PointData pd = m_pointData[p];
        ZJsonObject c;
        ZJsonArray location;
        location.append(p.getX());
        location.append(p.getY());
        location.append(p.getZ());
        c.setEntry("location", location);
        c.setEntry("reviewed", pd.reviewed);
        c.setEntry("T-bar good", pd.tbarGood);
        c.setEntry("T-bar segmentation good", pd.tbarSegGood);
        c.setEntry("PSD good", pd.psdGood);
        c.setEntry("PSD segmentation good", pd.psdSegGood);
        c.setEntry("comment", pd.comment.toStdString());
        c.setEntry("not sure", pd.notSure);
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

    // metadata
    // load assignment ID, which might be empty or absent
    if (data.hasKey("assignment ID")) {
        m_assignmentID = QString::fromStdString(ZJsonParser::stringValue(data["assignment ID"]));
    } else {
        m_assignmentID = "";
    }

    m_points.clear();
    m_pointData.clear();
    ZJsonArray connections(data.value("connections"));
    for (size_t i=0; i<connections.size(); i++) {
        ZJsonObject pointData(connections.value(i));
        ZIntPoint p = ZJsonParser::toIntPoint(pointData["location"]);
        PointData pd;
        pd.reviewed = ZJsonParser::booleanValue(pointData["reviewed"]);
        pd.tbarGood= ZJsonParser::booleanValue(pointData["T-bar good"]);
        pd.tbarSegGood  = ZJsonParser::booleanValue(pointData["T-bar segmentation good"]);
        pd.psdGood = ZJsonParser::booleanValue(pointData["PSD good"]);
        pd.psdSegGood = ZJsonParser::booleanValue(pointData["PSD segmentation good"]);
        pd.comment = QString::fromStdString(ZJsonParser::stringValue(pointData["comment"]));
        pd.notSure = ZJsonParser::booleanValue(pointData["not sure"]);
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
    // warn if not all connections reviewed; you can easily totally disalow it instead
    bool complete = true;
    for (ZIntPoint p: m_points) {
        if (!m_pointData[p].reviewed) {
            complete = false;
            break;
        }
    }
    if (!complete) {
        QMessageBox incompleteBox;
        incompleteBox.setText("Unreviewed connections!");
        incompleteBox.setInformativeText("There are unreviewed connections! Complete the protocol anyway?\n\nContinue completing or cancel?");
        incompleteBox.setIcon(QMessageBox::Warning);
        incompleteBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
        incompleteBox.setDefaultButton(QMessageBox::Cancel);
        int ans = incompleteBox.exec();
        if (ans == QMessageBox::Cancel) {
            return;
        }
    }


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
    model->setHorizontalHeaderItem(HAS_COMMENT_COLUMN, new QStandardItem(QString("comment")));
    model->setHorizontalHeaderItem(NOT_SURE_COLUMN, new QStandardItem(QString("not sure")));
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
