#include "zflyemroitooldialog.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QWidget>

#include "ui_zflyemroitooldialog.h"
#include "flyem/zflyemproofmvc.h"
#include "flyem/zflyemroiproject.h"
#include "flyem/zflyemproofdoc.h"
#include "zwidgetmessage.h"

ZFlyEmRoiToolDialog::ZFlyEmRoiToolDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmRoiToolDialog)
{
  ui->setupUi(this);
  init();
}

ZFlyEmRoiToolDialog::~ZFlyEmRoiToolDialog()
{
  delete ui;
}

void ZFlyEmRoiToolDialog::init()
{
  connect(ui->projectComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(openProject(int)));
  connect(ui->show3dPushButton, SIGNAL(clicked()),
          this, SIGNAL(showing3DRoiCurve()));
  connect(ui->uploadPushButton, SIGNAL(clicked()), this, SLOT(uploadRoi()));
  connect(ui->newPushButton, SIGNAL(clicked()), this, SLOT(newProject()));
  connect(ui->nearestPushButton, SIGNAL(clicked()),
          this, SIGNAL(goingToNearestRoi()));
  connect(ui->prevPushButton, SIGNAL(clicked()), this, SLOT(prevSlice()));
  connect(ui->nextPushButton, SIGNAL(clicked()), this, SLOT(nextSlice()));
  connect(ui->estimatePushButton, SIGNAL(clicked()), this, SLOT(estimateRoi()));
  connect(ui->dvidRoiPushButton, SIGNAL(clicked()), this, SLOT(createRoiData()));

  clear();
  downloadAllProject();
}

void ZFlyEmRoiToolDialog::openProject(int index)
{
  if (index <= 0 || index > m_projectList.size()) {
    m_project = NULL;
    emit projectClosed();
  } else {
    m_project = m_projectList[index - 1];
    m_project->downloadAllRoi();
#ifdef _DEBUG_
      m_project->printSummary();
#endif
    emit projectActivited();
  }
}

bool ZFlyEmRoiToolDialog::isValidName(const QString &name) const
{
  bool isValid = false;
  if (ZFlyEmRoiProject::IsValidName(name.toStdString())) {
    isValid = true;
    foreach (ZFlyEmRoiProject *proj, m_projectList) {
      if (proj->getName() == name.toStdString()) {
        isValid = false;
      }
    }
  }

  return isValid;
}

ZFlyEmRoiProject* ZFlyEmRoiToolDialog::newProjectWithoutCheck(const QString &name)
{
  ZFlyEmRoiProject *project = new ZFlyEmRoiProject(name.toStdString(), this);
  ZWidgetMessage::ConnectMessagePipe(project, this);
  project->setDvidTarget(m_dvidReader.getDvidTarget(), false);

  return project;
}

ZFlyEmRoiProject* ZFlyEmRoiToolDialog::newProject(const QString &name)
{
  ZFlyEmRoiProject *project = NULL;
  if (isValidName(name)) {
    project = newProjectWithoutCheck(name);
  } else {
    QMessageBox::warning(
              this, "Failed to Create A Project",
              "Invalid project name: no space is allowed; "
              "no duplicated name is allowed.",
              QMessageBox::Ok);
  }

  return project;
}

void ZFlyEmRoiToolDialog::newProject()
{
  bool ok;
  QString text = QInputDialog::getText(this, tr("New ROI Project"),
                                       tr("Project name:"), QLineEdit::Normal,
                                       "", &ok);
  if (ok && !text.isEmpty()) {
    ZFlyEmRoiProject *project = newProject(text);
    if (project != NULL) {
      if (appendProject(project)) {
        if (m_projectList.size() == 1) {
          openProject(1);
        }
        uploadProjectList();
        dump(QString("Project \"%1\" is created.").
             arg(project->getName().c_str()));
      }
    }
//    updateWidget();
  }
}

void ZFlyEmRoiToolDialog::dump(const ZWidgetMessage &msg)
{
  ui->messageWidget->dump(msg.toHtmlString(), msg.isAppending());
}

void ZFlyEmRoiToolDialog::dump(const QString &msg)
{
  dump(ZWidgetMessage(msg, NeuTube::MSG_INFORMATION,
                      ZWidgetMessage::TARGET_TEXT_APPENDING));
}

void ZFlyEmRoiToolDialog::processMessage(const ZWidgetMessage &msg)
{
  dump(msg);
}

bool ZFlyEmRoiToolDialog::appendProject(ZFlyEmRoiProject *project)
{
  if (project != NULL) {
    if (!project->getName().empty()) {
      bool isValidProject = true;
      foreach (ZFlyEmRoiProject *proj, m_projectList) {
        if (proj->getName() == project->getName()) {
          isValidProject = false;
          break;
        }
      }
      if (isValidProject) {
        //project->downloadAllRoi();
        m_projectList.append(project);
        if (getDocument() != NULL) {
          project->setDataRange(getDocument()->getStack()->getBoundBox());
        }
        ui->projectComboBox->addItem(project->getName().c_str());
        return true;
      }
    }
  }

  return false;
}

