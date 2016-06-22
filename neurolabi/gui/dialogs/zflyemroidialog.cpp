#include "zflyemroidialog.h"
#include <QtConcurrentRun>
#include <QMessageBox>
#include <QCloseEvent>
#include <QInputDialog>
#include <QMenu>
#include <QDir>
#include <QScrollBar>

#include "neutubeconfig.h"
#include "ui_zflyemroidialog.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidinfo.h"
#include "dvid/zdvidwriter.h"
#include "mainwindow.h"
#include "zstackframe.h"
#include "zswcgenerator.h"
#include "zjsonfactory.h"
#include "zcuboid.h"
#include "zintcuboid.h"
#include "zflyemutilities.h"
#include "zstring.h"
#include "zfiletype.h"
#include "z3dvolumesource.h"
#include "zwindowfactory.h"
#include "z3dpunctafilter.h"
#include "z3dvolumeraycaster.h"
#include "z3dvolumeraycasterrenderer.h"

ZFlyEmRoiDialog::ZFlyEmRoiDialog(QWidget *parent) :
  QDialog(parent), ZProgressable(),
  ui(new Ui::ZFlyEmRoiDialog), m_project(NULL),
  m_isLoadingGrayScale(false), m_isAutoStepping(false),
  m_xintv(4), m_yintv(4)
{
  ui->setupUi(this);

  m_dvidDlg = ZDialogFactory::makeDvidDialog(this);
  m_zDlg = ZDialogFactory::makeSpinBoxDialog(this);
  m_dsDlg = ZDialogFactory::makeDownsampleDialog(this);

  connect(ui->loadGrayScalePushButton, SIGNAL(clicked()),
          this, SLOT(loadGrayscale()));
  connect(ui->dvidServerPushButton, SIGNAL(clicked()),
          this, SLOT(setDvidTarget()));
  connect(ui->estimateRoiPushButton,
          SIGNAL(clicked()), this, SLOT(estimateRoi()));
  connect(ui->saveResultPushButton, SIGNAL(clicked()), this, SLOT(addRoi()));
  connect(ui->zSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setZ(int)));
  connect(ui->fullRoiPreviewPushButton, SIGNAL(clicked()),
          this, SLOT(previewFullRoi()));
  connect(ui->uploadResultPushButton, SIGNAL(clicked()),
          this, SLOT(uploadRoi()));
  connect(ui->loadSynapsePushButton, SIGNAL(clicked()),
          this, SLOT(loadSynapse()));
  connect(ui->synapseVisibleCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(toggleSynapseView(bool)));
  connect(ui->viewAllSynapsePushButton, SIGNAL(clicked()),
          this, SLOT(viewAllSynapseIn3D()));
  connect(this, SIGNAL(currentSliceLoaded(int)),
          this, SLOT(loadNextSlice(int)));
  connect(ui->cloneProjectPushButton, SIGNAL(clicked()),
          this, SLOT(cloneProject()));

#if 0
  QSize iconSize(24, 24);
  ui->movexDecPushButton->setIconSize(iconSize);
  ui->movexIncPushButton->setIconSize(iconSize);
  ui->movexyDecPushButton->setIconSize(iconSize);
  ui->movexyIncPushButton->setIconSize(iconSize);
  ui->moveyDecPushButton->setIconSize(iconSize);
  ui->moveyIncPushButton->setIconSize(iconSize);

  ui->xDecPushButton->setIconSize(iconSize);
  ui->yDecPushButton->setIconSize(iconSize);
  ui->xyDecPushButton->setIconSize(iconSize);
  ui->xyIncPushButton->setIconSize(iconSize);
  ui->xIncPushButton->setIconSize(iconSize);
  ui->yIncPushButton->setIconSize(iconSize);

  QSize buttonSize(40, 24);
  ui->movexDecPushButton->setMaximumSize(buttonSize);
  ui->movexIncPushButton->setMaximumSize(buttonSize);
  ui->movexyDecPushButton->setMaximumSize(buttonSize);
  ui->movexyIncPushButton->setMaximumSize(buttonSize);
  ui->moveyDecPushButton->setMaximumSize(buttonSize);
  ui->moveyIncPushButton->setMaximumSize(buttonSize);

  ui->xDecPushButton->setMaximumSize(buttonSize);
  ui->yDecPushButton->setMaximumSize(buttonSize);
  ui->xyDecPushButton->setMaximumSize(buttonSize);
  ui->xyIncPushButton->setMaximumSize(buttonSize);
  ui->xIncPushButton->setMaximumSize(buttonSize);
  ui->yIncPushButton->setMaximumSize(buttonSize);
#endif

  ui->progressBar->setRange(0, 100);

  connect(this, SIGNAL(progressStart()), this, SLOT(startProgressSlot()));
  connect(this, SIGNAL(newDocReady()), this, SLOT(updateDataFrame()));
  connect(this, SIGNAL(progressFailed()), this, SLOT(endProgressSlot()));
  connect(this, SIGNAL(progressAdvanced(double)),
          this, SLOT(advanceProgressSlot(double)));
  connect(this, SIGNAL(progressDone()), this, SLOT(endProgressSlot()));
  connect(ui->projectComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(loadProject(int)));
  connect(this, SIGNAL(messageDumped(QString, bool)),
          this, SLOT(dump(QString, bool)));

  connect(ui->quickModeCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(setQuickMode(bool)));

  if (m_project != NULL) {
    m_project->setZ(ui->zSpinBox->value());
  }
  //m_project->setDvidTarget(m_dvidDlg->getDvidTarget());

  updateWidget();

  ZQtBarProgressReporter *reporter = new ZQtBarProgressReporter;
  reporter->setProgressBar(ui->progressBar);
  setProgressReporter(reporter);
  //setProgressBar(ui->progressBar);
  ui->progressBar->hide();

