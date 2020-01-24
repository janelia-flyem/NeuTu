#include "flyembodyinfodialog.h"

#include <iostream>
#include <stdlib.h>
#include <sstream>

#include <QMessageBox>
#include <QColor>
#include <QFileDialog>
#include <QColorDialog>
#include <QStandardItemModel>
#include <QItemSelection>
#include <QElapsedTimer>
#include <QInputDialog>

#if QT_VERSION >= 0x050000
#include <QtConcurrent>
#else
#include <QtConcurrentRun>
#endif

#include "zstring.h"

#include "zjsonarray.h"
#include "zjsonobject.h"
#include "zjsonparser.h"
#include "zjsonobjectparser.h"

#include "zglobal.h"

#include "logging/zlog.h"

#include "qt/gui/utilities.h"

#include "dvid/zdvidtarget.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidsynapse.h"
#include "dvid/zdvidroi.h"

#include "ui_flyembodyinfodialog.h"
#include "zdialogfactory.h"
#include "service/neuprintreader.h"
#include "neuprintquerydialog.h"
#include "flyem/zflyembodyannotation.h"

/*
 * this dialog displays a list of bodies and their properties; data is
 * loaded from DVID
 *
 * to add/remove/alter columns in body table:
 * -- when creating model, change # columns
 * -- in setBodyHeaders(), adjust headers
 * -- in updateModel(), adjust data load and initial sort order
 * -- in enum in .h file, add new column constant
 *
 * notes:
 * -- I really should be creating model and headers from a constant headers list
 *
 * -- at some point, it would be good to separate the load logic into its own class
 *
 * -- in fact, a lot of things need to be separated out; I wrote this when I
 *      was just learning Qt, and using the designer on top of that, so
 *      I didn't create new widgets to compose due to lack of
 *      experience and time; that's why everthing is in this one class
 *
 *
 * djo, 7/15
 *
 */

namespace {

const QString COLOR_GROUP_TAG = " " + QString(QChar(9636));
QString tag_name(const QString &name)
{
  return name + COLOR_GROUP_TAG;
}

QString untag_name(const QString &name) {
  return name.left(name.length() - COLOR_GROUP_TAG.length());
}

}

FlyEmBodyInfoDialog::FlyEmBodyInfoDialog(EMode mode, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FlyEmBodyInfoDialog)
{
    m_mode = mode;

    ui->setupUi(this);

    // office phone number = random seed
    qsrand(2094656);
    m_quitting = false;
    m_cancelLoading = false;
    m_connectionsLoading = false;


    // top body list stuff

    // first table manages list of bodies
    m_bodyModel = new QStandardItemModel(
          0, BODY_TABLE_COLUMN_COUNT, ui->bodyTableView);
    setBodyHeaders(m_bodyModel);

    m_bodyProxy = new QSortFilterProxyModel(this);
    m_bodyProxy->setSourceModel(m_bodyModel);
    m_bodyProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_bodyProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    // -1 = filter on all columns!  ALL!  no column left behind!
    m_bodyProxy->setFilterKeyColumn(-1);
    ui->bodyTableView->setModel(m_bodyProxy);

    // store body names for later use, plus the bodies
    //  we know don't have names
    m_bodyNames = QMap<uint64_t, QString>();
    m_namelessBodies = QSet<uint64_t>();

    // max body menu; not used by all data load methods; it's
    //  populated when appropriate
    ui->maxBodiesMenu->addItem("n/a");
    m_currentMaxBodies = 0;

    // color filter stuff

    // second table manages list of color filters, which 
    //  as a whole constitute the color map
    m_filterModel = new QStandardItemModel(0, 2, ui->filterTableView);
    setFilterHeaders(m_filterModel);

    m_filterProxy = new QSortFilterProxyModel(this);
    m_filterProxy->setSourceModel(m_filterModel);
    ui->filterTableView->setModel(m_filterProxy);
    ui->filterTableView->setColumnWidth(FILTER_NAME_COLUMN, 450);

    // this proxy is used to build color schemes; it's not hooked
    //  to a table view; match the filter settings of the body filter!
    m_schemeBuilderProxy = new QSortFilterProxyModel(this);
    m_schemeBuilderProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    // -1 = filter on all columns!  ALL!  no column left behind!
    m_schemeBuilderProxy->setFilterKeyColumn(-1);
    m_schemeBuilderProxy->setSourceModel(m_bodyModel);


    // connections tables stuff; table of bodies first
    m_connectionsTableState = CT_NONE;
    // table holding bodies that are input to the chosen body
    m_ioBodyModel = new QStandardItemModel(0, 3, ui->ioBodyTableView);
    setIOBodyHeaders(m_ioBodyModel);
    m_ioBodyProxy = new QSortFilterProxyModel(this);
    m_ioBodyProxy->setSourceModel(m_ioBodyModel);
    ui->ioBodyTableView->setModel(m_ioBodyProxy);
    // set needed col width here

    // table of connection sites (ie, synapses)
    m_connectionsModel = new QStandardItemModel(0, 3, ui->connectionsTableView);
    setConnectionsHeaders(m_connectionsModel);
    m_connectionsProxy = new QSortFilterProxyModel(this);
    m_connectionsProxy->setSourceModel(m_connectionsModel);
    ui->connectionsTableView->setModel(m_connectionsProxy);
    // set col width here?

    // UI connects
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(onCloseButton()));
    connect(ui->refreshButton, SIGNAL(clicked()), this, SLOT(onRefreshButton()));
//    connect(ui->allNamedPushButton, SIGNAL(clicked()), this, SLOT(onAllNamedButton()));
    connect(ui->queryNamePushButton, SIGNAL(clicked()),
            this, SLOT(onQueryByNameButton()));
    connect(ui->queryTypePushButton, SIGNAL(clicked()),
            this, SLOT(onQueryByTypeButton()));
    connect(ui->queryRoiPushButton, SIGNAL(clicked()),
            this, SLOT(onQueryByRoiButton()));
    connect(ui->queryStatusPushButton, SIGNAL(clicked()),
            this, SLOT(onQueryByStatusButton()));
    connect(ui->findSimilarPushButton, SIGNAL(clicked()),
            this, SLOT(onFindSimilarButton()));
    connect(ui->customQueryPushButton, SIGNAL(clicked()),
            this, SLOT(onCustomQuery()));

    connect(ui->saveColorFilterButton, SIGNAL(clicked()), this, SLOT(onSaveColorFilter()));
    connect(ui->addColorMapPushButton, SIGNAL(clicked()),
            this, SLOT(onAddGroupColorMap()));
    connect(ui->importBodiesPushButton, SIGNAL(clicked()),
            this, SLOT(onImportBodies()));
    connect(ui->exportBodiesButton, SIGNAL(clicked(bool)), this, SLOT(onExportBodies()));
    connect(ui->exportConnectionsButton, SIGNAL(clicked(bool)), this, SLOT(onExportConnections()));
    connect(ui->copyConnectionPushButton, SIGNAL(clicked()),
            this, SLOT(onCopySelectedConnections()));
    connect(ui->saveButton, SIGNAL(clicked(bool)), this, SLOT(onSaveColorMap()));
    connect(ui->loadButton, SIGNAL(clicked(bool)), this, SLOT(onLoadColorMap()));
    connect(ui->moveUpButton, SIGNAL(clicked(bool)), this, SLOT(onMoveUp()));
    connect(ui->moveDownButton, SIGNAL(clicked(bool)), this, SLOT(onMoveDown()));
    connect(ui->bodyTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickBodyTable(QModelIndex)));
    connect(ui->gotoBodiesButton, SIGNAL(clicked(bool)), this, SLOT(onGotoBodies()));
    connect(ui->bodyFilterField, SIGNAL(textChanged(QString)), this, SLOT(bodyFilterUpdated(QString)));
    connect(ui->regexCheckBox, SIGNAL(clicked(bool)), this, SLOT(updateBodyFilterAfterLoading()));
    connect(ui->clearFilterButton, SIGNAL(clicked(bool)), ui->bodyFilterField, SLOT(clear()));
    connect(ui->toBodyListButton, SIGNAL(clicked(bool)), this, SLOT(moveToBodyList()));
    connect(ui->deleteButton, SIGNAL(clicked(bool)), this, SLOT(onDeleteButton()));
    connect(ui->filterTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickFilterTable(QModelIndex)));
    connect(ui->ioBodyTableView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(onDoubleClickIOBodyTable(QModelIndex)));
    connect(ui->connectionsTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickIOConnectionsTable(QModelIndex)));
    connect(ui->ioBodyTableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(onIOConnectionsSelectionChanged(QItemSelection,QItemSelection)));
    connect(ui->roiComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onRoiChanged(int)));
    connect(ui->namedCheckBox, SIGNAL(clicked(bool)),
            this, SLOT(onNamedOnlyToggled()));
    connect(QApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(applicationQuitting()));

    // data update connects
    // register our type so we can signal/slot it across threads:
    connect(this, SIGNAL(dataChanged(ZJsonValue)), this, SLOT(updateModel(ZJsonValue)));
    connect(this, SIGNAL(appendingData(ZJsonValue, int)),
            this, SLOT(appendModel(ZJsonValue, int)));
    //Review-TZ: consider merging these two in onLoadCompleted()
    connect(this, SIGNAL(loadCompleted()), this, SLOT(updateStatusAfterLoading()));
    connect(this, SIGNAL(loadCompleted()), this, SLOT(updateBodyFilterAfterLoading()));
//    connect(this, SIGNAL(loadCompleted()), this, SLOT(updateColorScheme()));
//    connect(this, SIGNAL(loadCompleted()), this, SLOT(updateFilterIdMap()));
    connect(this, SIGNAL(filterIdMapUpdated()), this, SLOT(updateColorScheme()));
    connect(this, SIGNAL(groupIdMapUpdated()), this, SLOT(updateColorScheme()));
    connect(this, SIGNAL(jsonLoadBookmarksError(QString)), this, SLOT(onjsonLoadBookmarksError(QString)));
    connect(this, SIGNAL(jsonLoadColorMapError(QString)), this, SLOT(onjsonLoadColorMapError(QString)));
    connect(this, SIGNAL(colorMapLoaded(ZJsonValue)), this, SLOT(onColorMapLoaded(ZJsonValue)));
    connect(this, SIGNAL(ioBodiesLoaded()), this, SLOT(onIOBodiesLoaded()));
    connect(this, SIGNAL(ioBodyLoadFailed()), this, SLOT(onIOBodyLoadFailed()));
    connect(this, SIGNAL(ioNoBodiesLoaded()), this, SLOT(onIONoBodiesLoaded()));
    prepareWidget();
}

void FlyEmBodyInfoDialog::prepareWidget()
{
  logInfo("Prepare widget: " + ToString(m_mode));

//  setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  if (m_mode == EMode::QUERY || m_mode == EMode::NEUPRINT) {
    setWindowTitle("Body Information (Selected)");
    clearStatusLabel();
    ui->roiComboBox->hide();

    ui->roiLabel->hide();
//    ui->avabilityLabel->hide();

//    ui->line->hide();
//    ui->line_3->hide();
    ui->horizontalSpacer->changeSize(0, 0);
    ui->iconLabel->setText("");

    if (m_mode == EMode::NEUPRINT) {
      setWindowTitle("Body Information (NeuPrint)");
      ui->refreshButton->hide();
    } else {
      ui->maxBodiesLabel->hide();
      ui->maxBodiesMenu->hide();
      ui->namedCheckBox->hide();
    }
  } else {
    setWindowTitle("Body Information (Sequencer)");
    QPixmap pixmap(":/images/document.png");
    pixmap = pixmap.scaled(16, 16);
    ui->iconLabel->setPixmap(pixmap);
//    ui->iconLabel->setMask(pixmap.mask());
  }

  if (m_mode != EMode::NEUPRINT) {
    neutu::HideLayout(ui->queryLayout, true);
    delete ui->queryLayout;
//    ui->queryNamePushButton->hide();
//    ui->queryStatusPushButton->hide();
//    ui->findSimilarPushButton->hide();
//    ui->queryRoiPushButton->hide();
  }
//  ui->allNamedPushButton->hide(); //obsolete
}

namespace {

std::string get_annotation_primary_neurite(const ZJsonObject &bodyData)
{
  ZJsonObjectParser parser;
  return
      parser.getValue(bodyData, ZFlyEmBodyAnnotation::KEY_PRIMARY_NEURITE, "");
}

std::string get_annotation_name(const ZJsonObject &bodyData)
{
  std::string name;
  if (bodyData.hasKey("name")) {
    name = ZJsonParser::stringValue(bodyData["name"]);
  } else if (bodyData.hasKey("instance")) {
    name = ZJsonParser::stringValue(bodyData["instance"]);
  }

  return name;
}

std::string get_annotation_type(const ZJsonObject &bodyData)
{
  std::string type;
  if (bodyData.hasKey("type")) {
    type = ZJsonParser::stringValue(bodyData["type"]);
  } else if (bodyData.hasKey("class")) {
    type = ZJsonParser::stringValue(bodyData["class"]);
  }

  return type;
}

}
void FlyEmBodyInfoDialog::setBodyList(const ZJsonArray &bodies)
{
  m_bodyNames.clear();
  m_namelessBodies.clear();

  emit dataChanged(bodies);
}

