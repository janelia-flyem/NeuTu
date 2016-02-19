#include "flyembodysplitprojectdialog.h"
#include <QFileDialog>
#include <QProgressDialog>
#include <QtConcurrentRun>
#include <QMenu>
#include <QAction>
#include <QScrollBar>
#include <QKeyEvent>

#include "ui_flyembodysplitprojectdialog.h"
#include "mainwindow.h"
#include "zstackframe.h"
#include "zstackview.h"
#include "zstackdoc.h"
#include "zflyemnewbodysplitprojectdialog.h"
#include "dvid/zdvidreader.h"
#include "zstackskeletonizer.h"
#include "neutubeconfig.h"
#include "zimage.h"
#include "flyem/zflyemneuronimagefactory.h"
#include "zdviddialog.h"
#include "zdialogfactory.h"
#include "dvid/zdvidwriter.h"
#include "flyem/zflyemneuronbodyinfo.h"
#include "zstackpatch.h"
#include "dvid/zdviddata.h"
#include "zmessage.h"
#include "zmessagemanager.h"

FlyEmBodySplitProjectDialog::FlyEmBodySplitProjectDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmBodySplitProjectDialog)
{
  ui->setupUi(this);

  m_loadBodyDlg = NULL;

  //connect(this, SIGNAL(accepted()), this, SLOT(clear()));
  connect(ui->view2dBodyPushButton, SIGNAL(clicked()),
          this, SLOT(showData2d()));
  connect(ui->view3dBodyPushButton, SIGNAL(clicked()),
          this, SLOT(showData3d()));
  connect(ui->viewSplitPushButton,
          SIGNAL(clicked()), this, SLOT(showResult3d()));
  connect(ui->viewResultQuickPushButton,
          SIGNAL(clicked()), this, SLOT(showResult3dQuick()));
  connect(ui->donePushButton, SIGNAL(clicked()), this, SLOT(clear()));
  connect(ui->loadBodyPushButton, SIGNAL(clicked()), this, SLOT(loadBody()));
  connect(ui->loadBookmarkButton, SIGNAL(clicked()),
          this, SLOT(loadBookmark()));
  connect(ui->bookmarkVisibleCheckBox, SIGNAL(toggled(bool)),
          &m_project, SLOT(showBookmark(bool)));
  connect(ui->quickViewPushButton, SIGNAL(clicked()), this, SLOT(quickView()));
  connect(ui->fullGrayscaleCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(viewFullGrayscale(bool)));
  connect(ui->updatePushButton, SIGNAL(clicked()),
          this, SLOT(viewFullGrayscale()));
  /*
  connect(ui->prevPushButton, SIGNAL(clicked()),
          this, SLOT(viewPreviousSlice()));
  connect(ui->nextPushButton, SIGNAL(clicked()),
          this, SLOT(viewNextSlice()));
  connect(ui->fullGrayscalePushButton, SIGNAL(clicked()),
          this, SLOT(viewFullGrayscale()));
          */
  connect(ui->saveSeedPushButton, SIGNAL(clicked()),
          this, SLOT(saveSeed()));

  ui->bookmarkView->setModel(&m_bookmarkList);

  updateWidget();

  m_project.setBookmarkVisible(ui->bookmarkVisibleCheckBox->isChecked());

  m_sideViewScene = new QGraphicsScene(this);
  //m_sideViewScene->setSceneRect(0, 0, ui->sideView->width(), ui->sideView->height());
  ui->sideView->setScene(m_sideViewScene);
//  ui->sideView->setFocus();

  setFocusPolicy(Qt::StrongFocus);
  //ui->outputWidget->setText("Load a body to start.");

#ifndef _DEBUG_
  ui->pushButton->hide();
#endif

  m_dvidDlg = ZDialogFactory::makeDvidDialog(this);

  createMenu();
  connectSignalSlot();

  m_messageManager = NULL;
}


FlyEmBodySplitProjectDialog::~FlyEmBodySplitProjectDialog()
{
  delete ui;
  //delete m_sideViewScene;
}

