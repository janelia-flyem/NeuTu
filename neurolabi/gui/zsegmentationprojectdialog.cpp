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
  connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)),
          m_model, SLOT(loadSegmentationTarget(QModelIndex)));
}

ZSegmentationProjectDialog::~ZSegmentationProjectDialog()
{
  m_model->clear();
  delete ui;
}

MainWindow* ZSegmentationProjectDialog::getMainWindow()
{
  return dynamic_cast<MainWindow*>(this->parentWidget());
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