void FlyEmBodyInfoDialog::setBodyList(const std::set<uint64_t> &bodySet)
{
  logInfo(QString("Set a list %1 bodies").arg(bodySet.size()));

  ZJsonArray bodies;
  ZDvidReader &reader = m_sequencerReader;
  if (reader.isReady()) {
    m_bodyNames.clear();
    m_namelessBodies.clear();

    // I need to preserve order below
    QList<uint64_t> bodyList;
    for (uint64_t bodyId: bodySet) {
        bodyList << bodyId;
    }

    // get synapse counts all at once if you can
    QMap<uint64_t, int> preCountMap;
    QMap<uint64_t, int> postCountMap;


    // the "if m_hasLabelsz" code here used to also have "if m_mode == Emode::QUERY";
    //  however, I found it also needs to run in Emode::NEUPRINT as well (that mode was added later);
    //  as far as I can tell, though, there's no reason it can't or shouldn't run in Emode::SEQUENCER
    //  as well, so I removed the whole Emode test altogether
    if (m_hasLabelsz) {
        QList<int> preCounts = reader.readSynapseLabelszBodies(bodyList, dvid::ELabelIndexType::PRE_SYN);
        QList<int> postCounts = reader.readSynapseLabelszBodies(bodyList, dvid::ELabelIndexType::POST_SYN);
        for (int i=0; i<bodyList.size(); i++) {
            preCountMap[bodyList[i]] = preCounts[i];
            postCountMap[bodyList[i]] = postCounts[i];
        }
    }

    for (uint64_t bodyId : bodyList) {
      ZJsonObject bodyData = reader.readBodyAnnotationJson(bodyId);

      // remove name if empty
      if (bodyData.hasKey("name") &&
          ZJsonParser::stringValue(bodyData["name"]).empty()) {
        bodyData.removeKey("name");
      }

      // remove status if empty; change status > body status
      if (bodyData.hasKey("status")) {
        if (!ZJsonParser::stringValue(bodyData["status"]).empty()) {
          bodyData.setEntry("body status", bodyData["status"]);
        }
        // don't really need to remove this, but why not
        bodyData.removeKey("status");
      }

      int npre = 0;
      int npost = 0;
      // I think this mode check needs to remain; if mode isn't QUERY, we can drop empty
      //    bodies; but if mode = QUERY, it's a single user-selected body, and we should
      //    always find its synapses
      if ((m_mode == EMode::QUERY || !bodyData.isEmpty()) && m_hasLabelsz) {
          npre = preCountMap[bodyId];
          npost = postCountMap[bodyId];
      } else {
          // brute-force fallback if we don't have labelsz in DVID:
          std::vector<ZDvidSynapse> synapses = reader.readSynapse(
              bodyId, dvid::EAnnotationLoadMode::PARTNER_LOCATION);

          for (size_t i=0; i<synapses.size(); i++) {
              if (synapses[i].getKind() == ZDvidSynapse::EKind::KIND_PRE_SYN) {
                  npre++;
              } else {
                  npost++;
              }
          }
      }

      bodyData.setEntry("body ID", bodyId);
      bodyData.setEntry("body T-bars", npre);
      bodyData.setEntry("body PSDs", npost);
      bodies.append(bodyData);
    }
  }
  emit dataChanged(bodies);
}

int FlyEmBodyInfoDialog::getMaxBodies() const
{
  return m_currentMaxBodies;
}

void FlyEmBodyInfoDialog::simplify()
{
  ui->roiComboBox->hide();
  ui->exportBodiesButton->hide();
  ui->gotoBodiesButton->hide();
  ui->colorTab->hide();
  ui->saveColorFilterButton->hide();
  ui->closeButton->hide();
  ui->refreshButton->hide();
  ui->clearFilterButton->hide();
//  ui->connectionBodyLabel->hide();
  ui->roiLabel->hide();
  ui->tabWidget->removeTab(0);
  ui->connectionsTableLabel->hide();
  ui->connectionsTableView->hide();
  ui->regexCheckBox->setChecked(true);
  ui->regexCheckBox->hide();
  ui->bodyFilterLabel->setText("Body filter (regular expression supported):");
//  ui->regexCheckBox->setEnabled(false);
//  ui->regexCheckBox->setText("Regular expression supported");
}

void FlyEmBodyInfoDialog::setupMaxBodyMenu() {
    // should probably generate from a list at some point
    ui->maxBodiesMenu->clear();
    if (m_mode == EMode::NEUPRINT) {
      ui->maxBodiesMenu->addItem("---", QVariant(0));
    }

    ui->maxBodiesMenu->addItem("100", QVariant(100));
    ui->maxBodiesMenu->addItem("500", QVariant(500));
    ui->maxBodiesMenu->addItem("1000", QVariant(1000));
    ui->maxBodiesMenu->addItem("5000", QVariant(5000));
    ui->maxBodiesMenu->addItem("10000", QVariant(10000));
    ui->maxBodiesMenu->addItem("50000", QVariant(50000));
    ui->maxBodiesMenu->addItem("100000", QVariant(100000));

    if (m_mode == EMode::NEUPRINT) {
      ui->maxBodiesMenu->setCurrentIndex(0);
    } else {
      ui->maxBodiesMenu->setCurrentIndex(1);
    }

    m_currentMaxBodies = ui->maxBodiesMenu->itemData(
          ui->maxBodiesMenu->currentIndex()).toInt();

    // connect the signal now, *after* the entries added
    connect(ui->maxBodiesMenu, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onMaxBodiesChanged(int)));
}

void FlyEmBodyInfoDialog::onDoubleClickBodyTable(QModelIndex modelIndex)
{
    if (modelIndex.column() == BODY_ID_COLUMN) {
        activateBody(modelIndex);
    } else if (modelIndex.column() == BODY_NPRE_COLUMN || modelIndex.column() == BODY_NPOST_COLUMN) {
        gotoPrePost(modelIndex);
    }
}

void FlyEmBodyInfoDialog::logInfo(const QString &msg) const
{
  KLOG << ZLog::Info()
       << ZLog::Description(msg.toStdString())
       << ZLog::Window("FlyEmBodyInfoDialog");
}

QString FlyEmBodyInfoDialog::ToString(EMode mode)
{
  switch (mode) {
  case EMode::NEUPRINT:
    return "neuprint";
  case EMode::QUERY:
    return "connection";
  case EMode::SEQUENCER:
    return "sequencer";
  }

  return "";
}

ZDvidReader& FlyEmBodyInfoDialog::getIoBodyReader()
{
  if (!m_ioBodyReader.isReady()) {
    m_ioBodyReader.open(m_reader.getDvidTarget());
    m_ioBodyReader.setVerbose(false);
  }

  return m_ioBodyReader;
}

void FlyEmBodyInfoDialog::activateBody(QModelIndex modelIndex)
{
    QStandardItem *item = m_bodyModel->itemFromIndex(m_bodyProxy->mapToSource(modelIndex));
    uint64_t bodyId = item->data(Qt::DisplayRole).toULongLong();

    logInfo(QString("%1 ativated").arg(bodyId));

    // double-click = select and goto
    // shift-double-click = select and goto, but don't clear previous bodies from 3d views
    Qt::KeyboardModifiers modifiers  = QApplication::queryKeyboardModifiers();
    if (modifiers.testFlag(Qt::ShiftModifier) ||
        modifiers.testFlag(Qt::ControlModifier)) {
        emit addBodyActivated(bodyId);
    } else {
        // technically also catches alt, ctrl double-click
        emit bodyActivated(bodyId);
    }

}

QList<uint64_t> FlyEmBodyInfoDialog::getSelectedBodyList() const
{
  QList<uint64_t> bodyIDList;

  if (ui->bodyTableView->selectionModel()->hasSelection()) {
    QModelIndexList indices = ui->bodyTableView->selectionModel()->selectedIndexes();
    foreach(QModelIndex modelIndex, indices) {
      // if the item is in the first column (body ID), extract body ID, put in list
      if (modelIndex.column() == BODY_ID_COLUMN) {
        QStandardItem *item = m_bodyModel->itemFromIndex(m_bodyProxy->mapToSource(modelIndex));
        bodyIDList.append(item->data(Qt::DisplayRole).toULongLong());
      }
    }
  }

  return bodyIDList;
}

void FlyEmBodyInfoDialog::onGotoBodies()
{
  QList<uint64_t> bodyIDList = getSelectedBodyList();
  if (!bodyIDList.isEmpty()) {
    emit bodiesActivated(bodyIDList);
  }
}

void FlyEmBodyInfoDialog::dvidTargetChanged(ZDvidTarget target) {
    #ifdef _DEBUG_
        std::cout << "dvid target changed to " << target.getUuid() << std::endl;
    #endif

    // if target isn't null, trigger data load in a thread
    m_currentDvidTarget = target;
    m_defaultSynapseLabelsz = target.getSynapseLabelszName();
    if (target.isValid()) {
        // clear the model regardless at this point
        m_bodyModel->clear();
        if (m_mode == EMode::SEQUENCER) {
          setStatusLabel("Loading...");
        }

        // open the reader; reuse it so we don't multiply connections on
        //  the server
        m_reader.setVerbose(false);
        if (m_reader.open(m_currentDvidTarget)) {
          m_sequencerReader.open(m_reader.getDvidTarget());
          m_sequencerReader.setVerbose(false);
          m_hasLabelsz = labelszPresent();
          m_hasBodyAnnotation = bodyAnnotationsPresent();
          // need to clear this to ensure it's repopulated exactly once
          ui->maxBodiesMenu->clear();

          if (m_mode == EMode::SEQUENCER) {
            updateRoi();
          }
          if (m_mode == EMode::SEQUENCER || m_mode == EMode::NEUPRINT) {
            loadData();
          }
        } else {
          QMessageBox errorBox;
          errorBox.setText("Error connecting to DVID");
          errorBox.setInformativeText("Could not open DVID!  Check server information!");
          errorBox.setStandardButtons(QMessageBox::Ok);
          errorBox.setIcon(QMessageBox::Warning);
          errorBox.exec();
          setStatusLabel("Load failed!");
        }
    }
}

void FlyEmBodyInfoDialog::loadData()
{
    setStatusLabel("Loading...");

    if (m_mode == EMode::NEUPRINT) {
      if (getNeuPrintReader()) {
        if (ui->namedCheckBox->isChecked()) {
          NeuPrintReader *reader = getNeuPrintReader();
          if (reader) {
            ui->bodyFilterField->clear();
            setStatusLabel("Loading...");
            setBodyList(reader->queryAllNamedNeuron());
          }
        } else {
          if (ui->maxBodiesMenu->count() == 0) {
            setupMaxBodyMenu();
          }

          setBodyList(getNeuPrintReader()->queryTopNeuron(getMaxBodies()));
        }
      }
    } else {
      QString loadingThreadId = "importBodiesDvid";
      m_cancelLoading = true;
      m_futureMap.waitForFinished(loadingThreadId);
      //    QFuture<void> *future = m_futureMap.getFuture(loadingThreadId);
      //    if (future != NULL) {
      //      future->waitForFinished();
      //    }
      m_cancelLoading = false;

      // we can load this info from different sources, depending on
      //  what's available in DVID
      if (m_hasBodyAnnotation) {
        // both of these need body annotations:
        if (m_hasLabelsz) {
          // how about labelsz data?
          // only set up menu on first load:
          if (ui->maxBodiesMenu->count() == 0) {
            setupMaxBodyMenu();
          }
          m_futureMap[loadingThreadId] =
              QtConcurrent::run(this, &FlyEmBodyInfoDialog::importBodiesDvid2);
        } else {
          // this is the fallback method; it needs body annotations only
          m_futureMap[loadingThreadId] =
              QtConcurrent::run(this, &FlyEmBodyInfoDialog::importBodiesDvid);
        }
      } else {
        // ...but sometimes, we've got nothing
        emit loadCompleted();
      }
    }

}

void FlyEmBodyInfoDialog::updateRoi()
{
  ZJsonObject obj = m_reader.readDataMap();
  ZJsonObject labelszObj = obj.value("roi_synapse_labelsz");
  std::vector<std::string> keyList = labelszObj.getAllKey();
  disconnect(ui->roiComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(onRoiChanged(int)));
  updateRoi(keyList);
  connect(ui->roiComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(onRoiChanged(int)));
}

void FlyEmBodyInfoDialog::updateRoi(const std::vector<std::string> &roiList)
{
  ui->roiComboBox->clear();
  ui->roiComboBox->addItem("---");
  for (std::vector<std::string>::const_iterator iter = roiList.begin();
       iter != roiList.end(); ++iter) {
    const std::string &roi = *iter;
    ui->roiComboBox->addItem(roi.c_str());
  }
}

ZDvidRoi* FlyEmBodyInfoDialog::getRoi(const QString &name)
{
  m_reader.readRoi(name.toStdString(), &(m_roiStore[name]));

  if (m_roiStore.contains(name)){
    return &(m_roiStore[name]);
  }

  return NULL;
}

void FlyEmBodyInfoDialog::applicationQuitting() {
    m_quitting = true;
}

