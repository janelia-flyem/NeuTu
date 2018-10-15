#include "neu3window.h"

#include <QDockWidget>
#include <QMessageBox>
#include <QProgressDialog>
#include <QTimer>
#include <QtConcurrent>

#if defined(_USE_WEBENGINE_)
#include <QWebEngineView>
#endif

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
#include "flyem/zflyemdoc3dbodystateaccessor.h"
#include "zactionlibrary.h"
#include "zarbsliceviewparam.h"
#include "flyem/zflyemarbmvc.h"
#include "zstackdocaccessor.h"
#include "zstackobjectsourcefactory.h"
#include "dialogs/zneu3sliceviewdialog.h"
#include "zrandomgenerator.h"
#include "zqtbarprogressreporter.h"
#include "flyem/zflyemmessagewidget.h"
#include "zwidgetmessage.h"
#include "flyem/zproofreadwindow.h"
#include "flyem/zflyemproofmvccontroller.h"
#include "zstackobjectaccessor.h"
#include "flyem/zflyembodyidcolorscheme.h"
#include "flyem/zflyemarbdoc.h"
#include "dvid/zdvidlabelslice.h"
#include "flyem/zflyemtaskhelper.h"
#include "flyem/zflyembodyenv.h"

#include "protocols/taskprotocoltaskfactory.h"
#include "protocols/taskbodycleave.h"
#include "protocols/taskbodyhistory.h"
#include "protocols/taskbodymerge.h"
#include "protocols/taskbodyreview.h"
#include "protocols/taskfalsesplitreview.h"
#include "protocols/tasksplitseeds.h"
#include "protocols/tasktesttask.h"

/* Implementation details:
 *
 * Neu3Window is a main window class to provide UI for neu3. It consists of
 * several control panels and a 3D window instantiated from the Z3DWindow class.
 *
 *
 */
Neu3Window::Neu3Window(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::Neu3Window)
{
  ui->setupUi(this);
  m_actionLibrary = QSharedPointer<ZActionLibrary>(new ZActionLibrary(this));

  m_testTimer = new QTimer(this);
//  m_testTimer->setInterval(1000);
  connect(m_testTimer, SIGNAL(timeout()), this, SLOT(testBodyChange()));
  connect(this, &Neu3Window::updatingSliceWidget, this, &Neu3Window::updateSliceWidget,
          Qt::QueuedConnection);
//  initialize();
}

Neu3Window::~Neu3Window()
{
  if (m_dataContainer != NULL) {
    m_dataContainer->setExiting(true);
  }
//  delete m_webView;

  delete ui;
}

void Neu3Window::createDialogs()
{
  m_flyemSettingDlg = new FlyEmSettingDialog(this);
  connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(setOption()));

  m_browseOptionDlg = new ZNeu3SliceViewDialog(this);
}

void Neu3Window::initialize()
{
  initOpenglContext();

  createMessageWidget(); //message widget needs to be created as early as
                         //possible to receive messages

//  QWidget *widget = new QWidget(this);

//  QHBoxLayout *layout = new QHBoxLayout(this);
//  layout->setMargin(1);
//  widget->setLayout(layout);

  m_3dwin = m_dataContainer->makeNeu3Window();
//  m_3dwin->menuBar()->hide();
  m_3dwin->configureMenuForNeu3();
  m_3dwin->getBodyEnv()->setWindowEnv(this);

  connect(m_3dwin, SIGNAL(settingTriggered()), this, SLOT(setOption()));
  connect(m_3dwin, SIGNAL(neutuTriggered()), this, SLOT(openNeuTu()));
  connect(m_3dwin, SIGNAL(diagnosing()), this, SLOT(diagnose()));
  ZWidgetMessage::ConnectMessagePipe(m_3dwin, this);
  ZWidgetMessage::ConnectMessagePipe(getBodyDocument(), this);
  ZWidgetMessage::DisconnectMessagePipe(getBodyDocument(), m_3dwin);

  setCentralWidget(m_3dwin);

  createDialogs();
  createDockWidget();
  createTaskWindow();
  createMessageDock();
  createRoiWidget();
  configureToolBar();

  connectSignalSlot();

  m_dataContainer->retrieveRois();
}

