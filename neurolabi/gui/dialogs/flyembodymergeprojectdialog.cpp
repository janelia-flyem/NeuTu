#include "flyembodymergeprojectdialog.h"
#include "ui_flyembodymergeprojectdialog.h"

#include <QScrollBar>
#include <QInputDialog>

#include "zdviddialog.h"
#include "mainwindow.h"
#include "zqtbarprogressreporter.h"
#include "flyem/zflyembodymergeframe.h"
#include "zstackview.h"
#include "dvid/zdvidversionmodel.h"
#include "dvid/zdvidreader.h"
#include "zdialogfactory.h"
#include "zmessage.h"
#include "zmessagemanager.h"

FlyEmBodyMergeProjectDialog::FlyEmBodyMergeProjectDialog(QWidget *parent) :
  FlyEmProjectDialog(parent),
  ui(new Ui::FlyEmBodyMergeProjectDialog)
{
  ui->setupUi(this);

  ui->verionTreeView->setSelectionMode(QAbstractItemView::NoSelection);
  ui->verionTreeView->setExpandsOnDoubleClick(false);
  //ui->infoWidget->hide();
  m_project = new ZFlyEmBodyMergeProject(this);
  m_docTag = NeuTube::Document::FLYEM_MERGE;

  setupProgress();

  createMenu();
  connectSignalSlot();

  m_messageManager = NULL;
}

FlyEmBodyMergeProjectDialog::~FlyEmBodyMergeProjectDialog()
{
  delete ui;
}

void FlyEmBodyMergeProjectDialog::setPushButtonSlots()
{
  connect(ui->testPushButton, SIGNAL(clicked()), this, SLOT(test()));
  connect(ui->dvidServerPushButton, SIGNAL(clicked()),
          this, SLOT(setDvidTarget()));
  connect(ui->mergePushButton, SIGNAL(clicked()), m_project, SLOT(mergeBody()));
  connect(ui->loadGrayScalePushButton, SIGNAL(clicked()),
          this, SLOT(loadSlice()));
  connect(ui->moveyDecPushButton, SIGNAL(clicked()),
          this, SLOT(moveSliceUp()));
  connect(ui->moveyIncPushButton, SIGNAL(clicked()),
          this, SLOT(moveSliceDown()));
  connect(ui->movexDecPushButton, SIGNAL(clicked()),
          this, SLOT(moveSliceLeft()));
  connect(ui->movexIncPushButton, SIGNAL(clicked()),
          this, SLOT(moveSliceRight()));
  connect(ui->movexyDecPushButton, SIGNAL(clicked()),
          this, SLOT(moveSliceUpLeft()));
  connect(ui->movexyIncPushButton, SIGNAL(clicked()),
          this, SLOT(moveSliceDownRight()));

  connect(ui->prevSlicePushButton, SIGNAL(clicked()),
          this, SLOT(showPreviousSlice()));
  connect(ui->nextSlicePushButton, SIGNAL(clicked()),
          this, SLOT(showNextSlice()));

  connect(ui->uploadResultPushButton, SIGNAL(clicked()),
          m_project, SLOT(uploadResult()));
  connect(ui->startSplitPushButton, SIGNAL(clicked()),
          m_project, SLOT(notifySplit()));

  connect(ui->lockNodePushButton, SIGNAL(clicked()),
          this, SLOT(lockNode()));
  connect(ui->branchNodePushButton, SIGNAL(clicked()),
          this, SLOT(createVersionBranch()));
}

void FlyEmBodyMergeProjectDialog::test()
{
  if (m_project != NULL) {
    m_project->test();
  }
}

void FlyEmBodyMergeProjectDialog::moveSliceUp()
{
  if (m_project != NULL) {
    ui->ySpinBox->setValue(ui->ySpinBox->value() - ui->heightSpinBox->value() / 2);
    loadSlice();
  }
}

void FlyEmBodyMergeProjectDialog::moveSliceDown()
{
  if (m_project != NULL) {
    ui->ySpinBox->setValue(ui->ySpinBox->value() + ui->heightSpinBox->value() / 2);
    loadSlice();
  }
}

void FlyEmBodyMergeProjectDialog::moveSliceLeft()
{
  if (m_project != NULL) {
    ui->xSpinBox->setValue(ui->xSpinBox->value() - ui->widthSpinBox->value() / 2);
    loadSlice();
  }
}

void FlyEmBodyMergeProjectDialog::moveSliceRight()
{
  if (m_project != NULL) {
    ui->xSpinBox->setValue(ui->xSpinBox->value() + ui->widthSpinBox->value() / 2);
    loadSlice();
  }
}

