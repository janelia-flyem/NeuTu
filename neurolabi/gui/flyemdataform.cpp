#include "flyemdataform.h"
#include <iostream>
#include <fstream>
#include <QFileDialog>
#include <QTextBrowser>
#include <QListView>
#include <QInputDialog>
#include <QStatusBar>
#include <QMenu>
#include <QMessageBox>
#include <QGraphicsPixmapItem>
#include <QMenu>
#include <QtConcurrentRun>

#include "neutubeconfig.h"
#include "tz_error.h"
#include "zswctree.h"
#include "zstackdoc.h"
#include "ui_flyemdataform.h"
#include "flyem/zflyemdataframe.h"
#include "flyem/zflyemstackframe.h"
#include "widgets/zimagewidget.h"
#include "zfiletype.h"
#include "zimage.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"
#include "flyem/zflyemneuronbodyinfo.h"
#include "z3dpunctafilter.h"
#include "z3dcompositor.h"
#include "z3dvolumeraycaster.h"
#include "z3daxis.h"
#include "swcexportdialog.h"
#include "zdialogfactory.h"
#include "zprogressmanager.h"
#include "z3dvolumesource.h"
#include "z3dvolume.h"
#include "zwindowfactory.h"

FlyEmDataForm::FlyEmDataForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::FlyEmDataForm),
  m_statusBar(NULL),
  m_neuronContextMenu(NULL),
  m_showSelectedModelAction(NULL),
  m_showSelectedModelWithBoundBoxAction(NULL),
  m_changeClassAction(NULL),
  m_neighborSearchAction(NULL),
  m_showSelectedBodyAction(NULL),
  m_secondaryNeuronContextMenu(NULL),
  m_showSecondarySelectedModelAction(NULL)
{
  ui->setupUi(this);
  ui->slaveQueryView->setParent(ui->InformationTab->widget(1));

  ui->progressBar->hide();
  m_neuronList = new ZFlyEmNeuronListModel(this);
  ui->queryView->setModel(m_neuronList);
  m_secondaryNeuronList = new ZFlyEmNeuronListModel(this);
  ui->slaveQueryView->setModel(m_secondaryNeuronList);
  ui->queryView->setSelectionMode(QAbstractItemView::ExtendedSelection);
#ifdef _DEBUG_2
  connect(ui->queryView, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(assignClass(QModelIndex)));
#endif
  connect(ui->queryView, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(viewModel(QModelIndex)));

  connect(ui->queryView, SIGNAL(clicked(QModelIndex)),
          this, SLOT(updateStatusBar(QModelIndex)));
  connect(ui->queryView, SIGNAL(clicked(QModelIndex)),
          this, SLOT(updateInfoWindow(QModelIndex)));
  connect(ui->queryView, SIGNAL(clicked(QModelIndex)),
          this, SLOT(updateSlaveQuery(QModelIndex)));
  connect(ui->queryView, SIGNAL(clicked(QModelIndex)),
          this, SLOT(updateThumbnail(QModelIndex)));
  connect(ui->slaveQueryView, SIGNAL(clicked(QModelIndex)),
          this, SLOT(updateThumbnailSecondary(QModelIndex)));
  connect(this, SIGNAL(thumbnailItemReady(QList<QGraphicsItem*>,int)),
          this, SLOT(updateThumbnail(QList<QGraphicsItem*>,int)));

  //customize
  //ui->testPushButton->hide();


#ifndef _DEBUG_
  ui->importButton->hide();
  ui->menuButton->hide();
//  ui->processPushButton->hide();
  ui->testPushButton->hide();
  ui->generalPushButton->hide();
  ui->optionPushButton->hide();
  ui->addDataPushButton->hide();
  ui->label->hide();
#endif

  createAction();
  createContextMenu();
  ui->queryView->setContextMenu(m_neuronContextMenu);
  ui->slaveQueryView->setContextMenu(m_secondaryNeuronContextMenu);

  m_progressManager = new ZProgressManager(this);
  m_specialProgressReporter.setProgressBar(ui->progressBar);
  m_progressManager->setProgressReporter(&m_specialProgressReporter);

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


  m_thumbnailScene = new QGraphicsScene;
  ui->thumbnailView->setScene(m_thumbnailScene);

  ui->overallLayout->setMargin(10);
  ui->thumbnailView->resize(256, ui->thumbnailView->height());
  ui->outputTextEdit->resize(190, ui->outputTextEdit->height());
  setLayout(ui->overallLayout);

  createMenu();

  m_swcExportDlg = new SwcExportDialog(this);
  m_3dWindowFactory.setParentWidget(this->parentWidget());
}

FlyEmDataForm::~FlyEmDataForm()
{
  //delete m_thumbnailImage;
  delete ui;
  delete m_thumbnailScene;

  for (QMap<QString, QFuture<void> >::iterator iter = m_threadFutureMap.begin();
       iter != m_threadFutureMap.end(); ++iter) {
    if (!iter->isFinished()) {
      iter->waitForFinished();
    }
  }
}

QSize FlyEmDataForm::sizeHint() const
{
#ifdef _DEBUG_2
  std::cout << geometry().x() << " " << geometry().y()
            << geometry().width() << " " << geometry().height() << std::endl;
#endif

  return geometry().size();
}

