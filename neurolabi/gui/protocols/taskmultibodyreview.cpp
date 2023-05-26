#include "taskmultibodyreview.h"

#include <iostream>
#include <stdlib.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QLabel>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "neutubeconfig.h"
#include "logging/zqslog.h"
#include "flyem/zflyembodyannotation.h"
#include "flyem/zflyembody3ddoc.h"
#include "flyem/zflyemproofdoc.h"
#include "flyem/zflyembodymanager.h"
#include "flyem/flyembodyannotationmanager.h"
#include "z3dmeshfilter.h"
#include "z3dwindow.h"
#include "zglmutils.h"
#include "zstackdocproxy.h"

// save/restore mechanism copied from taskmergereview.c
namespace {
    // https://sashat.me/2017/01/11/list-of-20-simple-distinct-colors/
    // these colors are used in the cleave protocol as well, so they are familiar in this 
    //   order; moved white to the end as (a) it disappears in the table view, and (b) cleave
    //   protocol reserves it for other uses, but (c) I want to restore it when we're done,
    //   as it's neu3's default mesh color
    static const std::vector<glm::vec4> INDEX_COLORS({
        glm::vec4(230,  25,  75, 255) / 255.0f, // red
        glm::vec4(255, 225,  25, 255) / 255.0f, // yellow
        glm::vec4(  0, 130, 200, 255) / 255.0f, // blue
        glm::vec4(245, 130,  48, 255) / 255.0f, // orange
        glm::vec4(145,  30, 180, 255) / 255.0f, // purple
        glm::vec4( 70, 240, 240, 255) / 255.0f, // cyan
        glm::vec4( 60, 180,  75, 255) / 255.0f, // green
        glm::vec4(240,  50, 230, 255) / 255.0f, // magenta
        glm::vec4(210, 245,  60, 255) / 255.0f, // lime
        glm::vec4(250, 190, 190, 255) / 255.0f, // pink
        glm::vec4(  0, 128, 128, 255) / 255.0f, // teal
        glm::vec4(230, 190, 255, 255) / 255.0f, // lavender
        glm::vec4(170, 110,  40, 255) / 255.0f, // brown
        glm::vec4(255, 250, 200, 255) / 255.0f, // beige
        glm::vec4(128,   0,   0, 255) / 255.0f, // maroon
        glm::vec4(170, 255, 195, 255) / 255.0f, // mint
        glm::vec4(128, 128,   0, 255) / 255.0f, // olive
        glm::vec4(255, 215, 180, 255) / 255.0f, // coral
        glm::vec4(  0,   0, 128, 255) / 255.0f, // navy
        glm::vec4(128, 128, 128, 255) / 255.0f, // gray
        glm::vec4(255, 255, 255, 255) / 255.0f, // white (default neu3 mesh color)
    });

    // we set some values shared by all TaskMultiBodyReview instances, and restore them
    //  when we're done
    // note that we start using coarse meshes, then allow the user to toggle in the UI
    static bool applySharedSettingsNeeded = true;
    static bool useCoarseMeshes = true;
    static int minDsLevel;
    static int maxDsLevel;
}

TaskMultiBodyReview::TaskMultiBodyReview(QJsonObject json, ZFlyEmBody3dDoc * bodyDoc)
{
    if (json[KEY_TASKTYPE] != VALUE_TASKTYPE) {
        // wrong type, don't load the json
        return;
    }

    m_bodyDoc = bodyDoc;

    m_visibleBodies = QSet<uint64_t>();
    m_selectedBodies = QSet<uint64_t>();

    applySharedSettings(m_bodyDoc);
    loadJson(json);

    // this is the list of statuses the user is allowed to change
    ZFlyEmProofDoc* proofDoc = m_bodyDoc->getDataDocument();
    m_userBodyStatuses = proofDoc->getBodyAnnotationManager()->getBodyStatusList();

    setupUI();
    setupDVID();
}

// constants
const QString TaskMultiBodyReview::KEY_TASKTYPE = "task type";
const QString TaskMultiBodyReview::VALUE_TASKTYPE = "multibody review";
const QString TaskMultiBodyReview::KEY_BODYIDS = "body IDs";

// yes, that's the capitalization we use...
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

    if (!m_originalStatusesLoaded) {
        m_originalStatuses.clear();
        for (ZFlyEmBodyAnnotation ann: m_bodyAnnotations) {
            m_originalStatuses << ann.getStatus();
        }
        m_originalStatusesLoaded = true;
    }

}