void FlyEmBodyMergeProjectDialog::moveSliceUpLeft()
{
  if (m_project != NULL) {
    ui->xSpinBox->setValue(ui->xSpinBox->value() - ui->widthSpinBox->value() / 2);
    ui->ySpinBox->setValue(ui->ySpinBox->value() - ui->heightSpinBox->value() / 2);
    loadSlice();
  }
}

void FlyEmBodyMergeProjectDialog::showPreviousSlice()
{
  if (m_project != NULL) {
    ui->zSpinBox->setValue(ui->zSpinBox->value() - ui->stepSpinBox->value());
    loadSlice();
  }
}

void FlyEmBodyMergeProjectDialog::showNextSlice()
{
  if (m_project != NULL) {
    ui->zSpinBox->setValue(ui->zSpinBox->value() + ui->stepSpinBox->value());
    loadSlice();
  }
}

void FlyEmBodyMergeProjectDialog::moveSliceDownRight()
{
  if (m_project != NULL) {
    ui->xSpinBox->setValue(ui->xSpinBox->value() + ui->widthSpinBox->value() / 2);
    ui->ySpinBox->setValue(ui->ySpinBox->value() + ui->heightSpinBox->value() / 2);
    loadSlice();
  }
}

void FlyEmBodyMergeProjectDialog::loadSlice()
{
  if (m_project != NULL) {
    ZFlyEmBodyMergeFrame *frame = m_project->getDataFrame();
    if (frame != NULL) {
      ZRect2d rectRoi = frame->document()->getRect2dRoi();
      if (rectRoi.isValid()) {
        ui->xSpinBox->setValue(rectRoi.getX0() + rectRoi.getWidth() / 2);
        ui->ySpinBox->setValue(rectRoi.getY0() + rectRoi.getHeight() / 2);
        ui->widthSpinBox->setValue(rectRoi.getWidth());
        ui->heightSpinBox->setValue(rectRoi.getHeight());
      }
    }

    int x = ui->xSpinBox->value();
    int y = ui->ySpinBox->value();
    int z = ui->zSpinBox->value();
    int width = ui->widthSpinBox->value();
    int height = ui->heightSpinBox->value();

    m_project->loadSlice(x, y, z, width, height);
  }
}

void FlyEmBodyMergeProjectDialog::createMenu()
{
  m_mainMenu = new QMenu(this);
  ui->menuPushButton->setMenu(m_mainMenu);

  QAction *actionView3d = new QAction("3D Body View", this);
  connect(actionView3d, SIGNAL(triggered()), m_project, SLOT(showBody3d()));

  m_mainMenu->addAction(actionView3d);

}

void FlyEmBodyMergeProjectDialog::connectProjectSignalSlot()
{
  connect(m_project, SIGNAL(progressAdvanced(double)),
          this, SIGNAL(progressAdvanced(double)));
  connect(m_project, SIGNAL(progressStarted()),
          this, SIGNAL(progressStarted()));
  connect(m_project, SIGNAL(progressEnded()),
          this, SIGNAL(progressEnded()));
  connect(m_project, SIGNAL(newDocReady(ZStackDocReader*, bool)),
          this, SIGNAL(newDocReady(ZStackDocReader*, bool)));
  connect(m_project, SIGNAL(selectionChanged(ZStackObjectSelector)),
          this, SLOT(notifySelection(ZStackObjectSelector)));
  connect(m_project, SIGNAL(bodyMerged(QList<uint64_t>)),
          this, SLOT(notifyBodyMerged(QList<uint64_t>)));
}

void FlyEmBodyMergeProjectDialog::connectSignalSlot()
{
  setPushButtonSlots();
  connectProjectSignalSlot();

  connect(ui->verionTreeView, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(changeDvidNode(QModelIndex)));
  connect(ui->labelCheckBox, SIGNAL(toggled(bool)),
          m_project, SLOT(setLoadingLabel(bool)));
}

void FlyEmBodyMergeProjectDialog::updateVersionTree()
{
  ZDvidVersionModel *model = new ZDvidVersionModel(this);

  //model->getDag().setRoot("root");
  //model->getDag().addNode("v1", "root");

  ui->verionTreeView->setModel(model);

  ZDvidReader reader;
  reader.open(getDvidTarget());


  model->setDag(reader.readVersionDag());

//  model->setRoot("root");
//  model->setRoot(getDvidTarget().getUuid());
//  model->addNode("v1", getDvidTarget().getUuid());
//  model->addNode("v2", getDvidTarget().getUuid());
//  model->addNode("v3", "v1");
//  model->addNode("v3", "v2");
}