void FlyEmBodySplitProjectDialog::connectSignalSlot()
{
  connect(ui->bookmarkView, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(locateBookmark(QModelIndex)));
  /*
  connect(ui->bookmarkView, SIGNAL(pressed(QModelIndex)),
          this, SLOT(showBookmarkContextMenu(QModelIndex)));
*/
  //Progress signal-slot
  connect(this, SIGNAL(progressDone()),
          getMainWindow(), SIGNAL(progressDone()));

  connect(this, SIGNAL(messageDumped(QString,bool)),
          this, SLOT(dump(QString,bool)));
  connect(this, SIGNAL(sideViewCanceled()), this, SLOT(resetSideView()));

  connect(&m_project, SIGNAL(messageGenerated(QString, bool)),
          this, SIGNAL(messageDumped(QString, bool)));

  connect(&m_project, SIGNAL(progressStarted(QString,int)),
          this, SIGNAL(progressStarted(QString,int)));
  connect(&m_project, SIGNAL(progressAdvanced(double)),
          this, SIGNAL(progressAdvanced(double)));
  connect(&m_project, SIGNAL(progressDone()), this, SIGNAL(progressDone()));
//  connect(&m_project, SIGNAL(messageGenerated(QString)),
//          this, SLOT(dump(QString)));
  connect(&m_project, SIGNAL(errorGenerated(QString)),
          this, SLOT(dumpError(QString)));

  connect(this, SIGNAL(progressStarted(QString, int)),
          getMainWindow(), SLOT(startProgress(QString, int)));
  connect(this, SIGNAL(progressAdvanced(double)),
          getMainWindow(), SLOT(advanceProgress(double)));
  connect(this, SIGNAL(progressDone()), getMainWindow(), SLOT(endProgress()));
}

void FlyEmBodySplitProjectDialog::createMenu()
{
  m_mainMenu = new QMenu(this);
  ui->menuPushButton->setMenu(m_mainMenu);

  m_showBodyMaskAction = new QAction("Show Body Mask", this);
  m_mainMenu->addAction(m_showBodyMaskAction);
  m_showBodyMaskAction->setCheckable(true);
  connect(m_showBodyMaskAction, SIGNAL(triggered(bool)),
          this, SLOT(showBodyMask(bool)));

  QAction *removeBookmarkAction = new QAction("Remove All Bookmarks", this);
  m_mainMenu->addAction(removeBookmarkAction);
  connect(removeBookmarkAction, SIGNAL(triggered()),
          this, SLOT(removeAllBookmark()));

  QAction *exportSplitAction = new QAction("Export splits", this);
  m_mainMenu->addAction(exportSplitAction);
  connect(exportSplitAction, SIGNAL(triggered()), this, SLOT(exportSplits()));

  QMenu *seedMenu = m_mainMenu->addMenu("Seed");
  QAction *recoverSeedAction = new QAction("Recover", this);
  seedMenu->addAction(recoverSeedAction);
  connect(recoverSeedAction, SIGNAL(triggered()), this, SLOT(recoverSeed()));

  QAction *selectSeedAction = new QAction("Select", this);
  seedMenu->addAction(selectSeedAction);
  connect(selectSeedAction, SIGNAL(triggered()), this, SLOT(selectSeed()));

  QMenu *batchMenu = m_mainMenu->addMenu("Batch");
  QAction *allSeedSummaryAction = new QAction("Check Work Progress", this);
  batchMenu->addAction(allSeedSummaryAction);
  connect(allSeedSummaryAction, SIGNAL(triggered()),
          this, SLOT(checkAllSeed()));

  QAction *allSeedProcessAction = new QAction("Process All Seeds", this);
  batchMenu->addAction(allSeedProcessAction);
  connect(allSeedProcessAction, SIGNAL(triggered()),
          this, SLOT(processAllSeed()));
}

void FlyEmBodySplitProjectDialog::closeEvent(QCloseEvent */*event*/)
{
  clear();
}

void FlyEmBodySplitProjectDialog::checkCurrentBookmark()
{
  QItemSelectionModel *sel = ui->bookmarkView->selectionModel();
  QModelIndexList selected = sel->selectedIndexes();

  foreach (const QModelIndex &index, selected) {
    ZFlyEmBookmark *bookmark = m_bookmarkList.getBookmark(index.row());
    bookmark->setChecked(true);
    m_bookmarkList.update(index.row());
  }
}

void FlyEmBodySplitProjectDialog::setDvidTarget(const ZDvidTarget &target)
{
  m_project.setDvidTarget(target);
  //checkServerStatus();
}