void FlyEmDataForm::createMenu()
{
  m_mainMenu = new QMenu(this);
  ui->menuButton->setMenu(m_mainMenu);

  m_exportMenu = new QMenu(this);
  ui->exportButton->setMenu(m_exportMenu);

  QAction *showModelAction = new QAction("Show Model", this);
  m_mainMenu->addAction(showModelAction);
  connect(showModelAction, SIGNAL(triggered()),
          this, SLOT(on_showModelPushButton_clicked()));

  QAction *exportCsvAction = new QAction("CSV", this);
  m_exportMenu->addAction(exportCsvAction);
  connect(exportCsvAction, SIGNAL(triggered()),
          this, SLOT(on_savePushButton_clicked()));

  QAction *exportSwcAction = new QAction("SWC", this);
  m_exportMenu->addAction(exportSwcAction);
  connect(exportSwcAction, SIGNAL(triggered()),
          this, SLOT(on_saveSwcPushButton_clicked()));

  QAction *exportVolumeRenderAction = new QAction("Figure (3D Body)", this);
  m_exportMenu->addAction(exportVolumeRenderAction);
  connect(exportVolumeRenderAction, SIGNAL(triggered()),
          this, SLOT(exportVolumeRenderingFigure()));

  QAction *exportTypeLabelAction = new QAction("Type Labels", this);
  m_exportMenu->addAction(exportTypeLabelAction);
  connect(exportTypeLabelAction, SIGNAL(triggered()),
          this, SLOT(exportTypeLabelFile()));

/*
  QAction *exportAction = new QAction("Export", this);
  m_mainMenu->addAction(exportAction);
  connect(exportAction, SIGNAL(triggered()), this, SLOT(exportResult()));

  m_importRoiAction = new QAction("Import ROI", this);
  m_mainMenu->addAction(m_importRoiAction);
  m_importRoiAction->setCheckable(true);
  connect(m_importRoiAction, SIGNAL(triggered()), this, SLOT(importRoi()));

  m_autoStepAction = new QAction("Auto Step", this);
  m_mainMenu->addAction(m_autoStepAction);
  m_autoStepAction->setCheckable(true);
  connect(m_autoStepAction, SIGNAL(toggled(bool)),
          this, SLOT(runAutoStep(bool)));

  m_applyTranslateAction = new QAction("Apply Translation", this);
  m_mainMenu->addAction(m_applyTranslateAction);
  //m_applyTranslateAction->setCheckable(true);
  connect(m_applyTranslateAction, SIGNAL(triggered()),
          this, SLOT(applyTranslate()));

  m_deleteProjectAction = new QAction("Delete Project", this);
  m_mainMenu->addAction(m_deleteProjectAction);
  connect(m_deleteProjectAction, SIGNAL(triggered()),
          this, SLOT(deleteProject()));
          */
}

void FlyEmDataForm::on_pushButton_clicked()
{
  emit showSummaryTriggered();
}

void FlyEmDataForm::on_processPushButton_clicked()
{
  emit processTriggered();
}

void FlyEmDataForm::on_queryPushButton_clicked()
{
  emit queryTriggered();
}

void FlyEmDataForm::appendOutput(const QString &text)
{
  ui->outputTextEdit->append(text);
}

void FlyEmDataForm::setQueryOutput(const ZFlyEmNeuron *neuron)
{
  m_neuronList->clear();
  appendQueryOutput(neuron);
}

void FlyEmDataForm::appendQueryOutput(const ZFlyEmNeuron *neuron)
{
  //ui->queryTextWidget->append(neuron.c_str());
  if (neuron != NULL) {
    m_neuronList->append(neuron);
  }
}

void FlyEmDataForm::on_testPushButton_clicked()
{
  emit testTriggered();
}

QProgressBar* FlyEmDataForm::getProgressBar()
{
  return ui->progressBar;
}

void FlyEmDataForm::on_generalPushButton_clicked()
{
  emit generalTriggered();
}

void FlyEmDataForm::on_optionPushButton_clicked()
{
  emit optionTriggered();
}

void FlyEmDataForm::on_addDataPushButton_clicked()
{
  QString fileName = QFileDialog::getOpenFileName(
        this, tr("Load FlyEM Database"), "", tr("Json files (*.json)"),
        NULL/*, QFileDialog::DontUseNativeDialog*/);

  if (!fileName.isEmpty()) {
    ZFlyEmDataFrame *frame = dynamic_cast<ZFlyEmDataFrame*>(this->parentWidget());
    if (frame != NULL) {
      frame->load(fileName.toStdString(), true);
    }
  }
}

void FlyEmDataForm::on_savePushButton_clicked()
{
  QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save Query Output"), "", tr("Text files (*.csv)"),
        NULL);
  if (!fileName.isEmpty()) {
    m_neuronList->exportCsv(fileName);
  }
}

void FlyEmDataForm::updateStatusBar(const QModelIndex &index)
{
  if (m_statusBar != NULL) {
    m_statusBar->showMessage(
          m_neuronList->data(index, Qt::StatusTipRole).toString());
  }
}

void FlyEmDataForm::updateInfoWindow(const QModelIndex &index)
{
  QVector<const ZFlyEmNeuron*> neuronArray =
      m_neuronList->getNeuronArray(index);

  ui->infoWindow->clear();
  foreach (const ZFlyEmNeuron* neuron, neuronArray) {
    std::ostringstream stream;
    neuron->print(stream);
    ui->infoWindow->append(stream.str().c_str());
  }
}

ZFlyEmDataFrame* FlyEmDataForm::getParentFrame() const
{
  return dynamic_cast<ZFlyEmDataFrame*>(this->parentWidget());
}

void FlyEmDataForm::updateSlaveQuery(const QModelIndex &index)
{
  ZFlyEmDataFrame *frame = getParentFrame();
  if (frame != NULL) {
    if (m_neuronList->getColumnName(index.column()) == "Type") {
      const ZFlyEmNeuron *neuron = m_neuronList->getNeuron(index.row());
      if (neuron->hasType()) {
        m_secondaryNeuronList->clear();
        ZFlyEmDataBundle *bundle = frame->getDataBundle();
        std::vector<ZFlyEmNeuron> &neuronArray = bundle->getNeuronArray();
        for (std::vector<ZFlyEmNeuron>::iterator iter = neuronArray.begin();
             iter != neuronArray.end(); ++iter) {
          ZFlyEmNeuron &buddyNeuron = *iter;
          if (buddyNeuron.getType() == neuron->getType()) {
            m_secondaryNeuronList->append(&buddyNeuron);
          }
        }
      }
    } else {
      QVector<const ZFlyEmNeuron*> neuronArray =
          m_neuronList->getNeuronArray(index);
      m_secondaryNeuronList->clear();
      foreach (const ZFlyEmNeuron *neuron, neuronArray) {
        m_secondaryNeuronList->append(neuron);
      }
    }
  }
}

void FlyEmDataForm::assignClass(const QModelIndex &index)
{
  if (index.row() > m_neuronList->rowCount()) {
    appendOutput("Invalid index in assignClass");
    return;
  }

  if (index.column() == 0) {
    ZFlyEmNeuron *neuron = m_neuronList->getNeuron(index);
    if (neuron != NULL) {
      const std::vector<const ZFlyEmNeuron*> &topMatch = neuron->getTopMatch();
      QString className = "";
      if (!topMatch.empty()) {
        className = topMatch[0]->getType().c_str();
      }

      //Popup dialog
      bool ok;
      className = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                        tr("Class name:"), QLineEdit::Normal,
                                        className, &ok);
      if (ok) {
        neuron->setType(className.toStdString());
        m_neuronList->notifyRowDataChanged(index.row());
      }
    }
  }
}

