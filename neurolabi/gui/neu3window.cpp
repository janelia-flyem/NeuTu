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
#include "zglobal.h"
#include "sandbox/zbrowseropener.h"
#include "flyem/zflyemmisc.h"
#include "dialogs/flyemsettingdialog.h"

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
//  m_3dwin->menuBar()->hide();
  m_3dwin->configureMenuForNeu3();
  connect(m_3dwin, SIGNAL(settingTriggered()), this, SLOT(setOption()));

  m_flyemSettingDlg = new FlyEmSettingDialog(this);
  connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(setOption()));

  setCentralWidget(m_3dwin);

  createDockWidget();
  createTaskWindow();
  createRoiWidget();
  createToolBar();

  connectSignalSlot();

  m_dataContainer->retrieveRois();
}

void Neu3Window::connectSignalSlot()
{
  connect(m_3dwin, SIGNAL(showingPuncta(bool)), this, SLOT(showSynapse(bool)));
  connect(m_3dwin, SIGNAL(showingTodo(bool)), this, SLOT(showTodo(bool)));
  connect(m_3dwin, SIGNAL(testing()), this, SLOT(test()));
  connect(m_3dwin, SIGNAL(browsing(double,double,double)),
          this, SLOT(browse(double,double,double)));
//  connect(m_3dwin, SIGNAL(keyPressed(QKeyEvent*)),
//          this, SLOT(processKeyPressed(QKeyEvent*)));
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
  connect(m_dataContainer->getCompleteDocument(), SIGNAL(bodySelectionChanged()),
          this, SLOT(updateBodyState()));
}

void Neu3Window::updateBodyState()
{
#ifdef _DEBUG_
  std::cout << "Update state: "
            << m_dataContainer->getCompleteDocument()->getSelectedBodySet(
                 neutube::BODY_LABEL_ORIGINAL).size() << " bodies" << std::endl;
#endif
  if (m_dataContainer->getCompleteDocument()->getSelectedBodySet(
        neutube::BODY_LABEL_ORIGINAL).size() == 1) {
    m_dataContainer->enableSplit(flyem::BODY_SPLIT_ONLINE);
  } else {
    m_dataContainer->disableSplit();
  }
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
    m_dataContainer->hide();
    succ = true;
    QString windowTitle = QString("%1 [%2]").
        arg(dlg->getDvidTarget().getSourceString(false).c_str()).
        arg(dlg->getDvidTarget().getLabelBlockName().c_str());
    setWindowTitle(windowTitle);
  }

  delete dlg;

  return succ;
}

void Neu3Window::setOption()
{
  m_flyemSettingDlg->loadSetting();
  m_flyemSettingDlg->exec();
}