void FlyEmBodyInfoDialog::setBodyHeaders(QStandardItemModel * model) {
    model->setHorizontalHeaderItem(BODY_ID_COLUMN, new QStandardItem("Body ID"));
    model->setHorizontalHeaderItem(
          BODY_PRIMARY_NEURITE, new QStandardItem("primary neurite"));
    model->setHorizontalHeaderItem(BODY_TYPE_COLUMN, new QStandardItem("type"));
    model->setHorizontalHeaderItem(BODY_NAME_COLUMN, new QStandardItem("instance"));
    model->setHorizontalHeaderItem(BODY_NPRE_COLUMN, new QStandardItem("# pre"));
    model->setHorizontalHeaderItem(BODY_NPOST_COLUMN, new QStandardItem("# post"));
    model->setHorizontalHeaderItem(BODY_STATUS_COLUMN, new QStandardItem("status"));
}

void FlyEmBodyInfoDialog::setFilterHeaders(QStandardItemModel * model) {
    model->setHorizontalHeaderItem(
          FILTER_NAME_COLUMN, new QStandardItem(QString("Filter")));
    model->setHorizontalHeaderItem(
          FILTER_COUNT_COLUMN, new QStandardItem(QString("#Bodies")));
    model->setHorizontalHeaderItem(
          FILTER_COLOR_COLUMN, new QStandardItem(QString("Color")));
}

void FlyEmBodyInfoDialog::setIOBodyHeaders(QStandardItemModel * model) {
    model->setHorizontalHeaderItem(IOBODY_ID_COLUMN, new QStandardItem(QString("Body ID")));
    model->setHorizontalHeaderItem(IOBODY_NAME_COLUMN, new QStandardItem(QString("instance")));
    model->setHorizontalHeaderItem(IOBODY_NUMBER_COLUMN, new QStandardItem(QString("#")));
}

void FlyEmBodyInfoDialog::setConnectionsHeaders(QStandardItemModel * model) {
    model->setHorizontalHeaderItem(CONNECTIONS_X_COLUMN, new QStandardItem(QString("x")));
    model->setHorizontalHeaderItem(CONNECTIONS_Y_COLUMN, new QStandardItem(QString("y")));
    model->setHorizontalHeaderItem(CONNECTIONS_Z_COLUMN, new QStandardItem(QString("z")));
}

void FlyEmBodyInfoDialog::setStatusLabel(QString label) {
  if (m_mode == EMode::SEQUENCER) {
    label = "<font color=\"#008000\">" + label + "</font>";
//    QPixmap pixmap(":/images/document.png");
//    pixmap = pixmap.scaled(16, 16);
//    ui->bodiesLabel->setPixmap(pixmap);
  }

  ui->bodiesLabel->setText(label);
}

void FlyEmBodyInfoDialog::clearStatusLabel()
{
    setStatusLabel("Bodies");
}

void FlyEmBodyInfoDialog::updateStatusLabel() {
    qlonglong nPre = 0;
    qlonglong nPost = 0;
    for (qlonglong i=0; i<m_bodyProxy->rowCount(); i++) {
        nPre += m_bodyProxy->data(m_bodyProxy->index(i, BODY_NPRE_COLUMN)).toLongLong();
        nPost += m_bodyProxy->data(m_bodyProxy->index(i, BODY_NPOST_COLUMN)).toLongLong();
    }
    QString label = QString(
          "Bodies (%1/%2) shown; %3/%4 pre-syn, %5/%6 post-syn").
        arg(m_bodyProxy->rowCount()).arg(m_bodyModel->rowCount()).
        arg(nPre).arg(m_totalPre).arg(nPost).arg(m_totalPost);
    setStatusLabel(label);

    logInfo(label);

    // have I mentioned how much I despise C++ strings?

//    std::ostringstream outputStream;
//    outputStream << "Bodies (" << m_bodyProxy->rowCount() << "/" << m_bodyModel->rowCount() << " shown; ";
//    outputStream << nPre << "/" <<  m_totalPre << " pre-syn, ";
//    outputStream << nPost << "/" <<  m_totalPost << " post-syn)";
//    setStatusLabel(QString::fromStdString(outputStream.str()));
}

void FlyEmBodyInfoDialog::updateStatusAfterLoading() {
    if (m_bodyModel->rowCount() > 0) {
        updateStatusLabel();
    } else {
        clearStatusLabel();
    }
}

void FlyEmBodyInfoDialog::updateBodyFilterAfterLoading() {
    // if the user typed something into the filter box while
    //  the body data was loading, we need to kick it to update:
    if (ui->bodyFilterField->text().size() > 0) {
        bodyFilterUpdated(ui->bodyFilterField->text());
    }

    updateFilterIdMap();
}

void FlyEmBodyInfoDialog::bodyFilterUpdated(QString filterText) {
    if (ui->regexCheckBox->isChecked()) {
        m_bodyProxy->setFilterRegExp(filterText);
    } else {
        m_bodyProxy->setFilterFixedString(filterText);
    }

    // turns out you need to explicitly tell it to resort after the filter
    //  changes; if you don't, and new filter shows more items, those items
    //  will appear somewhere lower than the existing items
    m_bodyProxy->sort(m_bodyProxy->sortColumn(), m_bodyProxy->sortOrder());
    updateStatusLabel();
}

bool FlyEmBodyInfoDialog::bodyAnnotationsPresent() {
    // check for data name and key
    if (!m_reader.hasData(m_currentDvidTarget.getBodyAnnotationName())) {
        #ifdef _DEBUG_
            std::cout << "UUID doesn't have body annotations" << std::endl;
        #endif
        return false;
    }
    return true;
}

bool FlyEmBodyInfoDialog::labelszPresent() {
    if (!m_reader.hasData(m_currentDvidTarget.getSynapseLabelszName())) {
        #ifdef _DEBUG_
            std::cout << "UUID doesn't have labelsz instance" << std::endl;
        #endif
        return false;
    }
    return true;
}

void FlyEmBodyInfoDialog::onMaxBodiesChanged(int index)
{
  int maxBodies = ui->maxBodiesMenu->itemData(index).toInt();

  logInfo(QString("Update bodies triggered by max body number change: %1").arg(maxBodies));

  if (maxBodies > 1000) {
    int ans = QMessageBox::Ok;
    if (m_mode == EMode::SEQUENCER) {
      QMessageBox mb;
      mb.setText("That's a lot of bodies!");
      mb.setInformativeText("Warning!  Loading more than 1000 bodies may take 5-10 minutes or potentially longer.\n\nContinue loading?");
      mb.setIcon(QMessageBox::Warning);
      mb.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
      mb.setDefaultButton(QMessageBox::Cancel);
      ans = mb.exec();
    }

    if (ans != QMessageBox::Ok) {
      ui->maxBodiesMenu->setCurrentIndex(ui->maxBodiesMenu->findData(m_currentMaxBodies));
      return;
    }
  }
  m_currentMaxBodies = maxBodies;
  onRefreshButton();
}

void FlyEmBodyInfoDialog::onRoiChanged(int index)
{
  std::string labelszName = m_defaultSynapseLabelsz;
  if (index > 0) {
    ZJsonObject obj = m_reader.readDataMap();
    ZJsonObject labelszObj(obj.value("roi_synapse_labelsz"));
    std::string roiName = ui->roiComboBox->itemText(index).toStdString();
    if (labelszObj.hasKey(roiName.c_str())) {
      labelszName = ZJsonParser::stringValue(labelszObj[roiName.c_str()]);
    }
  }
  m_reader.getDvidTarget().setSynapseLabelszName(labelszName);
  m_currentDvidTarget.setSynapseLabelszName(labelszName);
  m_sequencerReader.getDvidTarget().setSynapseLabelszName(labelszName);

  onRefreshButton();
}

void FlyEmBodyInfoDialog::onNamedOnlyToggled()
{
  bool namedOnly = ui->namedCheckBox->isChecked();
  ui->maxBodiesMenu->setDisabled(namedOnly);
  ui->roiComboBox->setDisabled(namedOnly);

//  ui->queryNamePushButton->setDisabled(namedOnly);
//  ui->queryStatusPushButton->setDisabled(namedOnly);
//  ui->findSimilarPushButton->setDisabled(namedOnly);
//  ui->queryRoiPushButton->setDisabled(namedOnly);

  onRefreshButton();
}

bool FlyEmBodyInfoDialog::isValidBookmarkFile(ZJsonObject jsonObject) {
    // validation is admittedly limited for now; ultimately, I
    //  expect we'll be getting the info from DVID rather than
    //  a file anyway, so I'm not going to spend much time on it
    
    // in a perfect world, all these json constants would be collected
    //  somewhere central

    if (!jsonObject.hasKey("data") || !jsonObject.hasKey("metadata")) {
        emit jsonLoadBookmarksError("This file is missing 'data' or 'metadata'. Are you sure this is a Fly EM JSON file?");
        return false;
    }

    ZJsonObject metadata = (ZJsonObject) jsonObject.value("metadata");
    if (!metadata.hasKey("description")) {
        emit jsonLoadBookmarksError("This file is missing 'metadata/description'. Are you sure this is a Fly EM JSON file?");
        return false;
    }

    ZString description = ZJsonParser::stringValue(metadata["description"]);
    if (description != "bookmarks") {
        emit jsonLoadBookmarksError("This json file does not have description 'bookmarks'!");
        return false;
    }

    ZJsonValue data = jsonObject.value("data");
    if (!data.isArray()) {
        emit jsonLoadBookmarksError("The data section in this json file is not an array!");
        return false;
    }

    // we could/should test all the elements of the array to see if they
    //  are bookmarks, but enough is enough...

    return true;
}

/*
 * this method reads body information from DVID, only from
 * body annotations; it's the fallback routine that requires
 * the least in DVID but also provides the least information
 *
 * note: max bodies number is ignored for this routine
 */
void FlyEmBodyInfoDialog::importBodiesDvid()
{
    bool namedOnly = ui->namedCheckBox->isChecked();

    QElapsedTimer fullTimer;
    QElapsedTimer dvidTimer;
    int64_t dvidTime = 0;
    int64_t fullTime = 0;
    fullTimer.start();

    // note: this method is run in a different thread than the rest
    //  of the GUI, so we must open our own DVID reader
    ZDvidReader &reader = m_sequencerReader;
    if (m_sequencerReader.isReady()) {
        m_bodyNames.clear();
        m_namelessBodies.clear();

        // the specific form of json we're passing around is a
        //  historical accident; it could be refactored someday

        // get all the bodies that have annotations in this UUID;
        // note that this list contains body IDs in strings, *plus*
        //  some other nonnumeric strings (!!), so we remove those
        dvidTimer.start();
        QString bodyAnnotationName = reader.getDvidTarget().getBodyAnnotationName().c_str();
        dvidTime += dvidTimer.elapsed();
        QStringList keyList = reader.readKeys(bodyAnnotationName);
        QMutableListIterator<QString> iter(keyList);
        bool ok;
        while (iter.hasNext()) {
            QString bodyIDstr = iter.next();
            // we don't care about the result of this conversion, only that
            //  it *can* be parsed:
            bodyIDstr.toLongLong(&ok);
            if (!ok) {
                iter.remove();
            }
        }

        //Skip for debugging
        #ifdef _DEBUG_2
        keyList.clear();
        #endif

        // read all the body annotations at once
        dvidTimer.restart();
        QList<ZJsonObject> bodyAnnotationList = reader.readJsonObjectsFromKeys(
              bodyAnnotationName, keyList);
        dvidTime += dvidTimer.elapsed();

        #ifdef _DEBUG_
            std::cout << "populating body info dialog:" << std::endl;
            std::cout << "    reading body annotations from " << bodyAnnotationName.toStdString() << std::endl;
            std::cout << "    # body annotation keys = " << keyList.size() << std::endl;
            std::cout << "    # body anntations = " << bodyAnnotationList.size() << std::endl;
        #endif


        // if we're doing only named bodies, filter out unnamed bodies; filter from
        //  both keyList and bodyAnnotationList
        if (namedOnly) {
            // this is an awkward double iteration...
            QMutableListIterator<QString> keyIter(keyList);
            QMutableListIterator<ZJsonObject> annIter(bodyAnnotationList);
            while (keyIter.hasNext()) {
                keyIter.next();
//                QString bodyIDstr = keyIter.next();
                keyIter.next();
                ZJsonObject bodyData = annIter.next();
                std::string name = get_annotation_name(bodyData);
//                bool hasName = false;
//                if (bodyData.hasKey("name")) {
//                    std::string name = ZJsonParser::stringValue(bodyData["name"]);
//                    if (!name.empty()) {
//                        hasName = true;
//                    }
//                }
                if (name.empty()) {
                    // remove the key and data
                    keyIter.remove();
                    annIter.remove();
                }
            }
        }

        QList<int> preCounts;
        QList<int> postCounts;
        if (m_hasLabelsz) {
            // get all the synapse counts at once
            QList<uint64_t> bodyIDs;
            foreach (QString bodyIDstr, keyList) {
                bodyIDs.append(bodyIDstr.toULongLong());
            }
            dvidTimer.restart();
            preCounts = reader.readSynapseLabelszBodies(bodyIDs, dvid::ELabelIndexType::PRE_SYN);
            postCounts = reader.readSynapseLabelszBodies(bodyIDs, dvid::ELabelIndexType::POST_SYN);
            dvidTime += dvidTimer.elapsed();
        }

        // main loop: adjust all the body data
        ZJsonArray bodies;
        qlonglong bodyID;
        for (int i=0; i<keyList.size(); i++) {
            if (m_quitting || m_cancelLoading) {
                #ifdef _DEBUG_
                    std::cout << "Sequencer loading canceled." << std::endl;
                #endif
                return;
            }

            // remember, we've already removed the values that won't convert,
            //  so we don't *need* to check the return...but I'll leave it in anyway
            bodyID = keyList[i].toLongLong(&ok);
            if (ok) {
                // grab the previously retrieved data and modify it:
                ZJsonObject bodyData = bodyAnnotationList[i];

                std::string name = get_annotation_name(bodyData);
                if (!name.empty()) {
                  m_bodyNames[bodyID] = QString(name.c_str());
                }
                // remove name if empty; store it otherwise
//                if (bodyData.hasKey("name")) {
//                    std::string name = ZJsonParser::stringValue(bodyData["name"]);
//                    if (name.empty()) {
//                        bodyData.removeKey("name");
//                    } else {
//                        // keyList contains body ID strings
//                        m_bodyNames[bodyID] = QString(name.c_str());
//                    }
//                }

                // remove status if empty; change status => body status
                if (bodyData.hasKey("status")) {
                    if (!ZJsonParser::stringValue(bodyData["status"]).empty()) {
                        bodyData.setEntry("body status", bodyData["status"]);
                    } 
                    // don't really need to remove this, but why not
                    bodyData.removeKey("status");
                }

                if (!bodyData.isEmpty() && m_hasLabelsz) {
                    bodyData.setEntry("body T-bars", preCounts[i]);
                    bodyData.setEntry("body PSDs", postCounts[i]);
                }
                bodies.append(bodyData);
            } 
        }

        fullTime = fullTimer.elapsed();
        #ifdef _DEBUG_
            std::cout << "sequencer load: DVID time/total time (ms) = " << dvidTime << "/" << fullTime << std::endl;
        #endif

        // no "loadCompleted()" here; it's emitted in updateModel(), when it's done
        emit dataChanged(bodies);
    } else {
        // but we need to clear the loading message if we can't read from DVID
        emit loadCompleted();
    }
}

