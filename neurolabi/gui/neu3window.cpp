#include "neu3window.h"

#include <QDockWidget>
#include <QMessageBox>

#include <QProgressDialog>

#include "ui_neu3window.h"
#include "z3dwindow.h"
#include "zstackdoc.h"
#include "zdialogfactory.h"
#include "zsysteminfo.h"
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
#include "flyem/zflyemproofdoc.h"
#include "zstackdochelper.h"
#include "dialogs/stringlistdialog.h"
#include "widgets/zbodylistwidget.h"
#include "widgets/taskprotocolwindow.h"
#include "flyem/zflyembodylistmodel.h"
#include "zroiwidget.h"
#include "zstackdocproxy.h"

Neu3Window::Neu3Window(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::Neu3Window)
{
  ui->setupUi(this);

//  initialize();
}

Neu3Window::~Neu3Window()
{
  if (m_dataContainer != NULL) {
    m_dataContainer->setExiting(true);
  }

  delete ui;
}

void Neu3Window::initialize()
{
  initOpenglContext();

  QWidget *widget = new QWidget(this);

  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setMargin(1);
  widget->setLayout(layout);

  m_3dwin = m_dataContainer->makeNeu3Window();

  setCentralWidget(m_3dwin);

  createDockWidget();
  createTaskWindow();
  createRoiWidget();
  createToolBar();

  connectSignalSlot();
}

void Neu3Window::connectSignalSlot()
{
  connect(m_3dwin, SIGNAL(showingPuncta(bool)), this, SLOT(showSynapse(bool)));
  connect(m_3dwin, SIGNAL(showingTodo(bool)), this, SLOT(showTodo(bool)));
  connect(m_3dwin, SIGNAL(testing()), this, SLOT(test()));
  connect(getBodyDocument(), SIGNAL(swcSelectionChanged(QList<ZSwcTree*>,QList<ZSwcTree*>)),
          this, SLOT(processSwcChangeFrom3D(QList<ZSwcTree*>,QList<ZSwcTree*>)));
  connect(getBodyDocument(), SIGNAL(meshSelectionChanged(QList<ZMesh*>,QList<ZMesh*>)),
          this, SLOT(processMeshChangedFrom3D(QList<ZMesh*>,QList<ZMesh*>)));

  connect(getBodyDocument(), &ZFlyEmBody3dDoc::bodyMeshLoaded,
          this, &Neu3Window::zoomToBodyMesh, Qt::QueuedConnection);

  connect(getBodyDocument(), SIGNAL(meshArchiveLoadingStarted()),
          this, SLOT(meshArchiveLoadingStarted()));
  connect(getBodyDocument(), SIGNAL(meshArchiveLoadingProgress(float)),
          this, SLOT(meshArchiveLoadingProgress(float)));
  connect(getBodyDocument(), SIGNAL(meshArchiveLoadingEnded()),
          this, SLOT(meshArchiveLoadingEnded()));

  // Loading an ID that corresponds to an archive trigers the loading of other meshes
  // and the ZBodyListWidget needs to show them.  The synchronizing of that widget
  // with the body list is most efficient if it occurs on the single bodyMeshesAdded
  // signal emitted after all the meshes are loaded, not on the multiple bodyMeshLoaded
  // signals emitted with each mesh.

  connect(getBodyDocument(), &ZFlyEmBody3dDoc::bodyMeshesAdded,
          this, &Neu3Window::syncBodyListModel, Qt::QueuedConnection);
  connect(m_dataContainer, SIGNAL(roiLoaded()), this, SLOT(updateRoiWidget()));
}

void Neu3Window::initOpenglContext()
{
  m_sharedContext = new Z3DCanvas("Init Canvas", 32, 32, this);
  m_sharedContext->show();

  // initialize OpenGL
  if (!ZSystemInfo::instance().initializeGL()) {
    QString msg = ZSystemInfo::instance().errorMessage();
    msg += ". 3D functions will be disabled.";
    QMessageBox::warning(this, qApp->applicationName(), "OpenGL Initialization.\n" + msg);
  }

  ZSystemInfo::instance().setStereoSupported(m_sharedContext->format().stereo());
  m_sharedContext->hide();
}

