#include "flyembodysplitprojectdialog.h"
#include "ui_flyembodysplitprojectdialog.h"
#include "mainwindow.h"
#include "zstackframe.h"
#include "zflyemnewbodysplitprojectdialog.h"

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
  connect(ui->donePushButton, SIGNAL(clicked()), this, SLOT(clear()));
  connect(ui->loadBodyPushButton, SIGNAL(clicked()), this, SLOT(loadBody()));

  updateWidget();
}

FlyEmBodySplitProjectDialog::~FlyEmBodySplitProjectDialog()
{
  delete ui;
}

void FlyEmBodySplitProjectDialog::closeEvent(QCloseEvent */*event*/)
{
  clear();
}

void FlyEmBodySplitProjectDialog::setDvidTarget(const ZDvidTarget &target)
{
  m_project.setDvidTarget(target);
}

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
  updateWidget();
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

void FlyEmBodySplitProjectDialog::showData2d()
{
  if (m_project.hasDataFrame()) {
    m_project.showDataFrame();
  } else {
    getMainWindow()->initBodySplitProject();
  }
  updateWidget();
}

void FlyEmBodySplitProjectDialog::showData3d()
{
  if (m_project.hasDataFrame()) {
    m_project.showDataFrame3d();
  }
}

void FlyEmBodySplitProjectDialog::showResult3d()
{
  m_project.showResult3d();
}

MainWindow* FlyEmBodySplitProjectDialog::getMainWindow()
{
  return dynamic_cast<MainWindow*>(this->parentWidget());
}

const ZDvidTarget& FlyEmBodySplitProjectDialog::getDvidTarget() const
{
  return m_project.getDvidTarget();
}

void FlyEmBodySplitProjectDialog::setDataFrame(ZStackFrame *frame)
{
  connect(frame, SIGNAL(destroyed()), this, SLOT(shallowClearDataFrame()));

  m_project.setDataFrame(frame);
}

void FlyEmBodySplitProjectDialog::setLoadBodyDialog(
    ZFlyEmNewBodySplitProjectDialog *dlg)
{
  m_loadBodyDlg = dlg;
}

void FlyEmBodySplitProjectDialog::loadBody()
{
  if (m_loadBodyDlg->exec()) {
    setDvidTarget(m_loadBodyDlg->getDvidTarget());
    setBodyId(m_loadBodyDlg->getBodyId());

    updateWidget();
  }
}

bool FlyEmBodySplitProjectDialog::isBodyLoaded() const
{
  return m_project.getBodyId() > 0;
}

void FlyEmBodySplitProjectDialog::updateWidget()
{
  ui->loadBodyPushButton->setEnabled(!isBodyLoaded());
  ui->view2dBodyPushButton->setEnabled(isBodyLoaded());
  ui->view3dBodyPushButton->setEnabled(m_project.hasDataFrame());
  ui->viewSplitPushButton->setEnabled(m_project.hasDataFrame());
  ui->commitPushButton->setEnabled(m_project.hasDataFrame());
  ui->donePushButton->setEnabled(isBodyLoaded());

  if (!isBodyLoaded()) {
    ui->infoWidget->setText("No body loaded.");
  } else {
    ui->infoWidget->setText(
          QString("<p>DVID Server: %1</p>"
                  "<p>Body ID: %2</p>").
          arg(m_project.getDvidTarget().getSourceString().c_str()).
          arg(m_project.getBodyId()));
  }
}