void FlyEmDataForm::viewModel(const QModelIndex &index)
{
  if (index.row() > m_neuronList->rowCount()) {
    appendOutput("Invalid index in viewSwc");
    return;
  }

  getProgressManager()->notifyProgressStarted();
  getProgressManager()->notifyProgressAdvanced(0.5);
//  ui->progressBar->setValue(50);
//  ui->progressBar->show();
  //QApplication::processEvents();

  //const ZFlyEmNeuron *neuron = m_neuronList->getNeuron(index);
  QVector<const ZFlyEmNeuron*> neuronArray =
      m_neuronList->getNeuronArray(index);

#ifdef _DEBUG_2
  std::cout << m_neuronList->headerData(index.column(), Qt::Horizontal).toString().toStdString()
             << std::endl;

#endif

  if (!neuronArray.isEmpty()) {
    //ZStackFrame *frame = new ZStackFrame;
    ZStackDoc *doc = new ZStackDoc;

    foreach (const ZFlyEmNeuron* neuron, neuronArray) {
      //doc->addSwcTree(neuron->getModel()->clone(), true);
      ZSwcTree *model = neuron->getModel();
      //ZStackDoc *doc = frame->document().get();
//      doc->blockSignals(true);

      doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
      if (model != NULL) {
        doc->addObject(model->clone(), true);
      }

      std::vector<ZPunctum*> puncta = neuron->getSynapse();
      for (std::vector<ZPunctum*>::iterator iter = puncta.begin();
           iter != puncta.end(); ++iter) {
        doc->addObject(*iter);
      }
      doc->endObjectModifiedMode();
      doc->notifyObjectModified();
//      doc->blockSignals(false);
//      doc->updateModelData(ZStackDoc::SWC_DATA);
//      doc->updateModelData(ZStackDoc::PUNCTA_DATA);
    }

    ZWindowFactory factory;
    factory.setParentWidget(this->parentWidget());
    factory.open3DWindow(doc);

    //frame->open3DWindow(this->parentWidget());
    //delete frame;
  } else if (m_neuronList->headerData(index.column(), Qt::Horizontal) ==
             "Volume Path") {
    if (!m_neuronList->data(index).toString().isEmpty()) {
      emit volumeTriggered(m_neuronList->data(index).toString());
    }
  }

  getProgressManager()->notifyProgressEnded();

  //ui->progressBar->hide();
}

ZStackDoc *FlyEmDataForm::showViewSelectedModel(ZFlyEmQueryView *view)
{
  ui->progressBar->setValue(50);
  ui->progressBar->show();
  //QApplication::processEvents();

  QItemSelectionModel *sel = view->selectionModel();

#ifdef _DEBUG_2
  appendOutput(QString("%1 rows selected").arg(sel->selectedIndexes().size()).toStdString());
#endif

  //ZStackFrame *frame = new ZStackFrame;

  ZStackDoc *doc = new ZStackDoc;
  view->getModel()->retrieveModel(
        sel->selectedIndexes(), doc);
  ui->progressBar->setValue(75);
  //QApplication::processEvents();

  ZWindowFactory factory;
  factory.setParentWidget(this->parentWidget());
  Z3DWindow *window = factory.open3DWindow(doc);
  window->getPunctaFilter()->setColorMode("Original Point Color");

  /*
  Z3DWindow *window = frame->open3DWindow(this->parentWidget());
  window->getPunctaFilter()->setColorMode("Original Point Color");
  ZStackDoc *hostDoc = frame->document().get();

  delete frame;
  */

  ui->progressBar->hide();

  return doc;
}

ZStackDoc *FlyEmDataForm::showViewSelectedBody(ZFlyEmQueryView *view)
{
  ui->progressBar->setValue(50);
  ui->progressBar->show();
  //QApplication::processEvents();

  QItemSelectionModel *sel = view->selectionModel();

#ifdef _DEBUG_2
  appendOutput(QString("%1 rows selected").arg(sel->selectedIndexes().size()).toStdString());
#endif

  ZScalableStack *stack = view->getModel()->retrieveBody(
        sel->selectedIndexes());
  Z3DWindow *window = m_3dWindowFactory.make3DWindow(stack);

  ZStackDoc *hostDoc = NULL;

  if (window != NULL) {
    hostDoc = window->getDocument();
    window->show();
    window->raise();
  }


#if 0
  ZStackFrame *frame = new ZStackFrame;

  ZIntPoint dsIntv = view->getModel()->retrieveBody(
        sel->selectedIndexes(), frame->document().get());
  ui->progressBar->setValue(75);
  //QApplication::processEvents();

  Z3DWindow *window = frame->open3DWindow(this->parentWidget());
  window->getVolumeSource()->getVolume(0)->setScaleSpacing(
        glm::vec3(dsIntv.getX() + 1, dsIntv.getY() + 1, dsIntv.getZ() + 1));
  window->getVolumeSource()->getVolume(0)->scaleOffset(
        dsIntv.getX() + 1, dsIntv.getY() + 1, dsIntv.getZ() + 1);

  ZStackDoc *hostDoc = frame->document().get();

  delete frame;
#endif

  ui->progressBar->hide();

  return hostDoc;
}

void FlyEmDataForm::showSelectedModel()
{
  showViewSelectedModel(ui->queryView);
}

void FlyEmDataForm::showSelectedBody()
{
  showViewSelectedBody(ui->queryView);
}

/*
void FlyEmDataForm::showSelectedBodyCoarse()
{
  showViewSelectedBodyCoarse(ui->queryView);
}
*/

void FlyEmDataForm::showSelectedModelWithBoundBox()
{
  ZStackDoc *hostDoc = showViewSelectedModel(ui->queryView);
  ZFlyEmDataFrame *frame = getParentFrame();
  if (frame != NULL) {
    ZFlyEmDataBundle *dataBundle = frame->getDataBundle();
    if (dataBundle->hasBoundBox()) {
      hostDoc->addObject(dataBundle->getBoundBox()->clone());
    }
  }
}

void FlyEmDataForm::showSecondarySelectedModel()
{
  showViewSelectedModel(ui->slaveQueryView);
}

void FlyEmDataForm::on_showModelPushButton_clicked()
{
  showSelectedModel();
}

void FlyEmDataForm::setPresenter(ZFlyEmNeuronPresenter *presenter)
{
  m_neuronList->setPresenter(presenter);
}