#ifndef _DEBUG_
  ui->testPushButton->hide();
#endif

  createMenu();
}

ZFlyEmRoiDialog::~ZFlyEmRoiDialog()
{
  m_projectList.clear();
  delete ui;
}

void ZFlyEmRoiDialog::createMenu()
{
  m_nextMenu = new QMenu(this);

  QAction *actionNext1 = new QAction("1", this);
  m_nextMenu->addAction(actionNext1);
  //ui->nextPushButton->setMenu(m_nextMenu);

  m_mainMenu = new QMenu(this);
  ui->menuPushButton->setMenu(m_mainMenu);
  QAction *exportAction = new QAction("Export", this);
  m_mainMenu->addAction(exportAction);
  connect(exportAction, SIGNAL(triggered()), this, SLOT(exportResult()));

  QAction *exportRoiObjectAction = new QAction("Export ROI Object", this);
  m_mainMenu->addAction(exportRoiObjectAction);
  connect(exportRoiObjectAction, SIGNAL(triggered()),
          this, SLOT(exportRoiObject()));

  QAction *exportRoiBlockObjectAction = new QAction("Export ROI Block", this);
  m_mainMenu->addAction(exportRoiBlockObjectAction);
  connect(exportRoiBlockObjectAction, SIGNAL(triggered()),
          this, SLOT(exportRoiBlockObject()));


  m_importRoiAction = new QAction("Import ROI", this);
  m_mainMenu->addAction(m_importRoiAction);
//  m_importRoiAction->setCheckable(true);
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
}

void ZFlyEmRoiDialog::clear()
{
  ui->projectComboBox->clear();
  foreach (ZFlyEmRoiProject *proj, m_projectList) {
    delete proj;
  }
  m_projectList.clear();
  m_project = NULL;
}

void ZFlyEmRoiDialog::updateWidget()
{
  ui->pushButton->setEnabled(m_dvidTarget.isValid());
  ui->quickPrevPushButton->setEnabled(!ui->quickModeCheckBox->isChecked());
  ui->quickNextPushButton_3->setEnabled(!ui->quickModeCheckBox->isChecked());

  if (m_project != NULL) {
    QString text = QString("<p>DVID: %1</p>"
                           "<p>Local: %2</p>"
                           "<p>Z Range: [%3, %4]; Opened Slice: Z = %5; ROI: %6</p>").
        arg(m_project->getDvidTarget().getName().c_str()).
        arg(m_project->getDvidTarget().getLocalFolder().c_str()).
        arg(m_project->getDvidInfo().getMinZ()).
        arg(m_project->getDvidInfo().getMaxZ()).
        arg(m_project->getDataZ()).arg(m_project->hasOpenedRoi());
    ui->infoWidget->setText(text);
    ui->zSpinBox->setValue(m_project->getDataZ());

    ui->loadGrayScalePushButton->setEnabled(m_project->getDvidTarget().isValid());
    ui->searchPushButton->setEnabled(m_project->getDvidTarget().isValid());
    ui->estimateRoiPushButton->setEnabled(m_project->hasDataFrame());
    ui->loadSynapsePushButton->setEnabled(true);
  } else {
    ui->infoWidget->setText("");
    ui->loadGrayScalePushButton->setEnabled(false);
    ui->searchPushButton->setEnabled(false);
    ui->estimateRoiPushButton->setEnabled(false);
    ui->loadSynapsePushButton->setEnabled(false);
  }
}

ZFlyEmRoiProject* ZFlyEmRoiDialog::getProject(size_t index)
{
  if (index >= (size_t) m_projectList.size()) {
    return NULL;
  }

  return m_projectList[index];
}

bool ZFlyEmRoiDialog::appendProject(ZFlyEmRoiProject *project)
{
  if (project != NULL) {
    if (!project->getName().empty()) {
      bool isValidProject = true;
      foreach (ZFlyEmRoiProject *proj, m_projectList) {
        if (proj->getName() == project->getName()) {
          isValidProject = false;
        }
      }
      if (isValidProject) {
        //project->downloadAllRoi();
        m_projectList.append(project);
        ui->projectComboBox->addItem(project->getName().c_str());
        return true;
      }
    }
  }

  return false;
}

void ZFlyEmRoiDialog::downloadAllProject()
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    clear();
    QByteArray value = reader.readKeyValue("roi_curve", "projects");
    ZJsonArray array;
    array.decode(value.constData());
    for (size_t i = 0; i < array.size(); ++i) {
      std::string name(ZJsonParser::stringValue(array.at(i)));
      if (!name.empty()) {
        ZFlyEmRoiProject *project = newProject(name);
        appendProject(project);
      }
    }
    m_project = getProject(0);
    if (m_project != NULL) {
      setZ(ui->zSpinBox->value());
    }
  }
}


void ZFlyEmRoiDialog::setDvidTarget()
{
  if (m_dvidDlg->exec()) {
    m_dvidTarget = m_dvidDlg->getDvidTarget();
    dump(QString("Dvid server set to %1").
         arg(m_dvidTarget.getSourceString().c_str()));

    //load all projects
    downloadAllProject();
    if (!m_projectList.isEmpty()) {
      dump(QString("%1 projects found. The current project is set to \"%2\"").
           arg(m_projectList.size()).arg(m_project->getName().c_str()), true);
    } else {
      dump("No project exists. You need to new a project to proceed.", true);
    }

    updateWidget();

    /*
    if (m_project != NULL) {
      m_project->setDvidTarget(m_dvidTarget);
      updateWidget();
    }
    */
  }
}