bool FlyEmBodySplitProjectDialog::isReadyForSplit(const ZDvidTarget &target)
{
  return m_project.isReadyForSplit(target);

#if 0
  bool succ = true;

  QStringList infoList;

  ZDvidReader reader;
  if (reader.open(target)) {
    if (!reader.hasSparseVolume()) {
      infoList.append(("Incomplete split database: data \"" +
                       target.getBodyLabelName() +
                      "\" missing").c_str());
      succ = false;
    }

    std::string splitLabelName = ZDvidData::getName(
          ZDvidData::ROLE_SPLIT_LABEL, ZDvidData::ROLE_BODY_LABEL,
          target.getBodyLabelName());

    if (!reader.hasData(splitLabelName)) {
      infoList.append(("Incomplete split database: data \"" + splitLabelName +
                       "\" missing").c_str());
      succ = false;
    }

    std::string splitStatusName =  ZDvidData::getName(
          ZDvidData::ROLE_SPLIT_STATUS, ZDvidData::ROLE_BODY_LABEL,
          target.getBodyLabelName());
    if (!reader.hasData(splitStatusName)) {
      infoList.append(("Incomplete split database: data \"" + splitStatusName +
                       "\" missing").c_str());
      succ = false;
    }
  } else {
    infoList.append("Cannot connect to database.");
    succ = false;
  }

  //dumpError();

  return succ;
#endif

}
/*
bool FlyEmBodySplitProjectDialog::checkServerStatus()
{
  bool succ = true;

  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    if (!reader.hasData(m_project.getSplitLabelName())) {
      dumpError(("Incomplete split database: " + m_project.getSplitLabelName() +
                " missing").c_str());
      succ = false;
    }

    if (!reader.hasData(m_project.getSplitStatusName())) {
      dumpError(("Incomplete split database: " + m_project.getSplitStatusName() +
                " missing").c_str());
      succ = false;
    }
  } else {
    dumpError("Invalid database.", true);
    succ = false;
  }

  return succ;
}
*/
void FlyEmBodySplitProjectDialog::setBodyId(int id)
{
  m_project.setBodyId(id);
}

int FlyEmBodySplitProjectDialog::getBodyId() const
{
  return m_project.getBodyId();
}

void FlyEmBodySplitProjectDialog::clear()
{
  m_project.clear();
  //updateWidget();
  resetSideView();

  //dump("Load a body to start.");
}

void FlyEmBodySplitProjectDialog::shallowClear()
{
  m_project.shallowClear();
  updateWidget();
}

void FlyEmBodySplitProjectDialog::shallowClearResultWindow()
{
  m_project.shallowClearResultWindow();
}

void FlyEmBodySplitProjectDialog::shallowClearDataFrame()
{
  m_project.shallowClearDataFrame();
  updateWidget();
}

bool FlyEmBodySplitProjectDialog::showData2d()
{
  dump("Showing body in 2D ...", true);

  if (m_project.hasDataFrame()) {
    m_project.showDataFrame();
    getMainWindow()->raise();
    dump("Done.", true);
    return true;
  } else {
    if (getMainWindow()->initBodySplitProject()) {
      updateWidget();
      dump("Done.", true);
      return true;
    } else {
      dump("Failed.", true);
    }
  }

  return false;
}

void FlyEmBodySplitProjectDialog::showData3d()
{
  if (m_project.hasDataFrame()) {
    dump("Showing body in 3D ...", true);
    m_project.showDataFrame3d();
    dump("done", true);
  }
}

void FlyEmBodySplitProjectDialog::showResult3d()
{
  dump("Showing splitting result ...", true);
  m_project.showResult3d();
  dump("Done.", true);
}

void FlyEmBodySplitProjectDialog::showResult3dQuick()
{
  dump("Showing splitting result ...", true);
  m_project.showResultQuickView();
  dump("Done.", true);
}


MainWindow* FlyEmBodySplitProjectDialog::getMainWindow()
{
  return qobject_cast<MainWindow*>(this->parentWidget());
}

QProgressDialog* FlyEmBodySplitProjectDialog::getProgressDialog()
{
  return getMainWindow()->getProgressDialog();
}

const ZDvidTarget& FlyEmBodySplitProjectDialog::getDvidTarget() const
{
  return m_project.getDvidTarget();
}

void FlyEmBodySplitProjectDialog::setDataFrame(ZStackFrame *frame)
{
  connect(frame, SIGNAL(destroyed()), this, SLOT(shallowClearDataFrame()));
  connect(frame, SIGNAL(keyEventEmitted(QKeyEvent*)),
          this, SLOT(processKeyEvent(QKeyEvent*)));

  m_project.setDataFrame(frame);
}

void FlyEmBodySplitProjectDialog::setLoadBodyDialog(
    ZFlyEmNewBodySplitProjectDialog *dlg)
{
  m_loadBodyDlg = dlg;
}

bool FlyEmBodySplitProjectDialog::loadBody()
{
  int bodyId = ui->bodyIdSpinBox->value();
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    if (reader.hasCoarseSparseVolume(bodyId)) {
 // if (m_loadBodyDlg->exec()) {
      //setDvidTarget(m_loadBodyDlg->getDvidTarget());
      setBodyId(bodyId);

      //updateSideView();;
      updateWidget();
      updateSideView();

      dump("Body loaded.");

      return showData2d();
    } else {
      dumpError(QString("The body %1 seems not exist.").arg(bodyId));
    }
  }

  return false;
  //}
}