void FlyEmDataForm::on_saveSwcPushButton_clicked()
{
  if (m_swcExportDlg->exec()) {
    QString dirPath = m_swcExportDlg->getSavePath();
    if (!dirPath.isEmpty()) {
      m_neuronList->exportSwc(dirPath, m_swcExportDlg->getCoordSpace());
    }

//  QString dirpath = QFileDialog::getExistingDirectory(this, tr("Export SWC"),
//    "", QFileDialog::ShowDirsOnly);

//  if (!dirpath.isEmpty()) {
//    m_neuronList->exportSwc(dirpath);
//  }
  }
}

void FlyEmDataForm::on_exportPushButton_clicked()
{
  QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save bundle"), "", tr("JSON files (*.json)"),
        NULL);
  if (!fileName.isEmpty()) {
    emit saveBundleTriggered(0, fileName);
  }
}

void FlyEmDataForm::changeNeuronClass()
{
  QItemSelectionModel *sel = ui->queryView->selectionModel();

  QModelIndexList indexList = sel->selectedIndexes();
  int neuronCount = 0;

  foreach (QModelIndex index, indexList) {
    if (m_neuronList->isNeuronKey(index)) {
      ++neuronCount;
    }
  }

  if (neuronCount > 0) {
    bool ok;

    QString className = QInputDialog::getText(
          this, QString("%1 neurons: Change Class").arg(neuronCount),
          tr("Class name:"), QLineEdit::Normal,
          "", &ok);
    if (ok) {
      foreach (QModelIndex index, indexList) {
        if (m_neuronList->isNeuronKey(index)) {
          ZFlyEmNeuron *neuron = m_neuronList->getNeuron(index);
          if (neuron != NULL) {
            neuron->setType(className.toStdString());
            m_neuronList->notifyRowDataChanged(index.row());
          }
        }
      }
    }
  } else {
    QMessageBox::warning(this,
          "No Neuron Selected",
          "At least one neuron key should be selected to change class");
  }
}

void FlyEmDataForm::createContextMenu()
{
  if (m_neuronContextMenu == NULL) {
    m_neuronContextMenu = new QMenu(this);
    m_neuronContextMenu->addAction(m_showSelectedModelAction);
    m_neuronContextMenu->addAction(m_showSelectedModelWithBoundBoxAction);
    m_neuronContextMenu->addAction(m_changeClassAction);
    m_neuronContextMenu->addAction(m_neighborSearchAction);
    m_neuronContextMenu->addAction(m_showSelectedBodyAction);
  }

  if (m_secondaryNeuronContextMenu == NULL) {
    m_secondaryNeuronContextMenu = new QMenu(this);
    m_secondaryNeuronContextMenu->addAction(m_showSecondarySelectedModelAction);
  }
}

void FlyEmDataForm::createAction()
{
  if (m_showSelectedModelAction == NULL) {
    m_showSelectedModelAction = new QAction("Show Model", this);
    connect(m_showSelectedModelAction, SIGNAL(triggered()),
            this, SLOT(showSelectedModel()));
  }

  if (m_showSelectedBodyAction == NULL) {
    m_showSelectedBodyAction = new QAction("Show Body", this);
    connect(m_showSelectedBodyAction, SIGNAL(triggered()),
            this, SLOT(showSelectedBody()));
  }

  if (m_showSelectedModelWithBoundBoxAction == NULL) {
    m_showSelectedModelWithBoundBoxAction =
        new QAction("Show Model with Bound Box", this);
    connect(m_showSelectedModelWithBoundBoxAction, SIGNAL(triggered()),
            this, SLOT(showSelectedModelWithBoundBox()));
  }

  if (m_showSecondarySelectedModelAction == NULL) {
    m_showSecondarySelectedModelAction = new QAction("Show Model", this);
    connect(m_showSecondarySelectedModelAction, SIGNAL(triggered()),
            this, SLOT(showSecondarySelectedModel()));
  }

  if (m_changeClassAction == NULL) {
    m_changeClassAction = new QAction("Change Class", this);
    connect(m_changeClassAction, SIGNAL(triggered()),
            this, SLOT(changeNeuronClass()));
  }

  if (m_neighborSearchAction == NULL) {
    m_neighborSearchAction = new QAction("Search neighbor", this);
    connect(m_neighborSearchAction, SIGNAL(triggered()),
            this, SLOT(searchNeighborNeuron()));
  }
}

void FlyEmDataForm::showNearbyNeuron()
{
  QItemSelectionModel *sel = ui->queryView->selectionModel();

  QModelIndexList indexList = sel->selectedIndexes();

  foreach (QModelIndex index, indexList) {
    if (m_neuronList->isNeuronKey(index)) {
      emit showNearbyNeuronTriggered(m_neuronList->getNeuron(index));
      break;
    }
  }
}

void FlyEmDataForm::searchNeighborNeuron()
{
  QItemSelectionModel *sel = ui->queryView->selectionModel();

  QModelIndexList indexList = sel->selectedIndexes();

  foreach (QModelIndex index, indexList) {
    if (m_neuronList->isNeuronKey(index)) {
      emit searchNeighborNeuronTriggered(m_neuronList->getNeuron(index));
      break;
    }
  }
}

void FlyEmDataForm::updateQueryTable()
{
  m_neuronList->notifyAllDataChanged();
}

void FlyEmDataForm::updateSlaveQueryTable()
{
  m_secondaryNeuronList->notifyAllDataChanged();
}

void FlyEmDataForm::updateThumbnail(const QModelIndex &index)
{
  ZFlyEmNeuron *neuron = m_neuronList->getNeuron(index);
  updateThumbnail(neuron);
}

QList<int> FlyEmDataForm::getSelectedNeuronList() const
{
  QList<int> neuronIdList;

  QItemSelectionModel *sel = ui->queryView->selectionModel();
  QVector<ZFlyEmNeuron*> neuronArray =
      m_neuronList->getNeuronArray(sel->selectedIndexes());
  foreach (ZFlyEmNeuron* neuron, neuronArray) {
    neuronIdList.append(neuron->getId());
  }

  return neuronIdList;
}

void FlyEmDataForm::updateThumbnail(
    QList<QGraphicsItem *> itemList, int bodyId)
{
  QList<int> bodyList = getSelectedNeuronList();
  bool itemAdded = false;
  if (!bodyList.isEmpty()) {
    if (bodyList.last() == bodyId) {
      initThumbnailScene();
      foreach (QGraphicsItem* item, itemList) {
        m_thumbnailScene->addItem(item);
      }
      itemAdded = true;
    }
  }
  if (!itemAdded) {
    foreach (QGraphicsItem* item, itemList) {
      delete item;
    }
  }
}