void ZFlyEmRoiDialog::loadPartialGrayscaleFunc(
    int x0, int x1, int y0, int y1, int z)
{
  if (m_project == NULL) {
    return;
  }

  //advance(0.1);
  emit progressAdvanced(0.1);
  ZDvidReader reader;
  if (z >= 0 && reader.open(m_project->getDvidTarget())) {
    if (m_project->getRoi(z) == NULL) {
      m_project->downloadRoi(z);
    }

    QString infoString = reader.readInfo("grayscale");

    qDebug() << infoString;

    ZDvidInfo dvidInfo;
    dvidInfo.setFromJsonString(infoString.toStdString());

    ZStack *stack = NULL;
    stack = reader.readGrayScale(x0, y0, z,
                                 x1 - x0 + 1,
                                 y1 - y0 + 1, 1);

    if (stack != NULL) {
      //advance(0.5);
      emit progressAdvanced(0.5);
      m_docReader.clear();
      m_docReader.setStack(stack);

      ZSwcTree *tree = m_project->getRoiSwc(z);
      if (tree != NULL) {
        //tree->setRole(ZStackObjectRole::ROLE_ROI);
        m_docReader.addObject(tree);
      }
      emit newDocReady();
    } else {
      emit progressFailed();
    }
  } else {
    emit progressFailed();
  }
}

void ZFlyEmRoiDialog::prepareQuickLoadFunc(
    const ZDvidTarget &target, const std::string &lowresPath, int z)
{
  //const ZDvidTarget &target = m_project->getDvidTarget();
  ZDvidReader reader;
  if (z >= 0 && reader.open(target)) {
    QString infoString = reader.readInfo("grayscale");
    ZDvidInfo dvidInfo;
    dvidInfo.setFromJsonString(infoString.toStdString());

    /*
    std::string lowresPath =
        target.getLocalLowResGrayScalePath(m_xintv, m_xintv, 0, z);
        */
    if (!lowresPath.empty()) {
      if (!QFileInfo(lowresPath.c_str()).exists()) {
        ZIntCuboid boundBox = reader.readBoundBox(z);
        ZStack *stack = NULL;
        if (!boundBox.isEmpty()) {
          stack = reader.readGrayScale(boundBox.getFirstCorner().getX(),
                                       boundBox.getFirstCorner().getY(),
                                       z, boundBox.getWidth(),
                                       boundBox.getHeight(), 1);
        } else {
          stack = reader.readGrayScale(
                dvidInfo.getStartCoordinates().getX(),
                dvidInfo.getStartCoordinates().getY(),
                z, dvidInfo.getStackSize()[0],
              dvidInfo.getStackSize()[1], 1);
          if (stack != NULL) {
            boundBox = ZFlyEmRoiProject::estimateBoundBox(
                  *stack, getDvidTarget().getBgValue());
            if (!boundBox.isEmpty()) {
              stack->crop(boundBox);
            }
            ZDvidWriter writer;
            if (writer.open(target)) {
              writer.writeBoundBox(boundBox, z);
            }
          }
        }

        if (stack != NULL) {
          std::string lowresDir = ZString(lowresPath).dirPath(lowresPath);
          /*
          std::string lowresDir =
              target.getLocalLowResGrayScalePath(m_xintv, m_xintv, 0);
              */
          if (QDir(lowresDir.c_str()).exists()) {
            stack->downsampleMin(m_xintv, m_xintv, 0);
            stack->save(lowresPath);
            emit messageDumped("Quick load ready.", true);
          }
          delete stack;
        }
      }
    }
  }
}

QString ZFlyEmRoiDialog::getQuickLoadThreadId(int z) const
{
  const ZDvidTarget &target = m_project->getDvidTarget();
  std::string lowresPath =
      target.getLocalLowResGrayScalePath(m_xintv, m_xintv, 0, z);
  QString threadId =
      QString("prepareQuickLoad:%1:%2").arg(lowresPath.c_str()).arg(z);

  return threadId;
}

bool ZFlyEmRoiDialog::isPreparingQuickLoad(int z) const
{
  bool isWaiting = false;
  QString threadId = getQuickLoadThreadId(z);
  if (m_threadFutureMap.contains(threadId)) {
    if (m_threadFutureMap[threadId].isRunning()) {
      isWaiting = true;
    }
  }

  return isWaiting;
}

void ZFlyEmRoiDialog::prepareQuickLoad(int z, bool waitForDone)
{
  if (z >= 0) {
    const ZDvidTarget &target = m_project->getDvidTarget();
    std::string lowresPath =
        target.getLocalLowResGrayScalePath(m_xintv, m_xintv, 0, z);

    if (!lowresPath.empty() && !QFileInfo(lowresPath.c_str()).exists()) {
      QString threadId = getQuickLoadThreadId(z);
      if (!isPreparingQuickLoad(z)) { //Create new thread
          QFuture<void> future =QtConcurrent::run(
                this, &ZFlyEmRoiDialog::prepareQuickLoadFunc,
                target, lowresPath, z);
          m_threadFutureMap[threadId] = future;
#ifdef _DEBUG_
          emit messageDumped(threadId, true);
#endif
      }

      if (waitForDone) {
        if (m_threadFutureMap.contains(threadId)) {
          m_threadFutureMap[threadId].waitForFinished();
        }
      }
    }
  }
}

