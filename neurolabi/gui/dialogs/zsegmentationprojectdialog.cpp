#include "zsegmentationprojectdialog.h"
#include "ui_zsegmentationprojectdialog.h"
#include "zsegmentationprojectmodel.h"
#include "mainwindow.h"
#include "zframefactory.h"
#include "zstackdocreader.h"
#include "zstackframe.h"

ZSegmentationProjectDialog::ZSegmentationProjectDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZSegmentationProjectDialog), m_model(NULL)
{
  ui->setupUi(this);
  m_model = new ZSegmentationProjectModel(this);

  ui->treeView->setExpandsOnDoubleClick(false);
  ui->treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(loadSegmentationTarget(QModelIndex)));

  createMenu();
}

ZSegmentationProjectDialog::~ZSegmentationProjectDialog()
{
  m_model->clear();
  delete ui;
}

void ZSegmentationProjectDialog::loadSegmentationTarget(
    const QModelIndex &index)
{
  prepareDataFrame();
  m_model->loadSegmentationTarget(index);
  m_model->getProject()->getDataFrame()->show();
}

MainWindow* ZSegmentationProjectDialog::getMainWindow()
{
  return qobject_cast<MainWindow*>(this->parentWidget());
}

ZStackFrame* ZSegmentationProjectDialog::newDataFrame()
{
  ZStackFrame *frame = ZFrameFactory::MakeStackFrame(
        NeuTube::Document::SEGMENTATION_TARGET);

  connect(frame, SIGNAL(closed(ZStackFrame*)),
          m_model->getProject(), SLOT(detachFrame()));

  return frame;
}

ZStackFrame *ZSegmentationProjectDialog::newDataFrame(ZStackDocReader &reader)
{
  ZStackFrame *frame = ZFrameFactory::MakeStackFrame(
        reader, NeuTube::Document::SEGMENTATION_TARGET);

  connect(frame, SIGNAL(closed(ZStackFrame*)),
          m_model->getProject(), SLOT(detachFrame()));
//  getMainWindow()->addStackFrame(frame);
//  getMainWindow()->presentStackFrame(frame);

  return frame;
}

void ZSegmentationProjectDialog::on_testPushButton_clicked()
{
  if (m_model->getProject() == NULL) {
    ZSegmentationProject *proj = new ZSegmentationProject(this);

    ZStackDocReader reader;
    ZStackFrame *dataFrame = newDataFrame(reader);

    proj->setDataFrame(dataFrame);
    m_model->setInternalData(proj);

    m_model->generateTestData();


    ui->treeView->setModel(m_model);

    getMainWindow()->addStackFrame(dataFrame);
    getMainWindow()->presentStackFrame(dataFrame);
  }
}

void ZSegmentationProjectDialog::on_updatePushButton_clicked()
{
  m_model->updateSegmentation();
}

void ZSegmentationProjectDialog::prepareDataFrame()
{
  if (m_model->getProject() != NULL) {
    if (m_model->getProject()->getDataFrame() == NULL) {
      ZStackFrame *dataFrame = newDataFrame();
      m_model->getProject()->setDataFrame(dataFrame);
      getMainWindow()->addStackFrame(dataFrame);
      getMainWindow()->presentStackFrame(dataFrame);
    }
  }
}

void ZSegmentationProjectDialog::on_openPushButton_clicked()
{
  QString fileName = getMainWindow()->getOpenFileName(
        "Load a project or a stack", "*.tif *.json");
  if (!fileName.isEmpty()) {
    ZSegmentationProject *proj = NULL;
    if (m_model->getProject() == NULL) {
      proj = new ZSegmentationProject(this);
      m_model->setInternalData(proj);
    } else {
      proj = m_model->getProject();
    }

    if (proj->getDataFrame() == NULL) {
      ZStackDocReader reader;
      ZStackFrame *dataFrame = newDataFrame(reader);

      proj->setDataFrame(dataFrame);
      m_model->loadStack(fileName);


      ui->treeView->setModel(m_model);
      getMainWindow()->addStackFrame(dataFrame);
      getMainWindow()->presentStackFrame(dataFrame);
    }
    proj->getDataFrame()->show();
  }
}

void ZSegmentationProjectDialog::on_savePushButton_clicked()
{
  if (m_model->getProject() != NULL) {
    QString fileName = m_model->getProject()->getSource();
    if (fileName.isEmpty()) {
      fileName = getMainWindow()->getSaveFileName(
            "Save a project", "*.json");
    }
    if (!fileName.isEmpty()) {
      m_model->getProject()->save(fileName);
    }
  }
}

void ZSegmentationProjectDialog::on_donePushButton_clicked()
{
  m_model->clear();
}

void ZSegmentationProjectDialog::createMenu()
{
  QMenu *mainMenu = new QMenu(this);
  ui->menuButton->setMenu(mainMenu);

  QAction *exportLeafAction = new QAction("Export Leaf Objects", this);
  connect(exportLeafAction, SIGNAL(triggered()),
          this, SLOT(exportLeafObjects()));
  mainMenu->addAction(exportLeafAction);

  QAction *exportLabelAction = new QAction("Export Label Field", this);
  connect(exportLabelAction, SIGNAL(triggered()),
          this, SLOT(exportLabelField()));
  mainMenu->addAction(exportLabelAction);
}

void ZSegmentationProjectDialog::exportLeafObjects()
{
  QString fileName = getMainWindow()->getDirectory("Export Objects");
  if (!fileName.isEmpty()) {
    m_model->getProject()->exportLeafObjects(fileName);
  }
}

void ZSegmentationProjectDialog::exportLabelField()
{
  QString fileName = getMainWindow()->getSaveFileName(
        "Export Label Field", "*.tif");
  if (!fileName.isEmpty()) {
    m_model->getProject()->exportLabelField(fileName);
  }
}