void FlyEmBodySplitProjectDialog::quickView()
{
  m_project.showBodyQuickView();
}

bool FlyEmBodySplitProjectDialog::isBodyLoaded() const
{
  return m_project.getBodyId() > 0;
}

void FlyEmBodySplitProjectDialog::updateButton()
{
  ui->loadBodyPushButton->setEnabled(
        !isBodyLoaded() && m_project.getDvidTarget().isValid());
  ui->bodyIdSpinBox->setEnabled(
        !isBodyLoaded()  && m_project.getDvidTarget().isValid());

  ui->view2dBodyPushButton->setEnabled(isBodyLoaded());
  ui->quickViewPushButton->setEnabled(isBodyLoaded());
  ui->view3dBodyPushButton->setEnabled(m_project.hasDataFrame());
  ui->viewSplitPushButton->setEnabled(m_project.hasDataFrame());
  ui->commitPushButton->setEnabled(false);
  ui->saveSeedPushButton->setEnabled(m_project.hasDataFrame());
  ui->donePushButton->setEnabled(isBodyLoaded());
}

void FlyEmBodySplitProjectDialog::updateWidget()
{
  updateButton();
  QString text;
  if (m_project.hasBookmark()) {
    text += QString("<p>%1 bookmarks</p>").
        arg(m_project.getBookmarkCount());
  }

  if (m_project.getDvidTarget().isValid()) {
    ui->dvidInfoLabel->setText(
          QString("Database: %1 (%2)").
          arg(m_project.getDvidTarget().getName().c_str()).
          arg(m_project.getDvidTarget().getSourceString().c_str()));
  } else {
    ui->dvidInfoLabel->setText("No database selected.");
  }

  if (!isBodyLoaded()) {
    text += "<p>No body loaded.</p>";
  } else {
    text += QString("<p>Body ID: %2</p>").
          arg(m_project.getBodyId());
  }
  ui->infoWidget->setText(text);

  updateBookmarkTable();
}

void FlyEmBodySplitProjectDialog::dump(const QString &info, bool appending)
{    
  QString text = "<p>" + info + "</p>";
  if (appending) {
    ui->outputWidget->append(text);
    ui->outputWidget->verticalScrollBar()->setValue(
          ui->outputWidget->verticalScrollBar()->maximum());
  } else {
    ui->oldOutputWidget->append(ui->outputWidget->toHtml());
    ui->outputWidget->clear();
    ui->outputWidget->setText(text);
  }
}

void FlyEmBodySplitProjectDialog::dumpError(const QString &info, bool appending)
{
  QString text = "<p><font color=\"#FF0000\">" + info + "</font></p>";

  dump(text, appending);
}

void FlyEmBodySplitProjectDialog::loadBookmark()
{
#if 0
  QString fileName = getMainWindow()->getOpenFileName("Load Bookmarks", "*.json");
  if (!fileName.isEmpty()) {
    m_project.loadBookmark(fileName);
    updateWidget();

    m_loadBodyDlg->setBodyIdComboBox(m_project.getBookmarkBodySet());

    dump("Bookmarks loaded.");
  }
#endif
}

void FlyEmBodySplitProjectDialog::updateBookmarkTable()
{
//  const ZFlyEmBookmarkArray *bookmarkArray = m_project.getBookmarkArray();
  if (m_project.getDocument() != NULL) {
    m_bookmarkList.clear();
    if (isBodyLoaded()) {
      //        m_project.clearBookmarkDecoration();
      //      foreach (ZFlyEmBookmark bookmark, bookmarkArray) {
      const TStackObjectList &objList = m_project.getDocument()->
          getObjectList(ZStackObject::TYPE_FLYEM_BOOKMARK);
      for (TStackObjectList::const_iterator iter = objList.begin();
           iter != objList.end(); ++iter) {
        const ZFlyEmBookmark *bookmark = dynamic_cast<ZFlyEmBookmark*>(*iter);
        if (bookmark->getBodyId() == m_project.getBodyId()) {
          m_bookmarkList.append(bookmark);
        }
      }
      //      m_project.addBookmarkDecoration(m_bookmarkList.getBookmarkArray());
    }
  }
}