void ZFlyEmRoiDialog::loadGrayscaleFunc(int z, bool lowres)
{
  if (m_project == NULL) {
    return;
  }

  //advance(0.1);
  emit progressAdvanced(0.1);
  ZDvidReader reader;
  const ZDvidTarget &target = m_project->getDvidTarget();
  if (z >= 0 && reader.open(target)) {
    if (m_project->getRoi(z) == NULL) {
      m_project->downloadRoi(z);
    }

    QString infoString = reader.readInfo("grayscale");

    qDebug() << infoString;

    ZDvidInfo dvidInfo;
    dvidInfo.setFromJsonString(infoString.toStdString());

    //int z = m_zDlg->getValue();
    //m_project->setZ(z);

    ZStack *stack = NULL;

    bool creatingLowres = false;
    std::string lowresPath =
        target.getLocalLowResGrayScalePath(m_xintv, m_xintv, 0, z);
    if (lowres) {
      m_project->setDsIntv(m_xintv, m_yintv, 0);
      prepareQuickLoad(z, true);
      if (QFileInfo(lowresPath.c_str()).exists() && !isPreparingQuickLoad(z)) {
        stack = new ZStack;
        stack->load(lowresPath);
      } else if (QDir(target.getLocalFolder().c_str()).exists()) {
        creatingLowres = true;
      }
    }

    if (!lowres || creatingLowres){
      if (!lowres) {
        m_project->setDsIntv(0, 0, 0);
      }

      ZIntCuboid boundBox = reader.readBoundBox(z);

      if (!boundBox.isEmpty()) {
        stack = reader.readGrayScale(boundBox.getFirstCorner().getX(),
                                     boundBox.getFirstCorner().getY(),
                                     z, boundBox.getWidth(),
                                     boundBox.getHeight(), 1);
      } else {
        stack = reader.readGrayScale(
              dvidInfo.getStartCoordinates().getX(),
              dvidInfo.getStartCoordinates().getY(),
              z, dvidInfo.getStackSize()[0],
            dvidInfo.getStackSize()[1], 1);
        if (stack != NULL) {
          boundBox = ZFlyEmRoiProject::estimateBoundBox(
                *stack, getDvidTarget().getBgValue());
          if (!boundBox.isEmpty()) {
            stack->crop(boundBox);
          }
          ZDvidWriter writer;
          if (writer.open(m_project->getDvidTarget())) {
            writer.writeBoundBox(boundBox, z);
          }
        }
      }

      if (stack != NULL) {
        if (creatingLowres) {
          emit messageDumped("Creating low res data ...", true);
          stack->downsampleMin(m_xintv, m_xintv, 0);
          stack->save(lowresPath);
          emit messageDumped(
                QString("Data saved into %1").arg(lowresPath.c_str()), true);
        }
      }
    }

    if (stack != NULL) {
      //advance(0.5);
      emit progressAdvanced(0.5);
      m_docReader.clear();
      m_docReader.setStack(stack);

      ZSwcTree *tree = m_project->getRoiSwc(
            z, FlyEm::GetFlyEmRoiMarkerRadius(stack->width(), stack->height()));
      if (tree != NULL) {
        m_docReader.addObject(tree);
      }
#ifdef _DEBUG_
      std::cout << "Object count in docreader: "
                << m_docReader.getObjectGroup().size() << std::endl;
      std::cout << "Swc count in docreader: "
                << m_docReader.getObjectGroup().getObjectList(ZStackObject::TYPE_SWC).size()
                << std::endl;
#endif
      emit newDocReady();
    } else {
      processLoadGrayscaleFailure();
    }
  } else {
    processLoadGrayscaleFailure();
  }
}

void ZFlyEmRoiDialog::processLoadGrayscaleFailure()
{
#ifdef _DEBUG_
  std::cout << "Failed to load grayscale data." << std::endl;
#endif
  m_isLoadingGrayScale = false;
  emit progressFailed();
}

void ZFlyEmRoiDialog::newDataFrame()
{
  ZStackFrame *frame = getMainWindow()->createStackFrame(
        m_docReader, NeuTube::Document::FLYEM_ROI);
  frame->document()->setStackBackground(NeuTube::IMAGE_BACKGROUND_BRIGHT);
  setDataFrame(frame);

  getMainWindow()->addStackFrame(frame);
  getMainWindow()->presentStackFrame(frame);
}

void ZFlyEmRoiDialog::updateDataFrame()
{
  if (m_project->hasDataFrame()) {
     m_project->setDocData(m_docReader);
  } else {
    newDataFrame();
  }
  m_project->updateSynapse();
  updateWidget();
  endProgress();
  startBackgroundJob();
  m_isLoadingGrayScale = false;

  if (m_isAutoStepping) {
    emit currentSliceLoaded(m_project->getDataZ());
  }
}

void ZFlyEmRoiDialog::loadSynapse()
{
  if (m_project != NULL) {
    QString fileName = getMainWindow()->getOpenFileName(
          "Load Synapses", "Point Cloud (*.json *.txt)");
    if (!fileName.isEmpty()) {
      m_project->loadSynapse(
            fileName.toStdString(), ui->synapseVisibleCheckBox->isChecked());
    }
  } else {
    dump("Loading synapses not allowed: No project available.");
  }
}

void ZFlyEmRoiDialog::importRoi()
{
  if (m_project != NULL) {
    QString fileName = getMainWindow()->getOpenFileName(
          "Import ROI", "SWC File (*.swc)");
    if (!fileName.isEmpty()) {
      ZSwcTree tree;
      tree.load(fileName.toStdString());
      m_project->importRoiFromSwc(&tree);
    }
  }
}

void ZFlyEmRoiDialog::toggleSynapseView(bool isOn)
{
  m_project->setSynapseVisible(isOn);
}

void ZFlyEmRoiDialog::startBackgroundJob()
{
  int z = m_project->getCurrentZ();
  if (z >= 0) {
    prepareQuickLoad(z);
    prepareQuickLoad(getNextZ());
    prepareQuickLoad(getPrevZ());
  }
}