void FlyEmDataForm::updateThumbnailSecondary(const QModelIndex &index)
{ 
  ZFlyEmNeuron *neuron = m_secondaryNeuronList->getNeuron(index);
  updateThumbnail(neuron);
}

void FlyEmDataForm::computeThumbnailFunc(ZFlyEmNeuron *neuron)
{
  ZFlyEmNeuronImageFactory imageFactory = *(getParentFrame()->getImageFactory());
  if (neuron != NULL) {
    if (!neuron->getThumbnailPath().empty()) {
      ZString str(neuron->getThumbnailPath());
      if (str.startsWith("http")) {
        ZDvidTarget target;

        target.setFromSourceString(str);
#if 0
        if (target.getAddress() == "emrecon100.janelia.priv" &&
            target.getUuid() == "2a3") {
          target.setBodyLabelName("sp2body");
        }
#endif
        ZDvidWriter writer;
        if (writer.open(target)) {
          //ZObject3dScan *body = neuron->getBody();
          ZDvidReader reader;
          reader.open(target);
          ZObject3dScan body;
          reader.readBody(neuron->getId(), &body);

          Stack *stack = imageFactory.createSurfaceImage(body);
          writer.writeThumbnail(neuron->getId(), stack);

          ZFlyEmNeuronBodyInfo bodyInfo;
          bodyInfo.setBodySize(body.getVoxelNumber());
          bodyInfo.setBoundBox(body.getBoundBox());
          writer.writeBodyInfo(neuron->getId(), bodyInfo.toJsonObject());

          //QList<QGraphicsItem *> currentItemList;
          //generateThumbnailItem(currentItemList, neuron);
        }
      }
    }
  }
}

Stack* FlyEmDataForm::loadThumbnailImage(ZFlyEmNeuron *neuron)
{
  Stack *stack;
  if (ZFileType::fileType(neuron->getThumbnailPath()) ==
      ZFileType::TIFF_FILE) {
    stack = C_Stack::readSc(neuron->getThumbnailPath().c_str());
  } else {
    ZString str(neuron->getThumbnailPath());
    if (str.startsWith("http")) {
      ZDvidTarget target;
      target.setFromSourceString(str);
      ZDvidReader reader;
      if (reader.open(target)) {
        bool isDataReady = false;
        if (reader.hasBodyInfo(neuron->getId())) {
          ZStack *stackObj = reader.readThumbnail(neuron->getId());
          if (stackObj != NULL) {
            stack = C_Stack::clone(stackObj->c_stack());
            delete stackObj;
            isDataReady = true;
          }
        }
      }
    }
  }

  return stack;
}

void FlyEmDataForm::generateThumbnailItem(
    QList<QGraphicsItem *> currentItemList, ZFlyEmNeuron *neuron)
{
  bool thumbnailReady = false;
  if (neuron != NULL) {
    if (!neuron->getThumbnailPath().empty()) {
      QGraphicsPixmapItem *thumbnailItem = new QGraphicsPixmapItem;
      QPixmap pixmap;
      if (pixmap.load(neuron->getThumbnailPath().c_str())) {
        thumbnailReady = true;
      } else {
        Stack *stack = loadThumbnailImage(neuron);

        if (stack != NULL) {
          ZImage image(C_Stack::width(stack), C_Stack::height(stack));
          image.setData(C_Stack::array8(stack));
          if (pixmap.convertFromImage(image)) {
            thumbnailReady = true;
          } else {
            dump("Failed to load the thumbnail.");
          }
          C_Stack::kill(stack);
        } else {
          //dump("Failed to load the thumbnail.");
        }
      }

      if (thumbnailReady) {
        thumbnailItem->setPixmap(pixmap);
        QTransform transform;

        double sceneWidth = m_thumbnailScene->width();
        double sceneHeight = m_thumbnailScene->height();
        double scale = std::min(
              double(sceneWidth - 10) / pixmap.width(),
              double(sceneHeight - 10) / pixmap.height());
        if (scale > 5) {
          scale = 5;
        }
        transform.scale(scale, scale);
        thumbnailItem->setTransform(transform);
        currentItemList.append(thumbnailItem);

        int sourceZDim = getParentFrame()->
            getMasterData()->getSourceDimension(NeuTube::Z_AXIS);
        int sourceYDim = getParentFrame()->getMasterData()->
            getSourceDimension(NeuTube::Y_AXIS);

        ZDvidTarget dvidTarget;
        dvidTarget.setFromSourceString(neuron->getThumbnailPath());
        ZDvidReader reader;
        if (reader.open(dvidTarget)) {
          ZFlyEmNeuronBodyInfo bodyInfo =
              reader.readBodyInfo(neuron->getId());
          int startY = bodyInfo.getBoundBox().getFirstCorner().getY();
          int startZ = bodyInfo.getBoundBox().getFirstCorner().getZ();
          int bodyHeight = bodyInfo.getBoundBox().getDepth();
          int bodySpan = bodyInfo.getBoundBox().getHeight();

          if (sourceZDim == 0) {
            QGraphicsTextItem *textItem = new QGraphicsTextItem;
            textItem->setHtml(
                  QString("<p><font color=\"red\">Z = %1</font></p>"
                          "<p><font color=\"red\">Height = %2</font></p>").
                  arg(startZ).arg(bodyHeight));
            currentItemList.append(textItem);
//            m_thumbnailScene->addItem(textItem);
          } else { //Draw range rect
            double x = 10;
            double y = 10;

            double scale = sceneWidth * 0.5 / sourceYDim;
            double height = sourceZDim * scale;
            double width = sourceYDim * scale;

            QGraphicsRectItem *rectItem = new QGraphicsRectItem(
                  sceneWidth - width - x, y, width, height);
            rectItem->setPen(QPen(QColor(255, 0, 0, 164)));
            currentItemList.append(rectItem);
//            m_thumbnailScene->addItem(rectItem);

            int z0 = getParentFrame()->
                getMasterData()->getSourceOffset(NeuTube::Z_AXIS);
            int y0 = getParentFrame()->
                getMasterData()->getSourceOffset(NeuTube::Y_AXIS);
            rectItem = new QGraphicsRectItem(
                  sceneWidth - width - x + (startY - y0) * scale,
                  y + (startZ - z0) * scale,
                  bodySpan * scale, bodyHeight * scale);
            rectItem->setPen(QPen(QColor(0, 255, 0, 164)));
            currentItemList.append(rectItem);
//            m_thumbnailScene->addItem(rectItem);
          }
        }
      } else {
        //fire off computation thread
        QString threadId =
            QString("computeThumbnailFunc:%1").arg(neuron->getId());
        QFuture<void> future =
            QtConcurrent::run(
              this, &FlyEmDataForm::computeThumbnailFunc, neuron);
        m_threadFutureMap[threadId] = future;
      }
    }
    emit thumbnailItemReady(currentItemList, neuron->getId());
  }
}