QAction* Neu3Window::getAction(ZActionFactory::EAction key)
{
  QAction *action = NULL;

  switch (key) {
  case ZActionFactory::ACTION_EXIT_SPLIT:
    action = m_actionLibrary->getAction(key, this, SLOT(exitSplit()));
    break;
//  case ZActionFactory::ACTION_START_SPLIT:
//    action = m_actionLibrary->getAction(key, this, SLOT(startSplit()));
//    break;
  default:
    break;
  }

  return action;
}

void Neu3Window::initGrayscaleWidget()
{
  if (m_sliceWidget == NULL) {
    LDEBUG() << "Init grayscale widget";
    m_sliceWidget = ZFlyEmArbMvc::Make(getDataDocument()->getDvidTarget());
//    ZFlyEmProofMvcController::DisableContextMenu(m_sliceWidget);
    ZFlyEmProofMvcController::Disable3DVisualization(m_sliceWidget);

    connect(m_sliceWidget, SIGNAL(sliceViewChanged(ZArbSliceViewParam)),
            this, SLOT(updateSliceViewGraph(ZArbSliceViewParam)));

    m_sliceWidget->setDefaultViewPort(
          getSliceViewParam(m_browsePos).getViewPort());

    updateSliceBrowserSelection();
    if (getDataDocument()->getDvidTarget().hasMultiscaleSegmentation()) {
      ZFlyEmProofMvcController::EnableHighlightMode(m_sliceWidget);
    }
    ZFlyEmProofMvcController::SetTodoDelegate(m_sliceWidget, getBodyDocument());
  }
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

  connect(getBodyDocument(), SIGNAL(interactionStateChanged()),
          this, SLOT(updateUI()));

  connect(getBodyDocument(), &ZFlyEmBody3dDoc::bodyMeshLoaded,
          this, &Neu3Window::zoomToBodyMesh);

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

//  connect(getBodyDocument(), &ZFlyEmBody3dDoc::bodyMeshesAdded,
//          this, &Neu3Window::syncBodyListModel);

  connect(m_dataContainer, SIGNAL(roiLoaded()), this, SLOT(updateRoiWidget()));
  connect(m_dataContainer->getCompleteDocument(), SIGNAL(bodySelectionChanged()),
          this, SLOT(updateBodyState()));

  connect(m_3dwin, SIGNAL(cameraRotated()), this, SLOT(processCameraRotation()));
//  connect(this, SIGNAL(closed()), this, SLOT(closeWebView()));
}

void Neu3Window::updateBodyState()
{
#ifdef _DEBUG_
  std::cout << "Update state: "
            << m_dataContainer->getCompleteDocument()->getSelectedBodySet(
                 neutube::EBodyLabelType::ORIGINAL).size() << " bodies" << std::endl;
#endif

#if 0
  if (m_dataContainer->getCompleteDocument()->getSelectedBodySet(
        neutube::BODY_LABEL_ORIGINAL).size() == 1) {
    m_dataContainer->enableSplit(flyem::EBodySplitMode::BODY_SPLIT_ONLINE);
  } else {
    m_dataContainer->disableSplit();
  }
#endif
}

