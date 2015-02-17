#include "flyembodymergeprojectdialog.h"
#include "ui_flyembodymergeprojectdialog.h"

#include <QScrollBar>

#include "zdviddialog.h"
#include "mainwindow.h"
#include "zqtbarprogressreporter.h"
#include "flyem/zflyembodymergeframe.h"
#include "zstackview.h"

FlyEmBodyMergeProjectDialog::FlyEmBodyMergeProjectDialog(QWidget *parent) :
  FlyEmProjectDialog(parent),
  ui(new Ui::FlyEmBodyMergeProjectDialog)
{
  ui->setupUi(this);

  m_project = new ZFlyEmBodyMergeProject(this);
  m_dvidDlg = NULL;
  m_docTag = NeuTube::Document::FLYEM_MERGE;

  setupProgress();

  createMenu();
  connectSignalSlot();
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

  connect(ui->labelCheckBox, SIGNAL(toggled(bool)),
          m_project, SLOT(setLoadingLabel(bool)));
  connect(ui->uploadResultPushButton, SIGNAL(clicked()),
          m_project, SLOT(uploadResult()));
  connect(ui->startSplitPushButton, SIGNAL(clicked()),
          m_project, SLOT(notifySplit()));
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
    ui->ySpinBox->setValue(ui->ySpinBox->value() - ui->heightSpinBox->value());
    loadSlice();
  }
}

void FlyEmBodyMergeProjectDialog::moveSliceDown()
{
  if (m_project != NULL) {
    ui->ySpinBox->setValue(ui->ySpinBox->value() + ui->heightSpinBox->value());
    loadSlice();
  }
}

void FlyEmBodyMergeProjectDialog::moveSliceLeft()
{
  if (m_project != NULL) {
    ui->xSpinBox->setValue(ui->xSpinBox->value() - ui->widthSpinBox->value());
    loadSlice();
  }
}

void FlyEmBodyMergeProjectDialog::moveSliceRight()
{
  if (m_project != NULL) {
    ui->xSpinBox->setValue(ui->xSpinBox->value() + ui->widthSpinBox->value());
    loadSlice();
  }
}

void FlyEmBodyMergeProjectDialog::moveSliceUpLeft()
{
  if (m_project != NULL) {
    ui->xSpinBox->setValue(ui->xSpinBox->value() - ui->widthSpinBox->value());
    ui->ySpinBox->setValue(ui->ySpinBox->value() - ui->heightSpinBox->value());
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
    ui->xSpinBox->setValue(ui->xSpinBox->value() + ui->widthSpinBox->value());
    ui->ySpinBox->setValue(ui->ySpinBox->value() + ui->heightSpinBox->value());
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

void FlyEmBodyMergeProjectDialog::connectSignalSlot()
{
  setPushButtonSlots();
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

void FlyEmBodyMergeProjectDialog::updateDataFrame(
    ZStackDocReader &docReader, bool readyForPaint)
{
  if (m_project->hasDataFrame()) {
    //m_project->getDataFrame()->view()->blockRedraw(true);
    m_project->getDataFrame()->document()->setReadForPaint(readyForPaint);
    m_project->setDocData(docReader);
  } else {
    ZStackFrame *frame = newDataFrame(docReader);
    m_project->setDataFrame(frame);
  }

//  updateWidget();
}

void FlyEmBodyMergeProjectDialog::consumeNewDoc(
    ZStackDocReader *docReader, bool readyForPaint)
{
  updateDataFrame(*docReader, readyForPaint);
  delete docReader;
}

void FlyEmBodyMergeProjectDialog::setDvidTarget()
{
  if (m_dvidDlg->exec()) {
    setDvidTargetD(m_dvidDlg->getDvidTarget());
    if (m_project != NULL) {
      m_project->setDvidTarget(getDvidTarget());
    }
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

