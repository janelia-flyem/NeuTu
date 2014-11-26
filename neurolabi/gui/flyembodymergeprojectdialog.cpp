#include "flyembodymergeprojectdialog.h"
#include "ui_flyembodymergeprojectdialog.h"

#include <QScrollBar>

#include "zdviddialog.h"
#include "mainwindow.h"
#include "zqtbarprogressreporter.h"

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
    ui->ySpinBox->setValue(ui->ySpinBox->value() - 512);
    loadSlice();
  }
}

void FlyEmBodyMergeProjectDialog::moveSliceDown()
{
  if (m_project != NULL) {
    ui->ySpinBox->setValue(ui->ySpinBox->value() + 512);
    loadSlice();
  }
}

void FlyEmBodyMergeProjectDialog::loadSlice()
{
  if (m_project != NULL) {
    int x = ui->xSpinBox->value();
    int y = ui->ySpinBox->value();
    int z = ui->zSpinBox->value();

    m_project->loadSlice(x, y, z);
  }
}

void FlyEmBodyMergeProjectDialog::createMenu()
{
  m_mainMenu = new QMenu(this);
  ui->menuPushButton->setMenu(m_mainMenu);
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
  connect(m_project, SIGNAL(newDocReady(ZStackDocReader*)),
          this, SIGNAL(newDocReady(ZStackDocReader*)));
}

void FlyEmBodyMergeProjectDialog::updateDataFrame(ZStackDocReader &docReader)
{
  if (m_project->hasDataFrame()) {
     m_project->setDocData(docReader);
  } else {
    ZStackFrame *frame = newDataFrame(docReader);
    m_project->setDataFrame(frame);
  }

//  updateWidget();
}

void FlyEmBodyMergeProjectDialog::consumeNewDoc(ZStackDocReader *docReader)
{
  updateDataFrame(*docReader);
  delete docReader;
}

void FlyEmBodyMergeProjectDialog::setDvidTarget()
{
  if (m_dvidDlg->exec()) {
    setDvidTargetD(m_dvidDlg->getDvidTarget());
    if (m_project != NULL) {
      m_project->setDvidTarget(getDvidTarget());
    }
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