/*
 * this method loads body information directly from DVID,
 * from body annotations and from the labelsz data type
 */
void FlyEmBodyInfoDialog::importBodiesDvid2()
{
    if (ui->namedCheckBox->isChecked()) {
      importBodiesDvid();
      return;
    }

    QElapsedTimer fullTimer;
    QElapsedTimer dvidTimer;
    int64_t dvidTime = 0;
    int64_t fullTime = 0;
    fullTimer.start();

    // note: this method is run in a different thread than the rest
    //  of the GUI, so we must open our own DVID reader
    ZDvidReader &reader = m_sequencerReader;
    if (reader.isReady()) {

        // user specifies how many bodies to get synapses for
        dvidTimer.start();
        ZJsonArray thresholdData = reader.readSynapseLabelsz(
              m_currentMaxBodies, dvid::ELabelIndexType::ALL_SYN);
        dvidTime += dvidTimer.elapsed();
        #ifdef _DEBUG_
            std::cout << "read top " << m_currentMaxBodies << " synapses from DVID in " << dvidTime << " ms" << std::endl;
        #endif

        // first, get the list of bodies that actually have annotations,
        //  so we don't try to retrieve annotations that aren't there
        dvidTimer.restart();
        QString bodyAnnotationName = QString::fromStdString(
              m_currentDvidTarget.getBodyAnnotationName());
        QSet<QString> bodyAnnotationKeys = reader.readKeys(bodyAnnotationName).toSet();
        dvidTime += dvidTimer.elapsed();

        #ifdef _DEBUG_
            std::cout << "populating body info dialog:" << std::endl;
            std::cout << "    reading body annotations from " << bodyAnnotationName.toStdString() << std::endl;
            std::cout << "    # body annotation keys = " << bodyAnnotationKeys.size() << std::endl;
            std::cout << "    # bodies read with synapses = " << thresholdData.size() << std::endl;
        #endif

        // now find which of the bodies in thresholdData have annotations;
        //  also grab the body IDs for the bulk labelsz call
        QStringList keyList;
        QList<uint64_t> bodyIDs;
        for (size_t i=0; i<thresholdData.size(); i++) {
            ZJsonObject thresholdEntry(thresholdData.value(i));
            uint64_t bodyID =
                uint64_t(ZJsonParser::integerValue(thresholdEntry["Label"]));
            bodyIDs.append(bodyID);
            QString bodyIDstring = QString::number(bodyID);
            if (bodyAnnotationKeys.contains(bodyIDstring)) {
                keyList.append(bodyIDstring);
            }
        }

        // read the body annotations and store them
        dvidTimer.restart();
        QList<ZJsonObject> bodyAnnotationList = reader.readJsonObjectsFromKeys(bodyAnnotationName, keyList);
        dvidTime += dvidTimer.elapsed();
        QMap<QString, ZJsonObject> bodyAnnotations;
        for (int i=0; i<bodyAnnotationList.size(); i++) {
            bodyAnnotations[keyList[i]] = bodyAnnotationList[i];
        }


        // get all the synapse counts at once
        int64_t storedSynapseTime = 0;
        storedSynapseTime = dvidTime;
        dvidTimer.restart();
        QList<int> preCounts = reader.readSynapseLabelszBodies(
              bodyIDs, dvid::ELabelIndexType::PRE_SYN);
        QList<int> postCounts = reader.readSynapseLabelszBodies(
              bodyIDs, dvid::ELabelIndexType::POST_SYN);
        dvidTime += dvidTimer.elapsed();
        storedSynapseTime = dvidTime - storedSynapseTime;


        // build the data structure we pass along to the table
        ZJsonArray bodies;
        ZJsonArray namedBodies;
        m_bodyNames.clear();
        m_namelessBodies.clear();

        // note: batching *may* not be needed any more, now that we've moved
        //  all the DVID reads into bulk form; however, not tested or timed,
        //  so it stays in for now
        int capacity = 20;
        int batchState = 0;
        for (size_t i=0; i<thresholdData.size(); i++) {
            --capacity;
            // if application is quitting, return = exit thread
            if (m_quitting || m_cancelLoading) {
#ifdef _DEBUG_
              std::cout << "Sequencer loading canceled." << std::endl;
#endif
                return;
            }

            ZJsonObject thresholdEntry(thresholdData.value(i));
            int64_t bodyID = ZJsonParser::integerValue(thresholdEntry["Label"]);
            QString bodyIDstring = QString::number(bodyID);

            ZJsonObject entry;
            entry.setEntry("body ID", bodyID);

            // body annotation info
            if (bodyAnnotationKeys.contains(bodyIDstring)) {
                // we've fetched this values in bulk earlier
                ZJsonObject bodyData ;
                if (bodyAnnotations.contains(bodyIDstring)) {
                    bodyData = bodyAnnotations[bodyIDstring];
                }

                std::string name = get_annotation_name(bodyData);
//                if (bodyData.hasKey("name")) {
//                  name = ZJsonParser::stringValue(bodyData["name"]);
//                } else if (bodyData.hasKey("instance")) {
//                  name = ZJsonParser::stringValue(bodyData["instance"]);
//                }
                if (!name.empty()) {
                  // store name for later use
                  m_bodyNames[bodyID] = QString(name.c_str());
                  entry.setEntry("name", name);

//                  namedBodies.append(entry);
                } else {
                  m_namelessBodies.insert(bodyID);
                }

                std::string type = get_annotation_type(bodyData);
                if (!type.empty()) {
                  entry.setEntry("class", type);
                }

                if (bodyData.hasKey("status")) {
                  if (!ZJsonParser::stringValue(bodyData["status"]).empty()) {
                    entry.setEntry("body status", bodyData["status"]);
                  }
                }

                entry.setNonEmptyEntry(
                      ZFlyEmBodyAnnotation::KEY_PRIMARY_NEURITE,
                      get_annotation_primary_neurite(bodyData));
            }

            // synapse info
            entry.setEntry("body T-bars", preCounts[i]);
            entry.setEntry("body PSDs", postCounts[i]);

            bodies.append(entry);

            if (capacity < 0) {
              emit appendingData(bodies.clone(), batchState);
              batchState++;
              bodies.clear();
              capacity = 20;
            }
        }

        emit appendingData(bodies, -1);

        fullTime = fullTimer.elapsed();
        // I left the timers active; I think we'll want them later, plus
        //  they should be very low overhead
        #ifdef _DEBUG_
            std::cout << "sequencer load: total time (ms) = " << fullTime << std::endl;
            std::cout << "sequencer load: DVID time (ms)  = " << dvidTime << std::endl;
            std::cout << "synapse count time: " << storedSynapseTime << std::endl;
            std::cout << "summary: " << storedSynapseTime << "/" << dvidTime << "/" << fullTime << std::endl;
        #endif

         emit namedBodyChanged(namedBodies);

    } else {
        // but we need to clear the loading message if we can't read from DVID
        emit loadCompleted();
    }
}

void FlyEmBodyInfoDialog::onRefreshButton() {
  if (m_mode == EMode::SEQUENCER || m_mode == EMode::NEUPRINT) {
    ui->bodyFilterField->clear();
    loadData();
  } else {
    emit refreshing();
  }
}

void FlyEmBodyInfoDialog::prepareQuery()
{
  ui->bodyFilterField->clear();
  ui->maxBodiesMenu->setCurrentIndex(0);
  ui->namedCheckBox->setChecked(false);
  ui->maxBodiesMenu->setEnabled(true);
  setStatusLabel("Loading...");
}

void FlyEmBodyInfoDialog::onAllNamedButton()
{
  NeuPrintReader *reader = getNeuPrintReader();
  if (reader) {
    prepareQuery();

    setBodyList(reader->queryAllNamedNeuron());
  }
}


void FlyEmBodyInfoDialog::onQueryByRoiButton()
{
  NeuPrintReader *reader = getNeuPrintReader();
  if (reader) {
    if (getNeuPrintRoiQueryDlg()->exec()) {
      prepareQuery();

      QList<uint64_t> bodyList = reader->queryNeuron(
            getNeuPrintRoiQueryDlg()->getInputRoi(),
            getNeuPrintRoiQueryDlg()->getOutputRoi());

      std::set<uint64_t> bodyIdArray;
      bodyIdArray.insert(bodyList.begin(), bodyList.end());

      setBodyList(bodyIdArray);
    }
  }
}