void Neu3Window::start()
{
  show();
  initialize();
  raise();
  showMaximized();

//  initNativeSliceBrowser();
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

//  ZProgressReporter reporter;

  ZDvidTargetProviderDialog *dlg = ZDialogFactory::makeDvidDialog(NULL);

  if (dlg->exec()) {
    m_dataContainer = ZFlyEmProofMvc::Make(ZStackMvc::ROLE_DOCUMENT);
    m_dataContainer->getProgressSignal()->connectSlot(this);
    connect(m_dataContainer, &ZFlyEmProofMvc::dvidReady,
            this, &Neu3Window::start);
    ZWidgetMessage::ConnectMessagePipe(m_dataContainer, this);
    QtConcurrent::run(m_dataContainer, &ZFlyEmProofMvc::setDvidTarget,
                      dlg->getDvidTarget());
//    m_dataContainer->setDvidTarget(dlg->getDvidTarget());

    m_dataContainer->hide();
    succ = true;
    QString windowTitle = QString("%1 [%2]").
        arg(dlg->getDvidTarget().getSourceString(false).c_str()).
        arg(dlg->getDvidTarget().getSegmentationName().c_str());
    setWindowTitle(windowTitle);
  }

  delete dlg;

  return succ;
}

void Neu3Window::setOption()
{
  m_flyemSettingDlg->loadSetting();
  m_flyemSettingDlg->exec();
  GET_FLYEM_CONFIG.saveSettings();
}