/*
void FlyEmBodySplitProjectDialog::showBookmarkContextMenu(const QModelIndex &index)
{
  m_pressedIndex = index;

//  const ZFlyEmBookmark &bookmark = m_bookmarkList.getBookmark(index.row());


}
*/
void FlyEmBodySplitProjectDialog::locateBookmark(const QModelIndex &index)
{
  const ZFlyEmBookmark *bookmark = m_bookmarkList.getBookmark(index.row());
  m_project.locateBookmark(*bookmark);
}

void FlyEmBodySplitProjectDialog::updateSideView()
{
  //startProgress("Generating side view ...");
  //getProgressDialog()->setRange(0, 100);

  initSideViewScene();

  /*QFuture<void> res = */QtConcurrent::run(
        this, &FlyEmBodySplitProjectDialog::updateSideViewFunc);
}

void FlyEmBodySplitProjectDialog::resetSideView()
{
  m_sideViewScene->clear();
  ui->sideViewLabel->setText("Side view");
}

void FlyEmBodySplitProjectDialog::initSideViewScene()
{
  m_sideViewScene->clear();
  m_sideViewScene->setSceneRect(ui->sideView->viewport()->rect());
  m_sideViewScene->setBackgroundBrush(QBrush(Qt::gray));
}

void FlyEmBodySplitProjectDialog::updateSideViewFunc()
{
  int bodyId = getBodyId();

  ui->sideViewLabel->setText("Side view: generating ...");
  //m_sideViewScene->clear();


  QGraphicsPixmapItem *thumbnailItem = new QGraphicsPixmapItem;
  QPixmap pixmap;
  bool thumbnailReady = false;

  Stack *stack = NULL;
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    if (reader.hasBodyInfo(bodyId)) {
      ZStack *stackObj = reader.readThumbnail(getBodyId());
      if (stackObj != NULL) {
        stack = C_Stack::clone(stackObj->c_stack());
        delete stackObj;
      }
    }

    ZDvidInfo dvidInfo = reader.readGrayScaleInfo();

    if (stack == NULL) {
      ZDvidWriter writer;
      if (writer.open(getDvidTarget())) {
        ZObject3dScan body;
        reader.readBody(bodyId, &body);
        ZFlyEmNeuronImageFactory factory;

        factory.setSizePolicy(ZFlyEmNeuronImageFactory::SIZE_BOUND_BOX,
                              ZFlyEmNeuronImageFactory::SIZE_BOUND_BOX,
                              ZFlyEmNeuronImageFactory::SIZE_BOUND_BOX);
        factory.setDownsampleInterval(7, 7, 7);
        factory.setSourceDimension(dvidInfo.getStackSize()[0],
            dvidInfo.getStackSize()[1], dvidInfo.getStackSize()[2]);

        stack = factory.createSurfaceImage(body);
        writer.writeThumbnail(bodyId, stack);
        ZFlyEmNeuronBodyInfo bodyInfo;
        bodyInfo.setBodySize(body.getVoxelNumber());
        bodyInfo.setBoundBox(body.getBoundBox());
        writer.writeBodyInfo(bodyId, bodyInfo.toJsonObject());
      }
    }

    if (stack != NULL) {
      ZImage image(C_Stack::width(stack), C_Stack::height(stack));
      image.setData(C_Stack::array8(stack));
      if (pixmap.convertFromImage(image)) {
        thumbnailReady = true;
      } else {
        dump("Failed to load the thumbnail.");
      }
      C_Stack::kill(stack);
    }

    if (thumbnailReady) {
      thumbnailItem->setPixmap(pixmap);
      QTransform transform;

      double sceneWidth = m_sideViewScene->width();
      double sceneHeight = m_sideViewScene->height();
      double scale = std::min(
            double(sceneWidth - 10) / pixmap.width(),
            double(sceneHeight - 10) / pixmap.height());
      if (scale > 5) {
        scale = 5;
      }

      transform.scale(scale, scale);
      thumbnailItem->setTransform(transform);
      m_sideViewScene->addItem(thumbnailItem);

      int sourceZDim = dvidInfo.getStackSize()[2];
      int sourceYDim = dvidInfo.getStackSize()[1];

      ZFlyEmNeuronBodyInfo bodyInfo =
          reader.readBodyInfo(bodyId);
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
        m_sideViewScene->addItem(textItem);
      } else { //Draw range rect
        double x = 10;
        double y = 10;

        double scale = sceneWidth * 0.5 / sourceYDim;
        double height = sourceZDim * scale;
        double width = sourceYDim * scale;

        QGraphicsRectItem *rectItem = new QGraphicsRectItem(
              sceneWidth - width - x, y, width, height);
        rectItem->setPen(QPen(QColor(255, 0, 0, 164)));
        m_sideViewScene->addItem(rectItem);

        int z0 = dvidInfo.getStartCoordinates().getZ();
        int y0 = dvidInfo.getStartCoordinates().getY();
        rectItem = new QGraphicsRectItem(
              sceneWidth - width - x + (startY - y0) * scale,
              y + (startZ - z0) * scale,
              bodySpan * scale, bodyHeight * scale);
        rectItem->setPen(QPen(QColor(0, 255, 0, 164)));
        m_sideViewScene->addItem(rectItem);
      }
    }
  }

  if (bodyId == getBodyId()) {
    m_sideViewScene->addItem(thumbnailItem);
    ui->sideViewLabel->setText(QString("Side view: %1").arg(bodyId));
    emit sideViewReady();
  }
}