bool Neu3Window::loadDvidTarget()
{
  bool succ = false;

  ZDvidDialog *dlg = new ZDvidDialog(NULL);
  if (dlg->exec()) {
    m_dataContainer = ZFlyEmProofMvc::Make(
          dlg->getDvidTarget(), ZStackMvc::ROLE_DOCUMENT);
    succ = true;
    QString windowTitle = QString("%1 [%2]").
        arg(dlg->getDvidTarget().getSourceString(false).c_str()).
        arg(dlg->getDvidTarget().getLabelBlockName().c_str());
    setWindowTitle(windowTitle);

    m_dataContainer->getROIs();
  }

  delete dlg;

  return succ;
}

void Neu3Window::createDockWidget()
{
  QDockWidget *dockWidget = new QDockWidget("Bodies", this);

#if 0
  FlyEmBodyInfoDialog *widget = m_dataContainer->getBodyInfoDlg();
  widget->simplify();
#endif

//  StringListDialog *widget = new StringListDialog(this);

  m_bodyListWidget = new ZBodyListWidget(this);

  connect(m_bodyListWidget, SIGNAL(bodyAdded(uint64_t)), this, SLOT(loadBody(uint64_t)));
  connect(m_bodyListWidget, SIGNAL(bodyRemoved(uint64_t)), this, SLOT(unloadBody(uint64_t)));
  connect(m_bodyListWidget, SIGNAL(bodySelectionChanged(QSet<uint64_t>)),
          this, SLOT(setBodySelection(QSet<uint64_t>)));
  connect(this, SIGNAL(bodySelected(uint64_t)),
          m_bodyListWidget, SLOT(selectBodySliently(uint64_t)));
  connect(this, SIGNAL(bodyDeselected(uint64_t)),
          m_bodyListWidget, SLOT(deselectBodySliently(uint64_t)));
  connect(getBodyDocument(), SIGNAL(bodyRemoved(uint64_t)),
          m_bodyListWidget, SLOT(removeBody(uint64_t)));

  dockWidget->setWidget(m_bodyListWidget);

  dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
  dockWidget->setFeatures(
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
}

void Neu3Window::createTaskWindow() {
    QDockWidget *dockWidget = new QDockWidget("Tasks", this);
    TaskProtocolWindow *window = new TaskProtocolWindow(getDataDocument(), getBodyDocument(), this);

    // add connections here; for now, I'm connecting up the same way
    //  Ting connected the ZBodyListWidget, down to reusing the names
    connect(window, SIGNAL(bodyAdded(uint64_t)), this, SLOT(addBody(uint64_t)));
    connect(window, SIGNAL(allBodiesRemoved()), this, SLOT(removeAllBodies()));
    connect(window, SIGNAL(bodySelectionChanged(QSet<uint64_t>)),
            this, SLOT(setBodySelection(QSet<uint64_t>)));

    // start up the TaskWindow UI (must come after connections are
    //  established!)
    window->init();

    dockWidget->setWidget(window);
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
    dockWidget->setFeatures(
          QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
}

void Neu3Window::createRoiWidget() {
//    QDockWidget *dockWidget = new QDockWidget(this);
    m_roiWidget = new ZROIWidget("ROI", this);

//    dockWidget->setWidget(widget);
    m_roiWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
    m_roiWidget->setFeatures(
          QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, m_roiWidget);
}

void Neu3Window::updateRoiWidget()
{
  m_dataContainer->updateRoiWidget(m_roiWidget, m_3dwin);
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

ZFlyEmBody3dDoc* Neu3Window::getBodyDocument() const
{
  ZFlyEmBody3dDoc *doc = NULL;
  if (m_3dwin != NULL) {
    doc = qobject_cast<ZFlyEmBody3dDoc*>(m_3dwin->getDocument());
  }

  return doc;
}

ZFlyEmProofDoc* Neu3Window::getDataDocument() const
{
  ZFlyEmProofDoc *doc = NULL;
  if (m_dataContainer != NULL) {
    doc = m_dataContainer->getCompleteDocument();
  }

  return doc;
}

void Neu3Window::addBody(uint64_t bodyId)
{
  m_bodyListWidget->getModel()->addBody(bodyId);
}

class Neu3Window::DoingBulkUpdate
{
public:
  DoingBulkUpdate(Neu3Window *w) : m_window(w) { m_window->m_doingBulkUpdate = true; }
  ~DoingBulkUpdate() { m_window->m_doingBulkUpdate = false; }
private:
  Neu3Window *m_window;
};

void Neu3Window::loadBody(uint64_t bodyId)
{
  m_dataContainer->selectBody(bodyId, m_doingBulkUpdate);
}

void Neu3Window::unloadBody(uint64_t bodyId)
{
  m_dataContainer->deselectBody(bodyId, m_doingBulkUpdate);
}

void Neu3Window::removeBody(uint64_t bodyId)
{
  m_bodyListWidget->getModel()->removeBody(bodyId);
}

void Neu3Window::removeAllBodies()
{
  // Supress some expensive and unnecessary updates after each body removal.
  DoingBulkUpdate doingBulkUpdate(this);

  m_bodyListWidget->getModel()->removeAllBodies();
}

void Neu3Window::test()
{
  m_bodyListWidget->getModel()->addBody(1);
}

void Neu3Window::setBodySelection(const QSet<uint64_t> &bodySet)
{
  std::set<uint64_t> tmpBodySet;
  tmpBodySet.insert(bodySet.begin(), bodySet.end());

  getBodyDocument()->setBodyModelSelected(bodySet);
}

namespace {
  static bool zoomToLoadedBody = true;
}

void Neu3Window::enableZoomToLoadedBody(bool enable)
{
  zoomToLoadedBody = enable;
}

bool Neu3Window::zoomToLoadedBodyEnabled()
{
  return zoomToLoadedBody;
}

void Neu3Window::zoomToBodyMesh()
{
  if (!zoomToLoadedBodyEnabled()) {
    return;
  }

  QList<ZMesh*> meshList = getBodyDocument()->getMeshList();
  if (meshList.size() == 1) {
    ZMesh *mesh = meshList.front();
    m_3dwin->gotoPosition(mesh->getBoundBox());
  }
}

void Neu3Window::processSwcChangeFrom3D(
    QList<ZSwcTree *> selected, QList<ZSwcTree *> deselected)
{
  foreach (ZSwcTree *tree, selected) {
    if (tree->getLabel() > 0) {
      emit bodySelected(tree->getLabel());
    }
  }

  foreach (ZSwcTree *tree, deselected) {
    if (tree->getLabel() > 0) {
      emit bodyDeselected(tree->getLabel());
    }
  }
}

void Neu3Window::processMeshChangedFrom3D(
    QList<ZMesh *> selected, QList<ZMesh *> deselected)
{
  foreach (ZMesh *mesh, selected) {
    if (mesh->getLabel() > 0) {
      emit bodySelected(mesh->getLabel());
    }
  }

  foreach (ZMesh *mesh, deselected) {
    if (mesh->getLabel() > 0) {
      emit bodyDeselected(mesh->getLabel());
    }
  }
}

void Neu3Window::syncBodyListModel()
{
  // Supress some expensive and unnecessary updates after each body removal.
  DoingBulkUpdate doingBulkUpdate(this);

  QList<ZMesh*> meshList = ZStackDocProxy::GetGeneralMeshList(getBodyDocument());
//      getBodyDocument()->getMeshList();
  ZFlyEmBodyListModel *listModel = m_bodyListWidget->getModel();


  QList<int> rowsToRemove;
  //Remove rows that are not in the document
  for (int i = 0; i < listModel->rowCount(); i++) {
    bool found = false;
    for (ZMesh *mesh : meshList) {
      if (mesh->getLabel() > 0) {
        if (mesh->getLabel() == listModel->getBodyId(i)) {
          found = true;
          break;
        }
      }
    }
    if (!found) {
      rowsToRemove.append(i);
    }
  }
  listModel->removeRowList(rowsToRemove);

  for (ZMesh *mesh : meshList) {
    if (mesh->getLabel() > 0) {
      listModel->addBody(mesh->getLabel());
    }
  }
}

static const int PROGRESS_MAX = 100;

void Neu3Window::meshArchiveLoadingStarted()
{
  if (!m_progressDialog) {
    m_progressDialog =
        new QProgressDialog("Loading meshes", QString(), 0, PROGRESS_MAX, this);
    m_progressDialog->setWindowModality(Qt::WindowModal);
    m_progressDialog->setMinimumDuration(0);
    m_progressDialog->setValue(0);
  }
}

void Neu3Window::meshArchiveLoadingProgress(float fraction)
{
  if (m_progressDialog) {
    m_progressDialog->setValue(fraction * PROGRESS_MAX);
  }
}

void Neu3Window::meshArchiveLoadingEnded()
{
  if (m_progressDialog) {
    m_progressDialog->setValue(PROGRESS_MAX);
  }
}