void FlyEmDataForm::updateThumbnailLive(ZFlyEmNeuron *neuron)
{
  if (neuron == NULL) {
    return;
  }

  //initThumbnailScene();

  bool isWaiting = false;
  QString threadId =
      QString("computeThumbnailFunc:%1").arg(neuron->getId());
  QList<QGraphicsItem*> itemList;

  if (m_threadFutureMap.contains(threadId)) {
    if (m_threadFutureMap[threadId].isRunning()) {
      isWaiting = true;
      QGraphicsTextItem *textItem = new QGraphicsTextItem;
      textItem->setHtml(
            QString("<p><font color=\"green\">The thumbail is being computed.</font></p>"
                    "<p><font color=\"green\">It may take several minutes.</p>"
                    "<p><font color=\"green\">Please come back later.</font></p>"));

      itemList.append(textItem);
      //m_thumbnailScene->addItem(textItem);
    }
  } else {
    generateThumbnailItem(itemList, neuron);
  }
#if 0
  bool thumbnailReady = false;
  if (neuron != NULL && !isWaiting) {
    if (!neuron->getThumbnailPath().empty()) {
      QGraphicsPixmapItem *thumbnailItem = new QGraphicsPixmapItem;
      QPixmap pixmap;
      if (pixmap.load(neuron->getThumbnailPath().c_str())) {
        thumbnailReady = true;
      } else {
        Stack *stack = NULL;
        if (ZFileType::fileType(neuron->getThumbnailPath()) ==
            ZFileType::TIFF_FILE) {
          stack = C_Stack::readSc(neuron->getThumbnailPath().c_str());
        } else {
          ZString str(neuron->getThumbnailPath());
          if (str.startsWith("http")) {
            ZDvidTarget target;
            target.setFromSourceString(str);
            ZDvidReader reader;
            if (reader.open(target)) {
              bool isDataReady = false;
              if (reader.hasBodyInfo(neuron->getId())) {
                ZStack *stackObj = reader.readThumbnail(neuron->getId());
                if (stackObj != NULL) {
                  stack = C_Stack::clone(stackObj->c_stack());
                  delete stackObj;
                  isDataReady = true;
                }
              }

              if (!isDataReady) {
                //fire off computation thread
                QFuture<void> future =
                    QtConcurrent::run(
                      this, &FlyEmDataForm::computeThumbnailFunc, neuron);
                m_threadFutureMap[threadId] = future;
                QGraphicsTextItem *textItem = new QGraphicsTextItem;
                QString font = "color=\"green\"";
                textItem->setHtml(
                      QString(
                        "<p><font %1>The thumbail is being computed.</font></p>"
                        "<p><font %1>It may take several minutes.</font></p>"
                        "<p><font %1>Please come back later.</font></p>").
                      arg(font));
                m_thumbnailScene->addItem(textItem);
              }
            }
          }
        }

        if (stack != NULL) {
          ZImage image(C_Stack::width(stack), C_Stack::height(stack));
          image.setData(C_Stack::array8(stack));
#ifdef _DEBUG_2
          image.save((GET_DATA_DIR + "/test.png").c_str());
#endif
          if (pixmap.convertFromImage(image)) {
            thumbnailReady = true;
          } else {
            dump("Failed to load the thumbnail.");
          }
          C_Stack::kill(stack);
        } else {
          //dump("Failed to load the thumbnail.");
        }
      }

      if (thumbnailReady) {
        thumbnailItem->setPixmap(pixmap);
        QTransform transform;

        double sceneWidth = m_thumbnailScene->width();
        double sceneHeight = m_thumbnailScene->height();
        double scale = std::min(
              double(sceneWidth - 10) / pixmap.width(),
              double(sceneHeight - 10) / pixmap.height());
        if (scale > 5) {
          scale = 5;
        }
        transform.scale(scale, scale);
        thumbnailItem->setTransform(transform);
        m_thumbnailScene->addItem(thumbnailItem);

        int sourceZDim = getParentFrame()->
            getMasterData()->getSourceDimension(NeuTube::Z_AXIS);
        int sourceYDim = getParentFrame()->getMasterData()->
            getSourceDimension(NeuTube::Y_AXIS);

        ZDvidTarget dvidTarget;
        dvidTarget.setFromSourceString(neuron->getThumbnailPath());
        ZDvidReader reader;
        if (reader.open(dvidTarget)) {
          ZFlyEmNeuronBodyInfo bodyInfo =
              reader.readBodyInfo(neuron->getId());
          int startY = bodyInfo.getBoundBox().getFirstCorner().getY();
          int startZ = bodyInfo.getBoundBox().getFirstCorner().getZ();
          int bodyHeight = bodyInfo.getBoundBox().getDepth();
          int bodySpan = bodyInfo.getBoundBox().getHeight();

          if (sourceZDim == 0) {
            QGraphicsTextItem *textItem = new QGraphicsTextItem;
            textItem->setHtml(
                  QString("<p><font color=\"red\">Z = %1</font></p>"
                          "<p><font color=\"red\">Height = %2</font></p>").
                  arg(startZ).arg(bodyHeight));
            m_thumbnailScene->addItem(textItem);
          } else { //Draw range rect
            double x = 10;
            double y = 10;

            double scale = sceneWidth * 0.5 / sourceYDim;
            double height = sourceZDim * scale;
            double width = sourceYDim * scale;

            QGraphicsRectItem *rectItem = new QGraphicsRectItem(
                  sceneWidth - width - x, y, width, height);
            rectItem->setPen(QPen(QColor(255, 0, 0, 164)));
            m_thumbnailScene->addItem(rectItem);

            int z0 = getParentFrame()->
                getMasterData()->getSourceOffset(NeuTube::Z_AXIS);
            int y0 = getParentFrame()->
                getMasterData()->getSourceOffset(NeuTube::Y_AXIS);
            rectItem = new QGraphicsRectItem(
                  sceneWidth - width - x + (startY - y0) * scale,
                  y + (startZ - z0) * scale,
                  bodySpan * scale, bodyHeight * scale);
            rectItem->setPen(QPen(QColor(0, 255, 0, 164)));
            m_thumbnailScene->addItem(rectItem);
          }
        }
      }
    }
  }
#endif
}

