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

#include "neutubeconfig.h"
#include "tz_error.h"
#include "zswctree.h"
#include "zstackdoc.h"
#include "ui_flyemdataform.h"
#include "flyem/zflyemdataframe.h"
#include "flyem/zflyemstackframe.h"
#include "zimagewidget.h"
#include "zfiletype.h"
#include "zimage.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"
#include "flyem/zflyemneuronbodyinfo.h"

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
#ifdef _DEBUG_
  connect(ui->queryView, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(assignClass(QModelIndex)));
#endif
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

  //customize
  //ui->testPushButton->hide();
  ui->generalPushButton->hide();

  createAction();
  createContextMenu();
  ui->queryView->setContextMenu(m_neuronContextMenu);
  ui->slaveQueryView->setContextMenu(m_secondaryNeuronContextMenu);

  m_specialProgressReporter.setProgressBar(ui->progressBar);
  setProgressReporter(&m_specialProgressReporter);

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  //m_thumbnailImage = new QImage;
  //m_thumbnailImage->load(":/images/hideobj.png");
  m_thumbnailScene = new QGraphicsScene;
  ui->thumbnailView->setScene(m_thumbnailScene);
  /*
  m_thumbnailWidget = new ZImageWidget(ui->thumnailContainer, m_thumbnailImage);
  m_thumbnailWidget->setProjRegion(
        QRect(QPoint(0, 0), ui->thumnailContainer->size()));
        */

  ui->overallLayout->setMargin(10);
  ui->thumbnailView->resize(256, ui->thumbnailView->height());
  ui->outputTextEdit->resize(190, ui->outputTextEdit->height());
  setLayout(ui->overallLayout);
}

FlyEmDataForm::~FlyEmDataForm()
{
  //delete m_thumbnailImage;
  delete ui;
  delete m_thumbnailScene;
}