void FlyEmBodySplitProjectDialog::startProgress(const QString &label)
{
  getMainWindow()->getProgressDialog()->setRange(0, 100);
  getMainWindow()->getProgressDialog()->setLabelText(label);
  getMainWindow()->getProgressDialog()->show();
}

void FlyEmBodySplitProjectDialog::on_pushButton_clicked()
{
  ZDvidReader reader;
  reader.open(m_project.getDvidTarget());

  int id = reader.readMaxBodyId();
  dump(QString("Max body ID: %1").arg(id));
//  std::cout << id << std::endl;

#if 0
  ZDvidWriter writer;
  writer.open(m_project.getDvidTarget());
  writer.writeMaxBodyId(50000000);

  ZDvidReader reader;
  reader.open(m_project.getDvidTarget());
  int id = reader.readMaxBodyId();
  std::cout << id << std::endl;
#endif

#if 0
  const ZObject3dScan *wholeBody =
      m_project.getDataFrame()->document()->getSparseStack()->getObjectMask();
  wholeBody->save(GET_TEST_DATA_DIR + "/body_0.sobj");

  const ZStack *stack = m_project.getDataFrame()->document()->getLabelField();
  std::map<int, ZObject3dScan*> *objSet = ZObject3dScan::extractAllObject(
        stack->array8(), stack->width(), stack->height(), stack->depth(),
        0, NULL);

  const ZIntPoint &dsIntv =
      m_project.getDataFrame()->document()->getSparseStack()->getDownsampleInterval();

  for (std::map<int, ZObject3dScan*>::const_iterator iter = objSet->begin();
       iter != objSet->end(); ++iter) {
    if (iter->first > 0) {
      ZObject3dScan *obj = iter->second;
      obj->translate(stack->getOffset().getX(), stack->getOffset().getY(),
                     stack->getOffset().getZ());
      obj->upSample(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());
      ZString output = GET_TEST_DATA_DIR + "/body_";
      output.appendNumber(iter->first);
      obj->save(output + ".sobj");
      delete obj;
    }
  }

  delete objSet;
#endif

#ifdef _DEBUG_2
  //updateSideView();
  startProgress("Test ...");
  ZObject3dScan obj;
  obj.load(GET_DATA_DIR + "/benchmark/432.sobj");

  ZFlyEmNeuronImageFactory factory;
  factory.setDownsampleInterval(7, 7, 7);
  tic();
  Stack *stack = factory.createSurfaceImage(obj);
  ptoc();
  C_Stack::kill(stack);
  emit progressDone();
#endif

#ifdef _DEBUG_2
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    ZStackFrame *frame = m_project.getDataFrame();
    if (frame != NULL) {
      int currentSlice = frame->view()->sliceIndex();
      int width = frame->document()->getStackWidth();
      int height = frame->document()->getStackHeight();
      ZIntPoint offset = frame->document()->getStackOffset();
      int z = currentSlice + offset.getZ();
      ZStack *stack = reader.readGrayScale(offset.getX(), offset.getY(), z,
                                           width, height, 1);
      ZStackPatch *patch = new ZStackPatch(stack);
      patch->setZOrder(-1);
      patch->setSource("#testpatch");
      frame->document()->addStackPatch(patch);
    }
  }
#endif
}

void FlyEmBodySplitProjectDialog::on_dvidPushButton_clicked()
{

  if (m_dvidDlg->exec()) {
    clear();
    setDvidTarget(ZDvidTarget());
    const ZDvidTarget &target = m_dvidDlg->getDvidTarget();
    dump(QString("Connecting to Database: %1 (%2) ...").
         arg(target.getName().c_str()).
         arg(target.getSourceString().c_str()));
    if (isReadyForSplit(target)) {
      setDvidTarget(target);
      if (getDvidTarget().isValid()) {
        dump("Connected successfully.", true);
        dump("Load a body to start.", true);
      } else {
        dumpError("Invalid database.", true);
      }
    } else {
//      setDvidTarget(ZDvidTarget());
      dumpError("Connection failed.", true);
    }
    updateWidget();
  }
}