void FlyEmDataForm::updateThumbnail(ZFlyEmNeuron *neuron)
{
  if (neuron == NULL) {
    return;
  }

  initThumbnailScene();

  bool isWaiting = false;
  QString threadId =
      QString("computeThumbnailFunc:%1").arg(neuron->getId());
  if (m_threadFutureMap.contains(threadId)) {
    if (m_threadFutureMap[threadId].isRunning()) {
      isWaiting = true;
      QGraphicsTextItem *textItem = new QGraphicsTextItem;
      textItem->setHtml(
            QString("<p><font color=\"green\">The thumbail is being computed.</font></p>"
                    "<p><font color=\"green\">It may take several minutes.</p>"
                    "<p><font color=\"green\">Please come back later.</font></p>"));
      m_thumbnailScene->addItem(textItem);
    }
  }

  bool thumbnailReady = false;
  if (neuron != NULL && !isWaiting) {
    if (!neuron->getThumbnailPath().empty()) {
      QGraphicsPixmapItem *thumbnailItem = new QGraphicsPixmapItem;
      QPixmap pixmap;
      if (pixmap.load(neuron->getThumbnailPath().c_str())) {
        thumbnailReady = true;
      } else {
        Stack *stack = NULL;
        if (ZFileType::fileType(neuron->getThumbnailPath()) ==
            ZFileType::TIFF_FILE) {
          stack = C_Stack::readSc(neuron->getThumbnailPath().c_str());
        } else {
          ZString str(neuron->getThumbnailPath());
          if (str.startsWith("http")) {
            ZDvidTarget target;
            target.setFromSourceString(str);
            ZDvidReader reader;
            if (reader.open(target)) {
              bool isDataReady = false;
              if (reader.hasBodyInfo(neuron->getId())) {
#ifdef _DEBUG_
                std::cout << "Body info available. Reading thumbnail ..." << std::endl;
#endif
                ZStack *stackObj = reader.readThumbnail(neuron->getId());
                if (stackObj != NULL) {
                  stack = C_Stack::clone(stackObj->c_stack());
                  delete stackObj;
                  isDataReady = true;
                }
              }

              if (!isDataReady) {
                //fire off computation thread
                QFuture<void> future =
                    QtConcurrent::run(
                      this, &FlyEmDataForm::computeThumbnailFunc, neuron);
                m_threadFutureMap[threadId] = future;
                QGraphicsTextItem *textItem = new QGraphicsTextItem;
                QString font = "color=\"green\"";
                textItem->setHtml(
                      QString(
                        "<p><font %1>The thumbail is being computed.</font></p>"
                        "<p><font %1>It may take several minutes.</font></p>"
                        "<p><font %1>Please come back later.</font></p>").
                      arg(font));
                m_thumbnailScene->addItem(textItem);
              }
            }
          }
        }

        if (stack != NULL) {
          ZImage image(C_Stack::width(stack), C_Stack::height(stack));
          image.setData(C_Stack::array8(stack));
#ifdef _DEBUG_2
          image.save((GET_DATA_DIR + "/test.png").c_str());
#endif
          if (pixmap.convertFromImage(image)) {
            thumbnailReady = true;
          } else {
            dump("Failed to load the thumbnail.");
          }
          C_Stack::kill(stack);
        } else {
          //dump("Failed to load the thumbnail.");
        }
      }

      if (thumbnailReady) {
        thumbnailItem->setPixmap(pixmap);
        QTransform transform;

        double sceneWidth = m_thumbnailScene->width();
        double sceneHeight = m_thumbnailScene->height();
        double scale = std::min(
              double(sceneWidth - 10) / pixmap.width(),
              double(sceneHeight - 10) / pixmap.height());
        if (scale > 5) {
          scale = 5;
        }
        transform.scale(scale, scale);
        thumbnailItem->setTransform(transform);
        m_thumbnailScene->addItem(thumbnailItem);

        int sourceZDim = getParentFrame()->
            getMasterData()->getSourceDimension(NeuTube::Z_AXIS);
        int sourceYDim = getParentFrame()->getMasterData()->
            getSourceDimension(NeuTube::Y_AXIS);

        ZDvidTarget dvidTarget;
        dvidTarget.setFromSourceString(neuron->getThumbnailPath());
        ZDvidReader reader;
        if (reader.open(dvidTarget)) {
          ZFlyEmNeuronBodyInfo bodyInfo =
              reader.readBodyInfo(neuron->getId());
          int startY = bodyInfo.getBoundBox().getFirstCorner().getY();
          int startZ = bodyInfo.getBoundBox().getFirstCorner().getZ();
          int bodyHeight = bodyInfo.getBoundBox().getDepth();
          int bodySpan = bodyInfo.getBoundBox().getHeight();

          if (sourceZDim == 0) {
            QGraphicsTextItem *textItem = new QGraphicsTextItem;
            textItem->setHtml(
                  QString("<p><font color=\"red\">Z = %1</font></p>"
                          "<p><font color=\"red\">Height = %2</font></p>").
                  arg(startZ).arg(bodyHeight));
            m_thumbnailScene->addItem(textItem);
          } else { //Draw range rect
            double x = 10;
            double y = 10;

            double scale = sceneWidth * 0.5 / sourceYDim;
            double height = sourceZDim * scale;
            double width = sourceYDim * scale;

            QGraphicsRectItem *rectItem = new QGraphicsRectItem(
                  sceneWidth - width - x, y, width, height);
            rectItem->setPen(QPen(QColor(255, 0, 0, 164)));
            m_thumbnailScene->addItem(rectItem);

            int z0 = getParentFrame()->
                getMasterData()->getSourceOffset(NeuTube::Z_AXIS);
            int y0 = getParentFrame()->
                getMasterData()->getSourceOffset(NeuTube::Y_AXIS);
            rectItem = new QGraphicsRectItem(
                  sceneWidth - width - x + (startY - y0) * scale,
                  y + (startZ - z0) * scale,
                  bodySpan * scale, bodyHeight * scale);
            rectItem->setPen(QPen(QColor(0, 255, 0, 164)));
            m_thumbnailScene->addItem(rectItem);
          }
        }
      }
    }
  }
}