void TaskMultiBodyReview::onLoaded() {
    loadBodyData();
    // setColors() must be after loadBodyData() as it uses m_bodyIDs
    setColors();
    updateTable();
}

void TaskMultiBodyReview::beforeDone() {
    restoreSharedSettings(m_bodyDoc);

    // need to reset the colors on the filter, because otherwise it'll hold
    // onto a body list that will soon disappear
    if (Z3DWindow *window = m_bodyDoc->getParent3DWindow()) {
        if (Z3DMeshFilter *filter = dynamic_cast<Z3DMeshFilter*>(window->getMeshFilter())) {
            filter->setColorIndexing(INDEX_COLORS, [=](uint64_t id) -> std::size_t {
                // return index for white from the table; this is neu3's
                // default mesh color
                return 20;
            });
        }
    }

}

void TaskMultiBodyReview::beforeLoading() {
    m_meshCheckbox->setChecked(useCoarseMeshes);
}

void TaskMultiBodyReview::setupUI() {
    m_widget = new QWidget();

    QVBoxLayout *topLayout = new QVBoxLayout(m_widget);
    m_widget->setLayout(topLayout);

    // body info table
    m_bodyTableView = new QTableView(m_widget);
    m_bodyModel = new QStandardItemModel(0, 5, m_bodyTableView);
    setTableHeaders(m_bodyModel);
    m_bodyTableView->setModel(m_bodyModel);
    connect(m_bodyTableView, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedTable(QModelIndex)));
    topLayout->addWidget(m_bodyTableView);

    // more buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout(m_widget);
    m_meshCheckbox = new QCheckBox("Coarse meshes", m_widget);
    m_meshCheckbox->setChecked(true);
    connect(m_meshCheckbox, SIGNAL(clicked(bool)), this, SLOT(onToggleMeshQuality()));
    buttonLayout->addWidget(m_meshCheckbox);
    buttonLayout->addStretch();
    QPushButton * refreshButton = new QPushButton("Refresh", m_widget);
    connect(refreshButton, SIGNAL(clicked(bool)), this, SLOT(onRefreshButton()));
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();
    QPushButton * allPRTButton = new QPushButton("Set all bodies to PRT", m_widget);
    connect(allPRTButton, SIGNAL(clicked(bool)), this, SLOT(onAllPRTButton()));
    buttonLayout->addWidget(allPRTButton);
    topLayout->addLayout(buttonLayout);
}