void FlyEmBodyMergeProjectDialog::updateDataFrame(
    ZStackDocReader &docReader, bool readyForPaint)
{
  if (m_project->hasDataFrame()) {
    //m_project->getDataFrame()->view()->blockRedraw(true);
    m_project->getDataFrame()->document()->setReadyForPaint(readyForPaint);
    m_project->setDocData(docReader);
  } else {
    ZStackFrame *frame = newDataFrame(docReader);
    m_project->setDataFrame(frame);
  }

  m_project->getDataFrame()->setDvidTarget(getDvidTarget());

//  updateWidget();
}

void FlyEmBodyMergeProjectDialog::consumeNewDoc(
    ZStackDocReader *docReader, bool readyForPaint)
{
  updateDataFrame(*docReader, readyForPaint);
  delete docReader;
}

void FlyEmBodyMergeProjectDialog::activateCurrentNode()
{
  const ZDvidTarget &target = getDvidTarget();
  if (target.isValid()) {
    getVersionModel()->activateNode(target.getUuid());
    m_project->setDvidTarget(target);
  }
}

void FlyEmBodyMergeProjectDialog::setDvidTarget()
{
  if (m_dvidDlg->exec()) {
    setDvidTargetD(m_dvidDlg->getDvidTarget());
    updateVersionTree();
    activateCurrentNode();
    updateInfo();
  }
}

void FlyEmBodyMergeProjectDialog::dump(const QString &str, bool appending)
{
  if (appending) {
    ui->outputWidget->append(str);
    ui->outputWidget->verticalScrollBar()->setValue(
          ui->outputWidget->verticalScrollBar()->maximum());
  } else {
    ui->outputWidget->setText(str);
  }
}

void FlyEmBodyMergeProjectDialog::showInfo(const QString &str, bool appending)
{
  if (appending) {
    ui->infoWidget->append(str);
  } else {
    ui->infoWidget->setText(str);
  }
}

void FlyEmBodyMergeProjectDialog::clear()
{
  delete m_project;
}

void FlyEmBodyMergeProjectDialog::setupProgress()
{
  ZQtBarProgressReporter *reporter = new ZQtBarProgressReporter;
  reporter->setProgressBar(ui->progressBar);
  setProgressReporter(reporter);
  ui->progressBar->hide();

  setProgressSignalSlot();
}

void FlyEmBodyMergeProjectDialog::notifySelection(
    const ZStackObjectSelector &selector)
{
  if (!selector.isEmpty()) {
//    std::set<ZStackObject*> newSelectedSet =
//        selector->getSelectedSet(ZStackObject::TYPE_OBJECT3D_SCAN);
    QString info;

    TStackObjectSet objSet =
        m_project->getDataFrame()->document()->getObjectGroup().
        getSelectedSet(ZStackObject::TYPE_OBJECT3D_SCAN);
    for (TStackObjectSet::const_iterator iter = objSet.begin();
         iter != objSet.end(); ++iter) {
      const ZObject3dScan *obj = dynamic_cast<ZObject3dScan*>(*iter);
      if (obj != NULL) {
        if (selector.isInSelectedSet(obj)) {
          info += QString("<b>%1</b> ").arg(obj->getLabel());
        } else {
          info += QString("%1 ").arg(obj->getLabel());
        }
        m_project->addSelected(obj->getLabel());
      } else {
        dump("NULL object in FlyEmBodyMergeProjectDialog::notifySelection");
      }
    }

//    for (std::vector<ZStackObject*>::const_iterator iter = objList.begin();
//         iter != objList.end(); ++iter) {
//      const ZObject3dScan *obj = dynamic_cast<ZObject3dScan*>(*iter);
//      if (obj != NULL) {
//        info += QString("<b>%1</b> ").arg(obj->getLabel());
//      } else {
//        dump("NULL object in FlyEmBodyMergeProjectDialog::notifySelection");
//      }
//    }

    std::vector<ZStackObject*>objList =
        selector.getDeselectedList(ZStackObject::TYPE_OBJECT3D_SCAN);
    for (std::vector<ZStackObject*>::const_iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      const ZObject3dScan *obj = dynamic_cast<ZObject3dScan*>(*iter);
      if (obj != NULL) {
        info += QString("<font color=\"#808080\"><s>%1</s></font> ").
            arg(obj->getLabel());
        m_project->removeSelected(obj->getLabel());
//        m_currentSelected.remove(obj->getLabel());
      } else {
        dump("NULL object in FlyEmBodyMergeProjectDialog::notifySelection");
      }
    }

    if (!info.isEmpty()) {
      dump("Selection: " + info, true);
    }
  }
}

void FlyEmBodyMergeProjectDialog::updateInfo()
{
  QString info;
  if (m_dvidTarget.isValid()) {
    info = QString("Database: <i>") + m_dvidTarget.getName().c_str() + "</i>";
    info += QString("<p>Uuid: %1</p>").arg(m_dvidTarget.getUuid().c_str());
  } else {
    info = "Load a database to start";
  }

  showInfo(info);
}

