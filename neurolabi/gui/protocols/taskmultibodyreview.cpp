#include "taskmultibodyreview.h"

#include <iostream>
#include <stdlib.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "logging/zqslog.h"
#include "flyem/zflyembody3ddoc.h"
#include "flyem/zflyembodymanager.h"
#include "dvid/zdvidreader.h"

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

    loadBodyData();

    setupUI();

}

// constants
const QString TaskMultiBodyReview::KEY_TASKTYPE = "task type";
const QString TaskMultiBodyReview::VALUE_TASKTYPE = "multibody review";
const QString TaskMultiBodyReview::KEY_BODYIDS = "body IDs";

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
    ZDvidReader reader;
    reader.setVerbose(false);
    if (!reader.open(m_bodyDoc->getDvidTarget())) {
      LERROR() << "TaskMultiBodyReview::loadBodyData() could not open DVID target for reading";
      return;
    }

    m_bodyIDs.clear();
    m_celltypes.clear();
    m_statuses.clear();

    std::vector<uint64_t> bodyIDs(m_visibleBodies.size());
    size_t i=0;
    for (uint64_t bodyID: m_visibleBodies) {
        bodyIDs[i] = bodyID;
        i++;
    }
    std::sort(bodyIDs.begin(), bodyIDs.end());
    std::vector<ZJsonObject> bodyData = reader.readBodyAnnotationJsons(bodyIDs);

    for (const auto &obj: bodyData) {
        QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(obj.dumpString()).toUtf8());
        QJsonObject data = doc.object();

        // m_bodyIDs << data["bodyid"].toInt();
        m_bodyIDs << uint64_t(data["bodyid"].toDouble());
        m_celltypes << data["type"].toString();
        m_statuses << data["status"].toString();

    }
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






    // testing, remove later
    QPushButton *tempButton = new QPushButton("Test", m_widget);
    connect(tempButton, SIGNAL(clicked(bool)), this, SLOT(onTestButton()));
    topLayout->addWidget(tempButton);


    updateTable();

}

void TaskMultiBodyReview::updateTable() {
    // clear and repopulate model
    m_bodyModel->clear();
    setTableHeaders(m_bodyModel);

    for (int row=0; row<m_bodyIDs.size(); row++) {

        LINFO() << "trying to set body ID " << m_bodyIDs[row];

        QStandardItem * bodyIDItem = new QStandardItem();
        bodyIDItem->setData(QVariant(QString::number(m_bodyIDs[row])), Qt::DisplayRole);
        m_bodyModel->setItem(row, BODYID_COLUMN, bodyIDItem);

        QStandardItem * cellTypeItem = new QStandardItem();
        cellTypeItem ->setData(QVariant(m_celltypes[row]), Qt::DisplayRole);
        m_bodyModel->setItem(row, CELL_TYPE_COLUMN, cellTypeItem );

        QStandardItem * statusItem = new QStandardItem();
        statusItem  ->setData(QVariant(m_statuses[row]), Qt::DisplayRole);
        m_bodyModel->setItem(row, STATUS_COLUMN, statusItem  );

    }



}

void TaskMultiBodyReview::onTestButton() {
    LINFO() << "test button pressed";

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
