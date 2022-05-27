#include "taskmultibodyreview.h"

#include <iostream>
#include <stdlib.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "neutubeconfig.h"
#include "logging/zqslog.h"
#include "flyem/zflyembodyannotation.h"
#include "flyem/zflyembody3ddoc.h"
#include "flyem/zflyembodymanager.h"

TaskMultiBodyReview::TaskMultiBodyReview(QJsonObject json, ZFlyEmBody3dDoc * bodyDoc)
{
    if (json[KEY_TASKTYPE] != VALUE_TASKTYPE) {
        // wrong type, don't load the json
        return;
    }

    m_bodyDoc = bodyDoc;

    m_visibleBodies = QSet<uint64_t>();
    m_selectedBodies = QSet<uint64_t>();

    loadJson(json);

    setupUI();

}

// constants
const QString TaskMultiBodyReview::KEY_TASKTYPE = "task type";
const QString TaskMultiBodyReview::VALUE_TASKTYPE = "multibody review";
const QString TaskMultiBodyReview::KEY_BODYIDS = "body IDs";

const QString TaskMultiBodyReview::STATUS_PRT = "Prelim Roughly traced";

QString TaskMultiBodyReview::taskTypeStatic()
{
  return VALUE_TASKTYPE;
}

TaskMultiBodyReview* TaskMultiBodyReview::createFromJson(QJsonObject json, ZFlyEmBody3dDoc * bodyDoc)
{
  return new TaskMultiBodyReview(json, bodyDoc);
}

QString TaskMultiBodyReview::taskType() const
{
  return taskTypeStatic();
}

QString TaskMultiBodyReview::actionString() {
    return "Review bodies:";
}

QString TaskMultiBodyReview::targetString() {
    return "(multiple bodies)";
}

QJsonObject TaskMultiBodyReview::addToJson(QJsonObject taskJson) {
    taskJson[KEY_BODYIDS] = "(multiple bodies)";
    taskJson[KEY_TASKTYPE] = VALUE_TASKTYPE;

    return taskJson;
}

bool TaskMultiBodyReview::loadSpecific(QJsonObject /* json */ ) {
    // nothing specific to load for this protocol
    return true;
}

void TaskMultiBodyReview::loadBodyData() {
    m_bodyIDs.clear();
    m_bodyAnnotations.clear();

    std::vector<uint64_t> bodyIDs(m_visibleBodies.size());
    size_t i=0;
    for (uint64_t bodyID: m_visibleBodies) {
        // need std::vector for retrieving body annotations, and QList for general use later
        m_bodyIDs << bodyID;
        bodyIDs[i] = bodyID;
        i++;
    }

    std::vector<ZJsonObject> bodyData = m_reader.readBodyAnnotationJsons(bodyIDs);
    for (const auto &obj: bodyData) {
        ZFlyEmBodyAnnotation ann;
        ann.loadJsonObject(obj);
        m_bodyAnnotations << ann;
    }
}

void TaskMultiBodyReview::onLoaded() {
    loadBodyData();
    updateTable();
}

void TaskMultiBodyReview::setupUI() {
    m_widget = new QWidget();

    QVBoxLayout *topLayout = new QVBoxLayout(m_widget);
    m_widget->setLayout(topLayout);

    // body info table
    m_bodyTableView = new QTableView(m_widget);
    m_bodyModel = new QStandardItemModel(0, 4, m_bodyTableView);
    setTableHeaders(m_bodyModel);
    m_bodyTableView->setModel(m_bodyModel);
    topLayout->addWidget(m_bodyTableView);


    // setup DVID reader, writer; create one of each and reuse so
    //  we don't multiply connections
    m_reader.setVerbose(false);
    if (!m_reader.open(m_bodyDoc->getDvidTarget())) {
      LERROR() << "TaskMultiBodyReview::setupUI() could not open DVID target for reading";
      QMessageBox errorBox;
      errorBox.setText("Error connecting to DVID");
      errorBox.setInformativeText("Could not open DVID!  Check server information!");
      errorBox.setStandardButtons(QMessageBox::Ok);
      errorBox.setIcon(QMessageBox::Warning);
      errorBox.exec();
      return;
    }
    if (!m_writer.open(m_bodyDoc->getDvidTarget())) {
      LERROR() << "TaskMultiBodyReview::setupUI() could not open DVID target for writing";
      QMessageBox errorBox;
      errorBox.setText("Error connecting to DVID");
      errorBox.setInformativeText("Could not open DVID!  Check server information!");
      errorBox.setStandardButtons(QMessageBox::Ok);
      errorBox.setIcon(QMessageBox::Warning);
      errorBox.exec();
      return;
    }

}

void TaskMultiBodyReview::updateTable() {
    // clear and repopulate model
    m_bodyModel->clear();
    setTableHeaders(m_bodyModel);

    QPushButton * tempButton;

    for (int row=0; row<m_bodyIDs.size(); row++) {
        QStandardItem * bodyIDItem = new QStandardItem();
        bodyIDItem->setData(QVariant(QString::number(m_bodyIDs[row])), Qt::DisplayRole);
        m_bodyModel->setItem(row, BODYID_COLUMN, bodyIDItem);

        QStandardItem * cellTypeItem = new QStandardItem();
        cellTypeItem->setData(QVariant(QString::fromStdString(m_bodyAnnotations[row].getType())), Qt::DisplayRole);
        m_bodyModel->setItem(row, CELL_TYPE_COLUMN, cellTypeItem);

        QStandardItem * statusItem = new QStandardItem();
        statusItem->setData(QVariant(QString::fromStdString(m_bodyAnnotations[row].getStatus())), Qt::DisplayRole);
        m_bodyModel->setItem(row, STATUS_COLUMN, statusItem);

        tempButton = new QPushButton();
        tempButton->setText("Set PRT");
        m_bodyTableView->setIndexWidget(m_bodyModel->index(row , BUTTON_COLUMN), tempButton);
        connect(tempButton, &QPushButton::clicked, m_bodyTableView, [this, row]() {
            onRowButton(row);
        });
    }
}

// testing
bool TaskMultiBodyReview::usePrefetching() {
    return false;
}

void TaskMultiBodyReview::onRowButton(int row) {
    ZFlyEmBodyAnnotation ann = m_bodyAnnotations[row];
    if (QString::fromStdString(ann.getStatus()) != STATUS_PRT) {
        setPRTStatus(m_bodyIDs[row], m_bodyAnnotations[row]);
    }
}

void TaskMultiBodyReview::onTestButton() {
    LINFO() << "test button pressed";

}

void TaskMultiBodyReview::setPRTStatus(uint64_t bodyId, ZFlyEmBodyAnnotation ann) {
    ann.setStatus(STATUS_PRT.toStdString());
    ann.setStatusUser(NeutubeConfig::GetUserName());
    m_writer.writeBodyAnnotation(bodyId, ann);

    loadBodyData();
    updateTable();
}


void TaskMultiBodyReview::setTableHeaders(QStandardItemModel * model) {
    model->setHorizontalHeaderItem(BODYID_COLUMN, new QStandardItem(QString("body ID")));
    model->setHorizontalHeaderItem(CELL_TYPE_COLUMN, new QStandardItem(QString("cell type")));
    model->setHorizontalHeaderItem(STATUS_COLUMN, new QStandardItem(QString("status")));
    model->setHorizontalHeaderItem(BUTTON_COLUMN, new QStandardItem(QString("button")));
}

QWidget * TaskMultiBodyReview::getTaskWidget() {
    return m_widget;
}