void ZFlyEmRoiDialog::loadGrayscale(int z)
{
  if (m_project == NULL || z < 0 || m_isLoadingGrayScale) {
    return;
  }

  bool lowres = ui->quickModeCheckBox->isChecked();

  bool loading = true;
  if (m_project->isRoiSaved() == false) {
    int ret = QMessageBox::warning(
          this, "Unsaved Result",
          "The current ROI is not saved. Do you want to continue?",
          QMessageBox::Yes | QMessageBox::No);
    loading = (ret == QMessageBox::Yes);
  }

  if (loading) {
    if (lowres) {
      const ZDvidTarget &target = m_project->getDvidTarget();
      const std::string &path =
          target.getLocalLowResGrayScalePath(m_xintv, m_yintv, 0);
      if (path.empty() || !QDir(path.c_str()).exists()) {
        QMessageBox::warning(
                  this, "No Quick Mode",
                  "The quick mode is not available for this data.",
                  QMessageBox::Ok);
        loading = false;
      }
    }
  }

  if (loading) {
    m_isLoadingGrayScale = true;
    resetProgress();
    startProgress();
    QtConcurrent::run(
          this, &ZFlyEmRoiDialog::loadGrayscaleFunc, z, lowres);
  }
}

void ZFlyEmRoiDialog::loadGrayscale(const ZIntCuboid &box)
{
  int z = box.getFirstCorner().getZ();
  if (m_project == NULL || z < 0 || m_isLoadingGrayScale) {
    return;
  }

  bool loading = true;
  if (m_project->isRoiSaved() == false) {
    int ret = QMessageBox::warning(
          this, "Unsaved Result",
          "The current ROI is not saved. Do you want to continue?",
          QMessageBox::Yes | QMessageBox::No);
    loading = (ret == QMessageBox::Yes);
  }

  if (loading) {
    m_isLoadingGrayScale = true;
    startProgress();
    QtConcurrent::run(
          this, &ZFlyEmRoiDialog::loadPartialGrayscaleFunc,
          box.getFirstCorner().getX(), box.getLastCorner().getX(),
          box.getFirstCorner().getY(), box.getLastCorner().getY(),
          box.getFirstCorner().getZ());
  }
}

void ZFlyEmRoiDialog::loadGrayscale()
{
  if (m_project == NULL) {
    return;
  }

  loadGrayscale(m_project->getCurrentZ());
}

void ZFlyEmRoiDialog::setDataFrame(ZStackFrame *frame)
{
  if (m_project == NULL) {
    return;
  }

  connect(frame, SIGNAL(destroyed()), this, SLOT(shallowClearDataFrame()));
  m_project->setDataFrame(frame);
}

MainWindow* ZFlyEmRoiDialog::getMainWindow()
{
  return qobject_cast<MainWindow*>(this->parentWidget());
}

void ZFlyEmRoiDialog::shallowClearDataFrame()
{
  if (m_project == NULL) {
    return;
  }

  m_project->shallowClearDataFrame();
  updateWidget();
}

void ZFlyEmRoiDialog::addRoi()
{
  if (m_project == NULL) {
    return;
  }

  if (!m_project->addRoi()) {
    dump("The result cannot be saved because the ROI is invalid.");
  } else {
    dump("ROI saved successfully.");
    updateWidget();
  }
}

void ZFlyEmRoiDialog::dump(const QString &str, bool appending)
{
  if (appending) {
    ui->outputWidget->append(str);
    ui->outputWidget->verticalScrollBar()->setValue(
          ui->outputWidget->verticalScrollBar()->maximum());
  } else {
    ui->outputWidget->setText(str);
  }
}

void ZFlyEmRoiDialog::setZ(int z)
{
  if (m_project == NULL) {
    return;
  }

  m_project->setZ(z);

  ui->nextSlicePushButton->setEnabled(getNextZ() >= 0);
  ui->prevSlicePushButton->setEnabled(getPrevZ() >= 0);
  //updateWidget();
}

void ZFlyEmRoiDialog::previewFullRoi()
{
  ZSwcTree *tree = m_project->getAllRoiSwc();

  if (tree != NULL) {
    ZStackDoc *doc = new ZStackDoc;
    doc->addObject(tree);
    Z3DWindow::Open(doc, this);

    //ZStackFrame *frame = new ZStackFrame();
    //frame->document()->addSwcTree(tree);

    //frame->open3DWindow(this);
    //delete frame;
  } else {
    dump("No ROI saved");
  }
}

void ZFlyEmRoiDialog::uploadRoi()
{
  if (m_project == NULL) {
    return;
  }

  int count = m_project->uploadRoi();
  dump(QString("%1 ROI curves uploaded").arg(count));
}

void ZFlyEmRoiDialog::estimateRoi()
{
  if (m_project == NULL) {
    return;
  }

  m_project->estimateRoi();
}

void ZFlyEmRoiDialog::on_searchPushButton_clicked()
{
  int z = m_project->findSliceToCreateRoi(ui->zSpinBox->value());
  if (z >= 0) {
    ui->zSpinBox->setValue(z);
    loadGrayscale(z);
  }
}

void ZFlyEmRoiDialog::loadNextSlice(int currentZ)
{
  if (m_isAutoStepping) {
    int z = currentZ + ui->stepSpinBox->value();
    if (z > m_project->getDvidInfo().getMaxZ()) {
      m_isAutoStepping = false;
    } else {
      emit messageDumped(QString("Loading next slice: %1").arg(z), true);
      ui->zSpinBox->setValue(z);
      loadGrayscale(z);
    }
  }
}