void Neu3Window::createDockWidget()
{
  m_bodyListDock = new QDockWidget("Bodies", this);

#if 0
  FlyEmBodyInfoDialog *widget = m_dataContainer->getBodyInfoDlg();
  widget->simplify();
#endif

//  StringListDialog *widget = new StringListDialog(this);

  createBodyListWidget();
  m_bodyListDock->setWidget(m_bodyListWidget);

  m_bodyListDock->setAllowedAreas(Qt::LeftDockWidgetArea);

  m_bodyListDock->setFeatures(
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  addDockWidget(Qt::LeftDockWidgetArea, m_bodyListDock);
}

void Neu3Window::createBodyListWidget()
{
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
  connect(getBodyDocument(), SIGNAL(addingBody(uint64_t)),
          this, SLOT(addBody(uint64_t)));
  connect(getBodyDocument(), SIGNAL(removingBody(uint64_t)),
          this, SLOT(removeBody(uint64_t)));


  ZFlyEmDoc3dBodyStateAccessor *sa = new ZFlyEmDoc3dBodyStateAccessor;
  sa->setDocument(getBodyDocument());
  //Using an accessor object to decouple ZFlyEmBodyListModel from ZFlyEmBody3dDoc
  m_bodyListWidget->getModel()->setBodyStateAccessor(sa);

}

void Neu3Window::createMessageWidget()
{
    m_messageWidget = new ZFlyEmMessageWidget(this);
}

void Neu3Window::createMessageDock()
{
  m_messageDock = new QDockWidget("Message", this);

  m_messageDock->setWidget(m_messageWidget);
  m_messageDock->setAllowedAreas(Qt::LeftDockWidgetArea);
  m_messageDock->setFeatures(
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  addDockWidget(Qt::LeftDockWidgetArea, m_messageDock);
}

void Neu3Window::initNativeSliceBrowser()
{
  if (m_nativeSliceDock == NULL) {
    m_nativeSliceDock = new QDockWidget("Grayscale", this);
    connect(m_nativeSliceDock, SIGNAL(visibilityChanged(bool)),
            this, SLOT(processSliceDockVisibility(bool)));

    initGrayscaleWidget();

    m_nativeSliceDock->setWidget(m_sliceWidget);
    m_nativeSliceDock->setAllowedAreas(Qt::RightDockWidgetArea);
    m_nativeSliceDock->setFeatures(
          QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable |
          QDockWidget::DockWidgetClosable);
    addDockWidget(Qt::NoDockWidgetArea, m_nativeSliceDock);
    m_nativeSliceDock->setFloating(true);
    m_nativeSliceDock->setAllowedAreas(Qt::NoDockWidgetArea);
  }

  m_browseWidth = DEFAULT_BROWSE_WIDTH;
  m_browseHeight = DEFAULT_BROWSE_HEIGHT;
}

void Neu3Window::initWebView()
{
#if defined(_USE_WEBENGINE_)
  if (m_webSliceDock == NULL) {
    m_webSliceDock = new QDockWidget("Grayscale", this);
    connect(m_webSliceDock, SIGNAL(visibilityChanged(bool)),
            this, SLOT(processSliceDockVisibility(bool)));
    m_webView = new QWebEngineView(m_webSliceDock);
    m_webSliceDock->setWidget(m_webView);
    m_webSliceDock->setAllowedAreas(Qt::RightDockWidgetArea);
    m_webSliceDock->setFeatures(
          QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable |
          QDockWidget::DockWidgetClosable);
    addDockWidget(Qt::NoDockWidgetArea, m_webSliceDock);
    m_webSliceDock->setFloating(true);
    m_webSliceDock->setAllowedAreas(Qt::NoDockWidgetArea);
  }
#endif
}

void Neu3Window::createTaskWindow() {
  // set up the factory for creating "protocol tasks"
  TaskProtocolTaskFactory &factory = TaskProtocolTaskFactory::getInstance();
  factory.registerJsonCreator(TaskBodyCleave::taskTypeStatic(), TaskBodyCleave::createFromJson);
  factory.registerGuiCreator(TaskBodyCleave::menuLabelCreateFromGuiBodyId(), TaskBodyCleave::createFromGuiBodyId);
  factory.registerGuiCreator(TaskBodyCleave::menuLabelCreateFromGui3dPoint(), TaskBodyCleave::createFromGui3dPoint);
  factory.registerJsonCreator(TaskBodyHistory::taskTypeStatic(), TaskBodyHistory::createFromJson);
  factory.registerJsonCreator(TaskBodyMerge::taskTypeStatic(), TaskBodyMerge::createFromJson);
  factory.registerJsonCreator(TaskBodyReview::taskTypeStatic(), TaskBodyReview::createFromJson);
  factory.registerJsonCreator(TaskFalseSplitReview::taskTypeStatic(), TaskFalseSplitReview::createFromJson);
  factory.registerJsonCreator(TaskSplitSeeds::taskTypeStatic(), TaskSplitSeeds::createFromJson);
  factory.registerJsonCreator(TaskTestTask::taskTypeStatic(), TaskTestTask::createFromJson);

  QDockWidget *dockWidget = new QDockWidget("Tasks", this);
    m_taskProtocolWidget =
        new TaskProtocolWindow(getDataDocument(), getBodyDocument(), this);

    /*
    m_taskProtocolWidget->setWindowFlags(
          Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
          */

    // add connections here; for now, I'm connecting up the same way
    //  Ting connected the ZBodyListWidget, down to reusing the names
    connect(m_taskProtocolWidget, SIGNAL(bodyAdded(uint64_t)), this, SLOT(addBody(uint64_t)));
    connect(m_taskProtocolWidget, SIGNAL(allBodiesRemoved()), this, SLOT(removeAllBodies()));
    connect(m_taskProtocolWidget, SIGNAL(bodySelectionChanged(QSet<uint64_t>)),
            this, SLOT(setBodyItemSelection(QSet<uint64_t>)));
    connect(m_taskProtocolWidget, SIGNAL(browseGrayscale(double,double,double,const QHash<uint64_t, QColor>&)),
            this, SLOT(browse(double,double,double,const QHash<uint64_t, QColor>&)));
    connect(m_taskProtocolWidget, SIGNAL(updateGrayscaleColor(const QHash<uint64_t,QColor>&)),
            this, SLOT(updateBrowserColor(const QHash<uint64_t,QColor>&)));
    ZWidgetMessage::ConnectMessagePipe(m_taskProtocolWidget, this);

    // make the OpenGL context current in case any task's widget changes any parameters
    //  of filters or renderers that could trigger rebuilding of glsl code
    m_sharedContext->getGLFocus();

    // start up the TaskWindow UI (must come after connections are
    //  established!)
    m_taskProtocolWidget->init();

    dockWidget->setWidget(m_taskProtocolWidget);
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

/*
void Neu3Window::closeWebView()
{
#if defined(_USE_WEBENGINE_)
  if (m_webView != NULL) {
    m_webView->setAttribute(Qt::WA_DeleteOnClose);
    m_webView->close();
    m_webView = NULL;
  }
#endif
}
*/

void Neu3Window::processSliceDockVisibility(bool on)
{
  if (on == false) {
    endBrowse();
  }
}

void Neu3Window::updateSliceViewGraph(const ZArbSliceViewParam &param)
{
  if (param.isValid()) {
    Z3DGraph *graph = ZFlyEmMisc::MakeSliceViewGraph(param);

    ZStackDocAccessor::AddObjectUnique(getBodyDocument(), graph);

    m_browsePos = param.getCenter().toPoint();

#ifdef _DEBUG_
    std::cout << "Browse pos: " << m_browsePos.toString() << std::endl;
#endif
  }
}

void Neu3Window::removeSliceViewGraph()
{
  ZStackDocAccessor::RemoveObject(
        getBodyDocument(), ZStackObject::TYPE_3D_GRAPH,
        ZStackObjectSourceFactory::MakeSlicViewObjectSource(), true);
}

void Neu3Window::updateBrowseSize()
{
  if (m_browseMode == BROWSE_NATIVE) {
    if (m_sliceWidget != NULL) {
      QRect rect = m_sliceWidget->getViewPort();
      m_browseWidth = rect.width();
      m_browseHeight = rect.height();
    }
  }
}

void Neu3Window::processCameraRotation()
{
  trackSliceViewPort();
  updateBrowseSize();
  updateSliceBrowser();
//  updateBrowser();
//  updateEmbeddedGrayscale();
//  updateGrayscaleWidget();
}

void Neu3Window::trackSliceViewPort() const
{
  if (m_sliceWidget != NULL) {
    LDEBUG() << "Slice viewport:" << m_sliceWidget->getViewPort();
  }
}

void Neu3Window::updateSliceWidget()
{
  LDEBUG() << "Updating slice widget";
  if (m_sliceWidget != NULL) {
    ZArbSliceViewParam viewParam = getSliceViewParam(m_browsePos);
    m_sliceWidget->setDefaultViewPort(viewParam.getViewPort());
    m_sliceWidget->resetViewParam(viewParam);

    trackSliceViewPort();
  }
}

void Neu3Window::updateSliceBrowser()
{
  switch (m_browseMode) {
  case BROWSE_NATIVE:
    if (m_nativeSliceDock != NULL) {
      m_nativeSliceDock->show();
      LDEBUG() << "m_nativeSliceDock->show called";
      updateSliceWidget();
//      QTimer::singleShot(3000, this, &Neu3Window::updateSliceWidget);
    }
    break;
  case BROWSE_NEUROGLANCER:
    if (m_webSliceDock != NULL) {
      m_webSliceDock->show();
      updateWebView();
    }
    break;
  default:
    break;
  }
}

void Neu3Window::updateSliceBrowserSelection()
{
  ZFlyEmProofMvcController::SelectBody(
        m_sliceWidget, getBodyDocument()->getInvolvedNormalBodySet());
//        getBodyDocument()->getNormalBodySet());
}

void Neu3Window::updateBrowserColor(const QHash<uint64_t, QColor> &idToColor)
{
  if (m_sliceWidget) {
    const ZSharedPointer<ZFlyEmBodyColorScheme>
        colorMap(new ZFlyEmBodyIdColorScheme(idToColor));

    ZFlyEmArbDoc* doc = m_sliceWidget->getCompleteDocument();
    ZDvidLabelSlice* slice = doc->getDvidLabelSlice(neutube::EAxis::ARB);
    slice->setCustomColorMap(colorMap);

     updateSliceBrowserSelection();
  }
}

void Neu3Window::updateWebView()
{
#if defined(_USE_WEBENGINE_)
  if (m_webView != NULL) {
    glm::quat r = m_3dwin->getCamera()->getNeuroglancerRotation();
    ZWeightedPoint rotation;
    rotation.set(r.x, r.y, r.z);
    rotation.setWeight(r.w);

    QUrl url(ZFlyEmMisc::GetNeuroglancerPath(
               m_dataContainer->getDvidTarget(), m_browsePos.toIntPoint(),
               rotation, getBodyDocument()->getNormalBodySet()));


    m_webView->setUrl(url);
  }
#endif
}

/*
void Neu3Window::updateBrowser()
{
#if defined(_USE_WEBENGINE_)
  if (m_webView != NULL) {
    browse(m_browsePos.getX(), m_browsePos.getY(), m_browsePos.getZ());
  }
#endif
}
*/

void Neu3Window::updateGrayscaleWidget()
{
  if (m_sliceWidget != NULL) {
    browse(m_browsePos.getX(), m_browsePos.getY(), m_browsePos.getZ());
  }
}

void Neu3Window::hideGrayscale()
{
  getBodyDocument()->hideArbGrayslice();
}

/*
void Neu3Window::updateEmbeddedGrayscale()
{
  browseInPlace(m_browsePos.getX(), m_browsePos.getY(), m_browsePos.getZ());
}
*/

ZArbSliceViewParam Neu3Window::getSliceViewParam(double x, double y, double z) const
{
  ZArbSliceViewParam viewParam;
  viewParam.setSize(m_browseWidth, m_browseHeight);
  viewParam.setCenter(iround(x), iround(y), iround(z));

  std::pair<glm::vec3, glm::vec3> ort = m_3dwin->getCamera()->getLowtisVec();
  viewParam.setPlane(ZPoint(ort.first.x, ort.first.y, ort.first.z),
                     ZPoint(ort.second.x, ort.second.y, ort.second.z));

  return viewParam;
}

ZArbSliceViewParam Neu3Window::getSliceViewParam(const ZPoint &center) const
{
  return getSliceViewParam(center.x(), center.y(), center.z());
}

/*
void Neu3Window::browseInPlace(double x, double y, double z)
{
  getBodyDocument()->updateArbGraySlice(getSliceViewParam(x, y, z));
}
*/

void Neu3Window::browse(double x, double y, double z)
{
  m_browsePos.set(x, y, z);

  if (m_browseMode == BROWSE_NONE) {
    if (m_browseOptionDlg->exec()) {
      startBrowser(m_browseOptionDlg->getBrowseMode());
    }
  } else {
    updateSliceBrowser();
  }
}

void Neu3Window::browse(
    double x, double y, double z, const QHash<uint64_t, QColor> &idToColor)
{
  m_browsePos.set(x, y, z);

  if (m_browseMode == BROWSE_NONE) {
    m_browseMode = BROWSE_NATIVE;
    initNativeSliceBrowser();
  }

  updateBrowserColor(idToColor);
  updateSliceBrowser();
}

void Neu3Window::startBrowser(EBrowseMode mode)
{
  m_browseMode = mode;

  switch (mode) {
  case BROWSE_NATIVE:
  {
    initNativeSliceBrowser();
    updateSliceBrowser();
  }
    break;
  case BROWSE_NEUROGLANCER:
  {
  #if defined(_USE_WEBENGINE_)
    initWebView();
    updateSliceBrowser();
  #endif
  }
    break;
  case BROWSE_NEUROGLANCER_EXT:
  {
    m_browseMode = BROWSE_NONE;
    glm::quat r = m_3dwin->getCamera()->getNeuroglancerRotation();
    ZWeightedPoint rotation;
    rotation.set(r.x, r.y, r.z);
    rotation.setWeight(r.w);

    ZBrowserOpener *bo = ZGlobal::GetInstance().getBrowserOpener();

    bo->open(ZFlyEmMisc::GetNeuroglancerPath(
               m_dataContainer->getDvidTarget(), m_browsePos.toIntPoint(),
               rotation, m_bodyListWidget->getModel()->getBodySet()));
  }
    break;
  default:
    break;
  }
}

void Neu3Window::endBrowse()
{
  m_browseMode = BROWSE_NONE;
  removeSliceViewGraph();
}

void Neu3Window::updateUI()
{
  updateWidget();
  m_taskProtocolWidget->updateTaskInteraction();
}

void Neu3Window::updateWidget()
{
  QAction *action = getAction(ZActionFactory::ACTION_EXIT_SPLIT);

  if (getBodyDocument()->isSplitActivated()) {
    action->setVisible(true);
    m_bodyListWidget->setEnabled(false);
    m_taskProtocolWidget->setEnabled(false);
  } else {
    action->setVisible(false);
    m_bodyListWidget->setEnabled(true);
    m_taskProtocolWidget->setEnabled(true);
  }

  action = getBodyDocument()->getAction(ZActionFactory::ACTION_COMMIT_SPLIT);
  action->setVisible(getBodyDocument()->isSplitFinished());
}

void Neu3Window::exitSplit()
{
  getBodyDocument()->deactivateSplit();
}

void Neu3Window::startSplit()
{
  getBodyDocument()->activateSplitForSelected();
}


void Neu3Window::configureToolBar()
{
  QAction *action = getAction(ZActionFactory::ACTION_EXIT_SPLIT);
  action->setVisible(false);
  m_3dwin->getToolBar()->addAction(action);

  action = getBodyDocument()->getAction(ZActionFactory::ACTION_COMMIT_SPLIT);
  action->setVisible(false);
  m_3dwin->getToolBar()->addAction(action);

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

void Neu3Window::closeEvent(QCloseEvent *event)
{
  QMainWindow::closeEvent(event);

  emit closed();
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
  if (m_testTimer->isActive()) {
    m_testTimer->stop();
  } else {
    m_testTimer->start();
  }
//  m_bodyListWidget->getModel()->addBody(1);
//  testBodyChange();
}

void Neu3Window::testBodyChange()
{
  m_3dwin->setColorMode(neutube3d::LAYER_MESH, "Mesh Source");
  static ZRandomGenerator rand;

  QSet<uint64_t> bodySet = m_bodyListWidget->getModel()->getBodySet();
  if (rand.rndint(10) % 2 ==0) {
    for (uint64_t bodyId : bodySet) {
      if (rand.rndint(bodySet.size()) == 1) {
        m_bodyListWidget->removeBody(bodyId);
        break;
      }
    }
//    std::set<uint64_t> getBodyDocument()->getBodySet();
  } else {
    if (bodySet.size() < 10) {
      ZIntPoint pos;
      uint64_t bodyId = m_dataContainer->getRandomBodyId(rand, &pos);
      if (!bodySet.contains(bodyId)) {
        m_bodyListWidget->addBody(bodyId);
      }
    }
  }
}

void Neu3Window::setBodyItemSelection(const QSet<uint64_t> &bodySet)
{
  getBodyDocument()->setBodyModelSelected(bodySet);
}

namespace {
  static bool zoomToLoadedBody = true;
}

//Todo: change global zoomToLoadedBody to a member variable
void Neu3Window::enableZoomToLoadedBody(bool enable)
{
  zoomToLoadedBody = enable;
}

bool Neu3Window::zoomToLoadedBodyEnabled()
{
  return zoomToLoadedBody;
}

void Neu3Window::zoomToBodyMesh(int /*numMeshLoaded*/)
{
  if (!zoomToLoadedBodyEnabled()) {
    return;
  }

  QList<ZMesh*> meshList =
      ZStackDocProxy::GetGeneralMeshList(getBodyDocument());
  LDEBUG() << "Mesh list size:" << meshList.size();
  if (!meshList.isEmpty()) {
    ZMesh *mesh = meshList.front();
    m_3dwin->gotoPosition(mesh->getBoundBox());
  }
}

void Neu3Window::processSwcChangeFrom3D(
    QList<ZSwcTree *> selected, QList<ZSwcTree *> deselected)
{
  {
    QSet<uint64_t> labelSet = ZStackObjectAccessor::GetLabelSet(selected);
    foreach (uint64_t label, labelSet) {
      if (label > 0) {
        emit bodySelected(label);
      }
    }
  }

  {
    QSet<uint64_t> labelSet = ZStackObjectAccessor::GetLabelSet(deselected);
    foreach (uint64_t label, labelSet) {
      if (label > 0) {
        emit bodySelected(label);
      }
    }
  }
}

void Neu3Window::processMessage(const ZWidgetMessage &msg)
{
  if (msg.getTarget() == ZWidgetMessage::TARGET_TEXT ||
      msg.getTarget() == ZWidgetMessage::TARGET_TEXT_APPENDING) {
    m_messageWidget->dump(msg.toHtmlString(), msg.isAppending());
  } else if (msg.getTarget() == ZWidgetMessage::TARGET_DIALOG) {
    ZDialogFactory::PromptMessage(msg, this);
  } else {
    m_3dwin->processMessage(msg);
  }
}

bool Neu3Window::cleaving() const
{
  return m_taskProtocolWidget->isInCleavingTask();
}

bool Neu3Window::allowingSplit(uint64_t bodyId) const
{
  return m_taskProtocolWidget->allowingSplit(bodyId);
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

  LDEBUG() << "Syncing body list";
  QList<ZMesh*> meshList = ZStackDocProxy::GetBodyMeshList(getBodyDocument());
  std::set<uint64_t> selected;
  for (ZMesh *mesh : meshList) {
    selected.insert(mesh->getLabel());
  }

  QSet<uint64_t> currentBodySet = getBodyDocument()->getNormalBodySet();
  selected.insert(currentBodySet.begin(), currentBodySet.end());

  ZFlyEmProofDoc *dataDoc = getBodyDocument()->getDataDocument();
#ifdef _DEBUG_
  std::string bodyStr;
  for (uint64_t bodyId : selected) {
    bodyStr += std::to_string(bodyId) + " ";
  }
  LDEBUG() << "Syncing" << bodyStr;
#endif
  dataDoc->setSelectedBody(selected, neutube::EBodyLabelType::MAPPED);
}

static const int PROGRESS_MAX = 100;

QProgressDialog* Neu3Window::getProgressDialog()
{
  if (!m_progressDialog) {
    m_progressDialog =
        new QProgressDialog("Loading ...", QString(), 0, PROGRESS_MAX, this);
    m_progressDialog->setWindowModality(Qt::WindowModal);
    m_progressDialog->setMinimumDuration(0);
    m_progressDialog->setValue(0);
  }

  return m_progressDialog;
}

void Neu3Window::meshArchiveLoadingStarted()
{
  getProgressDialog();
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

void Neu3Window::startProgress(const QString &title, int nticks)
{
  getProgressDialog()->setRange(0, nticks);
  startProgress(title);
}

void Neu3Window::startProgress(const QString &title)
{
  getProgressDialog()->setLabelText(title);
  startProgress();
}

void Neu3Window::startProgress()
{
  getProgressDialog()->show();
}

void Neu3Window::startProgress(double /*alpha*/)
{
  //Do nothing because it is the root progress
}

void Neu3Window::advanceProgress(double dp)
{
  if (getProgressDialog()->value() < getProgressDialog()->maximum()) {
    int range = getProgressDialog()->maximum() - getProgressDialog()->minimum();
    getProgressDialog()->setValue(
          getProgressDialog()->value() + iround(dp * range));
  }
}

void Neu3Window::endProgress()
{
  getProgressDialog()->reset();
}

void Neu3Window::openNeuTu()
{
  ZProofreadWindow *window = ZProofreadWindow::Make();
  window->show();

  window->getMainMvc()->setDvidTarget(m_dataContainer->getDvidTarget());
//  window->showMaximized();
}

void Neu3Window::diagnose()
{
  m_bodyListWidget->diagnose();
//  getBodyDocument()->logInfo();
}

void Neu3Window::on_actionNeuTu_Proofread_triggered()
{
  openNeuTu();
}

void Neu3Window::on_actionDiagnose_triggered()
{
  diagnose();
}
