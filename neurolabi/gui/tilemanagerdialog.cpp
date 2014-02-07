#include "tilemanagerdialog.h"
#include "ui_tilemanagerdialog.h"
#include "ztilemanager.h"
#include "zqtbarprogressreporter.h"

TileManagerDialog::TileManagerDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::TileManagerDialog)
{
  ui->setupUi(this);
  ui->progressBar->setVisible(false);
  m_progressReporter = new ZQtBarProgressReporter;
  m_progressReporter->setProgressBar(ui->progressBar);
}

TileManagerDialog::~TileManagerDialog()
{
  delete ui;
  delete m_progressReporter;
}

void TileManagerDialog::setTileManager(ZTileManager *manager)
{
  manager->setProgressReporter(m_progressReporter);
  ui->tileView->setScene(manager);
}