void ZFlyEmRoiDialog::on_testPushButton_clicked()
{
//  m_isAutoStepping = true;
//  loadGrayscale(ui->zSpinBox->value());

  if (m_project != NULL) {
    m_project->test();
  }
#if 0
  if (m_project == NULL) {
    return;
  }

  int z = m_project->getCurrentZ();

  ZStack *stack = NULL;
  std::string lowresPath =
      m_project->getDvidTarget().getLocalLowResGrayScalePath(m_xintv, m_xintv, 0, z);
  m_project->setDsIntv(m_xintv, m_yintv, 0);
  if (QFileInfo(lowresPath.c_str()).exists() && !isPreparingQuickLoad(z)) {
    stack = new ZStack;
    stack->load(lowresPath);
  }


  if (stack != NULL) {
    m_docReader.clear();
    m_docReader.setStack(stack);
    emit newDocReady();
  }
#endif

#if 0
  ZObject3dScan obj = m_project->getRoiObject();
  obj.downsampleMax(2, 2, 2);

  ZSwcTree *tree = ZSwcGenerator::createSurfaceSwc(obj);
  if (tree != NULL) {
    ZStackFrame *frame = new ZStackFrame();
    frame->document()->addSwcTree(tree);

    frame->open3DWindow(this);
    delete frame;
  }
#endif


  //obj.save(GET_DATA_DIR + "/test.sobj");

  //dump(QString("%1 saved.").arg((GET_DATA_DIR + "/test.sobj").c_str()));
}

#define ROI_SCALE 1.01

void ZFlyEmRoiDialog::on_xIncPushButton_clicked()
{
  if (m_project == NULL) {
    return;
  }

  m_project->scaleRoiSwc(ROI_SCALE, 1.0);
}

void ZFlyEmRoiDialog::on_xDecPushButton_clicked()
{
  if (m_project == NULL) {
    return;
  }

  m_project->scaleRoiSwc(1. / ROI_SCALE, 1.0);
}


void ZFlyEmRoiDialog::on_yDecPushButton_clicked()
{
  if (m_project == NULL) {
    return;
  }

  m_project->scaleRoiSwc(1.0, 1. / ROI_SCALE);
}

void ZFlyEmRoiDialog::on_yIncPushButton_clicked()
{
  if (m_project == NULL) {
    return;
  }

  m_project->scaleRoiSwc(1.0, ROI_SCALE);
}

void ZFlyEmRoiDialog::on_rotateLeftPushButton_clicked()
{
  if (m_project == NULL) {
    return;
  }

  m_project->rotateRoiSwc(-0.1);
}

void ZFlyEmRoiDialog::on_rotateRightPushButton_clicked()
{
  if (m_project == NULL) {
    return;
  }

  m_project->rotateRoiSwc(0.1);
}

void ZFlyEmRoiDialog::on_xyDecPushButton_clicked()
{
  if (m_project == NULL) {
    return;
  }

  m_project->scaleRoiSwc(1. / ROI_SCALE, 1. / ROI_SCALE);
}

void ZFlyEmRoiDialog::on_xyIncPushButton_clicked()
{
  if (m_project == NULL) {
    return;
  }
  m_project->scaleRoiSwc(ROI_SCALE, ROI_SCALE);
}

#define MOVE_STEP 5.0

void ZFlyEmRoiDialog::on_movexyDecPushButton_clicked()
{
  if (m_project == NULL) {
    return;
  }
  m_project->translateRoiSwc(-MOVE_STEP, -MOVE_STEP);
}

void ZFlyEmRoiDialog::on_movexyIncPushButton_clicked()
{
  if (m_project == NULL) {
    return;
  }
  m_project->translateRoiSwc(MOVE_STEP, MOVE_STEP);
}

void ZFlyEmRoiDialog::on_movexDecPushButton_clicked()
{
  if (m_project == NULL) {
    return;
  }
  m_project->translateRoiSwc(-MOVE_STEP, 0.0);
}

void ZFlyEmRoiDialog::on_movexIncPushButton_clicked()
{
  if (m_project == NULL) {
    return;
  }
  m_project->translateRoiSwc(MOVE_STEP, 0.0);
}

void ZFlyEmRoiDialog::on_moveyDecPushButton_clicked()
{
  if (m_project == NULL) {
    return;
  }
  m_project->translateRoiSwc(0.0, -MOVE_STEP);
}

void ZFlyEmRoiDialog::on_moveyIncPushButton_clicked()
{
  if (m_project == NULL) {
    return;
  }
  m_project->translateRoiSwc(0.0, MOVE_STEP);
}

void ZFlyEmRoiDialog::advanceProgressSlot(double p)
{
  advanceProgress(p);
}

void ZFlyEmRoiDialog::startProgressSlot()
{
  startProgress();
}

void ZFlyEmRoiDialog::endProgressSlot()
{
  endProgress();
}

void ZFlyEmRoiDialog::closeEvent(QCloseEvent *event)
{
  if (m_project == NULL) {
    return;
  }

  if (!m_project->isRoiSaved()) {
    int answer = QMessageBox::warning(
          this, "Unsaved Results",
          "The current ROI has not been saved. "
          "Do you want to close the project now?",
          QMessageBox::Yes | QMessageBox::No);
    if (answer == QMessageBox::No) {
      event->ignore();
    }
  }
  if (event->isAccepted()) {
    if (!m_project->isAllRoiCurveUploaded()) {
      int answer = QMessageBox::warning(
            this, "Unsaved Results",
            "Some ROIs seem not been uploaded into DVID. "
            "Do you want to close the project now?",
            QMessageBox::Yes | QMessageBox::No);
      if (answer == QMessageBox::No) {
        event->ignore();
      }
    }
  }
  if (event->isAccepted()) {
    m_project->closeDataFrame();
  }
}

void ZFlyEmRoiDialog::closeCurrentProject()
{
  if (m_project != NULL) {
    m_project->closeDataFrame();
  }
}

void ZFlyEmRoiDialog::loadProject(int index)
{
  closeCurrentProject();
  m_project = getProject(index);
  updateWidget();
}

bool ZFlyEmRoiDialog::isValidName(const std::string &name) const
{
  bool isValid = false;
  if (!name.empty() && !QString(name.c_str()).contains(' ')) {
    isValid = true;
    foreach (ZFlyEmRoiProject *proj, m_projectList) {
      if (proj->getName() == name) {
        isValid = false;
      }
    }
  }

  return isValid;
}