void TaskMultiBodyReview::setupDVID() {
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

void TaskMultiBodyReview::setColors() {
    if (Z3DWindow *window = m_bodyDoc->getParent3DWindow()) {
        if (Z3DMeshFilter *filter = dynamic_cast<Z3DMeshFilter*>(window->getMeshFilter())) {
            filter->setColorMode("Indexed Color");
            filter->setColorIndexing(INDEX_COLORS, [=](uint64_t id) -> std::size_t {
                return m_bodyIDs.indexOf(id);
            });
        }
    }
}

void TaskMultiBodyReview::updateTable() {
    // clear and repopulate model
    m_bodyModel->clear();
    setTableHeaders(m_bodyModel);

    QPushButton * actionButton;

    for (int row=0; row<m_bodyIDs.size(); row++) {
        QStandardItem * bodyIDItem = new QStandardItem();
        bodyIDItem->setData(QVariant(QString::number(m_bodyIDs[row])), Qt::DisplayRole);
        if (!m_tags.isEmpty()) {
            QStringList tagList;
            for (QString s: m_tags) {
                tagList << s;
            }
            bodyIDItem->setToolTip(tagList.join(","));
        }

        // adjust color swatch; rows are in body ID order, as are colors; also, try to respect
        //  current opacity
        float opacity = 1.0;
        if (Z3DWindow *window = m_bodyDoc->getParent3DWindow()) {
            if (Z3DMeshFilter *filter = dynamic_cast<Z3DMeshFilter*>(window->getMeshFilter())) {
                opacity = filter->opacity();
            }
        }
        QColor swatchColor = QColor(255 * INDEX_COLORS[row][0], 255 * INDEX_COLORS[row][1],
            255 * INDEX_COLORS[row][2], 255 * opacity * INDEX_COLORS[row][3]);
        bodyIDItem->setData(swatchColor, Qt::DecorationRole);
        m_bodyModel->setItem(row, BODYID_COLUMN, bodyIDItem);

        QStandardItem * cellTypeItem = new QStandardItem();
        cellTypeItem->setData(QVariant(QString::fromStdString(m_bodyAnnotations[row].getType())), Qt::DisplayRole);
        cellTypeItem->setToolTip(QString::fromStdString(m_bodyAnnotations[row].getType()));
        m_bodyModel->setItem(row, CELL_TYPE_COLUMN, cellTypeItem);

        QStandardItem * instanceItem = new QStandardItem();
        instanceItem->setData(QVariant(QString::fromStdString(m_bodyAnnotations[row].getInstance())), Qt::DisplayRole);
        instanceItem->setToolTip(QString::fromStdString(m_bodyAnnotations[row].getInstance()));
        m_bodyModel->setItem(row, INSTANCE_COLUMN, instanceItem);

        QStandardItem * statusItem = new QStandardItem();
        std::string status = m_bodyAnnotations[row].getStatus();
        statusItem->setData(QVariant(QString::fromStdString(status)), Qt::DisplayRole);
        statusItem->setToolTip(QString::fromStdString(status));
        m_bodyModel->setItem(row, STATUS_COLUMN, statusItem);

        actionButton = new QPushButton();
        m_bodyTableView->setIndexWidget(m_bodyModel->index(row , BUTTON_COLUMN), actionButton);

        // adjust button behavior based on current and original status
        std::string originalStatus = m_originalStatuses[row];
        if (originalStatus == STATUS_PRT.toStdString()) {
            // body is originally PRT, no action needed or possible
            actionButton->setText("Set PRT");
            actionButton->setDisabled(true);
        } else if (status == originalStatus) {
            // has its original status but not already PRT = can set PRT
            actionButton->setText("Set PRT");
            connect(actionButton, &QPushButton::clicked, m_bodyTableView, [this, row]() {
                onRowPRTButton(row);
            });
        } else {
            // it's PRT now, and can be set back to original status
            actionButton->setText("Revert");
            connect(actionButton, &QPushButton::clicked, m_bodyTableView, [this, row]() {
                onRowRevertButton(row);
            });
        }
    }

    m_bodyTableView->resizeColumnsToContents();
    m_bodyTableView->horizontalHeader()->setSectionResizeMode(CELL_TYPE_COLUMN, QHeaderView::Stretch);
    m_bodyTableView->horizontalHeader()->setSectionResizeMode(INSTANCE_COLUMN, QHeaderView::Stretch);

}

bool TaskMultiBodyReview::usePrefetching() {
    // prefetching was causing crashes in this protocol, not sure why
    return false;
}

void TaskMultiBodyReview::onClickedTable(QModelIndex index) {
    if (index.column() == BODYID_COLUMN) {
        // we don't have a sort proxy, so the table index = model index at this point
        m_bodyTableView->selectionModel()->select(index, QItemSelectionModel::Select);
        QSet<uint64_t> selected;
        selected << m_bodyIDs[index.row()];
        updateBodies(m_visibleBodies, selected);
    }
}

void TaskMultiBodyReview::onRowPRTButton(int row) {
    setPRTStatusForRow(row);
    loadBodyData();
    updateTable();
}

void TaskMultiBodyReview::onRowRevertButton(int row) {
    setOriginalStatusForRow(row);
    loadBodyData();
    updateTable();
}

void TaskMultiBodyReview::onRefreshButton() {
    loadBodyData();
    updateTable();
}

void TaskMultiBodyReview::onAllPRTButton() {
    for (auto row=0; row<m_bodyModel->rowCount(); row++) {
        setPRTStatusForRow(row);
    }
    loadBodyData();
    updateTable();
}

void TaskMultiBodyReview::onToggleMeshQuality() {
    useCoarseMeshes = m_meshCheckbox->isChecked();
    // can't figure out how to redraw immediately; see updateDisplay() method for notes
    QMessageBox infoBox;
    infoBox.setText("Mesh quality changed");
    infoBox.setInformativeText("Mesh quality change will take place when loading 'Next' or 'Prev' task");
    infoBox.setStandardButtons(QMessageBox::Ok);
    infoBox.setIcon(QMessageBox::Warning);
    infoBox.exec();
    if (useCoarseMeshes) {
        setCoarseMeshes(m_bodyDoc);
    } else {
        setOriginalMeshes(m_bodyDoc);
    }
}

void TaskMultiBodyReview::setPRTStatusForRow(int row) {
    ZFlyEmBodyAnnotation ann = m_bodyAnnotations[row];
    if (QString::fromStdString(ann.getStatus()) != STATUS_PRT) {
        // verify the user is allowed to do this!
        if (!m_userBodyStatuses.contains(QString::fromStdString(ann.getStatus())) ||
            !m_userBodyStatuses.contains(QString::fromStdString(STATUS_PRT.toStdString()))) {
            QMessageBox infoBox;
            infoBox.setText("Body status problem");
            QString message = "You do not have permission to change status " +
                QString::fromStdString(ann.getStatus()) + " to PRT";
            infoBox.setInformativeText(message);
            infoBox.setStandardButtons(QMessageBox::Ok);
            infoBox.setIcon(QMessageBox::Warning);
            infoBox.exec();
        } else {
            setStatus(m_bodyIDs[row], m_bodyAnnotations[row], STATUS_PRT.toStdString());
        }
    }
}

void TaskMultiBodyReview::setOriginalStatusForRow(int row) {
    ZFlyEmBodyAnnotation ann = m_bodyAnnotations[row];
    std::string originalStatus = m_originalStatuses[row];
    if (ann.getStatus() != originalStatus) {
        setStatus(m_bodyIDs[row], m_bodyAnnotations[row], originalStatus);
    }
}

void TaskMultiBodyReview::setStatus(uint64_t bodyId, ZFlyEmBodyAnnotation ann, std::string status) {
    ann.setStatus(status);
    m_writer.writeBodyAnnotation(bodyId, ann);
}

void TaskMultiBodyReview::setTableHeaders(QStandardItemModel * model) {
    model->setHorizontalHeaderItem(BODYID_COLUMN, new QStandardItem(QString("body ID")));
    model->setHorizontalHeaderItem(CELL_TYPE_COLUMN, new QStandardItem(QString("cell type")));
    model->setHorizontalHeaderItem(INSTANCE_COLUMN, new QStandardItem(QString("instance")));
    model->setHorizontalHeaderItem(STATUS_COLUMN, new QStandardItem(QString("status")));
    model->setHorizontalHeaderItem(BUTTON_COLUMN, new QStandardItem(QString("button")));
}

QWidget * TaskMultiBodyReview::getTaskWidget() {
    return m_widget;
}

void TaskMultiBodyReview::applySharedSettings(ZFlyEmBody3dDoc* bodyDoc) {
    if (applySharedSettingsNeeded) {
        applySharedSettingsNeeded = false;
        minDsLevel = bodyDoc->getMinDsLevel();
        maxDsLevel = bodyDoc->getMaxDsLevel();
        setCoarseMeshes(bodyDoc);
    }
}

void TaskMultiBodyReview::restoreSharedSettings(ZFlyEmBody3dDoc * bodyDoc) {
    if (!applySharedSettingsNeeded) {
          applySharedSettingsNeeded = true;
          setOriginalMeshes(bodyDoc);
    }
}

void TaskMultiBodyReview::setCoarseMeshes(ZFlyEmBody3dDoc * bodyDoc) {
    bodyDoc->useCoarseOnly();
    updateDisplay();
}

void TaskMultiBodyReview::setOriginalMeshes(ZFlyEmBody3dDoc * bodyDoc) {
    bodyDoc->setMinDsLevel(minDsLevel);
    bodyDoc->setMaxDsLevel(maxDsLevel);
    updateDisplay();
}

void TaskMultiBodyReview::updateDisplay() {
    // after I update the mesh quality settings, I want to force the display to update
    //  with the new mesh level; none of these seems to work for me

    // emit bodiesUpdated();

    // updateBodies(m_visibleBodies, m_selectedBodies);

    // QSet<uint64_t> tempVisible(m_visibleBodies);
    // QSet<uint64_t> tempSelected(m_selectedBodies);
    // QSet<uint64_t> empty;
    // updateBodies(empty, empty);
    // updateBodies(tempVisible, tempSelected);

    // bodyDoc->processObjectModified(ZStackObject::EType::MESH);

    // QList<ZMesh*> meshes = ZStackDocProxy::GetGeneralMeshList(bodyDoc);
    // for (auto itMesh = meshes.cbegin(); itMesh != meshes.cend(); itMesh++) {
    //     ZMesh *mesh = *itMesh;
    //     bodyDoc->bufferObjectModified(mesh, ZStackObjectInfo::STATE_MODIFIED);
    // }
    // bodyDoc->processObjectModified();
}
