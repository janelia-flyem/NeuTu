#include "neu3window.h"

#include <QDockWidget>

#include "ui_neu3window.h"
#include "z3dwindow.h"
#include "zstackdoc.h"
#include "zdialogfactory.h"
#include "z3dapplication.h"
#include "z3dcanvas.h"
#include "neutubeconfig.h"
#include "zwindowfactory.h"
#include "widgets/flyembodyinfowidget.h"
#include "widgets/zdvidserverwidget.h"
#include "dialogs/flyembodyinfodialog.h"
#include "flyem/zflyemproofmvc.h"
#include "dialogs/zdviddialog.h"
#include "z3dpunctafilter.h"
#include "flyem/zflyembody3ddoc.h"

Neu3Window::Neu3Window(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::Neu3Window)
{
  ui->setupUi(this);

  m_toolBar = NULL;
  m_dataContainer = NULL;

#ifdef _QT5_
  m_sharedContext = new Z3DCanvas("Init Canvas", 32, 32, this);
#else
  QGLFormat format = QGLFormat();
  format.setAlpha(true);
  format.setDepth(true);
  format.setDoubleBuffer(true);
  format.setRgba(true);
  format.setSampleBuffers(true);
  //format.setStereo(true);
  m_sharedContext = new Z3DCanvas("Init Canvas", 32, 32, format, this);
#endif

//  initialize();
}

Neu3Window::~Neu3Window()
{
  delete ui;
}

void Neu3Window::initialize()
{
  initOpenglContext();

//  ZStackDoc *doc = new ZStackDoc;
//  doc->loadFile(GET_TEST_DATA_DIR + "/_system/slice15_L11.Edit.swc");

  QWidget *widget = new QWidget(this);

  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setMargin(1);
  widget->setLayout(layout);

  ZWindowFactory factory;
  factory.setControlPanelVisible(false);
  factory.setObjectViewVisible(false);
  factory.setStatusBarVisible(false);
  factory.setParentWidget(this);

//  m_3dwin = factory.make3DWindow(doc);

//  m_3dwin = Z3DWindow::Make(doc, this);
//  layout->addWidget(m_3dwin);
//  setCentralWidget(widget);

  m_3dwin = m_dataContainer->makeNeu3Window();
  ZFlyEmBody3dDoc *bodydoc =
      qobject_cast<ZFlyEmBody3dDoc*>(m_3dwin->getDocument());
  bodydoc->showTodo(false);

  setCentralWidget(m_3dwin);

  createDockWidget();
  createToolBar();

  connectSignalSlot();
}

void Neu3Window::connectSignalSlot()
{
  connect(m_3dwin, SIGNAL(showingPuncta(bool)), this, SLOT(showSynapse(bool)));
  connect(m_3dwin, SIGNAL(showingTodo(bool)), this, SLOT(showTodo(bool)));
}

void Neu3Window::initOpenglContext()
{
  if (Z3DApplication::app() == NULL) {
    ZDialogFactory::Notify3DDisabled(this);
    return;
  }

  // initGL requires a valid OpenGL context
  if (m_sharedContext != NULL) {
    // initialize OpenGL
    if (!Z3DApplication::app()->initializeGL()) {
      QString msg = Z3DApplication::app()->getErrorMessage();
      msg += ". 3D functions will be disabled.";
//      report("OpenGL Initialization", msg.toStdString(),
//             NeuTube::MSG_ERROR);
    }

    if (NeutubeConfig::getInstance().isStereoEnabled()) {
      Z3DApplication::app()->setStereoSupported(m_sharedContext->format().stereo());
    } else {
      Z3DApplication::app()->setStereoSupported(false);
    }

    m_sharedContext->hide();
  }
}

bool Neu3Window::loadDvidTarget()
{
  bool succ = false;

  ZDvidDialog *dlg = new ZDvidDialog(NULL);
  if (dlg->exec()) {
    m_dataContainer = ZFlyEmProofMvc::Make(
          dlg->getDvidTarget(), ZStackMvc::ROLE_DOCUMENT);
    succ = true;
  }

  delete dlg;

  return succ;
}

void Neu3Window::createDockWidget()
{
  QDockWidget *dockWidget = new QDockWidget(this);

  FlyEmBodyInfoDialog *widget = m_dataContainer->getBodyInfoDlg();
  widget->simplify();
//      new FlyEmBodyInfoDialog(this);
//  ZDvidServerWidget *widget = new ZDvidServerWidget(this);
//  FlyEmBodyInfoWidget *widget = new FlyEmBodyInfoWidget(this);
  dockWidget->setWidget(widget);

  dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
  dockWidget->setFeatures(
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
}

void Neu3Window::createToolBar()
{
  /*
  m_toolBar = new QToolBar;
  QAction *viewSynapseAction = new QAction("Synapses", this);
  viewSynapseAction->setIcon(QIcon(":/images/synapse.png"));
  viewSynapseAction->setCheckable(true);
  viewSynapseAction->setChecked(true);
  connect(viewSynapseAction, SIGNAL(toggled(bool)),
          this, SLOT(showSynapse(bool)));
  m_toolBar->addAction(viewSynapseAction);

  addToolBar(m_toolBar);
  */
}

void Neu3Window::keyPressEvent(QKeyEvent *event)
{
  if (m_dataContainer != NULL) {
    m_dataContainer->processKeyEvent(event);
  }
}

void Neu3Window::showSynapse(bool on)
{
  if (m_3dwin != NULL) {
//    m_3dwin->getPunctaFilter()->setVisible(on);
    ZFlyEmBody3dDoc *doc =
        qobject_cast<ZFlyEmBody3dDoc*>(m_3dwin->getDocument());
    doc->showSynapse(on);
  }
}

void Neu3Window::showTodo(bool on)
{
  if (m_3dwin != NULL) {
    ZFlyEmBody3dDoc *doc =
        qobject_cast<ZFlyEmBody3dDoc*>(m_3dwin->getDocument());
    doc->showTodo(on);
  }
}
