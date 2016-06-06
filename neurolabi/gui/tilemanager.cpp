#include "tilemanager.h"

#include <QKeyEvent>
#include <QDebug>

#include "QsLog/QsLog.h"
#include "ui_tilemanager.h"
#include "ztilemanager.h"
#include "zqtbarprogressreporter.h"


TileManager::TileManager(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::TileManager)
{
  ui->setupUi(this);

  init();
}

TileManager::~TileManager()
{
  delete ui;
  delete m_progressReporter;
}

void TileManager::init()
{
  ui->progressBar->setVisible(false);
  m_progressReporter = new ZQtBarProgressReporter;
  m_progressReporter->setProgressBar(ui->progressBar);

  ui->tileView->setParentWindow(this);
  ui->tileView->setDragMode(QGraphicsView::ScrollHandDrag);
  //ui->tileView->setTransformationAnchor(QGraphicsView::NoAnchor);

  this->setContextMenuPolicy(Qt::CustomContextMenu);
  m_scaleFactor = 100;

  createMenu();
  connectSignalSlot();
}

void TileManager::setTileManager(ZTileManager *manager)
{
  manager->setProgressReporter(m_progressReporter);
//  manager->setScaleFactor(m_scaleFactor);
  manager->setParentView(ui->tileView);
  ui->tileView->setScene(manager);
  //ui->tileView->setSceneRect(0,0,ui->tileView->viewport()->frameSize().width(),ui->tileView->viewport()->frameSize().height());
//  ui->tileView->setSceneRect(0,0,1500,1500);
  ui->tileView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  ui->tileView->fitInView(manager->sceneRect(), Qt::KeepAspectRatio);
}

void TileManager::createMenu()
{
  m_contextMenu = new QMenu;
  m_showSwcAction = new QAction("Show SWC", this);
  m_showSwcAction->setCheckable(true);
  m_showSwcAction->setChecked(true);

  m_contextMenu->addAction(m_showSwcAction);
}

void TileManager::connectSignalSlot()
{
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(ShowContextMenu(const QPoint&)));
  connect(m_showSwcAction, SIGNAL(toggled(bool)), this, SLOT(showSwc(bool)));
}

void TileManager::ShowContextMenu(const QPoint &pos)
{
  QPoint globalPos = this->mapToGlobal(pos);
  m_contextMenu->popup(globalPos);
#if 0
  // for most widgets
  QPoint globalPos = this->mapToGlobal(pos);
  QMenu myMenu;
  myMenu.addAction("Show SWC Projection", this, SLOT(on_actionShowSWC_triggered()));
  myMenu.addAction("Turn off SWC Projection",this, SLOT(on_actionTurnOffSWC_triggered()));
  myMenu.exec(globalPos);
#endif
}

void TileManager::setDocument(ZSharedPointer<ZStackDoc> p_doc)
{
  m_doc = p_doc;
  connect(getDocument().get(),SIGNAL(swcModified()),ui->tileView,SLOT(update()));
  connect(getDocument().get(),SIGNAL(swcModified()),ui->tileView,SLOT(slotTest()));
}

void TileManager::updateView()
{
  ui->tileView->viewport()->update();
}

void TileManager::on_actionShowSWC_triggered()
{
  ui->tileView->setSWCVisibility(true);
  ui->tileView->viewport()->update();
}

void TileManager::on_actionTurnOffSWC_triggered()
{
  ui->tileView->setSWCVisibility(false);
  updateView();
}

void TileManager::showSwc(bool on)
{
  ui->tileView->setSWCVisibility(on);
  updateView();
}

void TileManager::zoom(int ds)
{
  int scaleFactorNew = m_scaleFactor + ds;
  if (scaleFactorNew > 1000) {
    scaleFactorNew = 1000;
  } else if (scaleFactorNew < 100) {
    scaleFactorNew = 100;
  }
  double s = ((qreal) scaleFactorNew)/m_scaleFactor;

  if (s != 1.0) {
    ui->tileView->scale(s, s);
    //ui->tileView->translate(ui->tileView->matrix().dx(),ui->tileView->matrix().dy());
    ui->tileView->viewport()->update();
    m_scaleFactor = scaleFactorNew;
  }
}

void TileManager::keyPressEvent(QKeyEvent *event)
{
  switch (event->key())
  {
  case Qt::Key_Equal:
    zoom(10);
    break;
  case Qt::Key_Minus:
    zoom(-10);
    break;
  default:
    break;
  }


}

void TileManager::closeProject()
{
  LINFO() << "End reconstruction";

  m_doc.reset();
  close();
  m_scaleFactor = 100;
}
