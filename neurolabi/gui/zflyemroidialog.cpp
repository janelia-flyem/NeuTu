#include "zflyemroidialog.h"
#include <QtConcurrentRun>
#include <QMessageBox>
#include <QCloseEvent>
#include <QInputDialog>
#include "neutubeconfig.h"
#include "ui_zflyemroidialog.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidinfo.h"
#include "dvid/zdvidwriter.h"
#include "mainwindow.h"
#include "zstackframe.h"

ZFlyEmRoiDialog::ZFlyEmRoiDialog(QWidget *parent) :
  QDialog(parent), ZProgressable(),
  ui(new Ui::ZFlyEmRoiDialog), m_project(NULL)
{
  ui->setupUi(this);

  m_dvidDlg = ZDialogFactory::makeDvidDialog(this);
  m_zDlg = ZDialogFactory::makeSpinBoxDialog(this);

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


  connect(this, SIGNAL(newDocReady()), this, SLOT(newDataFrame()));
  connect(this, SIGNAL(progressFailed()), ui->progressBar, SLOT(reset()));
  connect(this, SIGNAL(progressAdvanced(double)),
          this, SLOT(advanceProgressSlot(double)));
  connect(ui->projectComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(loadProject(int)));

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
}

ZFlyEmRoiDialog::~ZFlyEmRoiDialog()
{
  m_projectList.clear();
  delete ui;
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
  if (m_project != NULL) {
    QString text = QString("<p>DVID: %1</p>"
                           "<p>Z Range: [%2, %3]"
                           "<p>Opened Slice: Z = %4; ROI: %5</p>").
        arg(m_project->getDvidTarget().getName().c_str()).
        arg(m_project->getDvidInfo().getMinZ()).
        arg(m_project->getDvidInfo().getMaxZ()).
        arg(m_project->getDataZ()).arg(m_project->hasOpenedRoi());
    ui->infoWidget->setText(text);
    ui->zSpinBox->setValue(m_project->getDataZ());

    ui->loadGrayScalePushButton->setEnabled(m_project->getDvidTarget().isValid());
    ui->searchPushButton->setEnabled(m_project->getDvidTarget().isValid());
    ui->estimateRoiPushButton->setEnabled(m_project->hasDataFrame());
  } else {
    ui->loadGrayScalePushButton->setEnabled(false);
    ui->searchPushButton->setEnabled(false);
    ui->estimateRoiPushButton->setEnabled(false);
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
        project->downloadAllRoi();
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
  }
}


void ZFlyEmRoiDialog::setDvidTarget()
{
  if (m_dvidDlg->exec()) {
    m_dvidTarget = m_dvidDlg->getDvidTarget();

    //load all projects
    downloadAllProject();

    /*
    if (m_project != NULL) {
      m_project->setDvidTarget(m_dvidTarget);
      updateWidget();
    }
    */
  }
}

void ZFlyEmRoiDialog::loadGrayscaleFunc(int z)
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

    //int z = m_zDlg->getValue();
    //m_project->setZ(z);

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
        boundBox = ZFlyEmRoiProject::estimateBoundBox(*stack);
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
      //advance(0.5);
      emit progressAdvanced(0.5);
      m_docReader.clear();
      m_docReader.setStack(stack);

      ZSwcTree *tree = m_project->getRoiSwc(z);
      if (tree != NULL) {
        m_docReader.addObject(
              tree, NeuTube::Documentable_SWC, ZDocPlayer::ROLE_ROI);
      }
      emit newDocReady();
    } else {
      emit progressFailed();
    }
  } else {
    emit progressFailed();
  }
}

void ZFlyEmRoiDialog::newDataFrame()
{
  ZStackFrame *frame = getMainWindow()->createStackFrame(
        m_docReader, NeuTube::Document::FLYEM_ROI);
  frame->document()->setStackBackground(NeuTube::IMAGE_BACKGROUND_BRIGHT);
  setDataFrame(frame);

  getMainWindow()->addStackFrame(frame);
  getMainWindow()->presentStackFrame(frame);
  updateWidget();
  endProgress();
}

void ZFlyEmRoiDialog::loadGrayscale(int z)
{
  if (m_project == NULL) {
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
    startProgress();
    QtConcurrent::run(
          this, &ZFlyEmRoiDialog::loadGrayscaleFunc, z);
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
  return dynamic_cast<MainWindow*>(this->parentWidget());
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

void ZFlyEmRoiDialog::dump(const QString &str)
{
  ui->outputWidget->setText(str);
}

void ZFlyEmRoiDialog::setZ(int z)
{
  if (m_project == NULL) {
    return;
  }

  m_project->setZ(z);
  //updateWidget();
}

void ZFlyEmRoiDialog::previewFullRoi()
{
  ZSwcTree *tree = m_project->getAllRoiSwc();

  if (tree != NULL) {
    ZStackFrame *frame = new ZStackFrame();
    frame->document()->addSwcTree(tree);

    frame->open3DWindow(this);
    delete frame;
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


void ZFlyEmRoiDialog::on_testPushButton_clicked()
{
  ZObject3dScan obj = m_project->getRoiObject();

  obj.save(GET_DATA_DIR + "/test.sobj");

  dump(QString("%1 saved.").arg((GET_DATA_DIR + "/test.sobj").c_str()));
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

void ZFlyEmRoiDialog::loadProject(int index)
{
  if (m_project != NULL) { //will be modified for better switching
    m_project->closeDataFrame();
  }
  m_project = getProject(index);
  updateWidget();
}

bool ZFlyEmRoiDialog::isValidName(const std::string &name) const
{
  bool isValid = false;
  if (!name.empty()) {
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
  }

  return project;
}

void ZFlyEmRoiDialog::on_pushButton_clicked() //new project
{
  bool ok;
  QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
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