ZFlyEmRoiProject* ZFlyEmRoiDialog::newProject(const std::string &name)
{
  ZFlyEmRoiProject *project = NULL;
  if (isValidName(name)) {
    project = new ZFlyEmRoiProject(name, this);
    project->setDvidTarget(getDvidTarget());
  } else {
    QMessageBox::warning(
              this, "Failed to Create A Project",
              "Invalid project name: no space is allowed; "
              "no duplicated name is allowed.",
              QMessageBox::Ok);
  }

  return project;
}

void ZFlyEmRoiDialog::cloneProject(const std::string &name)
{
  if (isValidName(name)) {
    ZFlyEmRoiProject *project = m_project->clone(name);
    appendProject(project);
    ui->projectComboBox->setCurrentIndex(ui->projectComboBox->count() - 1);
    uploadProjectList();
  } else {
    QMessageBox::warning(
              this, "Failed to Clone A Project",
              "Invalid project name: no space is allowed; "
              "no duplicated name is allowed.",
              QMessageBox::Ok);
  }
}

void ZFlyEmRoiDialog::cloneProject()
{
  bool ok;
  QString text = QInputDialog::getText(this, tr("New ROI Project"),
                                       tr("Project name:"), QLineEdit::Normal,
                                       "", &ok);
  if (ok && !text.isEmpty()) {
    cloneProject(text.toStdString());
  }
}

void ZFlyEmRoiDialog::on_pushButton_clicked() //new project
{
  bool ok;
  QString text = QInputDialog::getText(this, tr("New ROI Project"),
                                       tr("Project name:"), QLineEdit::Normal,
                                       "", &ok);
  if (ok && !text.isEmpty()) {
    ZFlyEmRoiProject *project = newProject(text.toStdString());
    if (project != NULL) {
      if (appendProject(project)) {
        if (m_projectList.size() == 1) {
          loadProject(0);
        }
        uploadProjectList();
        dump(QString("Project \"%1\" is created.").
             arg(project->getName().c_str()));
      }
    }
    updateWidget();
  }
}

void ZFlyEmRoiDialog::uploadProjectList()
{
  ZDvidWriter writer;
  if (writer.open(getDvidTarget())) {
    ZJsonArray array;
    foreach (ZFlyEmRoiProject *proj, m_projectList) {
      array.append(proj->getName());
    }

    writer.writeJsonString("roi_curve", "projects", array.dumpString(0));
  }
}

void ZFlyEmRoiDialog::on_estimateVolumePushButton_clicked()
{
  if (m_project != NULL) {
    double volume = m_project->estimateRoiVolume('u');
    dump(QString("ROI Volume: ~%1 um^3").arg(volume));
  }
}

void ZFlyEmRoiDialog::exportRoiObjectBlockFunc(const QString &fileName)
{
  emit messageDumped("Generating ROI object ...", true);
  emit progressAdvanced(0.1);
  ZObject3dScan obj = m_project->getRoiObject(0, 0, 0);
  emit progressAdvanced(0.5);
  ZObject3dScan blockObj = m_project->getDvidInfo().getBlockIndex(obj);
  emit progressAdvanced(0.2);
  if (!obj.isEmpty()) {
    emit messageDumped("Saving ...", true);
    blockObj.save(fileName.toStdString());
    emit messageDumped("Done.", true);
    emit progressAdvanced(0.2);
  } else {
    emit messageDumped("No ROI found in this project. Nothing is saved", true);
  }
  endProgress();
}

void ZFlyEmRoiDialog::exportRoiObjectFunc(
    const QString &fileName, int xintv, int yintv, int zintv)
{
//  emit progressStart();
  emit messageDumped("Generating ROI object ...", true);
  emit progressAdvanced(0.1);
  ZObject3dScan obj = m_project->getRoiObject(xintv, yintv, zintv);
//  obj.downsampleMax(xintv, yintv, zintv);
  emit progressAdvanced(0.5);
  if (!obj.isEmpty()) {
    emit messageDumped("Saving ...", true);
    obj.save(fileName.toStdString());
    emit messageDumped("Done.", true);
    emit progressAdvanced(0.4);
  } else {
    emit messageDumped("No ROI found in this project. Nothing is saved", true);
  }
  endProgress();
  //emit progressDone();
}

void ZFlyEmRoiDialog::exportRoiBlockObject()
{
  if (m_project != NULL) {
    QString fileName = getMainWindow()->getSaveFileName(
          "Export ROI Object", "Sparse object files (*.sobj)");
    if (!fileName.isEmpty()) {
      startProgress();
      QtConcurrent::run(
            this, &ZFlyEmRoiDialog::exportRoiObjectBlockFunc, fileName);
        //endProgress();
    }
  }
}

void ZFlyEmRoiDialog::exportRoiObject()
{
  if (m_project != NULL) {
    QString fileName = getMainWindow()->getSaveFileName(
          "Export ROI Object", "Sparse object files (*.sobj)");
    if (!fileName.isEmpty()) {
      if (m_dsDlg->exec()) {
        int xIntv = m_dsDlg->getValue("X");
        int yIntv = m_dsDlg->getValue("Y");
        int zIntv = m_dsDlg->getValue("Z");

        startProgress();
        QtConcurrent::run(
              this, &ZFlyEmRoiDialog::exportRoiObjectFunc, fileName,
              xIntv, yIntv, zIntv);
        //endProgress();
      }
    }
  }
}