void FlyEmBodyInfoDialog::onQueryByNameButton()
{
  NeuPrintReader *reader = getNeuPrintReader();
  if (reader) {
    bool ok;

    QString text = QInputDialog::getText(this, tr("Find Neurons"),
                                         tr("Body Name:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok) {
      if (!text.isEmpty()) {
        prepareQuery();

        setBodyList(reader->queryNeuronByName(text));
      }
    }
  }
}

void FlyEmBodyInfoDialog::onQueryByTypeButton()
{
  NeuPrintReader *reader = getNeuPrintReader();
  if (reader) {
    bool ok;

    QString text = QInputDialog::getText(this, tr("Find Neurons"),
                                         tr("Type:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok) {
      if (!text.isEmpty()) {
        prepareQuery();

        setBodyList(reader->queryNeuronByType(text));
      }
    }
  }
}

void FlyEmBodyInfoDialog::onQueryByStatusButton()
{
  NeuPrintReader *reader = getNeuPrintReader();
  if (reader) {
    bool ok;

    QString text = QInputDialog::getText(this, tr("Find Bodies"),
                                         tr("Body Status:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok) {
      if (!text.isEmpty()) {
        prepareQuery();

        QStringList statusList = text.split(";", QString::SkipEmptyParts);
        std::transform(statusList.begin(), statusList.end(), statusList.begin(),
                       [](const QString &str) { return str.trimmed(); });
        setBodyList(reader->queryNeuronByStatus(statusList));
      }
    }
  }
}

namespace { //To be removed when the custom query dialog is ready
QString last_query = "MATCH (n:`hemibrain-Neuron` \n";
}

void FlyEmBodyInfoDialog::onCustomQuery()
{
  NeuPrintReader *reader = getNeuPrintReader();
  if (reader) {
    bool ok;

    QString text = QInputDialog::getMultiLineText(
          this, tr("Find Bodies"),
          tr("Cypher Query:\n(n:`hemibrain-Neuron` must exist in MATCH)"),
          last_query, &ok);
    if (ok) {
      last_query = text;
      if (!text.isEmpty()) {
        prepareQuery();

        setBodyList(reader->queryNeuronCustom(text));
      }
    }
  }
}

void FlyEmBodyInfoDialog::onFindSimilarButton()
{
  NeuPrintReader *reader = getNeuPrintReader();
  if (reader) {
    bool ok;

    QString defaultValue;
    QList<uint64_t> bodyIDList = getSelectedBodyList();
    if (!bodyIDList.isEmpty()) {
      defaultValue = QString::number(bodyIDList.front());
    }
    QString text = QInputDialog::getText(this, tr("Find Similar Neurons"),
                                         tr("Body:"), QLineEdit::Normal,
                                         defaultValue, &ok);
    if (ok) {
      if (!text.isEmpty()) {
        ZString str = text.toStdString();
        std::vector<uint64_t> bodyArray = str.toUint64Array();
        if (bodyArray.size() == 1) {
          prepareQuery();

          setBodyList(reader->findSimilarNeuron(bodyArray[0]));
        }
      }
    }
  }
}

void FlyEmBodyInfoDialog::onCloseButton() {
    close();
}

void FlyEmBodyInfoDialog::appendModel(ZJsonValue data, int state)
{
#ifdef _DEBUG_
  std::cout << "Batch: " << state << std::endl;
#endif

  if (state == 0) {
    m_bodyModel->clear();
    setBodyHeaders(m_bodyModel);
    m_totalPre = 0;
    m_totalPost = 0;
  }

  if (!data.isEmpty()) {
    ZJsonArray bookmarks(data);
#if 0
    std::cout << "Count: " << bookmarks.size() << std::endl;
#endif
    for (size_t i = 0; i < bookmarks.size(); ++i) {
      ZJsonObject bkmk(bookmarks.value(i));
      QList<QStandardItem*> itemList = getBodyItemList(bkmk);
      m_bodyModel->appendRow(itemList);
    }

    if (state == 0) {
      ui->bodyTableView->resizeColumnsToContents();
      ui->bodyTableView->setColumnWidth(BODY_NAME_COLUMN, 150);
    }
    // currently initially sorting on # pre-synaptic sites
//    ui->bodyTableView->sortByColumn(BODY_NPRE_COLUMN, Qt::DescendingOrder);
  }

  if (state == -1) {
    ui->bodyTableView->resizeColumnsToContents();
    ui->bodyTableView->setColumnWidth(BODY_NAME_COLUMN, 150);
    ui->bodyTableView->sortByColumn(BODY_NPRE_COLUMN, Qt::DescendingOrder);
    emit loadCompleted();
  }
}

QList<QStandardItem*> FlyEmBodyInfoDialog::getBodyItemList(
    const ZJsonObject &bkmk)
{
  QVector<QStandardItem*> itemArray;
  itemArray.resize(BODY_TABLE_COLUMN_COUNT);

  // carefully set data for column items so they will sort
  //  properly (eg, IDs numerically, not lexically)
  qulonglong bodyID = ZJsonParser::integerValue(bkmk["body ID"]);
  QStandardItem * bodyIDItem = new QStandardItem();
  bodyIDItem->setData(QVariant(bodyID), Qt::DisplayRole);
  bodyIDItem->setToolTip(
        "Double click to locate the body;\n"
        CTRL_KEY_NAME
        "+double click to append the body to the current selection");
  itemArray[BODY_ID_COLUMN] = bodyIDItem;

  std::string primaryNeurite = get_annotation_primary_neurite(bkmk);
  if (!primaryNeurite.empty()) {
    itemArray[BODY_PRIMARY_NEURITE] =
        new QStandardItem(QString::fromStdString(primaryNeurite));
  }

  std::string type = get_annotation_type(bkmk);
  if (!type.empty()) {
    itemArray[BODY_TYPE_COLUMN] = new QStandardItem(QString::fromStdString(type));
  }

  std::string name = get_annotation_name(bkmk);
//  if (bkmk.hasKey("name")) {
//    name = ZJsonParser::stringValue(bkmk["name"]);
//  } else if (bkmk.hasKey("instance")) {
//    name = ZJsonParser::stringValue(bkmk["instance"]);
//  }

  if (!name.empty()) {
    itemArray[BODY_NAME_COLUMN] = new QStandardItem(QString::fromStdString(name));
  }

  if (bkmk.hasKey("body T-bars")) {
    int nPre = ZJsonParser::integerValue(bkmk["body T-bars"]);
    m_totalPre += nPre;
    QStandardItem * preSynapseItem = new QStandardItem();
    preSynapseItem->setData(QVariant(nPre), Qt::DisplayRole);
    itemArray[BODY_NPRE_COLUMN] = preSynapseItem;
//      m_bodyModel->setItem(i, BODY_NPRE_COLUMN, preSynapseItem);
  }

  if (bkmk.hasKey("body PSDs")) {
    int nPost = ZJsonParser::integerValue(bkmk["body PSDs"]);
    m_totalPost += nPost;
    QStandardItem * postSynapseItem = new QStandardItem();
    postSynapseItem->setData(QVariant(nPost), Qt::DisplayRole);
    itemArray[BODY_NPOST_COLUMN] = postSynapseItem;
//      m_bodyModel->setItem(i, BODY_NPOST_COLUMN, postSynapseItem);
  }

  // note that this routine expects "body status", not "status";
  //  historical side-effect of the original file format we read from
  if (bkmk.hasKey("body status")) {
    std::string status = ZJsonParser::stringValue(bkmk["body status"]);
    itemArray[BODY_STATUS_COLUMN] = new QStandardItem(QString::fromStdString(status));
    //      m_bodyModel->setItem(i, BODY_STATUS_COLUMN, new QStandardItem(QString(status)));
  }

  return itemArray.toList();
}

/*
 * update the data model from json; input should be the
 * "data" part of our standard bookmarks json file
 *
 * input should be json array of dictionaries looking
 * like this (all values basically optional except 
 * body ID, extras ignored):
 * 
 *    {
 *      "name": "neuron name",
 *      "body status": "Not examined",
 *      "body PSDs": 0,
 *      "body T-bars": 1,
 *      "body ID": 15379594
 *    }
 */
void FlyEmBodyInfoDialog::updateModel(ZJsonValue data) {
    m_bodyModel->clear();
    setBodyHeaders(m_bodyModel);

    m_totalPre = 0;
    m_totalPost = 0;

    ZJsonArray bookmarks(data);

    logInfo(QString("Update model: %1 bodies").arg(bookmarks.size()));

    m_bodyModel->setRowCount(int(bookmarks.size()));
    m_bodyModel->blockSignals(true);
    for (size_t i = 0; i < bookmarks.size(); ++i) {
        ZJsonObject bkmk(bookmarks.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);

        QList<QStandardItem*> itemList = getBodyItemList(bkmk);
        for (int j = 0; j < itemList.size(); ++j) {
          m_bodyModel->setItem(i, j, itemList[j]);
          /* //Doesn't seem necessary here (TZ)
          if (i == bookmarks.size() - 1 && j == itemList.size() - 1) { //A trick to avoid frequent table update
            m_bodyModel->blockSignals(false);
          }
          */
        }
    }
    m_bodyModel->blockSignals(false);

    //Review-TZ: Consider moving this to updateBodyTableView() to reduce
    //           code redundancy.
    // the resize isn't reliable, so set the name column wider by hand
    ui->bodyTableView->resizeColumnsToContents();
    ui->bodyTableView->setColumnWidth(BODY_NAME_COLUMN, 150);

    // currently initially sorting on # pre-synaptic sites
    ui->bodyTableView->sortByColumn(BODY_NPRE_COLUMN, Qt::DescendingOrder);

    //Review-TZ: loadCompleted() signal seems unnecessary unless there's a
    //           possiblity of calling updateModel in a separate thread.
    //           Consider calling sth like onLoadCompleted() directly.
    emit loadCompleted();
}

void FlyEmBodyInfoDialog::onjsonLoadBookmarksError(QString message) {
    QMessageBox errorBox;
    errorBox.setText("Error loading body information");
    errorBox.setInformativeText(message);
    errorBox.setStandardButtons(QMessageBox::Ok);
    errorBox.setIcon(QMessageBox::Warning);
    errorBox.exec();
}

void FlyEmBodyInfoDialog::onjsonLoadColorMapError(QString message) {
    QMessageBox errorBox;
    errorBox.setText("Error loading color map");
    errorBox.setInformativeText(message);
    errorBox.setStandardButtons(QMessageBox::Ok);
    errorBox.setIcon(QMessageBox::Warning);
    errorBox.exec();
}

void FlyEmBodyInfoDialog::onSaveColorFilter() {
    if (ui->bodyFilterField->text().size() > 0) {
        // no duplicates
        if (m_filterModel->findItems(
              ui->bodyFilterField->text(), Qt::MatchExactly,
              FILTER_NAME_COLUMN).size() == 0) {

            updateColorFilter(ui->bodyFilterField->text());
        }
    }
}

bool FlyEmBodyInfoDialog::hasColorName(const QString &name) const
{
  return (m_filterModel->findItems(
            name, Qt::MatchExactly, FILTER_NAME_COLUMN).size() > 0);
}

void FlyEmBodyInfoDialog::onAddGroupColorMap()
{
  QString name = QInputDialog::getText(this, "Color Map", "Name");
  if (!name.isEmpty()) {
    name = tag_name(name);
    if (hasColorName(name)) {
      ZDialogFactory::Warn(
            "Name Conflict",
            QString("Cannot add the color map %1 because it already exists").arg(name),
            this);
    } else {
      addGroupColor(name);
    }
  }
}

void FlyEmBodyInfoDialog::onImportBodies()
{
  importBodies(ZDialogFactory::GetOpenFileName("Import Bodies", "", this));
}

void FlyEmBodyInfoDialog::onExportBodies() {
    if (m_bodyProxy->rowCount() > 0) {
        QString filename = ZDialogFactory::GetSaveFileName(
              "Export Bodies", "", this);
        if (!filename.isEmpty()) {
            exportBodies(filename);
        }
    }
}

void FlyEmBodyInfoDialog::onExportConnections() {
    if (m_ioBodyProxy->rowCount() > 0) {
        QString filename = QFileDialog::getSaveFileName(this, "Export connections");
        if (!filename.isNull()) {
            exportConnections(filename);
        }
    }
}

void FlyEmBodyInfoDialog::onMoveUp() {
    // get selected row; NOTE: table can't be sorted, so we don't have to map to model indices
    int selectedRow = -1;
    QModelIndexList selectedIndices = ui->filterTableView->selectionModel()->selectedIndexes();
    if (selectedIndices.size() == 0) {
        return;
    } else if (selectedIndices.size() == 1) {
        selectedRow = selectedIndices[0].row();
    } else {
        // multiple selection should be impossible
        return;
    }

    // if not top, remove it and insert one higher; move selection, too!
    if (selectedRow > 0) {
        m_filterModel->insertRow(selectedRow - 1, m_filterModel->takeRow(selectedRow));
        QModelIndex newSelection = m_filterModel->index(selectedRow - 1, 0);
        ui->filterTableView->selectionModel()->clearSelection();
        ui->filterTableView->selectionModel()->select(newSelection, QItemSelectionModel::Select);
    }

    // update scheme (table should update itself)
    updateColorScheme();
//    updateColorSchemeWithFilterCache();
}

void FlyEmBodyInfoDialog::onMoveDown() {
    // get selected row; NOTE: table can't be sorted, so we don't have to map to model indices
    int selectedRow = -1;
    QModelIndexList selectedIndices = ui->filterTableView->selectionModel()->selectedIndexes();
    if (selectedIndices.size() == 0) {
        return;
    } else if (selectedIndices.size() == 1) {
        selectedRow = selectedIndices[0].row();
    } else {
        // multiple selection should be impossible
        return;
    }

    // if not bottom, remove it and insert one lower; move selection, too!
    if (selectedRow < m_filterModel->rowCount() - 1) {
        m_filterModel->insertRow(selectedRow + 1, m_filterModel->takeRow(selectedRow));
        QModelIndex newSelection = m_filterModel->index(selectedRow + 1, 0);
        ui->filterTableView->selectionModel()->clearSelection();
        ui->filterTableView->selectionModel()->select(newSelection, QItemSelectionModel::Select);
    }

    // update scheme (table should update itself)
    updateColorScheme();
//    updateColorSchemeWithFilterCache();
}

void FlyEmBodyInfoDialog::onSaveColorMap() {
    QString filename = QFileDialog::getSaveFileName(this, "Save color map");
    if (!filename.isNull()) {
        saveColorMapDisk(filename);
    }
}

void FlyEmBodyInfoDialog::onLoadColorMap()
{
  QString filename = QFileDialog::getOpenFileName(this, "Load color map");
  if (!filename.isNull()) {
    ZJsonValue colors;
    if (colors.load(filename.toStdString())) {
      // validation is done later
      emit colorMapLoaded(colors);
    } else {
      emit jsonLoadColorMapError(
            "Error opening or parsing color map file " + filename);
    }
  }
}

bool FlyEmBodyInfoDialog::isValidColorMap(ZJsonValue colors) {
    // there's only so much we can do...but make an effort

    // NOTE: this format is a messy hack; should be revised!

    // it's an array
    if (!colors.isArray()) {
        emit jsonLoadColorMapError("Color map json file must contain an array");
        return false;
    }

    ZJsonArray colorArray(colors);
    if (colorArray.size() == 0) {
        return true;
    }

    // look at first element and check its structure; it's an object
    if (!ZJsonParser::IsObject(colorArray.at(0))) {
        emit jsonLoadColorMapError("Color map json file must contain an array of objects");
        return false;
    }

    // has keys "color", "filter":
    /* //Disable it for format extension. Just fall through there is an invalid entry.
    ZJsonObject first(colorArray.at(0), ZJsonValue::SET_INCREASE_REF_COUNT);
    if (!first.hasKey("color") || !first.hasKey("filter")) {
        emit jsonLoadColorMapError("Color map json entries must have 'color' and 'filter' keys");
        return false;
    }
    */

    // we could keep going, but let's stop for now
    // could also check:
    // key "color": array of four ints 0-255
    // key "filter": string

    return true;
}

void FlyEmBodyInfoDialog::onColorMapLoaded(ZJsonValue colors)
{
  if (!isValidColorMap(colors)) {
    // validator spits out its own errors
    return;
  }

  // clear existing color map and put in new values
  m_filterModel->clear();
  setFilterHeaders(m_filterModel);

  m_groupIdMap.clear();
  QList<QString> m_filterStringSet;

  // we've passed validation at this point, so we know it's an array;
  //  iterate over each color, filter and insert into table;
  ZJsonArray colorArray(colors);
  for (size_t i=0; i<colorArray.size(); i++) {
    ZJsonObject entry(colorArray.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);

    QString name;
    bool isGroup = entry.hasKey("group");

    if (isGroup) {
      name = tag_name(ZJsonParser::stringValue(entry["group"]).c_str());
    } else {
      name = ZJsonParser::stringValue(entry["filter"]).c_str();
    }

    if (!name.isEmpty()) {
      QString filter(name);
      QStandardItem * filterTextItem = new QStandardItem(filter);
      m_filterModel->appendRow(filterTextItem);

      std::vector<int64_t> rgba = ZJsonParser::integerArray(entry["color"]);
      QColor color = QColor(rgba[0], rgba[1], rgba[2], rgba[3]);
      int lastRow = m_filterModel->rowCount() - 1;
      setFilterTableModelColor(color, lastRow);
      int bodyCount = -1;

      if (isGroup) {
        QList<uint64_t> bodyList;
        std::vector<int64_t> bodyArray = ZJsonParser::integerArray(entry["ids"]);
        for (int64_t bodyId : bodyArray) {
          bodyList.append(uint64_t(bodyId));
        }
        m_groupIdMap[name] = bodyList;
        bodyCount = bodyList.size();
      } else {
        if (!m_filterIdMap.contains(name)) {
          updateFilterColorMap(name);
        }
        m_filterStringSet.append(name);
      }
      addBodyCountItem(lastRow, bodyCount);
    }
  }

  //Remove filters not in the table
  QMutableMapIterator<QString, QList<uint64_t>> miter(m_filterIdMap);
  while (miter.hasNext()) {
    miter.next();
    if (!m_filterStringSet.contains(miter.key())) {
      miter.remove();
    }
  }

  // update the table view
  ui->filterTableView->resizeColumnsToContents();
  ui->filterTableView->setColumnWidth(FILTER_NAME_COLUMN, 450);

#ifdef _DEBUG_
  std::cout << "Filter ID map: " << std::endl;
  for (auto iter = m_filterIdMap.begin(); iter != m_filterIdMap.end(); ++iter) {
    std::cout << "  " << iter.key().toStdString() << ": "
              << iter.value().size() << " bodies" << std::endl;
  }
  std::cout << "Group ID map: " << std::endl;
  for (auto iter = m_groupIdMap.begin(); iter != m_groupIdMap.end(); ++iter) {
    std::cout << "  " << iter.key().toStdString() << ": "
              << iter.value().size() << " bodies" << std::endl;
  }
#endif

  updateColorScheme();
  //    updateColorSchemeWithFilterCache();
}

QColor FlyEmBodyInfoDialog::makeRandomColor() const
{
  // Raveler's palette: h, s, v = (random.uniform(0, 6.283), random.uniform(0.3, 1.0),
  //  random.uniform(0.3, 0.8)); that approximately translates to:
  int randomH = qrand() % 360;
  int randomS = 76 + qrand() % 180;
  int randomV = 76 + qrand() % 128;

  return QColor::fromHsv(randomH, randomS, randomV);
}

void FlyEmBodyInfoDialog::addBodyCountItem(int row, int count)
{
  QStandardItem *countItem = new QStandardItem();
  countItem->setSelectable(false);
  if (count >= 0) {
    countItem->setData(count, Qt::DisplayRole);
  }
  m_filterModel->setItem(row, FILTER_COUNT_COLUMN, countItem);
}

void FlyEmBodyInfoDialog::addGroupColor(const QString &name)
{
  logInfo("Update color map");

  QStandardItem * filterTextItem = new QStandardItem(name);
  m_filterModel->appendRow(filterTextItem);
  int lastRow = m_filterModel->rowCount() - 1;

  addBodyCountItem(lastRow, m_bodyProxy->rowCount());

  QColor color = makeRandomColor();
  setFilterTableModelColor(color, lastRow);

  ui->filterTableView->resizeColumnsToContents();
  ui->filterTableView->setColumnWidth(FILTER_NAME_COLUMN, 450);

  // activate the tab, and dispatch the changed info
  ui->tabWidget->setCurrentIndex(COLORS_TAB);
//  updateGroupColorScheme(name, color, true);
  updateGroupIdMap(name);
}

bool FlyEmBodyInfoDialog::isGroupColorName(const QString &name) const
{
  return m_groupIdMap.contains(name);
}

bool FlyEmBodyInfoDialog::isFilterColorName(QString &name) const
{
  return !isGroupColorName(name);
}

void FlyEmBodyInfoDialog::updateColorFilter(QString filter, QString /*oldFilter*/) {
    // note: oldFilter currently unused; I was thinking about allowing an edit
    //  to a filter that would replace an older filter but didn't implement it

    logInfo("Update color filter");

    QStandardItem * filterTextItem = new QStandardItem(filter);
    m_filterModel->appendRow(filterTextItem);

    addBodyCountItem(m_filterModel->rowCount() - 1, -1);
    setFilterTableModelColor(makeRandomColor(), m_filterModel->rowCount() - 1);

    ui->filterTableView->resizeColumnsToContents();
    ui->filterTableView->setColumnWidth(FILTER_NAME_COLUMN, 450);

    // activate the tab, and dispatch the changed info
    ui->tabWidget->setCurrentIndex(COLORS_TAB);
//    updateColorSchemeWithFilterCache();
    updateFilterIdMap(filter);
}

void FlyEmBodyInfoDialog::onDoubleClickFilterTable(const QModelIndex &proxyIndex) {
    QModelIndex modelIndex = m_filterProxy->mapToSource(proxyIndex);
    if (proxyIndex.column() == FILTER_NAME_COLUMN) {
        // double-click on filter text; move to body list
        QString filterString = m_filterProxy->data(m_filterProxy->index(proxyIndex.row(),
            FILTER_NAME_COLUMN)).toString();
        if (isFilterColorName(filterString)) {
          ui->bodyFilterField->setText(filterString);
        }
    } else if (proxyIndex.column() == FILTER_COLOR_COLUMN) {
        // double-click on color; change it
        QColor currentColor = m_filterProxy->data(m_filterProxy->index(proxyIndex.row(),
            FILTER_COLOR_COLUMN), Qt::BackgroundRole).value<QColor>();
        QColor newColor = QColorDialog::getColor(currentColor, this, "Choose color");
        if (newColor.isValid()) {
            setFilterTableModelColor(newColor, modelIndex.row());
            updateColorScheme();
//            updateColorSchemeWithFilterCache();
        }
    }
}

void FlyEmBodyInfoDialog::setFilterTableModelColor(QColor color, int modelRow) {
    QStandardItem * filterColorItem = new QStandardItem();
    filterColorItem->setData(color, Qt::BackgroundRole);
    // table rows are selectable, but selecting changes how color is rendered;
    //  so disallow selection of color items
    Qt::ItemFlags flags = filterColorItem->flags();
    flags &= ~Qt::ItemIsSelectable;
    filterColorItem->setFlags(flags);
    m_filterModel->setItem(modelRow, FILTER_COLOR_COLUMN, filterColorItem);
}

void FlyEmBodyInfoDialog::onDeleteButton() {
    if (ui->filterTableView->selectionModel()->hasSelection()) {
        logInfo("Remove selected color filter");
        // we only allow single row selections; get the filter string
        //  from the selected row; only one, so take the first index thereof:
        QModelIndexList modelIndexList =
            ui->filterTableView->selectionModel()->selectedRows(0);
        //Make sure there is a selected index to void unexpected crash.
        if (!modelIndexList.isEmpty()) {
          QModelIndex viewIndex = modelIndexList.at(0);
          QModelIndex modelIndex = m_filterProxy->mapToSource(viewIndex);
          m_filterModel->removeRow(modelIndex.row());
          updateColorScheme();
        }
//        updateColorSchemeWithFilterCache();
    }
}

void FlyEmBodyInfoDialog::moveToBodyList() {
    // moves selected color filter to body list
    if (ui->filterTableView->selectionModel()->hasSelection()) {
        // we only allow single row selections; get the filter string
        //  from the selected row; only one, so take the first index thereof:
        QModelIndex viewIndex = ui->filterTableView->selectionModel()->selectedRows(0).at(0);
        QString filterString = m_filterModel->data(m_filterProxy->mapToSource(viewIndex)).toString();
        ui->bodyFilterField->setText(filterString);
        }
    }

void FlyEmBodyInfoDialog::updateFilterIdMap()
{
  m_filterIdMap.clear();
  for (int i=0; i<m_filterProxy->rowCount(); i++) {
    QString filterString = m_filterProxy->data(
          m_filterProxy->index(i, FILTER_NAME_COLUMN)).toString();

    if (isFilterColorName(filterString)) {
      m_schemeBuilderProxy->setFilterFixedString(filterString);
      QList<uint64_t> bodyList;
      for (int j=0; j<m_schemeBuilderProxy->rowCount(); j++) {
        qulonglong bodyId = m_schemeBuilderProxy->data(
              m_schemeBuilderProxy->index(j, BODY_ID_COLUMN)).toLongLong();
        bodyList.append(bodyId);
      }
      m_filterIdMap[filterString] = bodyList;
    }
  }

  emit filterIdMapUpdated();
}

void FlyEmBodyInfoDialog::updateFilterIdMap(const QString &filterString)
{
  m_schemeBuilderProxy->setFilterFixedString(filterString);
  QList<uint64_t> bodyList;
  for (int j=0; j<m_schemeBuilderProxy->rowCount(); j++) {
    qulonglong bodyId = m_schemeBuilderProxy->data(
          m_schemeBuilderProxy->index(j, BODY_ID_COLUMN)).toLongLong();
    bodyList.append(bodyId);
  }

  m_filterIdMap[filterString] = bodyList;

  emit filterIdMapUpdated();
}

void FlyEmBodyInfoDialog::updateColorScheme(
    const QString &name, const QColor &color)
{
  QList<uint64_t> bodyList;
  if (isGroupColorName(name)) {
    bodyList = m_groupIdMap.value(name);
  } else {
    bodyList = m_filterIdMap.value(name);
  }

  for (uint64_t bodyId : bodyList) {
    m_colorScheme.setBodyColor(bodyId, color);
  }
}

void FlyEmBodyInfoDialog::updateGroupIdMap(const QString &name)
{
  QList<uint64_t> bodyList;
  for (int j=0; j<m_bodyProxy->rowCount(); j++) {
    uint64_t bodyId = uint64_t(m_bodyProxy->data(
          m_bodyProxy->index(j, BODY_ID_COLUMN)).toLongLong());
    bodyList.append(bodyId);
  }
  /*
  for (int j=0; j<m_bodyModel->rowCount(); j++) {
    uint64_t bodyId = uint64_t(m_bodyModel->data(
          m_bodyModel->index(j, BODY_ID_COLUMN)).toLongLong());
    bodyList.append(bodyId);
  }
  */

  m_groupIdMap[name] = bodyList;

  emit groupIdMapUpdated();
}

void FlyEmBodyInfoDialog::updateGroupColorScheme(
    const QString &name, const QColor &color, bool updatingMap)
{
  QList<uint64_t> bodyList;
  for (int j=0; j<m_bodyModel->rowCount(); j++) {
    uint64_t bodyId = uint64_t(m_bodyModel->data(
          m_bodyModel->index(j, BODY_ID_COLUMN)).toLongLong());
    m_colorScheme.setBodyColor(bodyId, color);
    if (updatingMap) {
      bodyList.append(bodyId);
    }
  }

  if (updatingMap) {
    m_groupIdMap[name] = bodyList;
  }
}

void FlyEmBodyInfoDialog::updateFilterColorMap(const QString &filterString)
{
#ifdef _DEBUG_
  qint64 t = 0;
  QElapsedTimer timer;
  timer.start();
#endif
  m_schemeBuilderProxy->setFilterFixedString(filterString);
#ifdef _DEBUG_
  t += timer.elapsed();
  qDebug() << filterString << "setFilterFixedString time" << t << "ms";
#endif

  QList<uint64_t> bodyList;
  for (int j=0; j<m_schemeBuilderProxy->rowCount(); j++) {
    qulonglong bodyId = m_schemeBuilderProxy->data(
          m_schemeBuilderProxy->index(j, BODY_ID_COLUMN)).toLongLong();
//    m_colorScheme.setBodyColor(bodyId, color);
    bodyList.append(bodyId);
  }
  m_filterIdMap[filterString] = bodyList;
}

#if 0
void FlyEmBodyInfoDialog::updateColorSchemeWithFilterCache()
{
  m_colorScheme.clear();
  for (int i=0; i<m_filterProxy->rowCount(); i++) {
    QString filterString = m_filterProxy->data(
          m_filterProxy->index(i, FILTER_NAME_COLUMN)).toString();

    QColor color = m_filterProxy->data(
          m_filterProxy->index(i, FILTER_COLOR_COLUMN),
          Qt::BackgroundRole).value<QColor>();
    enum class EState {
      FILTER, FILTER_CACHED, BODY_GROUP
    };

    EState state = EState::FILTER;
    //      bool cached = false;
    if (m_groupIdMap.contains(filterString)) {
      state = EState::BODY_GROUP;
    } else if (m_filterIdMap.contains(filterString)) {
      state = EState::FILTER_CACHED;
    }

    switch (state) {
    case EState::BODY_GROUP:
    {
      QList<uint64_t> bodyList = m_groupIdMap.value(filterString);
      for (uint64_t bodyId : bodyList) {
        m_colorScheme.setBodyColor(bodyId, color);
      }
    }
      break;
    case EState::FILTER_CACHED:
    {
      QList<uint64_t> bodyList = m_filterIdMap.value(filterString);
      for (uint64_t bodyId : bodyList) {
        m_colorScheme.setBodyColor(bodyId, color);
      }
    }
      break;
    case EState::FILTER:
      updateFilterColorScheme(filterString, color);
      break;
    }

      /*
      if (state == EState::FILTER_CACHED) {
        QList<uint64_t> bodyList = m_filteredIdMap.value(filterString);
        for (uint64_t bodyId : bodyList) {
          m_colorScheme.setBodyColor(bodyId, color);
        }
      } else {
        updateFilterColorScheme(filterString, color);
      }
      */
  }
  m_colorScheme.buildColorTable();

  emit colorMapChanged(m_colorScheme);
}
#endif

void FlyEmBodyInfoDialog::updateColorScheme() {
  // loop over filters in filter table; attach filter to our scheme builder
  //  proxy; loop over the filtered body IDs and throw them and the color into
  //  the color scheme

//  m_filteredIdMap.clear();

  m_colorScheme.clear();
  for (int i=0; i<m_filterProxy->rowCount(); i++) {
    QString filterString = m_filterProxy->data(
          m_filterProxy->index(i, FILTER_NAME_COLUMN)).toString();
    QColor color = m_filterProxy->data(
          m_filterProxy->index(i, FILTER_COLOR_COLUMN),
          Qt::BackgroundRole).value<QColor>();
    updateColorScheme(filterString, color);

//    if (isGroupColorName(filterString)) {
//      updateGroupColorScheme(filterString, color, false);
//    } else {
//      updateFilterColorScheme(filterString, color);
//    }
  }
  m_colorScheme.buildColorTable();

  emit colorMapChanged(m_colorScheme);

  // test: print it out
  // m_colorScheme.print();

}

void FlyEmBodyInfoDialog::importBodies(QString filename)
{
  if (!filename.isEmpty()) {
    std::ifstream stream(filename.toStdString());
    if (stream.good()) {
      prepareQuery();
      ZJsonArray bodyArray;
      std::string line;
      while (std::getline(stream, line)) {
        uint64_t bodyId = ZString(line).firstUint64();
        if (bodyId > 0) {
          ZJsonObject bodyData;
          bodyData.setEntry("body ID", bodyId);
          bodyArray.append(bodyData);
        }
      }
      setBodyList(bodyArray);
    } else {
      ZDialogFactory::Warn(
            "Open File Failed", "Cannot open the file " + filename, this);
    }
  }
}

void FlyEmBodyInfoDialog::exportBodies(QString filename) {
    exportData(filename, EXPORT_BODIES);
}

void FlyEmBodyInfoDialog::exportConnections(QString filename) {
    exportData(filename, EXPORT_CONNECTIONS);
}

/*
 * this method outputs a text file with tabular data;
 * usually it's tab-separated, but there's a hack in
 * there for a headerless csv file, too
 */
void FlyEmBodyInfoDialog::exportData(QString filename, ExportKind kind) {
    QSortFilterProxyModel * proxy;
    QStandardItemModel * model;
    if (kind == EXPORT_BODIES) {
        proxy = m_bodyProxy;
        model = m_bodyModel;
    } else if (kind == EXPORT_CONNECTIONS) {
        proxy = m_ioBodyProxy;
        model = m_ioBodyModel;
    } else {
        QMessageBox errorBox;
        errorBox.setText("Data error!");
        errorBox.setInformativeText("Unknown export kind!  Export canceled.");
        errorBox.setStandardButtons(QMessageBox::Ok);
        errorBox.setIcon(QMessageBox::Warning);
        errorBox.exec();
        return;
    }

    // hack to support tab-separated and csv
    bool csv = filename.endsWith(".csv");
    std::string separator;
    if (csv) {
        separator = ",";
    } else {
        separator = "\t";
    }

    QFile outputFile(filename);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox errorBox;
        errorBox.setText("Error");
        errorBox.setInformativeText("Error opening file " + filename + " for export");
        errorBox.setStandardButtons(QMessageBox::Ok);
        errorBox.setIcon(QMessageBox::Warning);
        errorBox.exec();
        return;
    }

    QTextStream outputStream(&outputFile);

    // if csv file, no header
    if (!csv) {
        for (int i=0; i<model->columnCount(); i++) {
            if (i != 0) {
                outputStream << separator.c_str();
            }
            outputStream << model->horizontalHeaderItem(i)->text();
        }
        outputStream << "\n";
    }

    // csv needs to quote values
    for (int j=0; j<proxy->rowCount(); j++) {
        for (int i=0; i<proxy->columnCount(); i++) {
            if (i != 0) {
                outputStream << separator.c_str();
            }
            if (csv) {
                outputStream << '"';
            }
            outputStream << proxy->data(proxy->index(j, i)).toString();
            if (csv) {
                outputStream << '"';
            }
        }
        outputStream << "\n";
    }
    outputFile.close();
}

QString FlyEmBodyInfoDialog::getTableColorName(int index) const
{
  return m_filterModel->data(
        m_filterModel->index(index, FILTER_NAME_COLUMN)).toString();
}

QColor FlyEmBodyInfoDialog::getTableColor(int index) const
{
  return m_filterModel->data(
        m_filterModel->index(index, FILTER_COLOR_COLUMN), Qt::BackgroundRole).
      value<QColor>();
}

ZJsonArray FlyEmBodyInfoDialog::getColorMapAsJson(ZJsonArray colors)
{
  for (int i=0; i<m_filterModel->rowCount(); i++) {
    QString filterString = getTableColorName(i);
    QColor color = getTableColor(i);

    ZJsonArray rgba;
    rgba.append(color.red());
    rgba.append(color.green());
    rgba.append(color.blue());
    rgba.append(color.alpha());

    ZJsonObject entry;
    entry.setEntry("color", rgba);

    if (isGroupColorName(filterString)) {
      QString name = untag_name(filterString);
      entry.setEntry("group", name.toStdString());
      const auto &idList = m_groupIdMap.value(filterString);
      ZJsonArray idArray;
      for (uint64_t id : idList) {
        idArray.append(id);
      }
      entry.setEntry("ids", idArray);
    } else {
      entry.setEntry("filter", filterString.toStdString());
    }
    colors.append(entry);
  }

  return colors;
}

void FlyEmBodyInfoDialog::saveColorMapDisk(QString filename) {
    ZJsonArray colors;
    getColorMapAsJson(colors).dump(filename.toStdString());
}

void FlyEmBodyInfoDialog::gotoPrePost(QModelIndex modelIndex) {

    // this guard is not vital, but it's convenient; I suspect almost
    //  everything will work, but I'm not 100% sure about some of the
    //  instance variables
    if (m_connectionsLoading) {
        QMessageBox errorBox;
        errorBox.setText("Still loading");
        errorBox.setInformativeText("The previous body connections are still loading; please retry when they are finished.");
        errorBox.setStandardButtons(QMessageBox::Ok);
        errorBox.setIcon(QMessageBox::Warning);
        errorBox.exec();
        return;
    }
    m_connectionsLoading = true;

    // grab the body ID; same row, first column
    QModelIndex index = m_bodyProxy->mapToSource(modelIndex);
    QStandardItem *item = m_bodyModel->item(index.row(), BODY_ID_COLUMN);
    m_connectionsBody = item->data(Qt::DisplayRole).toULongLong();

    // get name, too, if it's there, then update label
    QStandardItem *item2 = m_bodyModel->item(index.row(), BODY_NAME_COLUMN);
    if (item2) {
        updateBodyConnectionLabel(
              m_connectionsBody, item2->data(Qt::DisplayRole).toString());
    } else {
        updateBodyConnectionLabel(m_connectionsBody, "");
    }


    // table shows inputs or outputs, based on whether the pre or post
    //  cell was double-clicked:
    if (index.column() == BODY_NPRE_COLUMN) {
        m_connectionsTableState = CT_OUTPUT;
    } else {
        m_connectionsTableState = CT_INPUT;
    }

    // set labels above tables
    ui->ioBodyTableLabel->setText("Loading");
    ui->connectionsTableLabel->setText("Connections");

    // activate tab & clear model
    ui->tabWidget->setCurrentIndex(CONNECTIONS_TAB);
    m_ioBodyModel->clear();
    m_connectionsModel->clear();

    // trigger retrieval of synapse partners
    m_futureMap["retrieveIOBodiesDvid"] =
        QtConcurrent::run(this, &FlyEmBodyInfoDialog::retrieveIOBodiesDvid, m_connectionsBody);
}

void FlyEmBodyInfoDialog::updateBodyConnectionLabel(uint64_t bodyID, QString bodyName) {

    std::ostringstream outputStream;
    outputStream << "Connections for ";
    if (bodyName.size() > 0) {
        outputStream << bodyName.toStdString() << " (ID " << bodyID << ")";
    } else {
        outputStream << "body ID " << bodyID;
    }
    ui->connectionBodyLabel->setText(QString::fromStdString(outputStream.str()));

}

void FlyEmBodyInfoDialog::retrieveIOBodiesDvid(uint64_t bodyID) {
    // I'm leaving all the timing prints commented out for future use
    // QElapsedTimer spottimer;
    // QElapsedTimer totaltimer;
    // spottimer.start();
    // totaltimer.start();

    // note: this method is run in a different thread than the rest
    //  of the GUI, so we must open our own DVID reader
    ZDvidReader &reader = getIoBodyReader();

    // testing
    // reader.setVerbose(true);

    if (reader.isReady()) {
        logInfo("Retieving body inputs and outputs");
        // std::cout << "open DVID reader: " << spottimer.restart() / 1000.0 << "s" << std::endl;
        std::vector<ZDvidSynapse> synapses;
        if (ui->roiComboBox->currentIndex() > 0) {
          synapses = reader.readSynapse(
                bodyID, *getRoi(ui->roiComboBox->currentText()),
                dvid::EAnnotationLoadMode::PARTNER_LOCATION);
        } else {
          synapses = reader.readSynapse(bodyID, dvid::EAnnotationLoadMode::PARTNER_LOCATION);
        }

        // std::cout << "read synapses: " << spottimer.restart() / 1000.0 << "s" << std::endl;
        // std::cout << "got " << synapses.size() << " synapses" << std::endl;

        // how many pre/post?
        int npre = 0;
        int npost = 0;
        for (size_t i=0; i<synapses.size(); i++) {
            if (synapses[i].getKind() == ZDvidSynapse::EKind::KIND_PRE_SYN) {
                npre++;
            } else {
                npost++;
            }
        }
        // std::cout << "counted pre/post: " << spottimer.restart() / 1000.0 << "s" << std::endl;
        // std::cout << "found " << npre << " pre and " << npost << " post sites" << std::endl;

        // at this point, we'll need to iterate over the list and find either
        //  the pre or post sites depending on whether we are looking for inputs
        //  or outputs
        // then for each site, find the partner site, then find the body at the 
        //  partner site; in a QMap, keep a list of synapses for each partner body
        //  (the qmap is an instance variable)
        // we only want to call dvid once, so we need to loop over synapses once
        //  to find at which x, y, z locations the partners are, then call dvid
        //  to get the bodies at those locations, then we can build the 
        //  final connections map; it's about a 3x performance improvement over
        //  calling dvid once per site

        m_connectionsSites.clear();

        std::vector<ZIntPoint> siteList;
        for (size_t i=0; i<synapses.size(); i++) {

            // if we are looking for input bodies, pick out the sites
            //  that are post-synaptic, and vice versa:
            if ( (m_connectionsTableState == CT_INPUT && synapses[i].getKind() == ZDvidSynapse::EKind::KIND_POST_SYN) ||
                 (m_connectionsTableState == CT_OUTPUT && synapses[i].getKind() == ZDvidSynapse::EKind::KIND_PRE_SYN) )  {

                // find the partner site and the body at that location
                // note that there could be multiple partners for outputs

                // currently Ting doesn't expose a method for retrieving relations
                //  directly; I've patched in a getter to use, temporarily
                std::vector<ZIntPoint> sites = synapses[i].getPartners();
                for (size_t j=0; j<sites.size(); j++) {
                    siteList.push_back(sites[j]);
                }

            } else {
                // there are other connection types we aren't handling right now; eg, convergent
            }
        }
        // std::cout << "built site list: " << spottimer.restart() / 1000.0 << "s" << std::endl;

        // get the body list from DVID
        std::vector<uint64_t> bodyList = reader.readBodyIdAt(siteList);
        if (bodyList.size() == 0) {
            emit ioNoBodiesLoaded();
            return;
        }
        // std::cout << "got labels for " << bodyList.size() << " points" << std::endl;
        // std::cout << "retrieved body list from DVID: " << spottimer.restart() / 1000.0 << "s" << std::endl;

        // copy into the map
        for (size_t i=0; i<siteList.size(); i++) {
            if (!m_connectionsSites.contains(bodyList[i])) {
                m_connectionsSites[bodyList[i]] = QList<ZIntPoint>();
            } 
            m_connectionsSites[bodyList[i]].append(siteList[i]);
        }
        // std::cout << "built qmap: " << spottimer.restart() / 1000.0 << "s" << std::endl;

        // name check; since we don't load all bodies in the top table,
        //  we may not have names available for every body that could
        //  appear in the lower table; try to fill in the gaps

        // find out which bodies we don't have names for but do have body annotations and
        //  thus *might* have names available
        QString bodyAnnotationName = QString::fromStdString(m_currentDvidTarget.getBodyAnnotationName());
        QSet<QString> bodyAnnotationKeys = reader.readKeys(bodyAnnotationName).toSet();
        QList<QString> keyList;
        for (size_t i=0; i<bodyList.size(); i++) {
            if (!m_bodyNames.contains(bodyList[i]) && !m_namelessBodies.contains(bodyList[i]) &&
                bodyAnnotationKeys.contains(QString::number(bodyList[i]))) {
                keyList.append(QString::number(bodyList[i]));
            }
        }

        // grab all the possible annotations at once, then those that have names,
        //  put in our name stash
        QList<ZJsonObject> bodyAnnotationList = reader.readJsonObjectsFromKeys(bodyAnnotationName, keyList);

        foreach (ZJsonObject bodyData, bodyAnnotationList) {
            uint64_t tempBodyID = ZJsonParser::integerValue(bodyData["body ID"]);
            std::string name = get_annotation_name(bodyData);
            if (name.empty()) {
              m_namelessBodies.insert(tempBodyID);
            } else {
              m_bodyNames[tempBodyID] = QString(name.c_str());
            }
            /*
            if (bodyData.hasKey("name")) {
                if (!ZJsonParser::stringValue(bodyData["name"]).empty()) {
                    m_bodyNames[tempBodyID] =
                        QString(ZJsonParser::stringValue(bodyData["name"]).c_str());
                } else {
                    m_namelessBodies.insert(tempBodyID);
                }
            } else {
                m_namelessBodies.insert(tempBodyID);
            }
            */
        }

        // std::cout << "got previously unknown names: " << spottimer.restart() / 1000.0 << "s" << std::endl;
        emit ioBodiesLoaded();
    } else {
        emit ioBodyLoadFailed();
    }
    // std::cout << "exiting retrieveIOBodiesDvid(): " << totaltimer.elapsed() / 1000.0 << "s" << std::endl;
}

void FlyEmBodyInfoDialog::onIOBodiesLoaded() {
    m_ioBodyModel->clear();
    setIOBodyHeaders(m_ioBodyModel);

    QList<uint64_t> partnerBodyIDs = m_connectionsSites.keys();


    m_totalConnections = 0;
    m_ioBodyModel->setRowCount(partnerBodyIDs.size());
    for (int i=0; i<partnerBodyIDs.size(); i++) {
        // carefully set data for column items so they will sort
        //  properly (eg, IDs numerically, not lexically)
        QStandardItem * bodyIDItem = new QStandardItem();
        bodyIDItem->setData(QVariant(quint64(partnerBodyIDs[i])), Qt::DisplayRole);
        m_ioBodyModel->setItem(i, IOBODY_ID_COLUMN, bodyIDItem);

        if (m_bodyNames.contains(partnerBodyIDs[i])) {
            m_ioBodyModel->setItem(i, IOBODY_NAME_COLUMN, new QStandardItem(m_bodyNames[partnerBodyIDs[i]]));
        }

        QStandardItem * numberItem = new QStandardItem();
        quint64 itemConnections = quint64(m_connectionsSites[partnerBodyIDs[i]].size());
        m_totalConnections += itemConnections;
        numberItem->setData(QVariant(itemConnections), Qt::DisplayRole);
        m_ioBodyModel->setItem(i, IOBODY_NUMBER_COLUMN, numberItem);
    }


    // table label: "Input (123)" or "Output (123)"
    // should factor this out
    std::ostringstream labelStream;
    if (m_connectionsTableState == CT_INPUT) {
        labelStream << "Inputs (" ;
    } else {
        labelStream << "Outputs (" ;
    }
    labelStream << m_totalConnections;
    labelStream << ")";
    ui->ioBodyTableLabel->setText(QString::fromStdString(labelStream.str()));


    // for some reason, this table gave me more trouble than the
    //  others; set its column behaviors individually
#if QT_VERSION >= 0x050000
    ui->ioBodyTableView->horizontalHeader()->setSectionResizeMode(IOBODY_ID_COLUMN, QHeaderView::ResizeToContents);
    ui->ioBodyTableView->horizontalHeader()->setSectionResizeMode(IOBODY_NAME_COLUMN, QHeaderView::Stretch);
    ui->ioBodyTableView->horizontalHeader()->setSectionResizeMode(IOBODY_NUMBER_COLUMN, QHeaderView::ResizeToContents);
#else
    ui->ioBodyTableView->horizontalHeader()->setResizeMode(IOBODY_ID_COLUMN, QHeaderView::ResizeToContents);
    ui->ioBodyTableView->horizontalHeader()->setResizeMode(IOBODY_NAME_COLUMN, QHeaderView::Stretch);
    ui->ioBodyTableView->horizontalHeader()->setResizeMode(IOBODY_NUMBER_COLUMN, QHeaderView::ResizeToContents);
#endif
    ui->ioBodyTableView->sortByColumn(IOBODY_NUMBER_COLUMN, Qt::DescendingOrder);

    m_connectionsLoading = false;

}

void FlyEmBodyInfoDialog::onIOBodyLoadFailed() {
    ui->ioBodyTableLabel->setText("load failed");
    m_connectionsLoading = false;
}

void FlyEmBodyInfoDialog::onIONoBodiesLoaded() {
    ui->ioBodyTableLabel->setText("no bodies to load");
    m_connectionsLoading = false;
}

void FlyEmBodyInfoDialog::onDoubleClickIOBodyTable(QModelIndex proxyIndex) {

    // grab the body ID; same row, first column
    QModelIndex modelIndex = m_ioBodyProxy->mapToSource(proxyIndex);
    QStandardItem *item = m_ioBodyModel->item(modelIndex.row(), IOBODY_ID_COLUMN);
    uint64_t bodyID = item->data(Qt::DisplayRole).toULongLong();

    if (proxyIndex.column() == IOBODY_ID_COLUMN || proxyIndex.column() == IOBODY_NAME_COLUMN) {
        // double-click on ID, name = select in body table
        QList<QStandardItem*> results = m_bodyModel->findItems(item->data(Qt::DisplayRole).toString(), Qt::MatchExactly, BODY_ID_COLUMN);
        if (results.size() > 0) {
            ui->bodyTableView->selectRow(m_bodyProxy->mapFromSource(results.at(0)->index()).row());
        }

    } else if (proxyIndex.column() == IOBODY_NUMBER_COLUMN) {
        // double-click on number connections = show connections list
        m_connectionsModel->clear();
        setConnectionsHeaders(m_connectionsModel);
        for (int i=0; i<m_connectionsSites[bodyID].size(); i++) {
            ZIntPoint point = m_connectionsSites[bodyID][i];

            // start with x, y, z; but should it be z, x, y?
            QStandardItem * xItem = new QStandardItem();
            xItem->setData(QVariant(point.getX()), Qt::DisplayRole);
            m_connectionsModel->setItem(i, CONNECTIONS_X_COLUMN, xItem);

            QStandardItem * yItem = new QStandardItem();
            yItem->setData(QVariant(point.getY()), Qt::DisplayRole);
            m_connectionsModel->setItem(i, CONNECTIONS_Y_COLUMN, yItem);

            QStandardItem * zItem = new QStandardItem();
            zItem->setData(QVariant(point.getZ()), Qt::DisplayRole);
            m_connectionsModel->setItem(i, CONNECTIONS_Z_COLUMN, zItem);
        }

        // for this table, we want all columns same width, filling full width
#if QT_VERSION >= 0x050000
        ui->connectionsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#else
        ui->connectionsTableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#endif
        ui->connectionsTableView->sortByColumn(CONNECTIONS_Z_COLUMN, Qt::AscendingOrder);
    }
}

void FlyEmBodyInfoDialog::onDoubleClickIOConnectionsTable(QModelIndex proxyIndex) {
    QModelIndex modelIndex = m_connectionsProxy->mapToSource(proxyIndex);

    QStandardItem *itemX = m_connectionsModel->item(modelIndex.row(), CONNECTIONS_X_COLUMN);
    int x = itemX->data(Qt::DisplayRole).toInt();

    QStandardItem *itemY = m_connectionsModel->item(modelIndex.row(), CONNECTIONS_Y_COLUMN);
    int y = itemY->data(Qt::DisplayRole).toInt();

    QStandardItem *itemZ = m_connectionsModel->item(modelIndex.row(), CONNECTIONS_Z_COLUMN);
    int z = itemZ->data(Qt::DisplayRole).toInt();

    emit pointDisplayRequested(x, y, z);
}

void FlyEmBodyInfoDialog::onCopySelectedConnections()
{
  QString result;

  if (m_connectionsBody > 0) {
    QString link = "";
    if (m_connectionsTableState == CT_INPUT) {
      link = "<-";
    } else if (m_connectionsTableState == CT_OUTPUT) {
      link = "->";
    }
    if (!link.isEmpty()) {
      QModelIndexList indexList =
          ui->ioBodyTableView->selectionModel()->selectedIndexes();
      QList<uint64_t> connectedBodies;
      for (const QModelIndex &index : indexList) {
        QModelIndex modelIndex =  m_ioBodyProxy->mapToSource(index);
        QStandardItem *item = m_ioBodyModel->item(modelIndex.row());
        auto bodyId = item->data(Qt::DisplayRole).toULongLong();
        if (bodyId > 0) {
          connectedBodies.append(uint64_t(bodyId));
        }
      }

      if (!connectedBodies.isEmpty()) {
        result = QString("%1 ").arg(m_connectionsBody) + link + " [ ";
        for (uint64_t body : connectedBodies) {
          result += QString("%1, ").arg(body);
        }
        result += "]";
      }
    }
  }

  ZGlobal::CopyToClipboard(result.toStdString());
}

void FlyEmBodyInfoDialog::onIOConnectionsSelectionChanged(QItemSelection /*selected*/,
    QItemSelection /*deselected*/) {

    // Shinya wants the table label to show the number of selected connections

    // the two input selections only contain the changes; we want the whole selection
    QModelIndexList indexList = ui->ioBodyTableView->selectionModel()->selectedIndexes();

    int total = 0;
    if (indexList.size() == 0) {
        // if there's no selection, we want all the connections
        total = m_totalConnections;
    } else {
        QSet<int> rows;
        foreach (QModelIndex proxyIndex, indexList) {
            rows.insert(m_ioBodyProxy->mapToSource(proxyIndex).row());
        }
        foreach (int row, rows) {
            total += m_ioBodyModel->item(row, IOBODY_NUMBER_COLUMN)->data(Qt::DisplayRole).toInt();
        }
    }

    // adjust the label; pick off the existing "Input" or "Output" and rebuild the rest
    QString currentLabel = ui->ioBodyTableLabel->text();
    QString firstPart = currentLabel.split("(").at(0);
    QString newLabel = firstPart + "(" + QString::number(total);
    if (total < m_totalConnections) {
        newLabel += " selected";
    }
    newLabel += ")";
    ui->ioBodyTableLabel->setText(newLabel);
}

FlyEmBodyInfoDialog::~FlyEmBodyInfoDialog()
{
    m_quitting = true;
    m_futureMap.waitForFinished(); //to avoid crash while quitting too early
    delete ui;
}

/*
void FlyEmBodyInfoDialog::setNeuPrintReader(
    std::unique_ptr<NeuPrintReader> reader)
{
  m_neuPrintReader = reader;
}
*/

std::string FlyEmBodyInfoDialog::getNeuprintUuid() const {
  return m_reader.getDvidTarget().getUuid();
}


NeuPrintReader* FlyEmBodyInfoDialog::getNeuPrintReader()
{
  if (!m_neuPrintReader) {
    if (!m_neuprintDataset.empty()) {
      m_neuPrintReader = std::unique_ptr<NeuPrintReader>(
            ZGlobal::GetInstance().makeNeuPrintReader());
      m_neuPrintReader->setCurrentDataset(m_neuprintDataset.c_str());
    } else {
      m_neuPrintReader = std::unique_ptr<NeuPrintReader>(
            ZGlobal::GetInstance().makeNeuPrintReaderFromUuid(
              getNeuprintUuid().c_str()));
    }

    setWindowTitle("Body Infomation @ NeuPrint:" +
                   getNeuPrintReader()->getServer() + ":" +
                   m_neuprintDataset.c_str());

//            m_reader.getDvidTarget().getUuid().c_str()));
  }

  if (!m_neuPrintReader) {
    setStatusLabel("<font color=\"#800000\">Oops! "
                   "Cannot connect NeuPrint!</font>");
  }

  return m_neuPrintReader.get();
}

NeuPrintQueryDialog* FlyEmBodyInfoDialog::getNeuPrintRoiQueryDlg()
{
  if (m_neuprintQueryDlg == nullptr) {
    m_neuprintQueryDlg = new NeuPrintQueryDialog(this);

    NeuPrintReader *reader = getNeuPrintReader();
    if (reader) {
      m_neuprintQueryDlg->setRoiList(reader->getRoiList());
    }
//    m_neuprintQueryDlg->setRoiList(getCompleteDocument()->getRoiList());
  }

  return m_neuprintQueryDlg;
}