void FlyEmBodySplitProjectDialog::on_commitPushButton_clicked()
{
  //m_project.saveSeed();

  if (ZDialogFactory::Ask("Commit Confirmation",
                          "Do you want to upload the splitting results now? "
                          "It may take several minutes to several hours, "
                          "depending on how large the body is.",
                          this)) {
    m_project.commitResult();
  }
}

void FlyEmBodySplitProjectDialog::downloadSeed()
{
  m_project.downloadSeed();
}

void FlyEmBodySplitProjectDialog::viewPreviousSlice()
{
  m_project.viewPreviousSlice();
}

void FlyEmBodySplitProjectDialog::viewNextSlice()
{
  m_project.viewNextSlice();
}

void FlyEmBodySplitProjectDialog::viewFullGrayscale()
{
  m_project.viewFullGrayscale(ui->fullGrayscaleCheckBox->isChecked());
//  m_project.updateBodyMask();
}

void FlyEmBodySplitProjectDialog::viewFullGrayscale(bool viewing)
{
  m_project.viewFullGrayscale(viewing);
  if (viewing) {
    m_project.updateBodyMask();
  }
}

void FlyEmBodySplitProjectDialog::saveSeed()
{
  m_project.saveSeed(true);
}

void FlyEmBodySplitProjectDialog::showBodyMask(bool on)
{
  m_project.setShowingBodyMask(on);
  m_project.updateBodyMask();
}

#if 0
void FlyEmBodySplitProjectDialog::removeAllBookmark()
{
  m_project.removeAllBookmark();
  updateWidget();
}
#endif

void FlyEmBodySplitProjectDialog::exportSplits()
{
  m_project.exportSplits();
}

void FlyEmBodySplitProjectDialog::checkAllSeed()
{
  ZDvidReader reader;

  if (reader.open(getDvidTarget())) {
    //int maxId = reader.readMaxBodyId();

    QStringList keyList = reader.readKeys(
          m_project.getSplitLabelName().c_str(),
          m_project.getSeedKey(0).c_str(),
          (m_project.getSeedKey(9) + "a").c_str());
    emit messageDumped("<i>Work summary</i>: ");
    int processedCount = 0;
    foreach (QString key, keyList) {
      QString message = QString("..%1").arg(key);
      ZString str = key.toStdString();
      int bodyId = str.lastInteger();

      if (m_project.isSeedProcessed(bodyId)) {
        ++processedCount;
        message += " processed.";
      }
      emit messageDumped(message, true);
    }
    emit messageDumped(
          QString("..#Seeded bodies: <font color=\"blue\">%1</font> "
                  "(<font color=\"green\">%2</font> processed).").
          arg(keyList.size()).arg(processedCount), true);
  }
}

void FlyEmBodySplitProjectDialog::processAllSeed()
{
  int maxBodyCount = -1;
  ZSpinBoxDialog *dlg = ZDialogFactory::makeSpinBoxDialog(this);
  dlg->setValueLabel("Max number of bodies to process:");
  dlg->setWindowTitle("Specifying #bodies to process");
  if (dlg->exec()) {
    maxBodyCount = dlg->getValue();
  } else {
    return;
  }

  m_project.clear();

  ZDvidReader reader;


  if (reader.open(getDvidTarget())) {
    //int maxId = reader.readMaxBodyId();

    QStringList keyList = reader.readKeys(
          m_project.getSplitLabelName().c_str(),
          m_project.getSeedKey(0).c_str(),
          (m_project.getSeedKey(9) + "a").c_str());

    if (keyList.empty()) {
      emit messageDumped(QString("No seed is available in the database."));
    } else {
      emit messageDumped(QString("Start processing %1 bodies...").
                         arg(keyList.size()));
      std::vector<int> processedSeed;
      for (int i = 0; i < keyList.size(); ++i) {
        if ((int) processedSeed.size() >= maxBodyCount && maxBodyCount >= 0) {
          emit messageDumped(QString("Max count reached."), true);
          break;
        }
        const QString &key = keyList[i];
        ZString str = key.toStdString();
        int bodyId = str.lastInteger();

        if (bodyId > 0) {
          if (m_project.isSeedProcessed(bodyId)) {
            emit messageDumped("Already processed.", true);
          } else {
            emit messageDumped(
                  QString("Processing %1/%2: %3").
                  arg(i + 1).arg(keyList.size()).arg(bodyId), true);
            ui->bodyIdSpinBox->setValue(bodyId);
            if (loadBody()) {
              processedSeed.push_back(bodyId);
              emit messageDumped("Running split ...", true);
              m_project.runSplit();
              m_project.commitResult();
              m_project.setSeedProcessed(bodyId);
              m_project.clear();
            } else {
              emit messageDumped(QString("Failed to load the body %1.").
                                 arg(bodyId), true);
            }
          }
        } else {
          emit messageDumped("Invalid seed", true);
        }
      }
      if (processedSeed.empty()) {
        emit messageDumped("No seed is processed", true);
      } else {
        QString message = QString("%1 bodies are processed: ").
            arg(processedSeed.size());
        for (size_t i = 0; i < processedSeed.size(); ++i) {
          message += QString("%1 ").arg(processedSeed[i]);
        }
        emit messageDumped(message, true);
      }

      emit messageDumped("Done.", true);
    }
  }
}