QSize FlyEmDataForm::sizeHint() const
{
#ifdef _DEBUG_2
  std::cout << geometry().x() << " " << geometry().y()
            << geometry().width() << " " << geometry().height() << std::endl;
#endif

  return geometry().size();
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
    if (m_neuronList->getColumnName(index.column()) == "Class") {
      const ZFlyEmNeuron *neuron = m_neuronList->getNeuron(index.row());
      if (neuron->hasClass()) {
        m_secondaryNeuronList->clear();
        ZFlyEmDataBundle *bundle = frame->getDataBundle();
        std::vector<ZFlyEmNeuron> &neuronArray = bundle->getNeuronArray();
        for (std::vector<ZFlyEmNeuron>::iterator iter = neuronArray.begin();
             iter != neuronArray.end(); ++iter) {
          ZFlyEmNeuron &buddyNeuron = *iter;
          if (buddyNeuron.getClass() == neuron->getClass()) {
            m_secondaryNeuronList->append(&buddyNeuron);
          }
        }
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
        className = topMatch[0]->getClass().c_str();
      }

      //Popup dialog
      bool ok;
      className = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                        tr("Class name:"), QLineEdit::Normal,
                                        className, &ok);
      if (ok) {
        neuron->setClass(className.toStdString());
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

  ui->progressBar->setValue(50);
  ui->progressBar->show();
  //QApplication::processEvents();

  //const ZFlyEmNeuron *neuron = m_neuronList->getNeuron(index);
  QVector<const ZFlyEmNeuron*> neuronArray =
      m_neuronList->getNeuronArray(index);

#ifdef _DEBUG_2
  std::cout << m_neuronList->headerData(index.column(), Qt::Horizontal).toString().toStdString()
             << std::endl;

#endif

  if (!neuronArray.isEmpty()) {
    ZStackFrame *frame = new ZStackFrame;

    foreach (const ZFlyEmNeuron* neuron, neuronArray) {
      //doc->addSwcTree(neuron->getModel()->clone(), true);
      ZSwcTree *model = neuron->getModel();
      ZStackDoc *doc = frame->document().get();
      doc->blockSignals(true);
      if (model != NULL) {
        doc->addSwcTree(model->clone(), true);
      }

      std::vector<ZPunctum*> puncta = neuron->getSynapse();
      for (std::vector<ZPunctum*>::iterator iter = puncta.begin();
           iter != puncta.end(); ++iter) {
        doc->addPunctum(*iter);
      }
      doc->blockSignals(false);
      doc->updateModelData(ZStackDoc::SWC_DATA);
      doc->updateModelData(ZStackDoc::PUNCTA_DATA);
    }

    frame->open3DWindow(this->parentWidget());
    delete frame;
  } else if (m_neuronList->headerData(index.column(), Qt::Horizontal) ==
             "Volume Path") {
    if (!m_neuronList->data(index).toString().isEmpty()) {
      emit volumeTriggered(m_neuronList->data(index).toString());
    }
  }

  ui->progressBar->hide();
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

  ZStackFrame *frame = new ZStackFrame;

  view->getModel()->retrieveModel(sel->selectedIndexes(), frame->document().get());
  ui->progressBar->setValue(75);
  //QApplication::processEvents();

  frame->open3DWindow(this->parentWidget());
  ZStackDoc *hostDoc = frame->document().get();

  delete frame;

  ui->progressBar->hide();

  return hostDoc;
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

  ZStackFrame *frame = new ZStackFrame;

  view->getModel()->retrieveBody(sel->selectedIndexes(), frame->document().get());
  ui->progressBar->setValue(75);
  //QApplication::processEvents();

  frame->open3DWindow(this->parentWidget());
  ZStackDoc *hostDoc = frame->document().get();

  delete frame;

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


void FlyEmDataForm::showSelectedModelWithBoundBox()
{
  ZStackDoc *hostDoc = showViewSelectedModel(ui->queryView);
  ZFlyEmDataFrame *frame = getParentFrame();
  if (frame != NULL) {
    ZFlyEmDataBundle *dataBundle = frame->getDataBundle();
    if (!dataBundle->getBoundBox().isEmpty()) {
      hostDoc->addSwcTree(dataBundle->getBoundBox().clone());
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
  QString dirpath = QFileDialog::getExistingDirectory(this, tr("Export SWC"),
    "", QFileDialog::ShowDirsOnly);

  if (!dirpath.isEmpty()) {
    m_neuronList->exportSwc(dirpath);
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
            neuron->setClass(className.toStdString());
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

void FlyEmDataForm::updateThumbnailSecondary(const QModelIndex &index)
{ 
  ZFlyEmNeuron *neuron = m_secondaryNeuronList->getNeuron(index);
  updateThumbnail(neuron);
}

void FlyEmDataForm::computeThumbnailFunc(ZFlyEmNeuron *neuron)
{
  if (neuron != NULL) {
    if (!neuron->getThumbnailPath().empty()) {
      ZString str(neuron->getThumbnailPath());
      if (str.startsWith("http")) {
        ZDvidTarget target;
        target.setFromSourceString(str);
        ZDvidWriter writer;
        if (writer.open(target)) {
          //ZObject3dScan *body = neuron->getBody();
          ZDvidReader reader;
          reader.open(target);
          ZObject3dScan body;
          reader.readBody(neuron->getId(), &body);

          Stack *stack =
              getParentFrame()->getImageFactory()->createSurfaceImage(body);
          writer.writeThumbnail(neuron->getId(), stack);

          ZFlyEmNeuronBodyInfo bodyInfo;
          bodyInfo.setBodySize(body.getVoxelNumber());
          bodyInfo.setBoundBox(body.getBoundBox());
          writer.writeBodyInfo(neuron->getId(), bodyInfo.toJsonObject());

          //neuron->deprecate(ZFlyEmNeuron::BODY);
        }
      }
    }
  }
}

void FlyEmDataForm::updateThumbnail(ZFlyEmNeuron *neuron)
{
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

                /*

                stack = getParentFrame()->getImageFactory()->createSurfaceImage(
                      *neuron->getBody());
                ZDvidWriter writer;
                if (writer.open(target)) {
                  writer.writeThumbnail(neuron->getId(), stack);
                }
                */
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
#if 0
        ZObject3dScan *body = neuron->getBody();
        int startZ = body->getMinZ();
        int bodyHeight = body->getMaxZ() - startZ + 1;
        int startY = body->getMinY();
        int bodySpan = body->getMaxY() - startY + 1;
        neuron->deprecate(ZFlyEmNeuron::BODY);
#endif

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
  appendOutput("<p>" + message + "</p>");
  //QApplication::processEvents();
}

void FlyEmDataForm::initThumbnailScene()
{
  m_thumbnailScene->clear();
  m_thumbnailScene->setSceneRect(ui->thumbnailView->viewport()->rect());
  m_thumbnailScene->setBackgroundBrush(QBrush(Qt::gray));

  /*

  double x0 = 0;
  double y0 = 0;
  double w = ui->thumbnailView->size().width() - 10;
  double h = ui->thumbnailView->size().height() - 10;

  m_thumbnailScene->setSceneRect(x0, y0, w, h);
  m_thumbnailScene->addRect(
        m_thumbnailScene->sceneRect(), QPen(QColor(255, 0, 0)));

  QGraphicsRectItem *item = new QGraphicsRectItem;
  int sourceWidth = 400;
  int sourceHeight = 8000;

  item->setRect(QRect(100, 1000, sourceWidth, sourceHeight));
  double ratio = std::min(double(sourceWidth) / w, double(sourceHeight) / h);
  */
}

void FlyEmDataForm::resizeEvent(QResizeEvent *)
{
  initThumbnailScene();
}

void FlyEmDataForm::showEvent(QShowEvent *)
{
  //initThumbnailScene();
}