void FlyEmBodyMergeProjectDialog::notifyBodyMerged(
    QList<uint64_t> bodyLabelList)
{
  if (!bodyLabelList.empty()) {
    QString info = "Merged: ";
    foreach (uint64_t label, bodyLabelList) {
      info += QString("%1 ").arg(label);
    }
    dump(info, true);
  }
}

ZDvidVersionModel* FlyEmBodyMergeProjectDialog::getVersionModel()
{
  return qobject_cast<ZDvidVersionModel*>(ui->verionTreeView->model());
}

void FlyEmBodyMergeProjectDialog::changeDvidNode(const QModelIndex &index)
{
  changeDvidNode(getVersionModel()->getVersionUuid(index));
}

void FlyEmBodyMergeProjectDialog::changeDvidNode(const std::string &newUuid)
{
  if (m_project->getDvidTarget().getUuid() != newUuid) {
    getVersionModel()->deactivateNode(getDvidTarget().getUuid());
    m_dvidTarget.setUuid(newUuid);
    m_project->changeDvidNode(newUuid);
    getVersionModel()->activateNode(newUuid);

    updateInfo();
    loadSlice();

    dump(QString("Uuid changed to ") + newUuid.c_str());
  }
}

QModelIndex FlyEmBodyMergeProjectDialog::getSelectedVersionIndex() const
{
  QItemSelectionModel *model = ui->verionTreeView->selectionModel();
  QModelIndexList selected = model->selectedIndexes();

  QModelIndex selectedIndex = QModelIndex();
  if (!selected.empty()) {
    selectedIndex = selected.first();
  }

  return selectedIndex;
}

void FlyEmBodyMergeProjectDialog::createVersionBranch()
{
  std::string newUuid = m_project->createVersionBranch();
  if (!newUuid.empty()) {
    getVersionModel()->addNode(newUuid.substr(0, 4),
                               m_project->getDvidTarget().getUuid());
    dump(QString("New node %1 created").arg(newUuid.c_str()));
  } else {
    dump(QString("Failed to create a version."));
  }
}

void FlyEmBodyMergeProjectDialog::lockNode()
{
#ifdef _DEBUG_
  std::cout << "Locking node ..." << std::endl;
#endif

  bool ok;
  QString text = QInputDialog::getText(
        this, tr("Lock Node"),
        QString("Lock message (%1):").arg(
          m_project->getDvidTarget().getUuid().c_str()),
        QLineEdit::Normal, "", &ok);
  if (ok) {
    if (m_project->lockNode(text)) {
      getVersionModel()->lockNode(m_project->getDvidTarget().getUuid());
    } else {
      dump(("Failed to lock " + m_project->getDvidTarget().getUuid()).c_str(),
           true);
    }
  }

#if 0
  QModelIndex selectedIndex = getSelectedVersionIndex();
  if (!selectedIndex.isValid()) {
    std::string uuid = getVersionModel()->getVersionUuid(selectedIndex);
    if (!uuid.empty()) {
      m_project->loadNode();
    }
  }
#endif
}

void FlyEmBodyMergeProjectDialog::enableMessageManager()
{
  if (m_messageManager == NULL) {
    m_messageManager = ZMessageManager::Make<MessageProcessor>(this);
  }
}

void FlyEmBodyMergeProjectDialog::MessageProcessor::processMessage(
    ZMessage *message, QWidget *host) const
{
  FlyEmBodyMergeProjectDialog *dlg =
      qobject_cast<FlyEmBodyMergeProjectDialog*>(host);

  switch (message->getType()) {
  case ZMessage::TYPE_FLYEM_MERGE:
    if (dlg != NULL) {
      if (dlg->getProject() != NULL) {
        dlg->getProject()->mergeBody();
      }
    }
    break;
  case ZMessage::TYPE_FLYEM_SPLIT:
    if (dlg != NULL) {
      const ZJsonObject &obj = message->getMessageBody();
      if (obj.hasKey("body_id") && !obj.hasKey("dvid_target")) {
        if (ZJsonParser::integerValue(obj["body_id"]) >= 0) {
          const ZDvidTarget& dvidTarget = dlg->getDvidTarget();
          message->setBodyEntry("dvid_target", dvidTarget.toJsonObject());
        }
      }
    }
  break;
  case ZMessage::TYPE_FLYEM_COARSE_3D_VIS:
    if (dlg != NULL) {
      if (ZJsonParser::integerValue(message->getMessageBody()["coarse_level"])
          == 1) {
        dlg->getProject()->showCoarseBody3d();
      }
    }
    break;
  default:
    break;
  }
}