void Neu3Window::createDockWidget()
{
  m_bodyListDock = new QDockWidget("Bodies", this);

#if 0
  FlyEmBodyInfoDialog *widget = m_dataContainer->getBodyInfoDlg();
  widget->simplify();
#endif

//  StringListDialog *widget = new StringListDialog(this);

  m_bodyListWidget = new ZBodyListWidget(this);

  connect(m_bodyListWidget, SIGNAL(bodyAdded(uint64_t)),
          this, SLOT(loadBody(uint64_t)));
  connect(m_bodyListWidget, SIGNAL(bodyRemoved(uint64_t)),
          this, SLOT(unloadBody(uint64_t)));
  connect(m_bodyListWidget, SIGNAL(bodyItemSelectionChanged(QSet<uint64_t>)),
          this, SLOT(setBodyItemSelection(QSet<uint64_t>)));
  connect(this, SIGNAL(bodySelected(uint64_t)),
          m_bodyListWidget, SLOT(selectBodyItemSliently(uint64_t)));
  connect(this, SIGNAL(bodyDeselected(uint64_t)),
          m_bodyListWidget, SLOT(deselectBodyItemSliently(uint64_t)));
  connect(getBodyDocument(), SIGNAL(bodyRemoved(uint64_t)),
          m_bodyListWidget, SLOT(removeBody(uint64_t)));

  m_bodyListDock->setWidget(m_bodyListWidget);

  m_bodyListDock->setAllowedAreas(Qt::LeftDockWidgetArea);
  m_bodyListDock->setFeatures(
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  addDockWidget(Qt::LeftDockWidgetArea, m_bodyListDock);
}

void Neu3Window::createTaskWindow() {
    QDockWidget *dockWidget = new QDockWidget("Tasks", this);
    TaskProtocolWindow *window =
        new TaskProtocolWindow(getDataDocument(), getBodyDocument(), this);

    // add connections here; for now, I'm connecting up the same way
    //  Ting connected the ZBodyListWidget, down to reusing the names
    connect(window, SIGNAL(bodyAdded(uint64_t)), this, SLOT(addBody(uint64_t)));
    connect(window, SIGNAL(allBodiesRemoved()), this, SLOT(removeAllBodies()));
    connect(window, SIGNAL(bodySelectionChanged(QSet<uint64_t>)),
            this, SLOT(setBodyItemSelection(QSet<uint64_t>)));

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
//    addDockWidget(Qt::LeftDockWidgetArea, m_roiWidget);
    tabifyDockWidget(m_bodyListDock, m_roiWidget);
}

void Neu3Window::updateRoiWidget()
{
  m_dataContainer->updateRoiWidget(m_roiWidget, m_3dwin);
}

void Neu3Window::browse(double x, double y, double z)
{
  ZBrowserOpener *bo = ZGlobal::GetInstance().getBrowserOpener();
  bo->open(ZFlyEmMisc::GetNeuroglancerPath(
             m_dataContainer->getDvidTarget(), ZIntPoint(x, y, z)));
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
  processKeyPressed(event);
}

void Neu3Window::processKeyPressed(QKeyEvent */*event*/)
{

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

  // With the optimized version of syncBodyListModel(), which no longer does the
  // expensive operation of showing all the meshes from a tar archive in the
  // ZBodyListWidget, the following steps are necessary.

  ZFlyEmProofDoc *dataDoc = getBodyDocument()->getDataDocument();
  QList<ZMesh*> meshList = ZStackDocProxy::GetGeneralMeshList(getBodyDocument());
  for (ZMesh *mesh : meshList) {
    dataDoc->deselectBody(mesh->getLabel());
  }

  getBodyDocument()->processBodySelectionChange();
}

void Neu3Window::test()
{
  m_bodyListWidget->getModel()->addBody(1);
}

void Neu3Window::setBodyItemSelection(const QSet<uint64_t> &bodySet)
{
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
  QSet<uint64_t> selectedSet;
  for (ZMesh *mesh : selected) {
    selectedSet.insert(mesh->getLabel());
  }

  QSet<uint64_t> deselectedSet;
  for (ZMesh *mesh : deselected) {
    deselectedSet.insert(mesh->getLabel());
  }

  // Make sure to update the ZFlyEmBody3dDoc's notion of selection, to avoid
  // immediate deselection of a mesh selected by clicking in the 3D view.
  // If the ZBodyListWidget has a complete list of the bodies it will trigger
  // the update to ZFlyEmBody3dDoc, but for large sets of bodies loaded from
  // a tar archive the ZBodyListWidget may not have the complete list (to avoid
  // performance problems).

  getBodyDocument()->setBodyModelSelected(selectedSet, deselectedSet);

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
  // Do not try to show all the meshes from a tar archive in the ZBodyListWidget,
  // as the large number of meshes can causes a significant slowdown.
  // But make sure to do some of the document updates that ZBodyListWidget
  // would have triggered for each of the meshes, or else they will not function
  // correctly (e.g., will not be pickable in the 3D view).

  QList<ZMesh*> meshList = ZStackDocProxy::GetGeneralMeshList(getBodyDocument());
  std::set<uint64_t> selected;
  for (ZMesh *mesh : meshList) {
    selected.insert(mesh->getLabel());
  }

  ZFlyEmProofDoc *dataDoc = getBodyDocument()->getDataDocument();
  dataDoc->setSelectedBody(selected, neutube::BODY_LABEL_MAPPED);
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
    // Don't use a fraction of 1, because then both this call and meshArchiveLoadingEnded()
    // will set the dialog to the maximum value, which seems to cause the dialog to stay open.

    if (fraction < 1.0f) {
      m_progressDialog->setValue(fraction * PROGRESS_MAX);
    }
  }
}

void Neu3Window::meshArchiveLoadingEnded()
{
  if (m_progressDialog) {
    m_progressDialog->setValue(PROGRESS_MAX);
  }
}