void FlyEmDataForm::dump(const QString &message)
{
  ui->outputTextEdit->clear();
  appendOutput("<p>" + message + "</p>");
  //QApplication::processEvents();
}

void FlyEmDataForm::initThumbnailScene()
{
  m_thumbnailScene->clear();
  m_thumbnailScene->setSceneRect(ui->thumbnailView->viewport()->rect());
  m_thumbnailScene->setBackgroundBrush(QBrush(Qt::gray));
}

void FlyEmDataForm::resizeEvent(QResizeEvent *)
{
  initThumbnailScene();
}

void FlyEmDataForm::showEvent(QShowEvent *)
{
  initThumbnailScene();
}

void FlyEmDataForm::saveVolumeRenderingFigure(
    ZFlyEmNeuron *neuron, const QString &output, const QString cameraFile)
{
  if (neuron != NULL) {
    ZObject3dScan *obj = neuron->getBody();

    int dsIntv = 3;
    obj->downsampleMax(dsIntv, dsIntv, dsIntv);

    ZStack *stack = obj->toStackObject(255);
    neuron->deprecate(ZFlyEmNeuron::BODY);

    ZSharedPointer<ZStackDoc> academy =
        ZSharedPointer<ZStackDoc>(new ZStackDoc);
    academy->loadStack(stack);

    int maxX = 0;
    int maxY = 0;
    int maxZ = 8089;
    ZFlyEmDataFrame *parentFrame = getParentFrame();
    ZDvidReader dvidReader;
    if (dvidReader.open(parentFrame->getDvidTarget())) {
      ZDvidInfo dvidInfo = dvidReader.readGrayScaleInfo();
      maxZ = dvidInfo.getMaxZ();
      maxX = dvidInfo.getMaxX();
      maxY = dvidInfo.getMaxY();
    }

    int dataRangeZ = (maxZ + 1) / (dsIntv + 1);
    int dataRangeX = (maxX + 1) / (dsIntv + 1);
    int dataRangeY = (maxY + 1) / (dsIntv + 1);

    Z3DWindow *stage = new Z3DWindow(academy, Z3DWindow::FULL_RES_VOLUME,
                                     false, NULL);

    stage->getVolumeRaycaster()->hideBoundBox();
    stage->getVolumeRaycasterRenderer()->setCompositeMode(
          "Direct Volume Rendering");
    stage->getAxis()->setVisible(false);

    Z3DCameraParameter* camera = stage->getCamera();


    ZPoint vec = ZPoint(0, 1, 0);

    glm::vec3 upVector = glm::vec3(0, 0, -1);

    if (!cameraFile.isEmpty()) {
      ZJsonObject cameraJson;
      cameraJson.load(cameraFile.toStdString());
      camera->set(cameraJson);
      glm::vec3 eyeSpec = camera->getEye();
      glm::vec3 centerSpec = camera->getCenter();
      vec = ZPoint(eyeSpec[0], eyeSpec[1], eyeSpec[2]) -
              ZPoint(centerSpec[0], centerSpec[1], centerSpec[2]);
      vec.normalize();

      upVector = camera->getUpVector();
    }

    camera->setProjectionType(Z3DCamera::Orthographic);
//    glm::vec3 eyeSpec = camera->getEye();
//    glm::vec3 centerSpec = camera->getCenter();

    ZPoint referenceCenter;
    referenceCenter.set(dataRangeX / 2, dataRangeY / 2, dataRangeZ / 2);

    double distNearToCenter = referenceCenter.length() * 2.0;
    double distEyeToNear = dataRangeZ * 0.5 / tan(camera->getFieldOfView() * 0.5);
    double distEyeToCenter = distEyeToNear + distNearToCenter;

    ZPoint eyePosition = referenceCenter + vec * distEyeToCenter;
    camera->setCenter(glm::vec3(referenceCenter[0], referenceCenter[1],
        referenceCenter[2]));
    camera->setEye(glm::vec3(eyePosition[0], eyePosition[1], eyePosition[2]));

    camera->setUpVector(upVector);

   // double eyeDistance = eyeDistance;//boundBox[3] - referenceCenter[1] + 2500;
    //double eyeDistance = 2000 - referenceCenter[1];
//    glm::vec3 viewVector(vec.x(), vec.y(), vec.z());

//    viewVector *= eyeDistance;
//    glm::vec3 eyePosition = referenceCenter - viewVector;

//    referenceCenter[2] = dataRangeZ / 2;// - stack->getOffset().getZ();

//    eyePosition[2] = dataRangeZ / 2;// - stack->getOffset().getZ();



    stage->resetCameraClippingRange();
    camera->setNearDist(distEyeToNear);

    stage->getCompositor()->setBackgroundFirstColor(0, 0, 0, 1);
    stage->getCompositor()->setBackgroundSecondColor(0, 0, 0, 1);

//    stage->show();
    //stage->showMaximized();

    stage->takeScreenShot(output, 4000, 4000, MonoView);
    stage->close();
    delete stage;
  }
}

void FlyEmDataForm::exportVolumeRenderingFigure()
{
  QString dirpath = QFileDialog::getExistingDirectory(this, tr("Export Figure"),
    "", QFileDialog::ShowDirsOnly);

  if (!dirpath.isEmpty()) {
    QString cameraFile;
    if(QMessageBox::information(
         this, "Camera Setting", "Do you want to sepcify camera settings?",
         QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes) {
      cameraFile = QFileDialog::getOpenFileName(
            this, tr("Load Camera Settings"), "", "*.json");
    }

    QItemSelectionModel *sel = ui->queryView->selectionModel();
    QVector<ZFlyEmNeuron*> neuronArray =
        m_neuronList->getNeuronArray(sel->selectedIndexes());
    dump(QString("Saving %1 figures ...").arg(neuronArray.size()));
    foreach (ZFlyEmNeuron *neuron, neuronArray) {
      QString output = dirpath + QString("/body_%1.tif").arg(neuron->getId());
      saveVolumeRenderingFigure(neuron, output, cameraFile);
      dump(output + " saved");
    }
    dump("Done.");
  }
}

void FlyEmDataForm::exportTypeLabelFile()
{
  QString fileName =
      QFileDialog::getOpenFileName(this, "Export Type Labels", "", "*.json");
  if (!fileName.isEmpty()) {
    //
  }
}