void FlyEmBodySplitProjectDialog::startSplit(const QString &message)
{
  ZJsonObject obj;
  obj.decodeString(message.toStdString().c_str());

  if (obj.hasKey("body_id")) {
    int64_t bodyId = ZJsonParser::integerValue(obj["body_id"]);
    if (bodyId > 0) {
      if (obj.hasKey("dvid_target")) {
        ZDvidTarget target;
        target.loadJsonObject(ZJsonObject(obj.value("dvid_target")));
        show();
        raise();
        startSplit(target, (uint64_t) bodyId);
      }
    }
  }
}

void FlyEmBodySplitProjectDialog::startSplit(
    const ZDvidTarget &dvidTarget, uint64_t bodyId)
{
  clear();
  setDvidTarget(dvidTarget);
  ui->bodyIdSpinBox->setValue(bodyId);
  loadBody();
}

void FlyEmBodySplitProjectDialog::recoverSeed()
{
  if (ZDialogFactory::Ask("Recover Seed", "All current seeds might be lost. "
                          "Do you want to continue?", this)) {
    m_project.recoverSeed();
  }
}

void FlyEmBodySplitProjectDialog::selectSeed()
{
  ZSpinBoxDialog *dlg = ZDialogFactory::makeSpinBoxDialog(this);
  dlg->setValueLabel("Label");
  dlg->getButton(ZButtonBox::ROLE_SKIP)->hide();
  dlg->setValue(1);
  if (dlg->exec()) {
    int label = dlg->getValue();
    selectSeed(label);
  }
  delete dlg;
}

void FlyEmBodySplitProjectDialog::selectSeed(int label)
{
  m_project.selectSeed(label);
}

void FlyEmBodySplitProjectDialog::enableMessageManager()
{
  if (m_messageManager == NULL) {
    m_messageManager = ZMessageManager::Make<MessageProcessor>(this);
  }
}

void FlyEmBodySplitProjectDialog::processKeyEvent(QKeyEvent *event)
{
  switch(event->key())
  {
  case Qt::Key_Space:
    if (event->modifiers() == Qt::ShiftModifier) {
      getMainWindow()->runBodySplit();
    }
    break;
  case Qt::Key_U:
    ui->updatePushButton->click();
    break;
  case Qt::Key_G:
    ui->fullGrayscaleCheckBox->toggle();
    break;
  default:
    break;
  }
}

void FlyEmBodySplitProjectDialog::keyPressEvent(QKeyEvent *event)
{
  processKeyEvent(event);
}

////////////////////////////////
void FlyEmBodySplitProjectDialog::MessageProcessor::processMessage(
    ZMessage *message, QWidget *host) const
{
  FlyEmBodySplitProjectDialog *dlg =
      qobject_cast<FlyEmBodySplitProjectDialog*>(host);

  switch (message->getType()) {
  case ZMessage::TYPE_FLYEM_SPLIT:
    if (dlg != NULL) {
      const ZJsonObject &obj = message->getMessageBody();
      if (obj.hasKey("body_id")) {
        int64_t bodyId = ZJsonParser::integerValue(obj["body_id"]);
        if (bodyId > 0) {
          if (obj.hasKey("dvid_target")) {
            ZDvidTarget target;
            target.loadJsonObject(ZJsonObject(obj.value("dvid_target")));
            dlg->show();
            dlg->raise();
            dlg->startSplit(target, (uint64_t) bodyId);
          }
        }
      }
    }
    message->deactivate();
    break;
  case ZMessage::TYPE_FLYEM_SPLIT_VIEW_3D_BODY:
    if (dlg != NULL) {
      dlg->showData3d();
    }
    message->deactivate();
    break;
  default:
    break;
  }
}