void ZFlyEmRoiDialog::exportResult()
{
  if (m_project != NULL) {
    ZObject3dScan obj = m_project->getRoiSlice();

    ZObject3dScan blockObj = m_project->getDvidInfo().getBlockIndex(obj);
    int minZ = blockObj.getMinZ();
    int maxZ = blockObj.getMaxZ();

    ZObject3dScan interpolated;
    for (int z = minZ; z <= maxZ; ++z) {
      interpolated.concat(blockObj.interpolateSlice(z));
    }

//    blockObj.fillHole();

    std::string fileName = GET_DATA_DIR + "/roi_" + m_project->getName();

    interpolated.save(fileName + ".sobj");

    ZJsonArray array = ZJsonFactory::MakeJsonArray(
          interpolated, ZJsonFactory::OBJECT_SPARSE);
    //ZJsonObject jsonObj;
    //jsonObj.setEntry("data", array);
    array.dump(fileName + ".json");
  }
}

void ZFlyEmRoiDialog::on_exportPushButton_clicked()
{
  exportResult();
}

int ZFlyEmRoiDialog::getNextZ() const
{
  int z = -1;
  if (m_project != NULL) {
    z = m_project->getDataZ() + ui->stepSpinBox->value();
    if (z >= m_project->getDvidInfo().getMaxZ()) {
      z = -1;
    }
  }

  return z;
}

int ZFlyEmRoiDialog::setPrevZ()
{
  int z = getPrevZ();
  ui->zSpinBox->setValue(z);
  m_project->setZ(z);

  return z;
}

int ZFlyEmRoiDialog::setNextZ()
{
  int z = getNextZ();
  ui->zSpinBox->setValue(z);
  m_project->setZ(z);

  return z;
}

int ZFlyEmRoiDialog::getPrevZ() const
{
  int z = -1;
  if (m_project != NULL) {
    z = m_project->getDataZ() - ui->stepSpinBox->value();
  }

  return z;
}

void ZFlyEmRoiDialog::on_nextSlicePushButton_clicked()
{
  loadGrayscale(setNextZ());
}

void ZFlyEmRoiDialog::on_prevSlicePushButton_clicked()
{
  loadGrayscale(setPrevZ());
}

void ZFlyEmRoiDialog::quickLoad(int z)
{
  if (z >= 0  && m_project != NULL) {
    const ZClosedCurve *curve0 = m_project->getRoi(z);
    ZClosedCurve curve;
    if (curve0 == NULL) {
      curve = m_project->estimateRoi(z);
    } else {
      curve = *curve0;
    }

    int margin = 100;
    ZCuboid cuboid = curve.getBoundBox();
    ZIntCuboid boundBox = cuboid.toIntCuboid();
    boundBox.setFirstZ(z);
    boundBox.expandX(margin);
    boundBox.expandY(margin);
    loadGrayscale(boundBox);
  }
}
void ZFlyEmRoiDialog::on_quickPrevPushButton_clicked()
{
  int z = setPrevZ();
  quickLoad(z);
}

void ZFlyEmRoiDialog::on_quickNextPushButton_3_clicked()
{
  int z = setNextZ();
  quickLoad(z);
}

void ZFlyEmRoiDialog::viewAllSynapseIn3D()
{
  if (m_project != NULL) {
    ZStackDoc *doc = m_project->makeAllSynapseDoc();
    if (doc != NULL) {
      ZWindowFactory factory;
      factory.setParentWidget(this);
      factory.setWindowTitle("Syanpse View");
      ZStack *stack = doc->getStack();
      if (stack != NULL) {
        stack->setOffset(
              stack->getOffset().getX() * (m_project->getCurrentDsIntv().getX() + 1),
              stack->getOffset().getY() * (m_project->getCurrentDsIntv().getY() + 1),
              stack->getOffset().getZ());
      }
      Z3DWindow *window = factory.make3DWindow(doc);
      window->getVolumeSource()->setXScale(
            m_project->getCurrentDsIntv().getX() + 1);
      window->getVolumeSource()->setYScale(
            m_project->getCurrentDsIntv().getY() + 1);
      window->getPunctaFilter()->setColorMode("Original Point Color");
      window->getPunctaFilter()->setSizeScale(0.5);
      window->getPunctaFilter()->setStayOnTop(false);
      window->getVolumeRaycasterRenderer()->setCompositeMode(
            "Direct Volume Rendering");
      window->setBackgroundColor(glm::vec3(0.0f), glm::vec3(0.0f));
      window->resetCamera();

      window->show();
      window->raise();
    }
  }
}

void ZFlyEmRoiDialog::runAutoStep(bool ok)
{
  if (ok) {
    m_isAutoStepping = true;
    loadGrayscale(ui->zSpinBox->value());
  } else{
    emit messageDumped("Canceling ...", true);
    m_isAutoStepping = false;
  }
}

void ZFlyEmRoiDialog::setQuickMode(bool quickMode)
{
  if (m_project != NULL) {
    if (quickMode) {
      m_project->setDsIntv(m_xintv, m_yintv, 0);
    } else {
      m_project->setDsIntv(0, 0, 0);
    }

    updateWidget();
  }
}

void ZFlyEmRoiDialog::applyTranslate()
{
  if (m_project != NULL) {
    m_project->applyTranslate();
  }
}

void ZFlyEmRoiDialog::deleteProject(ZFlyEmRoiProject *project)
{
  if (project != NULL) {
    m_projectList.removeOne(project);
    project->deleteAllData();
    delete project;
    uploadProjectList();
    //updateWidget();
  }
}

void ZFlyEmRoiDialog::deleteProject()
{
  if (m_project != NULL) {
    bool deleting = false;
    int ret = QMessageBox::warning(
          this, "Delete Project",
          QString("You are about to deleting the project '%1'. "
                  "Do you want to continue?").arg(m_project->getName().c_str()),
          QMessageBox::Yes | QMessageBox::No);
    deleting = (ret == QMessageBox::Yes);
    if (deleting) {
      ZFlyEmRoiProject *project = m_project;
      closeCurrentProject();
      ui->projectComboBox->removeItem(ui->projectComboBox->currentIndex());
      deleteProject(project);
    }
  }
}
