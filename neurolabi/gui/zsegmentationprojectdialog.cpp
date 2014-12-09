#include "zsegmentationprojectdialog.h"
#include "ui_zsegmentationprojectdialog.h"
#include "zsegmentationprojectmodel.h"
#include "mainwindow.h"
#include "zframefactory.h"
#include "zstackdocreader.h"
#include "zstackframe.h"

ZSegmentationProjectDialog::ZSegmentationProjectDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZSegmentationProjectDialog), m_model(NULL), m_dataFrame(NULL)
{
  ui->setupUi(this);
  m_model = new ZSegmentationProjectModel(this);

  ui->treeView->setExpandsOnDoubleClick(false);
  connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)),
          m_model, SLOT(loadSegmentationTarget(QModelIndex)));
}

ZSegmentationProjectDialog::~ZSegmentationProjectDialog()
{
  delete ui;
}

MainWindow* ZSegmentationProjectDialog::getMainWindow()
{
  return dynamic_cast<MainWindow*>(this->parentWidget());
}

ZStackFrame *ZSegmentationProjectDialog::newDataFrame(ZStackDocReader &reader)
{
  ZStackFrame *frame = ZFrameFactory::MakeStackFrame(reader);
//  getMainWindow()->addStackFrame(frame);
//  getMainWindow()->presentStackFrame(frame);

  return frame;
}

void ZSegmentationProjectDialog::on_testPushButton_clicked()
{
  if (m_dataFrame == NULL) {
    ZSegmentationProject *proj = new ZSegmentationProject(this);

    ZStackDocReader reader;
    m_dataFrame = newDataFrame(reader);

    proj->setDataFrame(m_dataFrame);
    m_model->setInternalData(proj);

    m_model->generateTestData();


    ui->treeView->setModel(m_model);

    getMainWindow()->addStackFrame(m_dataFrame);
    getMainWindow()->presentStackFrame(m_dataFrame);
  }
}

void ZSegmentationProjectDialog::on_updatePushButton_clicked()
{
  m_model->updateSegmentation();
}

void ZSegmentationProjectDialog::on_openPushButton_clicked()
{
  QString fileName = getMainWindow()->getOpenFileName("Load a stack", "*.tif");
  if (!fileName.isEmpty()) {
    if (m_dataFrame == NULL) {
      ZSegmentationProject *proj = new ZSegmentationProject(this);

      ZStackDocReader reader;
      m_dataFrame = newDataFrame(reader);

      proj->setDataFrame(m_dataFrame);
      m_model->setInternalData(proj);
      m_model->loadStack(fileName);
      ui->treeView->setModel(m_model);
      getMainWindow()->addStackFrame(m_dataFrame);
      getMainWindow()->presentStackFrame(m_dataFrame);
    } else {
      m_dataFrame->show();
    }
  }
}