int ZFlyEmRoiToolDialog::getSliceStep() const
{
  return ui->stepSpinBox->value();
}

void ZFlyEmRoiToolDialog::prevSlice()
{
  emit steppingSlice(-ui->stepSpinBox->value());
}

void ZFlyEmRoiToolDialog::nextSlice()
{
  emit steppingSlice(ui->stepSpinBox->value());
}

void ZFlyEmRoiToolDialog::goToNearestRoiSlice(int z)
{
  if (getProject() != NULL) {
    if (getProject()->hasRoi()) {
      getProject()->getNearestRoiZ(z);
    }
  }
}

void ZFlyEmRoiToolDialog::clear()
{
  ui->projectComboBox->clear();
  ui->projectComboBox->addItem("---");

  foreach (ZFlyEmRoiProject *proj, m_projectList) {
    delete proj;
  }
  m_projectList.clear();
  m_project = NULL;
  m_dvidReader.clear();
  m_dvidWriter.clear();
}

void ZFlyEmRoiToolDialog::updateDvidTarget()
{
  m_dvidReader.clear();
  m_dvidWriter.clear();

  ZFlyEmProofDoc *doc = getDocument();
  if (doc != NULL) {
    m_dvidReader.open(doc->getDvidTarget());
    m_dvidWriter.open(doc->getDvidTarget());
  }
}

void ZFlyEmRoiToolDialog::downloadAllProject()
{
  ZFlyEmProofDoc *doc = getDocument();
  if (doc != NULL) {
    m_dvidReader.open(doc->getDvidTarget());
    if (m_dvidReader.isReady()) {
      QByteArray value = m_dvidReader.readKeyValue(
            ZDvidData::GetName(ZDvidData::ROLE_ROI_CURVE), "projects");
      ZJsonArray array;
      array.decode(value.constData());
      for (size_t i = 0; i < array.size(); ++i) {
        std::string name(ZJsonParser::stringValue(array.at(i)));
        if (ZFlyEmRoiProject::IsValidName(name)) {
          ZFlyEmRoiProject *project = newProjectWithoutCheck(name.c_str());
          if (!appendProject(project)) {
            delete project;
          }
        }
      }
    }
  }
}

void ZFlyEmRoiToolDialog::uploadProjectList()
{
  if (m_dvidWriter.good()) {
    ZJsonArray array;
    foreach (ZFlyEmRoiProject *proj, m_projectList) {
      array.append(proj->getName());
    }
    m_dvidWriter.writeJsonString(
          ZDvidData::GetName(ZDvidData::ROLE_ROI_CURVE),
          "projects", array.dumpString(0));
  }
}

void ZFlyEmRoiToolDialog::updateRoi()
{
  ZFlyEmProofDoc *doc = getDocument();
  ZFlyEmRoiProject *project = getProject();

  if (doc != NULL && project != NULL) {
    project->resetRoi();
    QList<ZSwcTree*> treeList = doc->getSwcList();
    for (QList<ZSwcTree*>::iterator iter = treeList.begin();
         iter != treeList.end(); ++iter) {
      ZSwcTree *tree = *iter;
      if (tree->hasRole(ZStackObjectRole::ROLE_ROI)) {
        project->importRoiFromSwc(tree, true);
      }
    }
  }
}

void ZFlyEmRoiToolDialog::estimateRoi()
{
  emit estimatingRoi();
}

void ZFlyEmRoiToolDialog::uploadRoi()
{
  ZFlyEmRoiProject *project = getProject();
  if (project != NULL) {
    updateRoi();
#ifdef _DEBUG_
    project->printSummary();
#endif
    project->uploadRoi();
  }
}

void ZFlyEmRoiToolDialog::createRoiData()
{
  ZFlyEmRoiProject *project = getProject();
  if (project != NULL) {
    updateRoi();
    bool ok = false;
    QString roiName = QInputDialog::getText(
          this, tr("Create ROI Data"), tr("Data name:"), QLineEdit::Normal,
          project->getName().c_str(), &ok);

    if (!roiName.isEmpty() && ok) {
      project->createRoiData(roiName.toStdString(), this);
    }
  }
}

ZFlyEmProofMvc* ZFlyEmRoiToolDialog::getParentFrame() const
{
  return qobject_cast<ZFlyEmProofMvc*>(parentWidget());
}

ZFlyEmProofDoc* ZFlyEmRoiToolDialog::getDocument() const
{
  ZFlyEmProofMvc *mvc = getParentFrame();
  if (mvc != NULL) {
    return mvc->getCompleteDocument();
  }

  return NULL;
}

bool ZFlyEmRoiToolDialog::isActive() const
{
  return getProject() != NULL;
}
