#include "z3dwindow.h"

#include "z3dgl.h"
#include "z3dview.h"
#include <iostream>
#include <sstream>
#include <limits>
#include <QToolBar>

#include <QDesktopWidget>
#include <QMenuBar>
#include <QInputDialog>
#include <QLineEdit>
#include <QMimeData>
#include <QMessageBox>
#include <QInputDialog>
#include <QThread>
#include <QApplication>

#include "zstack.hxx"
#include "mvc/zstackdoc.h"
#include "zstackdocproxy.h"
#include "flyem/zflyembody3ddochelper.h"
#include "mvc/zstackframe.h"

#include "neutubeconfig.h"
#include "zglobal.h"
#include "qt/gui/utilities.h"
#include "qt/gui/loghelper.h"

#include "logging/utilities.h"
#include "logging/zlog.h"

#include "z3dpunctafilter.h"
#include "z3dswcfilter.h"
#include "z3dcompositor.h"
#include "zpunctum.h"
#include "zlocsegchain.h"
#include "z3dcanvas.h"
#include "z3dgraphfilter.h"
#include "zswcnetwork.h"
#include "zcloudnetwork.h"
#include "znormcolormap.h"
#include "swctreenode.h"
#include "dialogs/swctypedialog.h"
#include "dialogs/swcsizedialog.h"
#include "dialogs/swcskeletontransformdialog.h"
#include "zswcbranch.h"
#include "zswcdisttrunkanalyzer.h"
#include "zswcbranchingtrunkanalyzer.h"
#include "zfiletype.h"
#include "zswcsizetrunkanalyzer.h"
#include "zswcweighttrunkanalyzer.h"
#include "tubemodel.h"
#include "dialogs/informationdialog.h"
#include "zmoviescene.h"
#include "zmovieactor.h"
#include "zswcmovieactor.h"
#include "zmoviemaker.h"
#include "zmoviescript.h"
#include "zobjsmanagerwidget.h"
#include "zswcobjsmodel.h"
//#include "zpunctaobjsmodel.h"
#include "zdialogfactory.h"
#include "qcolordialog.h"
#include "dialogs/zalphadialog.h"
#include "zstring.h"
#include "zpunctumio.h"
#include "zswcglobalfeatureanalyzer.h"
#include "misc/miscutility.h"
#include "zstackdocmenufactory.h"
#include "swc/zswcsubtreeanalyzer.h"
#include "biocytin/zbiocytinfilenameparser.h"
#include "zvoxelgraphics.h"
#include "zstroke2darray.h"
#include "zswcgenerator.h"
#include "zstroke2d.h"
#include "zsparsestack.h"
#include "dialogs/zmarkswcsomadialog.h"
#include "zinteractivecontext.h"
#include "zwindowfactory.h"
#include "zstackviewparam.h"
#include "z3drendererbase.h"
#include "z3dsurfacefilter.h"
#include "zroiwidget.h"
#include "flyem/zflyemtodolistfilter.h"
#include "flyem/zflyembody3ddoc.h"
#include "flyem/zflyemproofdoc.h"
#include "flyem/zflyemtodoitem.h"
#include "zactionlibrary.h"
#include "zmenufactory.h"
#include "widgets/z3dtabwidget.h"
#include "dialogs/zswcisolationdialog.h"
#include "dialogs/helpdialog.h"
#include "z3dinteractionhandler.h"
#include "dialogs/zcomboeditdialog.h"
#include "dialogs/zflyembodycomparisondialog.h"
#include "z3dmeshfilter.h"
#include "zstackobjectsourcefactory.h"
#include "sandbox/zbrowseropener.h"
#include "zwidgetmessage.h"
#include "common/utilities.h"
#include "mvc/zstackdochelper.h"
#include "z3dwindowcontroller.h"
#include "data3d/zstackobjectconfig.h"
#include "flyem/zflyembodyenv.h"
#include "dialogs/zflyemtodoannotationdialog.h"
#include "dialogs/zflyemtodofilterdialog.h"
#include "zstackdocaccessor.h"

/*
class Sleeper : public QThread
{
public:
    static void usleep(unsigned long usecs){QThread::usleep(usecs);}
    static void msleep(unsigned long msecs){QThread::msleep(msecs);}
    static void sleep(unsigned long secs){QThread::sleep(secs);}
};
*/

Z3DWindow::Z3DWindow(
    ZSharedPointer<ZStackDoc> doc, Z3DView::EInitMode initMode,
    neutu3d::EWindowType windowType,
    bool stereoView, QWidget *parent)
  : QMainWindow(parent)
  , m_windowType(windowType)
  , m_doc(doc)
  , m_isClean(false)
  , m_blockingTraceMenu(false)
  , m_widgetsGroup(NULL)
  , m_settingsDockWidget(NULL)
  , m_objectsDockWidget(NULL)
  , m_advancedSettingDockWidget(NULL)
  , m_toolBar(NULL)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setFocusPolicy(Qt::StrongFocus);

  createActions();
  createMenus();
  createStatusBar();
  m_viewMenu->addAction("Reset Camera", this, SLOT(resetCamera()));
  m_viewMenu->addAction("Zoom to Selected Meshes", this, SLOT(zoomToSelectedMeshes()),
                        QKeySequence("m"));


  m_view = new Z3DView(m_doc.get(), initMode, stereoView, this);

  ZWidgetMessage::ConnectMessagePipe(m_doc.get(), this);
  /*
  connect(m_doc.get(), SIGNAL(messageGenerated(ZWidgetMessage)),
          this, SLOT(processMessage(ZWidgetMessage)));
          */

  setCentralWidget(getCanvas());
  connect(m_view, &Z3DView::networkConstructed, this, &Z3DWindow::init);

  createDockWindows(); // empty docks
  createContextMenu();
  customizeContextMenu();

  connect(m_view, &Z3DView::networkConstructed,
          this, &Z3DWindow::fillDockWindows);  // fill in real widgets later

  setAcceptDrops(true);
  m_mergedContextMenu = new QMenu(this);
  m_contextMenu = NULL;

  if (m_doc->getStack() != NULL) {
    setWindowTitle(m_doc->stackSourcePath().c_str());
  }

  m_doc->registerUser(this);
  createToolBar();

  m_buttonStatus[0] = true;  // showgraph
  m_buttonStatus[1] = false; // settings
  m_buttonStatus[2] = false; // objects
  m_buttonStatus[3] = false; // ROIs

  setWindowType(neutu3d::EWindowType::GENERAL);

  m_cuttingStackBound = false;

  m_dvidDlg = NULL;
  m_bodyCmpDlg = NULL;

  m_bodyEnv = ZSharedPointer<ZFlyEmBodyEnv>(new ZFlyEmBodyEnv);
}

Z3DWindow::~Z3DWindow()
{
  cleanup();

  delete m_actionLibrary;
  delete m_menuFactory;
}

void Z3DWindow::createStatusBar()
{
  statusBar()->showMessage("3D window ready.");
}

QToolBar* Z3DWindow::getToolBar() const
{
  return m_toolBar;
}

void Z3DWindow::createToolBar()
{
  if (getWindowType() == neutu3d::EWindowType::COARSE_BODY ||
      getWindowType() == neutu3d::EWindowType::BODY ||
      getWindowType() == neutu3d::EWindowType::SKELETON ||
      getWindowType() == neutu3d::EWindowType::MESH ||
      getWindowType() == neutu3d::EWindowType::NEU3) {
    m_toolBar = addToolBar("View");
    QAction *viewSynapseAction = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_SHOW_SYNAPSE, this);
    connect(viewSynapseAction, SIGNAL(toggled(bool)),
            this, SLOT(showPuncta(bool)));
    m_toolBar->addAction(viewSynapseAction);

    m_toolBar->addAction(getAction(ZActionFactory::ACTION_SHOW_TODO));
//    m_toolBar->addAction(getAction(ZActionFactory::ACTION_ACTIVATE_TODO_ITEM));
    m_toolBar->addAction(getAction(ZActionFactory::ACTION_ACTIVATE_TOSPLIT_ITEM));
    m_toolBar->addAction(getAction(ZActionFactory::ACTION_ACTIVATE_LOCATE));
    m_toolBar->addAction(getAction(ZActionFactory::ACTION_VIEW_DATA_EXTERNALLY));

//    QActionGroup *group = new QActionGroup(this);
//    group->addAction(getAction(ZActionFactory::ACTION_ACTIVATE_TOSPLIT_ITEM));
//    group->addAction(getAction(ZActionFactory::ACTION_ACTIVATE_LOCATE));
//    group->addAction(getAction(ZActionFactory::ACTION_VIEW_DATA_EXTERNALLY));

  }

  if (getWindowType() == neutu3d::EWindowType::NEU3) {
//    m_meshOpacitySlider = new QSlider(Qt::Horizontal, this);
//    m_meshOpacitySlider->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
//    m_meshOpacitySlider->setRange(0, 255);
//    m_toolBar->addWidget(m_meshOpacitySlider);

    m_meshOpacitySpinBox = new QDoubleSpinBox(this);
    m_meshOpacitySpinBox->setPrefix("Mesh Opacity: ");
    m_meshOpacitySpinBox->setRange(0, 1);
    m_meshOpacitySpinBox->setSingleStep(0.1);
    m_toolBar->addWidget(m_meshOpacitySpinBox);

    m_toolBar->addSeparator();
    m_toolBar->addAction(getAction(ZActionFactory::ACTION_SAVE_SPLIT_TASK));
    m_toolBar->addAction(getAction(ZActionFactory::ACTION_DELETE_SELECTED_SPLIT_SEED));
    m_toolBar->addAction(getAction(ZActionFactory::ACTION_DELETE_SPLIT_SEED));
  }

#if defined(_DEBUG_) && defined(_NEU3_)
  m_toolBar->addAction(getAction(ZActionFactory::ACTION_TEST));
#endif
}

void Z3DWindow::configureMenuForNeu3()
{
  m_helpAction->setVisible(false);
  QAction *settingAction = new QAction("&Settings", this);
  m_helpMenu->addAction(settingAction);
  connect(settingAction, SIGNAL(triggered()), this, SIGNAL(settingTriggered()));

  QAction *neutuAction = new QAction("NeuTu", this);
  m_helpMenu->addAction(neutuAction);
  connect(neutuAction, SIGNAL(triggered()), this, SIGNAL(neutuTriggered()));

  m_helpMenu->addAction(getAction(ZActionFactory::ACTION_ABOUT));
}

void Z3DWindow::zoomToSelectedSwcNodes()
{
  std::set<Swc_Tree_Node*> nodeSet = m_doc->getSelectedSwcNodeSet();
  if (!nodeSet.empty()) {
    ZCuboid cuboid = SwcTreeNode::boundBox(nodeSet);
    gotoPosition(cuboid);
  }
}

void Z3DWindow::init()
{
  connect(getDocument(), SIGNAL(stackBoundBoxChanged()),
          this, SLOT(updateCuttingBox()));

  connect(getPunctaFilter(), SIGNAL(punctumSelected(ZPunctum*, bool)),
          this, SLOT(selectedPunctumChangedFrom3D(ZPunctum*, bool)));
  if (getMeshFilter()) {
    connect(getMeshFilter(), SIGNAL(meshSelected(ZMesh*, bool)),
            this, SLOT(selectedMeshChangedFrom3D(ZMesh*, bool)));
  }
  connect(getSwcFilter(), SIGNAL(treeSelected(ZSwcTree*,bool)),
          this, SLOT(selectedSwcChangedFrom3D(ZSwcTree*,bool)));
  connect(getSwcFilter(), SIGNAL(treeNodeSelected(Swc_Tree_Node*,bool)),
          this, SLOT(selectedSwcTreeNodeChangedFrom3D(Swc_Tree_Node*,bool)));

  connect(getSwcFilter(), SIGNAL(treeNodeSelectConnection(Swc_Tree_Node*)),
          m_doc.get(), SLOT(selectSwcNodeConnection(Swc_Tree_Node*)));
  connect(getSwcFilter(), SIGNAL(treeNodeSelectFloodFilling(Swc_Tree_Node*)),
          m_doc.get(), SLOT(selectSwcNodeFloodFilling(Swc_Tree_Node*)));
  connect(getSwcFilter(), SIGNAL(addNewSwcTreeNode(double, double, double, double)),
          this, SLOT(addNewSwcTreeNode(double, double, double, double)));
  connect(getSwcFilter(), SIGNAL(extendSwcTreeNode(double, double, double, double)),
          this, SLOT(extendSwcTreeNode(double, double, double, double)));
  connect(getSwcFilter(), SIGNAL(connectingSwcTreeNode(Swc_Tree_Node*)), this,
          SLOT(connectSwcTreeNode(Swc_Tree_Node*)));

  if (getTodoFilter()) {
    connect(getTodoFilter(), SIGNAL(objectSelected(ZStackObject*,bool)),
            this, SLOT(selectedTodoChangedFrom3D(ZStackObject*,bool)));
    connect(getTodoFilter(), SIGNAL(annotatingObject(ZStackObject*)),
            this, SLOT(annotateTodo(ZStackObject*)));
//    connect(getTodoFilter(), SIGNAL(objectSelected(ZStackObject*,bool)),
//            this, SLOT(selectedObjectChangedFrom3D(ZStackObject*,bool)));
  }

  if (getGraphFilter() != NULL) {
    connect(getGraphFilter(), SIGNAL(objectSelected(ZStackObject*,bool)),
            this, SLOT(selectedGraphChangedFrom3D(ZStackObject*,bool)));
  }

  connect(m_doc.get(), SIGNAL(statusMessageUpdated(QString)),
          this, SLOT(notifyUser(QString)));

  // init windows size based on data
  setWindowSize();

  //
  getCanvas()->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(getCanvas(), SIGNAL(customContextMenuRequested(QPoint)),
          this, SLOT(show3DViewContextMenu(QPoint)));

  connect(getVolumeFilter(),
          SIGNAL(pointInVolumeLeftClicked(QPoint, glm::ivec3, Qt::KeyboardModifiers)),
          this, SLOT(pointInVolumeLeftClicked(QPoint, glm::ivec3, Qt::KeyboardModifiers)));



  connect(getInteractionHandler(), SIGNAL(objectsMoved(double,double,double)),
          this, SLOT(moveSelectedObjects(double,double,double)));
  connect(getCanvas()->getInteractionEngine(), SIGNAL(selectingSwcNodeInRoi(bool)),
          this, SLOT(selectSwcTreeNodeInRoi(bool)));
  connect(getCanvas()->getInteractionEngine(), SIGNAL(selectingSwcNodeTreeInRoi(bool)),
          this, SLOT(selectSwcTreeNodeTreeInRoi(bool)));
  connect(getCanvas()->getInteractionEngine(),
            SIGNAL(selectingTerminalBranchInRoi(bool)),
            this, SLOT(selectTerminalBranchInRoi(bool)));
  connect(getCanvas()->getInteractionEngine(), SIGNAL(croppingSwc()),
          this, SLOT(cropSwcInRoi()));
  connect(getCanvas()->getInteractionEngine(), SIGNAL(splittingBodyLocal()),
          this, SIGNAL(runningLocalSplit()));
  connect(getCanvas()->getInteractionEngine(), SIGNAL(splittingBody()),
          this, SIGNAL(runningSplit()));
  connect(getCanvas()->getInteractionEngine(), SIGNAL(splittingFullBody()),
          this, SIGNAL(runningFullSplit()));
  connect(getCanvas()->getInteractionEngine(), SIGNAL(deletingSelected()),
          this, SLOT(deleteSelected()));
  connect(getCanvas()->getInteractionEngine(), SIGNAL(selectingDownstreamSwcNode()),
          m_doc.get(), SLOT(selectDownstreamNode()));
  connect(getCanvas()->getInteractionEngine(), SIGNAL(selectingUpstreamSwcNode()),
          m_doc.get(), SLOT(selectUpstreamNode()));
  connect(getCanvas()->getInteractionEngine(), SIGNAL(selectingConnectedSwcNode()),
          m_doc.get(), SLOT(selectConnectedNode()));
//  connect(getCanvas()->getInteractionEngine(), SIGNAL(cameraRorated()),
//          this, SLOT(notifyCameraRotation()));
  connect(&(m_view->interactionHandler()), SIGNAL(cameraRotated()),
          this, SLOT(notifyCameraRotation()));
  connect(getCanvas()->getInteractionEngine(), SIGNAL(exitingEdit()),
          this, SLOT(syncActionToNormalMode()));


//  connect(m_canvas, SIGNAL(strokePainted(ZStroke2d*)),
//          this, SLOT(addPolyplaneFrom3dPaint(ZStroke2d*)));
  connect(getCanvas(), SIGNAL(strokePainted(ZStroke2d*)),
          this, SLOT(processStroke(ZStroke2d*)));
  connect(getCanvas(), SIGNAL(shootingTodo(int,int)),
          this, SLOT(shootTodo(int,int)));
  connect(getCanvas(), SIGNAL(locating(int, int)),
          this, SLOT(locateWithRay(int, int)));
  connect(getCanvas(), SIGNAL(browsing(int,int)),
          this, SLOT(browseWithRay(int, int)));
  connect(getCanvas(), SIGNAL(viewingDetail(int,int)),
          this, SLOT(showDetail(int,int)));

  m_swcIsolationDlg = new ZSwcIsolationDialog(this);
  if (getDocument() != NULL) {
    const ZResolution &res = getDocument()->getResolution();
    m_swcIsolationDlg->setScale(res.voxelSizeX(), res.voxelSizeY(),
                                res.voxelSizeZ());
  }

  m_helpDlg = new HelpDialog(this);

//  if (m_meshOpacitySlider != NULL) {
//    m_meshOpacitySlider->setValue(iround(getMeshFilter()->opacity() * 255));
//    connect(m_meshOpacitySlider, SIGNAL(valueChanged(int)),
//            this, SLOT(setMeshOpacity(int)));
//  }

  if (m_meshOpacitySpinBox != NULL && getMeshFilter()) {
    m_meshOpacitySpinBox->setValue(getMeshFilter()->opacity());
    connect(m_meshOpacitySpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(setMeshOpacity(double)));
    connect(getMeshFilter(), SIGNAL(opacityChanged(double)),
            this, SLOT(setMeshOpacity(double)));
  }
}

void Z3DWindow::setWindowSize()
{
  int width = 512;
  int height = 512;

  float objectWidth = m_view->boundBox().size().x;
  float objectHeight = m_view->boundBox().size().y;

  //get screen size
  QDesktopWidget *desktop = QApplication::desktop();
  QRect screenSize = desktop->availableGeometry();
  float screenWidth = screenSize.width() * 0.6;
  float screenHeight = screenSize.height() * 0.6;

  if (objectWidth > screenWidth || objectHeight > screenHeight) {
    float scale = std::max(objectWidth/screenWidth, objectHeight/screenHeight);
    width = std::max(width, (int)(objectWidth/scale));
    height = std::max(height, (int)(objectHeight/scale));
  } else {
    width = std::max(width, (int)(objectWidth));
    height = std::max(height, (int)(objectHeight));
  }
  resize(width+500, height);   //500 for dock widgets
}

void Z3DWindow::about()
{
  ZDialogFactory::About(this);
}

QAction* Z3DWindow::getAction(ZActionFactory::EAction item)
{
  QAction *action = NULL;
  switch (item) {
  case ZActionFactory::ACTION_3DWINDOW_TOGGLE_OBJECTS:
    action = m_toggleObjectsAction;
    break;
  case ZActionFactory::ACTION_3DWINDOW_TOGGLE_SETTING:
    action =m_toggleSettingsAction;
    break;
  case ZActionFactory::ACTION_ABOUT:
    action = m_actionLibrary->getAction(item, this, SLOT(about()));
    break;
  case ZActionFactory::ACTION_DESELECT_BODY:
    action = m_actionLibrary->getAction(item, this, SLOT(deselectBody()));
    break;
  case ZActionFactory::ACTION_SAVE_OBJECT_AS:
    action = m_actionLibrary->getAction(item, this, SLOT(saveSelectedSwc()));
    break;
  case ZActionFactory::ACTION_MEASURE_SWC_NODE_DIST:
    action = m_actionLibrary->getAction(
          item, this, SLOT(showSeletedSwcNodeDist()));
    break;
  case ZActionFactory::ACTION_MEASURE_SWC_NODE_LENGTH:
    action = m_actionLibrary->getAction(
          item, this, SLOT(showSeletedSwcNodeLength()));
    break;
  case ZActionFactory::ACTION_DELETE_SELECTED:
    if (NeutubeConfig::getInstance().getApplication() != "Biocytin") {
      action = m_actionLibrary->getAction(
            item, this, SLOT(removeSelectedObject()));
    } else {
      action = m_actionLibrary->getAction(
            item, this, SLOT(deleteSelectedSwcNode()));
    }
    break;
  case ZActionFactory::ACTION_SHOW_NORMAL_TODO:
    action = m_actionLibrary->getAction(
          item, this, SLOT(setNormalTodoVisible(bool)));
    break;
  case ZActionFactory::ACTION_REMOVE_TODO_BATCH:
    action = m_actionLibrary->getAction(
          item, this, SLOT(removeTodoBatch()));
    break;
  case ZActionFactory::ACTION_ADD_TODO_ITEM:
    action = m_actionLibrary->getAction(item, this, SLOT(addTodoMarker()));
    break;
  case ZActionFactory::ACTION_ADD_TODO_ITEM_CHECKED:
    action = m_actionLibrary->getAction(item, this, SLOT(addDoneMarker()));
    break;
  case ZActionFactory::ACTION_ADD_TODO_MERGE:
    action = m_actionLibrary->getAction(item, this, SLOT(addToMergeMarker()));
    break;
  case ZActionFactory::ACTION_ADD_TODO_SPLIT:
    action = m_actionLibrary->getAction(
          item, this, SLOT(addToSplitMarker()));
    break;
  case ZActionFactory::ACTION_ADD_TODO_TRACE_TO_SOMA:
    action = m_actionLibrary->getAction(
          item, this, SLOT(addTraceToSomaMarker()));
    break;
  case ZActionFactory::ACTION_ADD_TODO_NO_SOMA:
    action = m_actionLibrary->getAction(
          item, this, SLOT(addNoSomaMarker()));
    break;
  case ZActionFactory::ACTION_ADD_TODO_SVSPLIT:
    break;
  case ZActionFactory::ACTION_TODO_ITEM_ANNOT_SPLIT:
    action = m_actionLibrary->getAction(item, this, SLOT(setTodoItemToSplit()));
    break;
  case ZActionFactory::ACTION_TODO_ITEM_ANNOT_NORMAL:
    action = m_actionLibrary->getAction(item, this, SLOT(setTodoItemToNormal()));
    break;
  case ZActionFactory::ACTION_TODO_ITEM_ANNOT_IRRELEVANT:
    action = m_actionLibrary->getAction(item, this, SLOT(setTodoItemIrrelevant()));
    break;
  case ZActionFactory::ACTION_TODO_ITEM_ANNOT_TRACE_TO_SOMA:
    action = m_actionLibrary->getAction(item, this, SLOT(setTodoItemTraceToSoma()));
    break;
  case ZActionFactory::ACTION_TODO_ITEM_ANNOT_NO_SOMA:
    action = m_actionLibrary->getAction(item, this, SLOT(setTodoItemNoSoma()));
    break;
  case ZActionFactory::ACTION_FLYEM_UPDATE_BODY:
    action = m_actionLibrary->getAction(item, this, SLOT(updateBody()));
    break;
  case ZActionFactory::ACTION_FLYEM_COMPARE_BODY:
    action = m_actionLibrary->getAction(item, this, SLOT(compareBody()));
    break;
  case ZActionFactory::ACTION_COPY_POSITION:
    action = m_actionLibrary->getAction(item, this, SLOT(copyPosition()));
    break;
  case ZActionFactory::ACTION_SHOW_TODO:
    action = m_actionLibrary->getAction(item, this, SLOT(showTodo(bool)));
    break;
  case ZActionFactory::ACTION_ACTIVATE_TODO_ITEM:
    action = m_actionLibrary->getAction(
          item, this, SLOT(activateTodoAction(bool)));
    if (m_actionLibrary->actionCreatedUponRetrieval()) {
      m_interactActionGroup->addAction(action);
    }
    break;
  case ZActionFactory::ACTION_ACTIVATE_TOSPLIT_ITEM:
    action = m_actionLibrary->getAction(
          item, this, SLOT(activateTodoAction(bool)));
    if (action != NULL) {
      action->setShortcut(Qt::Key_B);
    }
    if (m_actionLibrary->actionCreatedUponRetrieval()) {
      m_interactActionGroup->addAction(action);
    }
    break;
  case ZActionFactory::ACTION_ACTIVATE_LOCATE:
    action = m_actionLibrary->getAction(
          item, this, SLOT(activateLocateAction(bool)));
//    if (action != NULL) {
//      action->setShortcut(Qt::Key_T);
//    }
    if (m_actionLibrary->actionCreatedUponRetrieval()) {
      m_interactActionGroup->addAction(action);
    }
    break;
  case ZActionFactory::ACTION_VIEW_DATA_EXTERNALLY:
    action = m_actionLibrary->getAction(
          item, this, SLOT(viewDataExternally(bool)));
//    if (action != NULL) {
//      action->setShortcut(Qt::SHIFT + Qt::Key_T);
//    }
    if (m_actionLibrary->actionCreatedUponRetrieval()) {
      m_interactActionGroup->addAction(action);
    }
    break;
  case ZActionFactory::ACTION_CHECK_TODO_ITEM:
    action = m_actionLibrary->getAction(item, this, SLOT(checkSelectedTodo()));
    break;
  case ZActionFactory::ACTION_UNCHECK_TODO_ITEM:
    action = m_actionLibrary->getAction(item, this, SLOT(uncheckSelectedTodo()));
    break;
  case ZActionFactory::ACTION_SAVE_SPLIT_TASK:
    action = m_actionLibrary->getAction(item, this, SLOT(saveSplitTask()));
    break;
  case ZActionFactory::ACTION_DELETE_SPLIT_SEED:
    action = m_actionLibrary->getAction(item, this, SLOT(deleteSplitSeed()));
    break;
  case ZActionFactory::ACTION_DELETE_SELECTED_SPLIT_SEED:
    action = m_actionLibrary->getAction(item, this, SLOT(deleteSelectedSplitSeed()));
    break;
  case ZActionFactory::ACTION_TEST:
    action = m_actionLibrary->getAction(item, this, SLOT(test()));
    break;
  case ZActionFactory::ACTION_PUNCTA_CHANGE_COLOR:
    action = m_actionLibrary->getAction(
          item, this, SLOT(changeSelectedPunctaColor()));
    break;
  case ZActionFactory::ACTION_PUNCTA_HIDE_SELECTED:
    action = m_actionLibrary->getAction(item, this, SLOT(hideSelectedPuncta()));
    break;
  case ZActionFactory::ACTION_PUNCTA_HIDE_UNSELECTED:
    action = m_actionLibrary->getAction(item, this, SLOT(hideUnselectedPuncta()));
    break;
  case ZActionFactory::ACTION_PUNCTA_SHOW_UNSELECTED:
    action = m_actionLibrary->getAction(item, this, SLOT(showUnselectedPuncta()));
    break;
  case ZActionFactory::ACTION_PUNCTA_SHOW_SELECTED:
    action = m_actionLibrary->getAction(item, this, SLOT(showSelectedPuncta()));
    break;
  case ZActionFactory::ACTION_PUNCTA_ADD_SELECTION:
    action = m_actionLibrary->getAction(item, this, SLOT(addPunctaSelection()));
    break;
  case ZActionFactory::ACTION_START_SPLIT:
    action = m_actionLibrary->getAction(item, this, SLOT(startBodySplit()));
    break;
  case ZActionFactory::ACTION_COPY_3DCAMERA:
    action = m_actionLibrary->getAction(item, this, SLOT(copyView()));
    break;
  case ZActionFactory::ACTION_PASTE_3DCAMERA:
    action = m_actionLibrary->getAction(item, this, SLOT(pasteView()));
    break;
  case ZActionFactory::ACTION_SAVE_ALL_MESH:
    action = m_actionLibrary->getAction(item, this, SLOT(saveAllVisibleMesh()));
    break;
//  case ZActionFactory::ACTION_SHOW_SPLIT_MESH_ONLY:
//    action = m_actionLibrary->getAction(item, this, SLOT(showMeshForSplitOnly(bool)));
//    break;
  default:
    action = getDocument()->getAction(item);
    break;
  }

  return action;
}

void Z3DWindow::setActionChecked(ZActionFactory::EAction item, bool on)
{
  QAction *action = getAction(item);
  if (action != NULL) {
    if (action->isCheckable()) {
      action->setChecked(on);
    }
  }
}

void Z3DWindow::createActions()
{
  ZOUT(LTRACE(), 5) << "Create actions";
  /*
  m_undoAction = m_doc->undoStack()->createUndoAction(this, tr("&Undo"));
  m_undoAction->setIcon(QIcon(":/images/undo.png"));
  m_undoAction->setShortcuts(QKeySequence::Undo);

  m_redoAction = m_doc->undoStack()->createRedoAction(this, tr("&Redo"));
  m_redoAction->setIcon(QIcon(":/images/redo.png"));
  m_redoAction->setShortcuts(QKeySequence::Redo);
  */

  m_actionLibrary = new ZActionLibrary(this);
  m_menuFactory = new ZStackDocMenuFactory;

  m_interactActionGroup = new QActionGroup(this);

//  m_undoAction = m_doc->getAction(ZActionFactory::ACTION_UNDO);
//  m_redoAction = m_doc->getAction(ZActionFactory::ACTION_REDO);

//  m_markSwcSomaAction = new QAction("Mark SWC Soma...", this);
//  connect(m_markSwcSomaAction, SIGNAL(triggered()), this, SLOT(markSwcSoma()));

  m_helpAction = new QAction("Help", this);
  connect(m_helpAction, SIGNAL(triggered()), this, SLOT(help()));

  m_diagnoseAction = new QAction("Diagnose", this);
  connect(m_diagnoseAction, &QAction::triggered, this, &Z3DWindow::diagnose);

  m_removeSelectedObjectsAction = new QAction("Delete", this);
  if (NeutubeConfig::getInstance().getApplication() != "Biocytin") {
    connect(m_removeSelectedObjectsAction, SIGNAL(triggered()), this,
            SLOT(removeSelectedObject()));
  } else {
    connect(m_removeSelectedObjectsAction, SIGNAL(triggered()), this,
        SLOT(deleteSelectedSwcNode()));
  }

  m_locateSwcNodeIn2DAction = new QAction("Locate node(s) in 2D", this);
  connect(m_locateSwcNodeIn2DAction, SIGNAL(triggered()), this,
          SLOT(locateSwcNodeIn2DView()));

  m_toggleAddSwcNodeModeAction = new QAction("Add neuron node", this);
  m_toggleAddSwcNodeModeAction->setCheckable(true);
  connect(m_toggleAddSwcNodeModeAction, SIGNAL(toggled(bool)), this,
          SLOT(toogleAddSwcNodeMode(bool)));

  m_toggleMoveSelectedObjectsAction =
      new QAction("Move Selected (Shift+Mouse)", this);
  m_toggleMoveSelectedObjectsAction->setShortcut(Qt::Key_V);
  m_toggleMoveSelectedObjectsAction->setIcon(QIcon(":/images/move.png"));
  m_toggleMoveSelectedObjectsAction->setCheckable(true);
  connect(m_toggleMoveSelectedObjectsAction, SIGNAL(toggled(bool)), this,
          SLOT(toogleMoveSelectedObjectsMode(bool)));

  //  m_toogleExtendSelectedSwcNodeAction = new QAction("Extend selected node", this);
  //  m_toogleExtendSelectedSwcNodeAction->setCheckable(true);
  //  connect(m_toogleExtendSelectedSwcNodeAction, SIGNAL(toggled(bool)), this,
  //          SLOT(toogleExtendSelectedSwcNodeMode(bool)));
  //  m_singleSwcNodeActionActivator.registerAction(m_toogleExtendSelectedSwcNodeAction, true);

  m_toggleSmartExtendSelectedSwcNodeAction = new QAction("Extend", this);
  m_toggleSmartExtendSelectedSwcNodeAction->setCheckable(true);
  m_toggleSmartExtendSelectedSwcNodeAction->setShortcut(Qt::Key_Space);
  m_toggleSmartExtendSelectedSwcNodeAction->setStatusTip(
        "Extend the currently selected node with mouse click.");
  m_toggleSmartExtendSelectedSwcNodeAction->setIcon(QIcon(":/images/extend.png"));
  connect(m_toggleSmartExtendSelectedSwcNodeAction, SIGNAL(toggled(bool)), this,
          SLOT(toogleSmartExtendSelectedSwcNodeMode(bool)));
  m_singleSwcNodeActionActivator.registerAction(
        m_toggleSmartExtendSelectedSwcNodeAction, true);

  m_changeSwcNodeTypeAction = new QAction("Change type", this);
  connect(m_changeSwcNodeTypeAction, SIGNAL(triggered()),
          this, SLOT(changeSelectedSwcNodeType()));

  m_setSwcRootAction = new QAction("Set as a root", this);
  connect(m_setSwcRootAction, SIGNAL(triggered()),
          this, SLOT(setRootAsSelectedSwcNode()));
  m_singleSwcNodeActionActivator.registerAction(m_setSwcRootAction, true);

  m_breakSwcConnectionAction = new QAction("Break", this);
  connect(m_breakSwcConnectionAction, SIGNAL(triggered()), this,
          SLOT(breakSelectedSwcNode()));
  m_singleSwcNodeActionActivator.registerAction(m_breakSwcConnectionAction, false);

  m_connectSwcNodeAction = new QAction("Connect", this);
  connect(m_connectSwcNodeAction, SIGNAL(triggered()), this,
          SLOT(connectSelectedSwcNode()));
  m_singleSwcNodeActionActivator.registerAction(m_connectSwcNodeAction, false);

  m_connectToSwcNodeAction = new QAction("Connect to", this);
  m_connectToSwcNodeAction->setShortcut(Qt::Key_C);
  m_connectToSwcNodeAction->setStatusTip(
        "Connect the currently selected node to another");
  connect(m_connectToSwcNodeAction, SIGNAL(triggered()), this,
          SLOT(startConnectingSwcNode()));
  m_connectToSwcNodeAction->setIcon(QIcon(":/images/connect_to.png"));
  m_singleSwcNodeActionActivator.registerAction(m_connectToSwcNodeAction, true);

  m_mergeSwcNodeAction = new QAction("Merge", this);
  connect(m_mergeSwcNodeAction, SIGNAL(triggered()), this,
          SLOT(mergeSelectedSwcNode()));
  m_singleSwcNodeActionActivator.registerAction(m_mergeSwcNodeAction, false);

  m_selectSwcConnectionAction = new QAction("Select Connection", this);
  connect(m_selectSwcConnectionAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectSwcNodeConnection()));

/*
  m_selectSwcNodeUpstreamAction = new QAction("Upstream", this);
  connect(m_selectSwcNodeUpstreamAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectUpstreamNode()));
          */
  /*
  m_selectSwcNodeDownstreamAction = new QAction("Downstream", this);
  connect(m_selectSwcNodeDownstreamAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectDownstreamNode()));
          */

  /*
  m_selectSwcNodeDownstreamAction =
      m_doc->getAction(ZActionFactory::ACTION_SELECT_DOWNSTREAM);

  m_selectSwcNodeUpstreamAction =
      m_doc->getAction(ZActionFactory::ACTION_SELECT_UPSTREAM);
      */

  /*
  m_selectSwcNodeBranchAction = new QAction("Branch", this);
  connect(m_selectSwcNodeBranchAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectBranchNode()));
          */
  /*
  m_selectSwcNodeBranchAction =
      m_doc->getAction(ZActionFactory::ACTION_SELECT_SWC_BRANCH);
*/

  m_selectSwcNodeTreeAction = new QAction("Tree", this);
  connect(m_selectSwcNodeTreeAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectTreeNode()));

  m_selectAllConnectedSwcNodeAction = new QAction("All Connected Nodes", this);
  connect(m_selectAllConnectedSwcNodeAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectConnectedNode()));

  m_selectAllSwcNodeAction = new QAction("All Nodes", this);
  connect(m_selectAllSwcNodeAction, SIGNAL(triggered()), m_doc.get(),
          SLOT(selectAllSwcTreeNode()));

  m_translateSwcNodeAction = new QAction("Translate", this);
  connect(m_translateSwcNodeAction, SIGNAL(triggered()), this,
          SLOT(tranlateSelectedSwcNode()));

  m_changeSwcNodeSizeAction = new QAction("Change size", this);
  connect(m_changeSwcNodeSizeAction, SIGNAL(triggered()), this,
          SLOT(changeSelectedSwcNodeSize()));

  m_saveSwcAction = new QAction("Save as", this);
  connect(m_saveSwcAction, SIGNAL(triggered()), this,
          SLOT(saveSelectedSwc()));

  m_changeSwcTypeAction = new QAction("Change type", this);
  connect(m_changeSwcTypeAction, SIGNAL(triggered()), this,
          SLOT(changeSelectedSwcType()));

  m_changeSwcSizeAction = new QAction("Change size", this);
  connect(m_changeSwcSizeAction, SIGNAL(triggered()), this,
          SLOT(changeSelectedSwcSize()));

  m_transformSwcAction = new QAction("Transform skeleton", this);
  connect(m_transformSwcAction, SIGNAL(triggered()), this,
          SLOT(transformSelectedSwc()));

  m_groupSwcAction = new QAction("Group", this);
  connect(m_groupSwcAction, SIGNAL(triggered()), this,
          SLOT(groupSelectedSwc()));

  m_breakForestAction = new QAction("Break forest", this);
  connect(m_breakForestAction, SIGNAL(triggered()), this,
          SLOT(breakSelectedSwc()));

  m_changeSwcColorAction = new QAction("Change color", this);
  connect(m_changeSwcColorAction, SIGNAL(triggered()), this,
          SLOT(changeSelectedSwcColor()));

  m_changeSwcAlphaAction = new QAction("Change transparency", this);
  connect(m_changeSwcAlphaAction, SIGNAL(triggered()), this,
          SLOT(changeSelectedSwcAlpha()));

  m_swcInfoAction = new QAction("Get Info", this);
  connect(m_swcInfoAction, SIGNAL(triggered()), this,
          SLOT(showSelectedSwcInfo()));

  m_swcNodeLengthAction = new QAction("Calculate length", this);
  connect(m_swcNodeLengthAction, SIGNAL(triggered()), this,
          SLOT(showSeletedSwcNodeLength()));

  m_refreshTraceMaskAction = new QAction("Refresh tracing mask", this);
  connect(m_refreshTraceMaskAction, SIGNAL(triggered()), this,
          SLOT(refreshTraceMask()));

    /*
  m_removeSwcTurnAction = new QAction("Remove turn", this);
  connect(m_removeSwcTurnAction, SIGNAL(triggered()),
          this, SLOT(removeSwcTurn()));


  m_resolveCrossoverAction = new QAction("Resolve crossover", this);
  connect(m_resolveCrossoverAction, SIGNAL(triggered()),
          m_doc.get(), SLOT(executeResolveCrossoverCommand()));
          */

  m_removeSwcTurnAction =
      m_doc->getAction(ZActionFactory::ACTION_REMOVE_TURN);

  m_resolveCrossoverAction =
      m_doc->getAction(ZActionFactory::ACTION_RESOLVE_CROSSOVER);

}

void Z3DWindow::createMenus()
{
  m_viewMenu = menuBar()->addMenu(tr("&View"));
  m_editMenu = menuBar()->addMenu(tr("&Edit"));
  m_helpMenu = menuBar()->addMenu(tr("&Help"));

  m_editMenu->addAction(getAction(ZActionFactory::ACTION_UNDO));
  m_editMenu->addAction(getAction(ZActionFactory::ACTION_REDO));
  m_editMenu->addSeparator();
//  m_editMenu->addAction(m_markSwcSomaAction);

  m_editMenu->addAction("Select Mesh by ID", this, SLOT(selectMeshByID()));
  m_editMenu->addAction("Select All Meshes", this, SLOT(selectAllMeshes()),
                        QKeySequence("ctrl+a"));
  m_editMenu->addSeparator();

  m_helpMenu->addAction(m_helpAction);
  m_helpMenu->addAction(m_diagnoseAction);
}

void Z3DWindow::createContextMenu()
{
  QMenu *contextMenu = new QMenu(this);

  m_saveSelectedPunctaAsAction = new QAction("save selected puncta as ...", this);
  connect(m_saveSelectedPunctaAsAction, SIGNAL(triggered()), this,
          SLOT(saveSelectedPunctaAs()));

  m_changePunctaNameAction = new QAction("Change name", this);
  connect(m_changePunctaNameAction, SIGNAL(triggered()), this,
          SLOT(changeSelectedPunctaName()));

  m_saveAllPunctaAsAction = new QAction("save all puncta as ...", this);
  connect(m_saveAllPunctaAsAction, SIGNAL(triggered()), this,
          SLOT(saveAllPunctaAs()));
  m_locatePunctumIn2DAction = new QAction("Locate punctum in 2D View", this);
   connect(m_locatePunctumIn2DAction, SIGNAL(triggered()), this,
           SLOT(locatePunctumIn2DView()));
  contextMenu->addAction(m_saveSelectedPunctaAsAction);
  contextMenu->addAction(m_saveAllPunctaAsAction);
  contextMenu->addAction(m_changePunctaNameAction);
  contextMenu->addAction(m_locatePunctumIn2DAction);
  contextMenu->addAction("Transform selected puncta",
                         this, SLOT(transformSelectedPuncta()));
  contextMenu->addAction(
        "Change color", this, SLOT(changeSelectedPunctaColor()));
  contextMenu->addAction("Transform all puncta",
                         this, SLOT(transformAllPuncta()));
  contextMenu->addAction("Convert to swc",
                         this, SLOT(convertPunctaToSwc()));
  contextMenu->addAction(m_removeSelectedObjectsAction);
  contextMenu->addAction(m_toggleMoveSelectedObjectsAction);
  m_contextMenuGroup["puncta"] = contextMenu;

  ZOUT(LTRACE(), 5) << "Create swc node menu";

  //Swc node
  contextMenu = new QMenu(this);


  contextMenu->addAction(m_toggleSmartExtendSelectedSwcNodeAction);
  contextMenu->addAction(m_connectToSwcNodeAction);
  contextMenu->addAction(m_selectSwcNodeTreeAction);
  contextMenu->addAction(m_toggleMoveSelectedObjectsAction);

  ZStackDocMenuFactory menuFactory;
  menuFactory.setSingleSwcNodeActionActivator(&m_singleSwcNodeActionActivator);
  menuFactory.makeSwcNodeContextMenu(getDocument(), this, contextMenu);
  contextMenu->addSeparator();
  contextMenu->addAction(m_locateSwcNodeIn2DAction);
  contextMenu->addAction(m_changeSwcNodeTypeAction);
  contextMenu->addAction(m_toggleAddSwcNodeModeAction);

  m_contextMenuGroup["swcnode"] = contextMenu;

  contextMenu = new QMenu(this);
  contextMenu->addAction(m_saveSwcAction);
  contextMenu->addAction(m_changeSwcTypeAction);
  contextMenu->addAction(m_changeSwcSizeAction);
  contextMenu->addAction(m_transformSwcAction);
  contextMenu->addAction(m_groupSwcAction);
  contextMenu->addAction(m_breakForestAction);
  contextMenu->addAction(m_changeSwcColorAction);
  contextMenu->addAction(m_changeSwcAlphaAction);
  contextMenu->addAction(m_swcInfoAction);

#ifdef _DEBUG_2
  contextMenu->addAction("Test", this, SLOT(test()));
#endif
  contextMenu->addAction(m_removeSelectedObjectsAction);
  contextMenu->addAction(m_toggleMoveSelectedObjectsAction);
  m_contextMenuGroup["swc"] = contextMenu;

  contextMenu = new QMenu(this);
  contextMenu->addAction("Accept", this, SLOT(convertSelectedChainToSwc()));
  contextMenu->addAction(m_removeSelectedObjectsAction);
  m_contextMenuGroup["chain"] = contextMenu;

  contextMenu = new QMenu(this);
  contextMenu->addAction("Trace", this, SLOT(traceTube()));
  //contextMenu->addAction("Trace Exp", this, SLOT(traceTube_Exp()));
  m_contextMenuGroup["trace"] = contextMenu;

  contextMenu = new QMenu(this);
  m_openVolumeZoomInViewAction = new QAction("Open Zoom In View", this);
  connect(m_openVolumeZoomInViewAction, SIGNAL(triggered()), this,
          SLOT(openZoomInView()));
  m_exitVolumeZoomInViewAction = new QAction("Exit Zoom In View", this);
  connect(m_exitVolumeZoomInViewAction, SIGNAL(triggered()), this,
          SLOT(exitZoomInView()));
  m_markPunctumAction = new QAction("Mark Punctum", this);
  connect(m_markPunctumAction, SIGNAL(triggered()), this,
          SLOT(markPunctum()));
  m_contextMenuGroup["volume"] = contextMenu;

  contextMenu = new QMenu(this);
  m_changeBackgroundAction = new QAction("Change Background", this);
  connect(m_changeBackgroundAction, SIGNAL(triggered()), this,
          SLOT(changeBackground()));

  m_toggleObjectsAction = new QAction("Objects", this);
  m_toggleObjectsAction->setCheckable(true);
  m_toggleObjectsAction->setChecked(
        m_objectsDockWidget->toggleViewAction()->isChecked());
  connect(m_toggleObjectsAction, SIGNAL(triggered(bool)),
          m_objectsDockWidget->toggleViewAction(), SIGNAL(triggered(bool)));

  m_toggleSettingsAction = new QAction("Settings", this);
  m_toggleSettingsAction->setCheckable(true);
  m_toggleSettingsAction->setChecked(
        m_settingsDockWidget->toggleViewAction()->isChecked());
  connect(m_toggleSettingsAction, SIGNAL(triggered(bool)),
          m_settingsDockWidget->toggleViewAction(), SIGNAL(triggered(bool)));

  m_contextMenuGroup["empty"] = contextMenu;
}

void Z3DWindow::customizeContextMenu()
{
  //Need modification
  m_selectSwcNodeTreeAction->setVisible(false);

  if (GET_APPLICATION_NAME == "Biocytin") {
    m_toggleAddSwcNodeModeAction->setVisible(false);
    m_toggleMoveSelectedObjectsAction->setVisible(false);
    //m_toogleExtendSelectedSwcNodeAction->setVisible(false);
    m_toggleSmartExtendSelectedSwcNodeAction->setVisible(false);
    m_translateSwcNodeAction->setVisible(false);
    //m_selectSwcNodeDownstreamAction->setVisible(false);
    //m_changeSwcNodeTypeAction->setVisible(false);
//    m_mergeSwcNodeAction->setVisible(false);
    m_changeSwcNodeSizeAction->setVisible(false);
    //m_setSwcRootAction->setVisible(false);
    //m_changeSwcTypeAction->setVisible(false);
    m_changeSwcSizeAction->setVisible(false);
    m_transformSwcAction->setVisible(false);
    m_changeSwcColorAction->setVisible(false);
    m_changeSwcAlphaAction->setVisible(false);
    m_refreshTraceMaskAction->setVisible(false);
  } else if (GET_APPLICATION_NAME == "FlyEM") {
    //m_toogleExtendSelectedSwcNodeAction->setVisible(false);
    m_toggleAddSwcNodeModeAction->setVisible(false);
    //m_toogleMoveSelectedObjectsAction->setVisible(false);
    m_toggleSmartExtendSelectedSwcNodeAction->setVisible(false);
    m_refreshTraceMaskAction->setVisible(false);
  }
}

void Z3DWindow::hideControlPanel()
{
  if (m_settingsDockWidget != NULL) {
    m_settingsDockWidget->hide();
    m_toggleSettingsAction->setChecked(false);
  }
}

void Z3DWindow::showControlPanel()
{
  if (m_settingsDockWidget != NULL) {
    m_settingsDockWidget->show();
    m_toggleSettingsAction->setChecked(true);
  }
}

void Z3DWindow::hideObjectView()
{
  if (m_objectsDockWidget != NULL) {
    m_objectsDockWidget->hide();
    m_toggleObjectsAction->setChecked(false);
  }
}

void Z3DWindow::hideStatusBar()
{
  if (statusBar() != NULL) {
    statusBar()->hide();
  }
}

void Z3DWindow::createDockWindows()
{
  m_settingsDockWidget = new QDockWidget(tr("Control and Settings"), this);
  m_settingsDockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
  m_settingsDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

  m_viewMenu->addSeparator();
  m_viewMenu->addAction(m_settingsDockWidget->toggleViewAction());

  m_objectsDockWidget = new QDockWidget(tr("Objects"), this);
  m_objectsDockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
  m_objectsDockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

  m_viewMenu->addAction(m_objectsDockWidget->toggleViewAction());

  m_roiDockWidget = new ZROIWidget(tr("ROIs"), this);
  m_roiDockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
  m_roiDockWidget->setVisible(false);
  m_viewMenu->addAction(m_roiDockWidget->toggleViewAction());

  addDockWidget(Qt::RightDockWidgetArea, m_roiDockWidget);
  addDockWidget(Qt::RightDockWidgetArea, m_objectsDockWidget);
  addDockWidget(Qt::RightDockWidgetArea, m_settingsDockWidget);
}

void Z3DWindow::fillDockWindows()
{
  m_widgetsGroup = std::make_shared<ZWidgetsGroup>("All", 1);

  QMenu *cameraMenu = new QMenu(this);
  QPushButton *cameraMenuButton = new QPushButton(tr("Camera"));
  cameraMenuButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  cameraMenuButton->setMenu(cameraMenu);

  QAction *resetCameraAction = new QAction("Reset Camera", this);
  connect(resetCameraAction, SIGNAL(triggered()), this, SLOT(resetCamera()));
  cameraMenu->addAction(resetCameraAction);

  QAction *flipViewAction = new QAction("Back View", this);
  connect(flipViewAction, SIGNAL(triggered()), this, SLOT(flipView()));
  cameraMenu->addAction(flipViewAction);

  QAction *xzViewAction = new QAction("X-Z View", this);
  connect(xzViewAction, SIGNAL(triggered()), this, SLOT(setXZView()));
  cameraMenu->addAction(xzViewAction);

  QAction *yzViewAction = new QAction("Y-Z View", this);
  connect(yzViewAction, SIGNAL(triggered()), this, SLOT(setYZView()));
  cameraMenu->addAction(yzViewAction);

  QAction *saveViewAction = new QAction("Save View", this);
  connect(saveViewAction, SIGNAL(triggered()), this, SLOT(saveView()));
  cameraMenu->addAction(saveViewAction);

  QAction *loadViewAction = new QAction("Load View", this);
  connect(loadViewAction, SIGNAL(triggered()), this, SLOT(loadView()));
  cameraMenu->addAction(loadViewAction);

  const NeutubeConfig &config = NeutubeConfig::getInstance();
  if (config.getZ3DWindowConfig().isUtilsOn()) {
    auto utils = m_view->globalParasWidgetsGroup();
    utils->addChild(*cameraMenuButton, 0);
    m_widgetsGroup->addChild(utils);
  }

  m_widgetsGroup->addChild(m_view->captureWidgetsGroup());

  const QList<neutu3d::ERendererLayer>& layerList = m_view->getLayerList();
  foreach (neutu3d::ERendererLayer layer, layerList) {
    m_widgetsGroup->addChild(m_view->getWidgetsGroup(layer));
  }
#if 0
  //volume

  if (getVolumeFilter() != NULL) {
    m_widgetsGroup->addChild(getVolumeFilter()->widgetsGroup());
  }

  if (getSwcFilter() != NULL) {
    m_widgetsGroup->addChild(getSwcFilter()->widgetsGroup());
  }

  if (getPunctaFilter() != NULL) {
    m_widgetsGroup->addChild(getPunctaFilter()->widgetsGroup());
  }

  if (getMeshFilter() != NULL) {
    m_widgetsGroup->addChild(getMeshFilter()->widgetsGroup());
  }

  if (getGraphFilter() != NULL) {
    m_widgetsGroup->addChild(getGraphFilter()->widgetsGroup());
  }

  if (getSurfaceFilter() != NULL) {
    m_widgetsGroup->addChild(getSurfaceFilter()->widgetsGroup());
  }

  if (getTodoFilter() != NULL) {
    m_widgetsGroup->addChild(getTodoFilter()->widgetsGroup());
  }
#endif

  //background
  m_widgetsGroup->addChild(m_view->backgroundWidgetsGroup());

  //axis
  m_widgetsGroup->addChild(m_view->axisWidgetsGroup());


  QTabWidget *tabs = createBasicSettingTabWidget();
  m_settingsDockWidget->setWidget(tabs);

  //Use QueuedConnection to avoid a strange crash
  connect(m_widgetsGroup.get(), SIGNAL(widgetsGroupChanged()),
          this, SLOT(updateSettingsDockWidget()), Qt::QueuedConnection);
  connect(m_widgetsGroup.get(), SIGNAL(requestAdvancedWidget(QString)),
          this, SLOT(openAdvancedSetting(QString)));

  customizeDockWindows(tabs);

  ZObjsManagerWidget* omw = new ZObjsManagerWidget(getDocument(), m_objectsDockWidget);
  connect(omw, SIGNAL(swcDoubleClicked(ZSwcTree*)), this, SLOT(swcDoubleClicked(ZSwcTree*)));
  connect(omw, SIGNAL(swcNodeDoubleClicked(Swc_Tree_Node*)), this, SLOT(swcNodeDoubleClicked(Swc_Tree_Node*)));
  connect(omw, SIGNAL(punctaDoubleClicked(ZPunctum*)), this, SLOT(punctaDoubleClicked(ZPunctum*)));
  connect(omw, SIGNAL(meshDoubleClicked(ZMesh*)), this, SLOT(meshDoubleClicked(ZMesh*)));
  m_objectsDockWidget->setWidget(omw);
}

int Z3DWindow::channelNumber()
{
  if (m_doc.get() == NULL) {
    return 0;
  }

  if (m_doc->hasStack()) {
    return m_doc->getStack()->channelNumber();
  } else {
    return 0;
  }
}

void Z3DWindow::customizeDockWindows(QTabWidget *m_settingTab)
{
  if (NeutubeConfig::getInstance().getApplication() == "Biocytin") {
    m_settingTab->setCurrentIndex(1);
  }
}

bool Z3DWindow::hasVolume()
{
  return channelNumber() > 0;
}

void Z3DWindow::recordView()
{
  m_cameraRecord = getCamera()->get();
}

void Z3DWindow::diffView()
{
  std::cout << "Eye: " << getCamera()->get().eye() - m_cameraRecord.eye()
            << std::endl;
  std::cout << "Center: "
            << getCamera()->get().center() - m_cameraRecord.center() << std::endl;
  std::cout << "Up vector: "
            << getCamera()->get().upVector() - m_cameraRecord.upVector() << std::endl;
}

void Z3DWindow::saveView()
{
  QString filename = QFileDialog::getSaveFileName(
        this, tr("Save View Parameters"), m_lastOpenedFilePath,
         tr("Json files (*.json)"));


  if (!filename.isEmpty()) {
    m_lastOpenedFilePath = filename;
    ZJsonObject cameraJson =getCamera()->get().toJsonObject();
    cameraJson.dump(filename.toStdString());
  }
}

void Z3DWindow::loadView()
{
  QString fileName = QFileDialog::getOpenFileName(
        this, tr("Load View Parameters"), m_lastOpenedFilePath,
        tr("Json files (*.json)"));

  if (!fileName.isEmpty()) {
    ZJsonObject cameraJson;
    cameraJson.load(fileName.toStdString());
    getCamera()->set(cameraJson);
  }
}

void Z3DWindow::copyView()
{
  ZGlobal::GetInstance().set3DCamera(
        getCamera()->get().toJsonObject().dumpString(0));
}

void Z3DWindow::pasteView()
{
  std::string config = ZGlobal::GetInstance().get3DCamera();
  if (!config.empty()) {
    ZJsonObject cameraJson;
    cameraJson.decode(config);
    if (!cameraJson.isEmpty()) {
      getCamera()->set(cameraJson);
    }
  }
}

void Z3DWindow::saveAllVisibleMesh()
{
  QString fileName = ZDialogFactory::GetSaveFileName("Save Mesh", ".obj", this);

  if (!fileName.isEmpty()) {
    ZStackDocProxy::SaveVisibleMesh(getDocument(), fileName);
  }
}

void Z3DWindow::zoomToSelectedMeshes()
{
  TStackObjectSet &meshSet = m_doc->getSelected(ZStackObject::EType::MESH);
  ZBBox<glm::dvec3> boundingBox;
  for (ZStackObject *obj : meshSet) {
    if (ZMesh* mesh = dynamic_cast<ZMesh*>(obj)) {
      boundingBox.expand(getMeshFilter()->meshBound(mesh));
    }
  }
  if (!boundingBox.empty()) {
      m_view->gotoPosition(boundingBox, 0);
  }
}

void Z3DWindow::selectMeshByID()
{
  bool ok = true;
  QString text = QInputDialog::getText(this, "Select Mesh", "Mesh ID:",
                                       QLineEdit::Normal, "", &ok);
  if (ok) {
    ZFlyEmBody3dDoc *doc = qobject_cast<ZFlyEmBody3dDoc*>(getDocument());
    if (doc) {
      ZMesh *meshToSelect = nullptr;
      uint64_t id = text.toULongLong(&ok);
      if (ok) {
        auto meshList = doc->getMeshList();
        for (ZMesh* mesh : meshList) {
          if (mesh->getLabel() == id) {
            meshToSelect = mesh;
            break;
          }
        }
      }
      if (meshToSelect) {
        doc->setMeshSelected(meshToSelect, true);
        QString message = "Mesh " + QString::number(meshToSelect->getLabel()) + " selected";
        m_doc->notifyWindowMessageUpdated(message);
      }
      else {
        QString title = "Selection Failed";
        QString warning = "No mesh has ID " + text;
        QMessageBox::warning(this, title, warning);
      }
    }
  }
}

void Z3DWindow::selectAllMeshes()
{
  ZFlyEmBody3dDoc *doc = qobject_cast<ZFlyEmBody3dDoc*>(getDocument());
  if (doc) {
    auto meshList = ZStackDocProxy::GetGeneralMeshList(doc);
    for (ZMesh* mesh : meshList) {
      doc->setMeshSelected(mesh, true);
    }
    QString message = QString::number(meshList.size());
    message += (meshList.size() == 1) ? " mesh selected" : " meshes selected";
    doc->notifyWindowMessageUpdated(message);
  }
}

void Z3DWindow::configureLayer(neutu3d::ERendererLayer layer, const ZJsonObject &obj)
{
  m_view->configureLayer(layer, obj);
}


void Z3DWindow::configure(const ZJsonObject &obj)
{
  m_view->configure(obj);
}


void Z3DWindow::skipKeyEvent(bool on)
{
  m_skippingKeyEvent = on;
}

void Z3DWindow::syncAction()
{
  QAction *action = getAction(ZActionFactory::ACTION_SHOW_TODO);
  if (action != NULL) {
    action->setChecked(m_view->isLayerVisible(neutu3d::ERendererLayer::TODO));
  }
}

void Z3DWindow::syncActionToNormalMode()
{
  blockSignals(true);
  setActionChecked(ZActionFactory::ACTION_ACTIVATE_TOSPLIT_ITEM, false);
  setActionChecked(ZActionFactory::ACTION_ACTIVATE_LOCATE, false);
  setActionChecked(ZActionFactory::ACTION_VIEW_DATA_EXTERNALLY, false);
  blockSignals(false);
}

void Z3DWindow::toggleObjects()
{
  m_settingsDockWidget->toggleViewAction()->toggle();
}

void Z3DWindow::toggleSetting()
{
  m_objectsDockWidget->toggleViewAction()->toggle();
}

void Z3DWindow::readSettings()
{
  QString windowKey = neutu3d::GetWindowKeyString(getWindowType()).c_str();
  QString settingString = NeutubeConfig::GetSettings().value(windowKey).
      toString();

  ZJsonObject jsonObj;

  qDebug() << settingString;

  jsonObj.decodeString(settingString.toStdString().c_str());

  configure(jsonObj);
}

void Z3DWindow::writeSettings()
{
  //ignore general window type for now
  if (getWindowType() != neutu3d::EWindowType::GENERAL) {
    ZJsonObject configJson = m_view->getSettings();

    std::string settingString = configJson.dumpString(0);

#ifdef _DEBUG_
    std::cout << settingString << std::endl;
#endif
    NeutubeConfig::GetSettings().setValue(
          neutu3d::GetWindowKeyString(getWindowType()).c_str(),
          settingString.c_str());
  }
}

bool Z3DWindow::hasRectRoi() const
{
  return getCanvas()->getInteractionEngine()->hasRectDecoration();
}

ZRect2d Z3DWindow::getRectRoi() const
{
  return getCanvas()->getInteractionEngine()->getRectDecoration();
}

void Z3DWindow::setMenuFactory(ZStackDocMenuFactory *factory)
{
  if (m_menuFactory != NULL) {
    delete m_menuFactory;
  }

  m_menuFactory = factory;
}


ZFlyEmBodyEnv* Z3DWindow::getBodyEnv() const
{
  return m_bodyEnv.get();
}

void Z3DWindow::cleanup()
{
  if (!m_isClean) {
    m_isClean = true;

    m_buttonStatus[0] = true;  // showgraph
    m_buttonStatus[1] = false; // settings
    m_buttonStatus[2] = false; // objects
    m_buttonStatus[3] = false; // rois
  }
}

bool Z3DWindow::readyForAction(ZActionFactory::EAction action) const
{
#if defined(_FLYEM_)
  if (action == ZActionFactory::ACTION_FLYEM_COMPARE_BODY) {
    ZFlyEmBody3dDoc *doc = qobject_cast<ZFlyEmBody3dDoc*>(getDocument());
    if (doc != NULL) {
      if (action == ZActionFactory::ACTION_FLYEM_COMPARE_BODY) {
        if (doc->updating()) {
          return false;
        }
      }
    } else {
      return false;
    }
  }
#endif

  if (action == ZActionFactory::ACTION_COPY_POSITION) {
    ZStackDoc *doc = getDocument();
    if (doc->getSelectedSwcNodeNumber() != 1) {
      return false;
    }
  }

  return true;
}

void Z3DWindow::onSelectionChangedFrom3D(Z3DGeometryFilter *filter,
    ZStackObject *p, ZStackObject::EType type, bool append)
{
  if (!append) {
    m_doc->deselectAllObject(type);
    if (type == ZStackObject::EType::GRAPH_3D) {
      m_doc->deselectAllObject(ZStackObjectRole::ROLE_3DGRAPH_DECORATOR);
    }
    filter->invalidate();
  }

  if (p != NULL) {
    m_doc->selectObject(p, true);
    filter->invalidate();
  }
}

void Z3DWindow::annotateTodo(ZStackObject *obj)
{
  ZFlyEmTodoAnnotationDialog dlg(this);
  ZFlyEmBody3dDoc *doc = qobject_cast<ZFlyEmBody3dDoc*>(getDocument());
  if (doc != NULL) {
    doc->annotateTodo(&dlg, obj);
  }
//  getCompleteDocument()->annotateSelectedTodoItem(&dlg, getView()->getSliceAxis());
}

void Z3DWindow::removeTodoBatch()
{
  ZFlyEmBody3dDoc *doc = qobject_cast<ZFlyEmBody3dDoc*>(getDocument());
  if (doc != NULL) {
    if (m_todoDlg == nullptr) {
      m_todoDlg = new ZFlyEmTodoFilterDialog(this);
      m_todoDlg->setWindowTitle("Remove Todo");
    }
    doc->removeTodo(m_todoDlg);
  }
}

void Z3DWindow::selectedTodoChangedFrom3D(ZStackObject *p, bool append)
{
  onSelectionChangedFrom3D(
        getTodoFilter(), p, ZStackObject::EType::FLYEM_TODO_ITEM, append);
}

void Z3DWindow::selectedGraphChangedFrom3D(ZStackObject *p, bool append)
{
  onSelectionChangedFrom3D(
        getGraphFilter(), p, ZStackObject::EType::GRAPH_3D, append);
}

void Z3DWindow::selectedObjectChangedFrom3D(ZStackObject *p, bool append)
{
  if (p == NULL) {
    if (!append) {
      Z3DWindowController::DeselectAllObject(this);
//      if (getTodoFilter()) { //temporary hack
//        m_doc->deselectAllObject(ZStackObject::EType::TYPE_FLYEM_TODO_ITEM);
//        getTodoFilter()->invalidate();
//      }
//      if (getGraphFilter()) {
//        getGraphFilter()->deselectAllGraph();
//      }
    }
    return;
  }

  switch (p->getType()) {
  case ZStackObject::EType::FLYEM_TODO_ITEM:
  {
    ZFlyEmBody3dDoc *doc = qobject_cast<ZFlyEmBody3dDoc*>(getDocument());
    if (doc != NULL) {
      doc->selectObject(p, append);
      getTodoFilter()->invalidate();
    }
  }
    break;
  default:
  {
    ZFlyEmBody3dDoc *doc = qobject_cast<ZFlyEmBody3dDoc*>(getDocument());
    if (doc != NULL) {
      if (!append) {
        if (getTodoFilter()) { //temporary hack
          m_doc->deselectAllObject(ZStackObject::EType::FLYEM_TODO_ITEM);
          getTodoFilter()->invalidate();
        }
        if (getGraphFilter()) {
          getGraphFilter()->deselectAllGraph();
        }
      }

      doc->selectObject(p, append);
      if (getGraphFilter()) {
        getGraphFilter()->invalidate();
      }
    }
  }
    break;
  }
}

void Z3DWindow::selectedPunctumChangedFrom3D(ZPunctum *p, bool append)
{
  if (p == NULL) {
    if (!append)
      m_doc->deselectAllPuncta();
    return;
  }

  if (append) {
    m_doc->setPunctumSelected(p, true);
  } else {
    m_doc->deselectAllObject();
    m_doc->setPunctumSelected(p, true);
  }

  m_doc->notifyWindowMessageUpdated(QString::fromStdString(p->toString()));

  statusBar()->showMessage(p->toString().c_str());
}


void Z3DWindow::selectedMeshChangedFrom3D(ZMesh* p, bool append)
{
  if (p == NULL) {
    if (!append) {
      if (m_doc->deselectAllMesh() > 0) {
        m_doc->notifyWindowMessageUpdated("No mesh is selected.");
      }
    }
    return;
  }

  if (append) {
    m_doc->setMeshSelected(p, true);
  } else {
//    m_doc->deselectAllObject();
    m_doc->deselectAllMesh();
    m_doc->setMeshSelected(p, true);
  }

  QString message = "Mesh " + QString::number(p->getLabel()) + " selected";
  m_doc->notifyWindowMessageUpdated(message);

  statusBar()->showMessage(p->getSource().c_str());
}

bool Z3DWindow::canSelectObject() const
{
  return !(getCanvas()->getInteractionEngine()->isStateOn(ZInteractionEngine::STATE_DRAW_LINE) ||
           getCanvas()->getInteractionEngine()->isStateOn(ZInteractionEngine::STATE_DRAW_STROKE) ||
           getCanvas()->getInteractionEngine()->isStateOn(ZInteractionEngine::STATE_DRAW_RECT) ||
           getCanvas()->getInteractionEngine()->isStateOn(ZInteractionEngine::STATE_MARK));
}

void Z3DWindow::selectedSwcChangedFrom3D(ZSwcTree *p, bool append)
{
  if (!canSelectObject()) {
    return;
  }

  if (!append) {
    if (m_doc->hasSelectedSwc()) {
      m_blockingTraceMenu = true;
    }
  }

  if (p == NULL) {
    if (!append)
      m_doc->deselectAllSwcs();
    return;
  }

  if (append) {
    m_doc->setSwcSelected(p, true);
  } else {
    m_doc->deselectAllObject();
    m_doc->setSwcSelected(p, true);
  }

  statusBar()->showMessage(p->getSource().c_str());
}

void Z3DWindow::selectedSwcTreeNodeChangedFrom3D(Swc_Tree_Node *p, bool append)
{
  if (!canSelectObject()) {
    return;
  }

  if (!append) {
    if (m_doc->hasSelectedSwcNode()) {
      m_blockingTraceMenu = true;
    }
  }

  if (p == NULL) {
    if (!append) {
      if (m_doc->hasSelectedSwcNode()) {
        m_doc->deselectAllSwcTreeNodes();
      }
    }
    return;
  }

  if (!append) {
    m_doc->deselectAllObject();
  }

  if (m_doc->isSwcNodeSelected(p) == 0) {
    m_doc->setSwcTreeNodeSelected(p, true);
  } else {
    m_doc->setSwcTreeNodeSelected(p, false);
  }

  statusBar()->showMessage(SwcTreeNode::toString(p).c_str());
}

void Z3DWindow::selectedSwcTreeNodeChangedFrom3D(
    QList<Swc_Tree_Node *> nodeArray, bool append)
{
  if (!canSelectObject()) {
    return;
  }

  if (!append) {
    if (m_doc->hasSelectedSwcNode()) {
      m_blockingTraceMenu = true;
    }
  }

  if (!append) {
    if (m_doc->hasSelectedSwcNode()) {
      m_doc->deselectAllSwcTreeNodes();
    }
  }

  m_doc->setSwcTreeNodeSelected(nodeArray.begin(), nodeArray.end(), append);

  statusBar()->showMessage(QString("%1 node(s) selected.").arg(nodeArray.size()));
}

void Z3DWindow::addNewSwcTreeNode(double x, double y, double z, double r)
{
  m_doc->executeAddSwcNodeCommand(
        ZPoint(x, y, z), r, ZStackObjectRole::ROLE_NONE);
      /*
  QUndoCommand *insertNewSwcTreeNodeCommand =
      new ZStackDocAddSwcNodeCommand(m_doc.get(), p);
  m_doc->undoStack()->push(insertNewSwcTreeNodeCommand);
      */
}

void Z3DWindow::extendSwcTreeNode(double x, double y, double z, double r)
{
  m_doc->executeSwcNodeExtendCommand(ZPoint(x, y, z), r);
}

void Z3DWindow::removeSwcTurn()
{
  m_doc->executeRemoveTurnCommand();
}

void Z3DWindow::startConnectingSwcNode()
{
  notifyUser("Click on the target node to connect.");
  getSwcFilter()->setInteractionMode(Z3DSwcFilter::EInteractionMode::ConnectSwcNode);
  getCanvas()->getInteractionContext().setSwcEditMode(
        ZInteractiveContext::SWC_EDIT_CONNECT);
  getCanvas()->updateCursor();
  //getCanvas()->setCursor(Qt::SizeBDiagCursor);
}

void Z3DWindow::connectSwcTreeNode(Swc_Tree_Node *tn)
{
  if (getDocument()->hasSelectedSwcNode()) {
    Swc_Tree_Node *target = SwcTreeNode::findClosestNode(
          getDocument()->getSelectedSwcNodeSet(), tn);
    m_doc->executeConnectSwcNodeCommand(target, tn);
    getSwcFilter()->setInteractionMode(Z3DSwcFilter::EInteractionMode::Select);
    getCanvas()->getInteractionContext().setSwcEditMode(
          ZInteractiveContext::SWC_EDIT_OFF);
    getCanvas()->updateCursor();
    //getCanvas()->setCursor(Qt::ArrowCursor);
  }
}



void Z3DWindow::swcDoubleClicked(ZSwcTree *tree)
{
  ZBBox<glm::dvec3> bd;
  getSwcFilter()->treeBound(tree, bd);
  m_view->gotoPosition(bd, 0);
}

void Z3DWindow::swcNodeDoubleClicked(Swc_Tree_Node *node)
{
  ZBBox<glm::dvec3> bd;
  getSwcFilter()->treeNodeBound(node, bd);
  m_view->gotoPosition(bd, 0);
}

void Z3DWindow::punctaDoubleClicked(ZPunctum *p)
{
  ZBBox<glm::dvec3> bd;
  getPunctaFilter()->punctumBound(*p, bd);
  if (hasVolume()) {
    if (getVolumeFilter()->isSubvolume())
      getVolumeFilter()->exitZoomInView();
  }
  m_view->gotoPosition(bd);
}

void Z3DWindow::meshDoubleClicked(ZMesh* p)
{
  m_view->gotoPosition(getMeshFilter()->meshBound(p), 0);
}

void Z3DWindow::pointInVolumeLeftClicked(
    QPoint pt, glm::ivec3 pos, Qt::KeyboardModifiers modifiers)
{
  glm::vec3 fpos = glm::vec3(pos);
  m_lastClickedPosInVolume = glm::ivec3(fpos);
  LDEBUG() << "Point in volume left clicked" << fpos;
  // only do tracing when we are not editing swc nodes or the preconditions for editing swc node are not met
  if (hasVolume() && channelNumber() == 1 &&
      m_toggleSmartExtendSelectedSwcNodeAction->isChecked() &&
      m_doc->getSelectedSwcNodeNumber() == 1) {
    if (modifiers == Qt::ControlModifier) {
      m_doc->executeSwcNodeExtendCommand(ZPoint(fpos[0], fpos[1], fpos[2]));
    } else {
      m_doc->executeSwcNodeSmartExtendCommand(ZPoint(fpos[0], fpos[1], fpos[2]));
    }
    // todo: check modifier and use normal extend if possible
    return;
  }
  //  if (m_toogleExtendSelectedSwcNodeAction->isChecked() && m_doc->selectedSwcTreeNodes()->size() == 1) {
  //    return;
  //  }
  if (m_blockingTraceMenu) {
    m_blockingTraceMenu = false;
  } else {
    if (hasVolume() && channelNumber() == 1 &&
        !m_toggleAddSwcNodeModeAction->isChecked() &&
        !m_toggleMoveSelectedObjectsAction->isChecked() &&
        !m_toggleSmartExtendSelectedSwcNodeAction->isChecked() &&
        NeutubeConfig::getInstance().getMainWindowConfig().isTracingOn()) {
      m_contextMenuGroup["trace"]->popup(getCanvas()->mapToGlobal(pt));
    }
  }
}

bool Z3DWindow::addingSwcNode() const
{
  return m_toggleAddSwcNodeModeAction->isChecked();
}

bool Z3DWindow::extendingSwc() const
{
  return m_toggleSmartExtendSelectedSwcNodeAction->isChecked();
}

bool Z3DWindow::movingObject() const
{
  return m_toggleMoveSelectedObjectsAction->isChecked();
}

void Z3DWindow::exitAddingSwcNode()
{
  m_toggleAddSwcNodeModeAction->setChecked(false);
}

void Z3DWindow::exitMovingObject()
{
  m_toggleMoveSelectedObjectsAction->setChecked(false);
}

void Z3DWindow::exitExtendingSwc()
{
  m_toggleSmartExtendSelectedSwcNodeAction->setChecked(false);
}

bool Z3DWindow::exitEditMode()
{
  bool acted = false;
  if (addingSwcNode()) {
    exitAddingSwcNode();
    acted = true;
  } else if (movingObject()) {
    exitMovingObject();
    acted = true;
  } else if (extendingSwc()) {
    exitExtendingSwc();
    acted = true;
  } else if (getSwcFilter()->interactionMode() ==
             Z3DSwcFilter::EInteractionMode::ConnectSwcNode) {
    getSwcFilter()->setInteractionMode(Z3DSwcFilter::EInteractionMode::Select);
    getCanvas()->setCursor(Qt::ArrowCursor);
    acted = true;
  }

  return acted;
}

void Z3DWindow::show3DViewContextMenu(QPoint pt)
{
  notifyUser(" ");
  if (exitEditMode()) {
    return;
  } else if (getCanvas()->suppressingContextMenu()) {
    return;
  }

  m_contextMenu = m_menuFactory->makeContextMenu(this, m_contextMenu);
  if (m_contextMenu != NULL) {
    if (!m_contextMenu->isEmpty()) {
      m_contextMenu->popup(getCanvas()->mapToGlobal(pt));
      return;
    }
  }

  if (getDocument()->getTag() == neutu::Document::ETag::FLYEM_SKELETON) {
    return;
  }

  QList<QAction*> actions;

  if (m_doc->hasSelectedSwc()) {
    //m_contextMenuGroup["swc"]->popup(m_canvas->mapToGlobal(pt));
    QList<QAction*> acts = m_contextMenuGroup["swc"]->actions();
    if (actions.empty()) {
      actions = acts;
    } else {
      while (true) {
        int i;
        for (i=0; i<actions.size(); ++i) {
          if (!acts.contains(actions[i])) {
            break;
          }
        }
        if (i == actions.size())
          break;
        else {
          actions.removeAt(i);
          continue;
        }
      }
    }
  } else {
    if (m_doc->hasSelectedSwcNode()) {
      updateContextMenu("swcnode");
      //m_contextMenuGroup["swcnode"]->popup(m_canvas->mapToGlobal(pt));
      QList<QAction*> acts = m_contextMenuGroup["swcnode"]->actions();
      if (actions.empty()) {
        actions = acts;
      } else {
        while (true) {
          int i;
          for (i=0; i<actions.size(); ++i) {
            if (!acts.contains(actions[i])) {
              break;
            }
          }
          if (i == actions.size())
            break;
          else {
            actions.removeAt(i);
            continue;
          }
        }
      }
    }
  }


  if (m_doc->hasSelectedPuncta()) {
    updateContextMenu("puncta");
    //m_contextMenuGroup["puncta"]->popup(m_canvas->mapToGlobal(pt));
    QList<QAction*> acts = m_contextMenuGroup["puncta"]->actions();
    if (actions.empty()) {
      actions = acts;
    } else {
      while (true) {
        int i;
        for (i=0; i<actions.size(); ++i) {
          if (!acts.contains(actions[i])) {
            break;
          }
        }
        if (i == actions.size())
          break;
        else {
          actions.removeAt(i);
          continue;
        }
      }
    }
  }

  if (!m_doc->hasSelectedPuncta() &&
      !m_doc->hasSelectedSwc() &&
      !m_doc->hasSelectedSwcNode()) {

    // first see if pt hit any position in volume
    if (channelNumber() > 0) {
      bool success;
#ifdef _QT5_
      glm::vec3 fpos = getVolumeFilter()->get3DPosition(
            pt.x() * qApp->devicePixelRatio(), pt.y() * qApp->devicePixelRatio(),
            getCanvas()->width() * qApp->devicePixelRatio(), getCanvas()->height() * qApp->devicePixelRatio(), success);
#else
      glm::vec3 fpos = getVolumeFilter()->get3DPosition(
            pt.x(), pt.y(), getCanvas()->width(), getCanvas()->height(), success);
#endif
      if (success) {
        m_lastClickedPosInVolume = glm::ivec3(fpos);
        updateContextMenu("volume");
        //m_contextMenuGroup["volume"]->popup(m_canvas->mapToGlobal(pt));
        QList<QAction*> acts = m_contextMenuGroup["volume"]->actions();
        if (actions.empty()) {
          actions = acts;
        } else {
          for (int i=0; i<acts.size(); ++i)
            if (!actions.contains(acts[i]))
              actions.push_back(acts[i]);
        }
        if (!actions.empty()) {
          m_mergedContextMenu->clear();
          m_mergedContextMenu->addActions(actions);
          m_mergedContextMenu->popup(getCanvas()->mapToGlobal(pt));
        }
        return;
      }
    }

    updateContextMenu("empty");
    //m_contextMenuGroup["empty"]->popup(m_canvas->mapToGlobal(pt));
    QList<QAction*> acts = m_contextMenuGroup["empty"]->actions();
    if (actions.empty()) {
      actions = acts;
    } else {
      for (int i=0; i<acts.size(); ++i)
        if (!actions.contains(acts[i]))
          actions.push_back(acts[i]);
    }
  }

  if (!actions.empty()) {
    m_mergedContextMenu->clear();
    m_mergedContextMenu->addActions(actions);
    m_mergedContextMenu->popup(getCanvas()->mapToGlobal(pt));
  }
}

void Z3DWindow::traceTube()
{
  /*
  m_doc->executeTraceTubeCommand(m_lastClickedPosInVolume[0],
      m_lastClickedPosInVolume[1],
      m_lastClickedPosInVolume[2]);
      */
  getCanvas()->setCursor(Qt::BusyCursor);
  m_doc->executeTraceSwcBranchCommand(m_lastClickedPosInVolume[0],
      m_lastClickedPosInVolume[1],
      m_lastClickedPosInVolume[2]);
  getSwcFilter()->setInteractionMode(Z3DSwcFilter::EInteractionMode::Select);
  getCanvas()->setCursor(Qt::ArrowCursor);
}

void Z3DWindow::openZoomInView()
{
  if (hasVolume()) {
    if (getVolumeFilter()->openZoomInView(m_lastClickedPosInVolume)) {
      m_view->gotoPosition(getVolumeFilter()->zoomInBound());
    }
  }
}

void Z3DWindow::exitZoomInView()
{
  if (hasVolume()) {
    getVolumeFilter()->exitZoomInView();
  }
}

void Z3DWindow::removeSelectedObject()
{
  m_doc->executeRemoveSelectedObjectCommand();
#if 0
  if (!m_doc->selectedSwcTreeNodes()->empty()) {
    m_doc->executeDeleteSwcNodeCommand();
  }

  if (!(m_doc->selectedChains()->empty() && m_doc->selectedPuncta()->empty() &&
        m_doc->selectedSwcs()->empty() )) {
    QUndoCommand *removeSelectedObjectsCommand =
        new ZStackDocRemoveSelectedObjectCommand(m_doc.get());
    m_doc->undoStack()->push(removeSelectedObjectsCommand);
  }
#endif
}

void Z3DWindow::markSelectedPunctaProperty1()
{
  TStackObjectSet &objSet = m_doc->getSelected(ZStackObject::EType::PUNCTUM);
  for (TStackObjectSet::iterator it=objSet.begin(); it != objSet.end(); it++) {
    ZPunctum *punctum = dynamic_cast<ZPunctum*>(*it);
    punctum->setProperty1("true");
    m_doc->updatePunctaObjsModel(punctum);
//    m_doc->punctaObjsModel()->updateData(punctum);
  }
}

void Z3DWindow::markSelectedPunctaProperty2()
{
  TStackObjectSet &objSet = m_doc->getSelected(ZStackObject::EType::PUNCTUM);
  for (TStackObjectSet::iterator it=objSet.begin(); it != objSet.end(); it++) {
    ZPunctum *punctum = dynamic_cast<ZPunctum*>(*it);
    punctum->setProperty2("true");
    m_doc->updatePunctaObjsModel(punctum);
//    m_doc->punctaObjsModel()->updateData(punctum);
  }
}

void Z3DWindow::markSelectedPunctaProperty3()
{
  TStackObjectSet &objSet = m_doc->getSelected(ZStackObject::EType::PUNCTUM);
  for (TStackObjectSet::iterator it=objSet.begin(); it != objSet.end(); it++) {
    ZPunctum *punctum = dynamic_cast<ZPunctum*>(*it);
    punctum->setProperty3("true");
    m_doc->updatePunctaObjsModel(punctum);
  }
}

void Z3DWindow::unmarkSelectedPunctaProperty1()
{
  TStackObjectSet &objSet = m_doc->getSelected(ZStackObject::EType::PUNCTUM);
  for (TStackObjectSet::iterator it=objSet.begin(); it != objSet.end(); it++) {
    ZPunctum *punctum = dynamic_cast<ZPunctum*>(*it);
    punctum->setProperty1("");
    m_doc->updatePunctaObjsModel(punctum);
  }
}

void Z3DWindow::unmarkSelectedPunctaProperty2()
{
  TStackObjectSet &objSet = m_doc->getSelected(ZStackObject::EType::PUNCTUM);
  for (TStackObjectSet::iterator it=objSet.begin(); it != objSet.end(); it++) {
    ZPunctum *punctum = dynamic_cast<ZPunctum*>(*it);
    punctum->setProperty2("");
    m_doc->updatePunctaObjsModel(punctum);
  }
}

void Z3DWindow::unmarkSelectedPunctaProperty3()
{
  TStackObjectSet &objSet = m_doc->getSelected(ZStackObject::EType::PUNCTUM);
  for (TStackObjectSet::iterator it=objSet.begin(); it != objSet.end(); it++) {
    ZPunctum *punctum = dynamic_cast<ZPunctum*>(*it);
    punctum->setProperty3("");
    m_doc->updatePunctaObjsModel(punctum);
  }
}

void Z3DWindow::saveSelectedPunctaAs()
{
  QString filename =
    QFileDialog::getSaveFileName(this, tr("Save Selected Puncta"), m_lastOpenedFilePath,
         tr("Puncta files (*.apo)"));

  if (!filename.isEmpty()) {
    m_lastOpenedFilePath = filename;
    QList<ZPunctum*> punctaList =
        m_doc->getSelectedObjectList<ZPunctum>(ZStackObject::EType::PUNCTUM);
    ZPunctumIO::save(filename, punctaList.begin(), punctaList.end());
  }
}

void Z3DWindow::emitAddTodoMarker(
    int x, int y, int z, bool checked, uint64_t bodyId)
{
  emit addingTodoMarker(x, y, z, checked, bodyId);
}

void Z3DWindow::emitAddTodoMarker(
    const ZIntPoint &pt, bool checked, uint64_t bodyId)
{
  emit addingTodoMarker(pt.getX(), pt.getY(), pt.getZ(), checked, bodyId);
}

void Z3DWindow::emitAddToMergeMarker(int x, int y, int z, uint64_t bodyId)
{
  emit addingToMergeMarker(x, y, z, bodyId);
}

void Z3DWindow::emitAddToMergeMarker(const ZIntPoint &pt, uint64_t bodyId)
{
  emit addingToMergeMarker(pt.getX(), pt.getY(), pt.getZ(), bodyId);
}

void Z3DWindow::emitAddToSplitMarker(int x, int y, int z, uint64_t bodyId)
{
  emit addingToSplitMarker(x, y, z, bodyId);
}

void Z3DWindow::emitAddToSplitMarker(const ZIntPoint &pt, uint64_t bodyId)
{
  emit addingToSplitMarker(pt.getX(), pt.getY(), pt.getZ(), bodyId);
}

void Z3DWindow::emitAddToSupervoxelSplitMarker(
    int x, int y, int z, uint64_t bodyId)
{
  emit addingToSupervoxelSplitMarker(x, y, z, bodyId);
}

void Z3DWindow::emitAddToSupervoxelSplitMarker(
    const ZIntPoint &pt, uint64_t bodyId)
{
  emit addingToSupervoxelSplitMarker(pt.getX(), pt.getY(), pt.getZ(), bodyId);
}

void Z3DWindow::emitAddTraceToSomaMarker(const ZIntPoint &pt, uint64_t bodyId)
{
  emit addingTraceToSomaMarker(pt.getX(), pt.getY(), pt.getZ(), bodyId);
}

void Z3DWindow::emitAddNoSomaMarker(const ZIntPoint &pt, uint64_t bodyId)
{
  emit addingNoSomaMarker(pt.getX(), pt.getY(), pt.getZ(), bodyId);
}

static void AddTodoMarker(
    Z3DWindow *window, neutu::EToDoAction action, bool checked)
{
  QList<Swc_Tree_Node*> swcNodeList =
      window->getDocument()->getSelectedSwcNodeList();
  if (swcNodeList.size() == 1) {
    Swc_Tree_Node *tn = swcNodeList.front();
    ZSwcTree *tree = window->getDocument()->nodeToSwcTree(tn);
    uint64_t bodyId = 0;
    if (tree != NULL) {
      bodyId = tree->getLabel();
    }
    ZIntPoint pt = SwcTreeNode::center(tn).toIntPoint();
    if (checked && (action != neutu::EToDoAction::NO_SOMA)) {
      window->emitAddTodoMarker(pt, checked, bodyId);
    } else {
      switch (action) {
      case neutu::EToDoAction::TO_DO:
        window->emitAddTodoMarker(pt, checked, bodyId);
        break;
      case neutu::EToDoAction::TO_MERGE:
        window->emitAddToMergeMarker(pt, bodyId);
        break;
      case neutu::EToDoAction::TO_SPLIT:
        window->emitAddToSplitMarker(pt, bodyId);
        break;
      case neutu::EToDoAction::TO_TRACE_TO_SOMA:
        window->emitAddTraceToSomaMarker(pt, bodyId);
        break;
      case neutu::EToDoAction::NO_SOMA:
        window->emitAddNoSomaMarker(pt, bodyId);
        break;
      case neutu::EToDoAction::DIAGNOSTIC: //todo
        LWARN() << "DIAGNOSTIC to be done";
        break;
      case neutu::EToDoAction::TO_DO_IRRELEVANT: //todo
        LWARN() << "TO_DO_IRRELEVANT to be done";
        break;
      case neutu::EToDoAction::TO_SUPERVOXEL_SPLIT: //Ignored
        LWARN() << "TO_SUPERVOXEL_SPLIT not available";
        break;
      default:
        break;
      }
    }
  }
}

void Z3DWindow::setNormalTodoVisible(bool visible)
{
  emit settingNormalTodoVisible(visible);
}

/*
void Z3DWindow::removeAllTodo()
{
}
*/

void Z3DWindow::updateTodoVisibility()
{
  getDocument()->notifyTodoModified();
}

void Z3DWindow::addTodoMarker()
{
  AddTodoMarker(this, neutu::EToDoAction::TO_DO, false);
}

void Z3DWindow::addToMergeMarker()
{
  AddTodoMarker(this, neutu::EToDoAction::TO_MERGE, false);
}

void Z3DWindow::addToSplitMarker()
{
  AddTodoMarker(this, neutu::EToDoAction::TO_SPLIT, false);
}

void Z3DWindow::addToSupervoxelSplitMarker()
{
  AddTodoMarker(this, neutu::EToDoAction::TO_SUPERVOXEL_SPLIT, false);
}

void Z3DWindow::addTraceToSomaMarker()
{
  AddTodoMarker(this, neutu::EToDoAction::TO_TRACE_TO_SOMA, false);
}

void Z3DWindow::addNoSomaMarker()
{
  AddTodoMarker(this, neutu::EToDoAction::NO_SOMA, false);
}

void Z3DWindow::addDoneMarker()
{
  AddTodoMarker(this, neutu::EToDoAction::TO_DO, true);
}

void Z3DWindow::setTodoItemToSplit()
{
  ZFlyEmBody3dDoc *doc = getDocument<ZFlyEmBody3dDoc>();
  if (doc != NULL) {
    doc->setTodoItemAction(neutu::EToDoAction::TO_SPLIT, false);
  }
}

void Z3DWindow::setTodoItemToNormal()
{
  ZFlyEmBody3dDoc *doc = getDocument<ZFlyEmBody3dDoc>();
  if (doc != NULL) {
    doc->setTodoItemAction(neutu::EToDoAction::TO_DO, false);
  }
}

void Z3DWindow::setTodoItemIrrelevant()
{
  ZFlyEmBody3dDoc *doc = getDocument<ZFlyEmBody3dDoc>();
  if (doc != NULL) {
    doc->setTodoItemAction(neutu::EToDoAction::TO_DO_IRRELEVANT, false);
  }
}

void Z3DWindow::setTodoItemTraceToSoma()
{
  ZFlyEmBody3dDoc *doc = getDocument<ZFlyEmBody3dDoc>();
  if (doc != NULL) {
    doc->setTodoItemAction(neutu::EToDoAction::TO_TRACE_TO_SOMA, false);
  }
}

void Z3DWindow::setTodoItemNoSoma()
{
  ZFlyEmBody3dDoc *doc = getDocument<ZFlyEmBody3dDoc>();
  if (doc != NULL) {
    doc->setTodoItemAction(neutu::EToDoAction::NO_SOMA, true);
  }
}

void Z3DWindow::updateBody()
{
  ZFlyEmBody3dDoc *doc = getDocument<ZFlyEmBody3dDoc>();
  if (doc != NULL) {
    doc->forceBodyUpdate();
  }
}

void Z3DWindow::copyPosition()
{
  ZStackDoc *doc = getDocument();
  std::set<Swc_Tree_Node*> nodeSet = doc->getSelectedSwcNodeSet();
  if (nodeSet.size() == 1) {
    ZPoint pos = SwcTreeNode::center(*(nodeSet.begin()));
    ZGlobal::GetInstance().setDataPosition(pos);
  } else {
    ZGlobal::GetInstance().clearStackPosition();
  }
}

void Z3DWindow::compareBody()
{
  ZFlyEmBody3dDoc *doc = getDocument<ZFlyEmBody3dDoc>();
  if (doc != NULL) {
    if (m_bodyCmpDlg == NULL) {
      m_bodyCmpDlg = new ZFlyEmBodyComparisonDialog(this);
      std::vector<std::string> uuidList = doc->getAncestorUuidList();
      m_bodyCmpDlg->setUuidList(uuidList);
      m_bodyCmpDlg->setCurrentUuidIndex(1);
    }

    m_bodyCmpDlg->clearPosition();

    if (m_bodyCmpDlg->exec()) {
      doc->compareBody(m_bodyCmpDlg);
    }
  }
}

void Z3DWindow::deselectBody()
{
  std::set<uint64_t> bodySet;

  QList<ZSwcTree*> swcList = getDocument()->getSelectedObjectList<ZSwcTree>();
  foreach (const ZSwcTree *tree, swcList) {
    if (tree->getLabel() > 0) {
      bodySet.insert(tree->getLabel());
    }
  }

  QList<Swc_Tree_Node*> swcNodeList =
      getDocument()->getSelectedSwcNodeList();
  for (QList<Swc_Tree_Node*>::const_iterator iter = swcNodeList.begin();
       iter != swcNodeList.end(); ++iter) {
    const Swc_Tree_Node *tn = *iter;
    ZSwcTree *tree = getDocument()->nodeToSwcTree(tn);
    uint64_t bodyId = 0;
    if (tree != NULL) {
      bodyId = tree->getLabel();
    }
    if (bodyId > 0) {
      bodySet.insert(bodyId);
    }
  }
  if (!bodySet.empty()) {
    emit deselectingBody(bodySet);
  }
}

void Z3DWindow::changeSelectedPunctaName()
{
  bool ok;
  QString text = QInputDialog::getText(this, tr("New Name"),
                                       tr("Punctum name:"), QLineEdit::Normal,
                                       "", &ok);
  if (ok) {
    //std::set<ZPunctum*> *punctumSet = m_doc->selectedPuncta();
    QList<ZPunctum*> punctumSet =
        m_doc->getSelectedObjectList<ZPunctum>(ZStackObject::EType::PUNCTUM);
    for (QList<ZPunctum*>::iterator iter = punctumSet.begin();
         iter != punctumSet.end(); ++iter) {
      ZPunctum *punctum = *iter;
      punctum->setName(text);
    }
    m_doc->notifyPunctumModified();
  }
}

void Z3DWindow::saveAllPunctaAs()
{
  QString filename =
    QFileDialog::getSaveFileName(this, tr("Save All Puncta"), m_lastOpenedFilePath,
         tr("Puncta files (*.apo)"));

  if (!filename.isEmpty()) {
    m_lastOpenedFilePath = filename;
    QList<ZPunctum*> punctaList = m_doc->getPunctumList();
    ZPunctumIO::save(filename, punctaList.begin(), punctaList.end());
  }
}

void Z3DWindow::markPunctum()
{
  m_doc->markPunctum(m_lastClickedPosInVolume[0], m_lastClickedPosInVolume[1], m_lastClickedPosInVolume[2]);
}

void Z3DWindow::openAdvancedSetting(const QString &name)
{
  if (!m_advancedSettingDockWidget) {
    m_advancedSettingDockWidget = new QDockWidget(tr("Advanced Settings"), this);
    m_advancedSettingDockWidget->setContentsMargins(20, 20, 20, 20);
    m_advancedSettingDockWidget->setMinimumSize(512, 512);
    QTabWidget *tabs = createAdvancedSettingTabWidget();
    m_advancedSettingDockWidget->setWidget(tabs);
    addDockWidget(Qt::RightDockWidgetArea, m_advancedSettingDockWidget);
    m_advancedSettingDockWidget->setFloating(true);
    m_viewMenu->addAction(m_advancedSettingDockWidget->toggleViewAction());
  }
  m_advancedSettingDockWidget->showNormal();
  QTabWidget *tabWidget = qobject_cast<QTabWidget*>(m_advancedSettingDockWidget->widget());
  for (int i=0; i<tabWidget->count(); i++) {
    if (tabWidget->tabText(i) == name) {
      tabWidget->setCurrentIndex(i);
      break;
    }
  }
}

void Z3DWindow::updateSettingsDockWidget()
{
  KINFO << "Updating dock widgets";
  //  QScrollArea *oldSA = qobject_cast<QScrollArea*>(m_settingsDockWidget->widget());
  //  int oldScrollBarValue = 0;
  //  if (oldSA) {
  //    QScrollBar *bar = oldSA->verticalScrollBar();
  //    oldScrollBarValue = bar->value();
  //  }
  //  QScrollArea *newSA = qobject_cast<QScrollArea*>(m_widgetsGroup->createWidget(this, true));
  //  newSA->verticalScrollBar()->setValue(oldScrollBarValue);
  QTabWidget *old = qobject_cast<QTabWidget*>(m_settingsDockWidget->widget());
  int oldIndex = 0;
  if (old)
    oldIndex = old->currentIndex();
  QTabWidget *tabs = createBasicSettingTabWidget();
  if (oldIndex >= 0 && oldIndex < tabs->count())
    tabs->setCurrentIndex(oldIndex);
  m_settingsDockWidget->setUpdatesEnabled(false);
  m_settingsDockWidget->setWidget(tabs);
  m_settingsDockWidget->setUpdatesEnabled(true);
  if (old) {
    old->setParent(NULL);
    delete old;
  }

  // for advanced setting widget
  if (m_advancedSettingDockWidget) {
    QTabWidget *old = qobject_cast<QTabWidget*>(m_advancedSettingDockWidget->widget());
    int oldIndex = 0;
    if (old)
      oldIndex = old->currentIndex();
    QTabWidget *tabs = createAdvancedSettingTabWidget();
    if (oldIndex >= 0 && oldIndex < tabs->count())
      tabs->setCurrentIndex(oldIndex);
    m_advancedSettingDockWidget->setUpdatesEnabled(false);
    m_advancedSettingDockWidget->setWidget(tabs);
    m_advancedSettingDockWidget->setUpdatesEnabled(true);
    if (old) {
      old->setParent(NULL);
      delete old;
    }
  }
  KINFO << "Dock widget updated";
}

void Z3DWindow::toogleAddSwcNodeMode(bool checked)
{
  if (checked) {
    //    if (m_toogleExtendSelectedSwcNodeAction->isChecked()) {
    //      m_toogleExtendSelectedSwcNodeAction->blockSignals(true);
    //      m_toogleExtendSelectedSwcNodeAction->setChecked(false);
    //      m_toogleExtendSelectedSwcNodeAction->blockSignals(false);
    //    }
    if (m_toggleSmartExtendSelectedSwcNodeAction->isChecked()) {
      m_toggleSmartExtendSelectedSwcNodeAction->blockSignals(true);
      m_toggleSmartExtendSelectedSwcNodeAction->setChecked(false);
      m_toggleSmartExtendSelectedSwcNodeAction->blockSignals(false);
    }
    getSwcFilter()->setInteractionMode(Z3DSwcFilter::EInteractionMode::AddSwcNode);
    getCanvas()->getInteractionContext().setSwcEditMode(
          ZInteractiveContext::SWC_EDIT_ADD_NODE);
    //m_canvas->setCursor(Qt::PointingHandCursor);
    notifyUser("Click to add a node");
  } else {
    getSwcFilter()->setInteractionMode(Z3DSwcFilter::EInteractionMode::Select);
    getCanvas()->getInteractionContext().setSwcEditMode(
          ZInteractiveContext::SWC_EDIT_OFF);
    //getCanvas()->setCursor(Qt::ArrowCursor);
  }
  getCanvas()->updateCursor();
}

//void Z3DWindow::toogleExtendSelectedSwcNodeMode(bool checked)
//{
//  if (checked) {
//    if (m_toogleAddSwcNodeModeAction->isChecked()) {
//      m_toogleAddSwcNodeModeAction->blockSignals(true);
//      m_toogleAddSwcNodeModeAction->setChecked(false);
//      m_toogleAddSwcNodeModeAction->blockSignals(false);
//    }
//    if (m_toogleSmartExtendSelectedSwcNodeAction->isChecked()) {
//      m_toogleSmartExtendSelectedSwcNodeAction->blockSignals(true);
//      m_toogleSmartExtendSelectedSwcNodeAction->setChecked(false);
//      m_toogleSmartExtendSelectedSwcNodeAction->blockSignals(false);
//    }
//    m_canvas->setCursor(Qt::PointingHandCursor);
//  } else {
//    m_canvas->setCursor(Qt::ArrowCursor);
//  }
//}

void Z3DWindow::toogleSmartExtendSelectedSwcNodeMode(bool checked)
{
  if (checked) {
    if (m_toggleAddSwcNodeModeAction->isChecked()) {
      m_toggleAddSwcNodeModeAction->blockSignals(true);
      m_toggleAddSwcNodeModeAction->setChecked(false);
      m_toggleAddSwcNodeModeAction->blockSignals(false);
    }
    //    if (m_toogleExtendSelectedSwcNodeAction->isChecked()) {
    //      m_toogleExtendSelectedSwcNodeAction->blockSignals(true);
    //      m_toogleExtendSelectedSwcNodeAction->setChecked(false);
    //      m_toogleExtendSelectedSwcNodeAction->blockSignals(false);
    //    }
    notifyUser("Left click to extend. Path calculation is off when 'Cmd/Ctrl' is pressed."
               "Right click to exit extending mode.");
    if (getDocument()->hasStackData()) {
      getSwcFilter()->setInteractionMode(Z3DSwcFilter::EInteractionMode::SmartExtendSwcNode);
    } else {
      getSwcFilter()->setInteractionMode(Z3DSwcFilter::EInteractionMode::PlainExtendSwcNode);
    }
    getCanvas()->getInteractionContext().setSwcEditMode(
          ZInteractiveContext::SWC_EDIT_SMART_EXTEND);
    //m_canvas->setCursor(Qt::PointingHandCursor);
  } else {
    getSwcFilter()->setInteractionMode(Z3DSwcFilter::EInteractionMode::Select);
    getCanvas()->getInteractionContext().setSwcEditMode(
          ZInteractiveContext::SWC_EDIT_OFF);
    //getCanvas()->setCursor(Qt::ArrowCursor);
  }
  getCanvas()->updateCursor();
}

QTabWidget* Z3DWindow::getSettingsTabWidget() const
{
  return qobject_cast<QTabWidget*>(m_settingsDockWidget->widget());
}

void Z3DWindow::changeBackground()
{
  showControlPanel();
  const auto& grps = m_widgetsGroup->getChildGroups();
  int index = std::find(grps.begin(), grps.end(), m_view->backgroundWidgetsGroup()) - grps.begin();
//  QTabWidget *tab = qobject_cast<QTabWidget*>(m_settingsDockWidget->widget());
  getSettingsTabWidget()->setCurrentIndex(index);
}

bool Z3DWindow::isBackgroundOn() const
{
  return getCompositor()->showingBackground();
}

void Z3DWindow::toogleMoveSelectedObjectsMode(bool checked)
{
  getInteractionHandler()->setMoveObjects(checked);
  getCanvas()->updateCursor();
  if (checked) {
    notifyUser("Shift + Mouse to move selected objects");
  }
}

void Z3DWindow::moveSelectedObjects(double x, double y, double z)
{
  m_doc->executeMoveObjectCommand(x, y, z,
                                  getPunctaFilter()->coordTransform(),
                                  getSwcFilter()->coordTransform());
}

void Z3DWindow::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("text/uri-list")) {
    event->acceptProposedAction();
  }
}

void Z3DWindow::dropEvent(QDropEvent *event)
{
  QList<QUrl> urls = event->mimeData()->urls();
  m_doc->loadFileList(urls);
}

void Z3DWindow::closeEvent(QCloseEvent * /*event*/)
{
  writeSettings();
  emit closed();
}

void Z3DWindow::keyPressEvent(QKeyEvent *event)
{
  if (m_skippingKeyEvent) {
    emit keyPressed(event);
    return;
  }

  neutu::LogKeyPressEvent(event, "Z3DWindow");
//  KINFO << QString("Key %1 pressed in Z3DWindow").
//           arg(neutu::GetKeyString(event->key(), event->modifiers()));

  ZInteractionEngine::EKeyMode keyMode = ZInteractionEngine::KM_NORMAL;
  switch(event->key())
  {
  case Qt::Key_Backspace:
  case Qt::Key_Delete:
  {
    deleteSelectedSwcNode();
    removeSelectedObject();
  }
    break;
  case Qt::Key_A:
    if (event->modifiers() == Qt::ControlModifier) {
      getDocument()->selectAllSwcTreeNode();
    }
    break;
  case Qt::Key_C:
  if (getDocument()->getTag() != neutu::Document::ETag::FLYEM_BODY_3D_COARSE &&
      getDocument()->getTag() != neutu::Document::ETag::FLYEM_BODY_3D){
    if (event->modifiers() == Qt::ControlModifier) {
      std::set<Swc_Tree_Node*> nodeSet = m_doc->getSelectedSwcNodeSet();
      if (nodeSet.size() > 0) {
        SwcTreeNode::clearClipboard();
      }
      for (std::set<Swc_Tree_Node*>::const_iterator iter = nodeSet.begin();
           iter != nodeSet.end(); ++iter) {
        SwcTreeNode::addToClipboard(*iter);
      }
    } else if (event->modifiers() == Qt::NoModifier) {
      if (m_doc->hasSelectedSwcNode()) {
        if (m_doc->getSelectedSwcNodeNumber() > 1) {
          m_doc->executeConnectSwcNodeCommand();
        } else {
          if (m_toggleMoveSelectedObjectsAction->isChecked()) {
            m_toggleMoveSelectedObjectsAction->toggle();
          }
          if (m_toggleSmartExtendSelectedSwcNodeAction->isChecked()) {
            m_toggleSmartExtendSelectedSwcNodeAction->toggle();
          }
          startConnectingSwcNode();
        }
      }
    }
  }
    break;
  case Qt::Key_P:
  {
    if (event->modifiers() == Qt::ControlModifier) {
      for (size_t i = 0; i < SwcTreeNode::clipboard().size(); ++i) {
        std::cout << SwcTreeNode::clipboard()[i] << std::endl;
      }
    }
  }
    break;
  case Qt::Key_S:
    if (event->modifiers() == Qt::ControlModifier) {
      m_doc->saveSwc(this);
    } else if (event->modifiers() == Qt::NoModifier) {
      if (getDocument()->getTag() == neutu::Document::ETag::NORMAL ||
          getDocument()->getTag() == neutu::Document::ETag::FLYEM_SKELETON ||
          getDocument()->getTag() == neutu::Document::ETag::BIOCYTIN_STACK) {
        keyMode = ZInteractionEngine::KM_SWC_SELECTION;
      }
    }
    break;
  case Qt::Key_I:
    getDocument()->executeInsertSwcNode();
    break;
  case Qt::Key_B:
#ifdef _DEBUG_2
    if (event->modifiers() == Qt::ControlModifier) {
      QList<ZSwcTree*> *treeList = m_doc->swcList();
      foreach (ZSwcTree *tree, *treeList) {
        ZString source = tree->source();
        std::cout << source.lastInteger() << std::endl;
      }
    }
#endif
    if (event->modifiers() == Qt::NoModifier) {
      m_doc->executeBreakSwcConnectionCommand();
    } else if (event->modifiers() == Qt::ControlModifier) {
      m_cuttingStackBound = !m_cuttingStackBound;
      updateCuttingBox();
    }
    break;
  case Qt::Key_Equal: // increase swc size scale
  {
    if (event->modifiers() == Qt::ControlModifier) {
      getSwcFilter()->setSizeScale(getSwcFilter()->sizeScale() + .1);
    }
  }
    break;
  case Qt::Key_Minus:  // decrease swc size scale
  {
    if (event->modifiers() == Qt::ControlModifier) {
      getSwcFilter()->setSizeScale(std::max(.1, getSwcFilter()->sizeScale() - .1));
    }
  }
    break;
  case Qt::Key_G:  // change swc display mode
  {
    if (event->modifiers() == Qt::ControlModifier) {
      getSwcFilter()->changeGeometryMode();
    }
  }
    break;
  case Qt::Key_Comma:
  case Qt::Key_Q:
    getDocument()->executeSwcNodeChangeSizeCommand(-0.5);
    break;
  case Qt::Key_F:
    if (event->modifiers() == Qt::ShiftModifier) {
      zoomToSelectedSwcNodes();
    }
    break;
  case Qt::Key_Period:
  case Qt::Key_E:
    getDocument()->executeSwcNodeChangeSizeCommand(0.5);
    break;
  case Qt::Key_X:
    if (event->modifiers() == Qt::NoModifier) {
      getDocument()->executeDeleteSwcNodeCommand();
    }
    break;
  case Qt::Key_N:
    if (event->modifiers() == Qt::NoModifier) {
      getDocument()->executeConnectIsolatedSwc();
    }
    break;
  case Qt::Key_Z:
    if (event->modifiers() == Qt::NoModifier) {
      ZFlyEmBody3dDoc *doc = qobject_cast<ZFlyEmBody3dDoc*>(getDocument());
      bool located = false;
      if (doc != NULL) {
        ZFlyEmToDoItem *item = doc->getOneSelectedTodoItem();
        if (item !=NULL) {
          locate2DView(item->getPosition().toPoint(), item->getRadius());
          located = true;
        }
        /*
        if (doc->hadTodoItemSelected()) {
          locateTodoItemIn2DView();
        }
        */
      }

      if (!located) {
        if (getDocument()->hasSelectedSwcNode()) {
          locateSwcNodeIn2DView();
        } else if (getDocument()->hasSelectedPuncta()) {
          locatePunctumIn2DView();
        }
      }
    } else if (event->modifiers() == Qt::ShiftModifier) {
      zoomToSelectedSwcNodes();
    }
    break;
  case Qt::Key_V:
    if (event->modifiers() == Qt::NoModifier) {
      if (!m_toggleMoveSelectedObjectsAction->isChecked()) {
        if (m_toggleSmartExtendSelectedSwcNodeAction->isChecked()) {
          m_toggleSmartExtendSelectedSwcNodeAction->toggle();
        }
        if (getSwcFilter()->interactionMode() == Z3DSwcFilter::EInteractionMode::ConnectSwcNode) {
            getSwcFilter()->setInteractionMode(Z3DSwcFilter::EInteractionMode::Select);
            getCanvas()->setCursor(Qt::ArrowCursor);
        }
        m_toggleMoveSelectedObjectsAction->toggle();
      }
    }
    break;
  case Qt::Key_Space:
    if (getDocument()->getSelectedSwcNodeNumber() == 1) {
      if (!m_toggleSmartExtendSelectedSwcNodeAction->isChecked()) {
        if (m_toggleMoveSelectedObjectsAction->isChecked()) {
          m_toggleMoveSelectedObjectsAction->toggle();
        }
        if (getSwcFilter()->interactionMode() == Z3DSwcFilter::EInteractionMode::ConnectSwcNode) {
            getSwcFilter()->setInteractionMode(Z3DSwcFilter::EInteractionMode::Select);
            getCanvas()->setCursor(Qt::ArrowCursor);
        }
        m_toggleSmartExtendSelectedSwcNodeAction->toggle();
      }
    } else {
      if (GET_APPLICATION_NAME == "FlyEM") {
        if (event->modifiers() == Qt::ShiftModifier) {
          QCursor oldCursor = getCanvas()->cursor();
          getCanvas()->setCursor(Qt::BusyCursor);
          getDocument()->runSeededWatershed();
          notifyUser("Body splitted");
          getCanvas()->setCursor(oldCursor);
        }
      }
    }
    break;
  case Qt::Key_R:
    if (event->modifiers() == Qt::NoModifier) {
      m_doc->executeResetBranchPoint();
    } else {
      m_doc->executeSetRootCommand();
    }
    break;
  case Qt::Key_1:
    if (getCanvas()->getInteractionEngine()->getKeyMode() ==
        ZInteractionEngine::KM_SWC_SELECTION) {
      m_doc->selectDownstreamNode();
    }
    break;
  case Qt::Key_2:
    if (getCanvas()->getInteractionEngine()->getKeyMode() ==
        ZInteractionEngine::KM_SWC_SELECTION) {
      m_doc->selectUpstreamNode();
    }
    break;
  case Qt::Key_3:
    if (getCanvas()->getInteractionEngine()->getKeyMode() ==
        ZInteractionEngine::KM_SWC_SELECTION) {
      m_doc->selectConnectedNode();
    }
    break;
  case Qt::Key_4:
    if (getCanvas()->getInteractionEngine()->getKeyMode() ==
        ZInteractionEngine::KM_SWC_SELECTION) {
      m_doc->inverseSwcNodeSelection();
    }
    break;
  case Qt::Key_5:
    if (getCanvas()->getInteractionEngine()->getKeyMode() ==
        ZInteractionEngine::KM_SWC_SELECTION) {
      if (m_swcIsolationDlg->exec()) {
        m_doc->selectNoisyTrees(m_swcIsolationDlg->getLengthThreshold(),
                                m_swcIsolationDlg->getDistanceThreshold());
      }
    }
    break;
  case Qt::Key_H:
    Z3DWindowController::ToggleMeshVisible(this);
    break;
  case Qt::Key_T:
    if (event->modifiers() == Qt::NoModifier) {
      if (getAction(ZActionFactory::ACTION_ACTIVATE_LOCATE)->isChecked()) {
        getAction(ZActionFactory::ACTION_VIEW_DATA_EXTERNALLY)->trigger();
      } else {
        getAction(ZActionFactory::ACTION_ACTIVATE_LOCATE)->trigger();
      }
    } else if (event->modifiers() == Qt::ShiftModifier) {
      getAction(ZActionFactory::ACTION_VIEW_DATA_EXTERNALLY)->trigger();
    }
    break;
  default:
    break;
  }

  getCanvas()->setKeyMode(keyMode);
}

QTabWidget *Z3DWindow::createBasicSettingTabWidget()
{
  QTabWidget *tabs = new QTabWidget();
  tabs->setElideMode(Qt::ElideNone);
  tabs->setUsesScrollButtons(true);
  for (const auto& grp : m_widgetsGroup->getChildGroups()) {
    if (grp->isGroup()) {
      QWidget *widget = grp->createWidget(true);
      tabs->addTab(widget, grp->getGroupName());
    }
  }
  return tabs;
}

QTabWidget *Z3DWindow::createAdvancedSettingTabWidget()
{
  QTabWidget *tabs = new QTabWidget();
  tabs->setElideMode(Qt::ElideNone);
  for (const auto& grp : m_widgetsGroup->getChildGroups()) {
    if (grp->isGroup() && grp->getGroupName() != "Capture" &&
        grp->getGroupName() != "Utils") {
      tabs->addTab(grp->createWidget(false), grp->getGroupName());
    }
  }
  return tabs;
}

void Z3DWindow::updateContextMenu(const QString &group)
{
  if (group == "empty") {
    m_contextMenuGroup["empty"]->clear();
    if (channelNumber() > 0 && getVolumeFilter()->volumeNeedDownsample() &&
        getVolumeFilter()->isSubvolume()) {
      m_contextMenuGroup["empty"]->addAction(m_exitVolumeZoomInViewAction);
    }
    //if (!m_doc->swcList()->empty() && m_swcFilter->isNodeRendering())
      //m_contextMenuGroup["empty"]->addAction(m_toogleExtendSelectedSwcNodeAction);
    /*
    if (channelNumber() > 0 && !m_doc->swcList()->empty() && m_swcFilter->isNodeRendering())
      m_contextMenuGroup["empty"]->addAction(m_toogleSmartExtendSelectedSwcNodeAction);
*/
    if (m_doc->hasSwc() && getSwcFilter()->isNodeRendering())
      m_contextMenuGroup["empty"]->addAction(m_toggleAddSwcNodeModeAction);
    if (m_doc->hasSwc() || m_doc->hasPuncta())
      m_contextMenuGroup["empty"]->addAction(m_toggleMoveSelectedObjectsAction);
    m_contextMenuGroup["empty"]->addAction(m_changeBackgroundAction);
    m_contextMenuGroup["empty"]->addAction(m_toggleObjectsAction);
    m_contextMenuGroup["empty"]->addAction(m_toggleSettingsAction);
  }

  if (group == "volume") {
    m_contextMenuGroup["volume"]->clear();
    if (getVolumeFilter()->volumeNeedDownsample()) {
      if (getVolumeFilter()->isSubvolume()) {
        m_contextMenuGroup["volume"]->addAction(m_exitVolumeZoomInViewAction);
      } else {
        m_contextMenuGroup["volume"]->addAction(m_openVolumeZoomInViewAction);
      }
    }
    //m_contextMenuGroup["volume"]->addAction(m_markPunctumAction);
    //if (!m_doc->swcList()->empty() && m_swcFilter->isNodeRendering())
      //m_contextMenuGroup["volume"]->addAction(m_toogleExtendSelectedSwcNodeAction);
    /*
    if (!m_doc->swcList()->empty() && m_swcFilter->isNodeRendering())
      m_contextMenuGroup["volume"]->addAction(m_toogleSmartExtendSelectedSwcNodeAction);
*/
    if (m_doc->hasSwc() && getSwcFilter()->isNodeRendering())
      m_contextMenuGroup["volume"]->addAction(m_toggleAddSwcNodeModeAction);
    if (m_doc->hasSwc() || m_doc->hasPuncta())
      m_contextMenuGroup["volume"]->addAction(m_toggleMoveSelectedObjectsAction);
    m_contextMenuGroup["volume"]->addAction(m_changeBackgroundAction);
    m_contextMenuGroup["volume"]->addAction(m_refreshTraceMaskAction);
    if (m_doc->getTag() == neutu::Document::ETag::FLYEM_SPLIT) {
      m_contextMenuGroup["volume"]->addAction(m_markPunctumAction);
    }
  }
  if (group == "swcnode") {
    getDocument()->updateSwcNodeAction();
    m_singleSwcNodeActionActivator.update(this);
  }
  if (group == "puncta") {
    m_saveSelectedPunctaAsAction->setEnabled(!m_doc->hasSelectedPuncta());
    m_saveAllPunctaAsAction->setEnabled(m_doc->hasPuncta());
    m_locatePunctumIn2DAction->setEnabled(
          m_doc->getSelected(ZStackObject::EType::PUNCTUM).size() == 1);
  }
}


void Z3DWindow::changeSelectedSwcNodeType()
{
  std::set<Swc_Tree_Node*> nodeSet = m_doc->getSelectedSwcNodeSet();
  if (nodeSet.size() > 0) {
    SwcTypeDialog dlg(ZSwcTree::SWC_NODE, NULL);
    if (dlg.exec()) {
      switch (dlg.pickingMode()) {
      case SwcTypeDialog::INDIVIDUAL:
        for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
             iter != nodeSet.end(); ++iter) {
          SwcTreeNode::setType(*iter, dlg.type());
        }
        break;
      case SwcTypeDialog::DOWNSTREAM:
        for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
             iter != nodeSet.end(); ++iter) {
          SwcTreeNode::setDownstreamType(*iter, dlg.type());
        }
        break;
      case SwcTypeDialog::CONNECTION:
      {
        Swc_Tree_Node *ancestor = *(nodeSet.begin());

        for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
             iter != nodeSet.end(); ++iter) {
          ancestor = SwcTreeNode::commonAncestor(ancestor, *iter);
        }

        for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
             iter != nodeSet.end(); ++iter) {
          SwcTreeNode::setUpstreamType(*iter, dlg.type(), ancestor);
        }
      }
        break;
      case SwcTypeDialog::LONGEST_LEAF:
      {
        Swc_Tree_Node *tn = SwcTreeNode::furthestNode(*(nodeSet.begin()),
                                                      SwcTreeNode::GEODESIC);
        SwcTreeNode::setPathType(*(nodeSet.begin()), tn, dlg.type());
      }
        break;
      default:
        break;
      }
      getSwcFilter()->addNodeType(dlg.type());

      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::setRootAsSelectedSwcNode()
{
  m_doc->executeSetRootCommand();
      /*
  std::set<Swc_Tree_Node*> *nodeSet = m_doc->selectedSwcTreeNodes();
  if (nodeSet->size() == 1) {
    m_doc->executeSetRootCommand();

    SwcTreeNode::setAsRoot(*nodeSet->begin());
    m_doc->notifySwcModified();

  }
      */
}

void Z3DWindow::breakSelectedSwcNode()
{
  std::set<Swc_Tree_Node*> nodeSet = m_doc->getSelectedSwcNodeSet();
  if (nodeSet.size() >= 2) {
    m_doc->executeBreakSwcConnectionCommand();
#if 0
    for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
         iter != nodeSet->end(); ++iter) {
      if (nodeSet->count((*iter)->parent) > 0) {
        SwcTreeNode::detachParent(*iter);
        ZSwcTree *tree = new ZSwcTree();
        tree->setDataFromNode(*iter);
        /*
        std::ostringstream stream;
        stream << "node-break" << "-" << tree;
        tree->setSource(stream.str());
        */
        m_doc->addSwcTree(tree, false);
      }
    }

    m_doc->notifySwcModified();
#endif
    /*
    std::set<Swc_Tree_Node*>::iterator iter = nodeSet->begin();
    Swc_Tree_Node *tn1 = *iter;
    ++iter;
    Swc_Tree_Node *tn2 = *iter;
    Swc_Tree_Node *child = NULL;
    if (tn1->parent == tn2) {
      child = tn1;
    } else if (tn2->parent == tn1) {
      child = tn2;
    }
    if (child != NULL) {
      Swc_Tree_Node_Detach_Parent(child);
      ZSwcTree *tree = new ZSwcTree();
      tree->setDataFromNode(child);
      m_doc->addSwcTree(tree);
      m_doc->requestRedrawSwc();
    }
    */
  }
}


void Z3DWindow::mergeSelectedSwcNode()
{
  m_doc->executeMergeSwcNodeCommand();
#if 0
  std::set<Swc_Tree_Node*> *nodeSet = m_doc->selectedSwcTreeNodes();

  if (nodeSet->size() > 1) {
    Swc_Tree_Node *coreNode = SwcTreeNode::merge(*nodeSet);
    if (coreNode != NULL) {
      if (SwcTreeNode::parent(coreNode) == NULL) {
        ZSwcTree *tree = new ZSwcTree();
        tree->setDataFromNode(coreNode);
        //m_doc->addSwcTree(tree, false);
        m_doc->executeAddSwcCommand(tree);
      }

      m_doc->executeDeleteSwcNodeCommand();
      //SwcTreeNode::kill(*nodeSet);

      //m_doc->removeEmptySwcTree();
      m_doc->notifySwcModified();
    }
  }
#endif
}

void Z3DWindow::deleteSelectedSwcNode()
{
  m_doc->executeDeleteSwcNodeCommand();
}

void Z3DWindow::locateSwcNodeIn2DView()
{
  if (m_doc->hasSelectedSwcNode()) {
    if (m_doc->getParentFrame() != NULL) {
      m_doc->getParentFrame()->zoomToSelectedSwcNodes();
      m_doc->getParentFrame()->raise();
      /*
      ZCuboid cuboid = SwcTreeNode::boundBox(*m_doc->selectedSwcTreeNodes());
      int cx, cy, cz;
      ZPoint center = cuboid.center();
      cx = iround(center.x());
      cy = iround(center.y());
      cz = iround(center.z());
      int radius = iround(std::max(cuboid.width(), cuboid.height()) / 2.0);
      m_doc->getParentFrame()->viewRoi(cx, cy, cz, radius);
      */
    } else {
      const std::set<Swc_Tree_Node*> &nodeSet = m_doc->getSelectedSwcNodeSet();

      ZCuboid cuboid = SwcTreeNode::boundBox(nodeSet);
      ZPoint center = cuboid.center();
      int cx, cy, cz;

      //-= document()->getStackOffset();
      cx = iround(center.x());
      cy = iround(center.y());
      cz = iround(center.z());
      int radius = iround(std::max(cuboid.width(), cuboid.height()) / 2.0);
      const int minRadius = 400;
      if (radius < minRadius) {
        radius = minRadius;
      }
      int width = radius * 2 + 1;

      emit locating2DViewTriggered(cx, cy, cz, width);
    }
  }
}

void Z3DWindow::locate2DView(const ZPoint &center, double radius)
{
  int width = iround(radius * 2 + 1);

  const int minWidth = 800;

  if (width < minWidth) {
    width = minWidth;
  }

  double cx = center.getX();
  double cy = center.getY();
  double cz = center.getZ();

  emit locating2DViewTriggered(iround(cx), iround(cy), iround(cz), width);
}

void Z3DWindow::locatePunctumIn2DView()
{
  QList<ZPunctum*> punctumList =
      m_doc->getSelectedObjectList<ZPunctum>(ZStackObject::EType::PUNCTUM);
  if (punctumList.size() == 1) {
    ZPunctum* punc = *(punctumList.begin());

    locate2DView(punc->getCenter(), punc->radius());
  }
}

void Z3DWindow::tranlateSelectedSwcNode()
{
  std::set<Swc_Tree_Node*> nodeSet = m_doc->getSelectedSwcNodeSet();

  if (!nodeSet.empty()) {
    SwcSkeletonTransformDialog dlg(NULL);
    if (SwcTreeNode::clipboard().size() >= 2) {
      Swc_Tree_Node node[2];
      for (size_t i = 0; i < 2; ++i) {
        SwcTreeNode::paste(node + i, i);
      }

      ZPoint offset = SwcTreeNode::center(node + 1) - SwcTreeNode::center(node);
      dlg.setTranslateValue(offset.x(), offset.y(), offset.z());
    }
    if (dlg.exec()) {
      double dx = dlg.getTranslateValue(SwcSkeletonTransformDialog::X);
      double dy = dlg.getTranslateValue(SwcSkeletonTransformDialog::Y);
      double dz = dlg.getTranslateValue(SwcSkeletonTransformDialog::Z);

      for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
           iter != nodeSet.end(); ++iter) {
        if (dlg.isTranslateFirst()) {
          SwcTreeNode::translate(*iter, dx, dy, dz);
        }

        SwcTreeNode::setPos(
              *iter,
              SwcTreeNode::x(*iter) * dlg.getScaleValue(SwcSkeletonTransformDialog::X),
              SwcTreeNode::y(*iter) * dlg.getScaleValue(SwcSkeletonTransformDialog::Y),
              SwcTreeNode::z(*iter) * dlg.getScaleValue(SwcSkeletonTransformDialog::Z));

        if (!dlg.isTranslateFirst()) {
          SwcTreeNode::translate(*iter, dx, dy, dz);
        }
      }
      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::connectSelectedSwcNode()
{
  m_doc->executeConnectSwcNodeCommand();
  /*
  std::set<Swc_Tree_Node*> *nodeSet = m_doc->selectedSwcTreeNodes();
  if (SwcTreeNode::connect(*nodeSet)) {
    m_doc->removeEmptySwcTree();
    m_doc->notifySwcModified();
  }
  */
}

void Z3DWindow::changeSelectedSwcType()
{
  //std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();
  std::set<ZSwcTree*> treeSet =
      m_doc->getSelectedObjectSet<ZSwcTree>(ZStackObject::EType::SWC);

  if (!treeSet.empty()) {
    SwcTypeDialog dlg(ZSwcTree::WHOLE_TREE, NULL);

    if (dlg.exec()) {
      switch (dlg.pickingMode()) {
      case SwcTypeDialog::INDIVIDUAL:
        for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
             iter != treeSet.end(); ++iter) {
          (*iter)->setType(dlg.type());
        }
        break;
      case SwcTypeDialog::MAIN_TRUNK:
        for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
             iter != treeSet.end(); ++iter) {
          ZSwcBranchingTrunkAnalyzer trunkAnalyzer;
          trunkAnalyzer.setDistanceWeight(15.0, 1.0);
          ZSwcPath branch = (*iter)->mainTrunk(&trunkAnalyzer);
          branch.setType(dlg.type());
        }
        break;
      case SwcTypeDialog::LONGEST_LEAF:
        for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
             iter != treeSet.end(); ++iter) {
          ZSwcBranchingTrunkAnalyzer trunkAnalyzer;
          trunkAnalyzer.setDistanceWeight(0.0, 1.0);
          ZSwcPath branch = (*iter)->mainTrunk(&trunkAnalyzer);
          branch.setType(dlg.type());
        }
        break;
      case SwcTypeDialog::FURTHEST_LEAF:
        for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
             iter != treeSet.end(); ++iter) {
          ZSwcBranchingTrunkAnalyzer trunkAnalyzer;
          trunkAnalyzer.setDistanceWeight(1.0, 0.0);
          ZSwcPath branch = (*iter)->mainTrunk(&trunkAnalyzer);
          branch.setType(dlg.type());
        }
        break;
      case SwcTypeDialog::TRAFFIC:
        for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
             iter != treeSet.end(); ++iter) {
          ZSwcDistTrunkAnalyzer trunkAnalyzer;
          trunkAnalyzer.labelTraffic(*iter, ZSwcTrunkAnalyzer::REACH_ROOT);
          (*iter)->setTypeByLabel();
        }
        break;
      case SwcTypeDialog::TRUNK_LEVEL:
        for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
             iter != treeSet.end(); ++iter) {
          //ZSwcDistTrunkAnalyzer trunkAnalyzer;
          ZSwcWeightTrunkAnalyzer trunkAnalyzer;
          (*iter)->setBranchSizeWeight();
          (*iter)->labelTrunkLevel(&trunkAnalyzer);
          //trunkAnalyzer.labelTrunk(*iter);
          (*iter)->setTypeByLabel();
        }
        break;
      case SwcTypeDialog::BRANCH_LEVEL:
        for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
             iter != treeSet.end(); ++iter) {
          //(*iter)->labelBranchLevel(0);
          (*iter)->labelBranchLevelFromLeaf();
          (*iter)->setTypeByLabel();
        }
        break;
      case SwcTypeDialog::ROOT:
        for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
             iter != treeSet.end(); ++iter) {
          Swc_Tree_Node *tn = (*iter)->firstRegularRoot();
          while (tn != NULL) {
            SwcTreeNode::setType(tn, dlg.type());
            tn = SwcTreeNode::nextSibling(tn);
          }
        }
        break;
      case SwcTypeDialog::SUBTREE:
      {
        ZSwcSubtreeAnalyzer analyzer;
        analyzer.setMinLength(10000.0);
        for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
             iter != treeSet.end(); ++iter) {
          ZSwcTree *tree = *iter;
          analyzer.decompose(tree);
          tree->setTypeByLabel();
        }
      }
      default:
        break;
      }

      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::changeSelectedSwcSize()
{
  std::set<ZSwcTree*> treeSet =
      m_doc->getSelectedObjectSet<ZSwcTree>(ZStackObject::EType::SWC);

  if (!treeSet.empty()) {
    SwcSizeDialog dlg(NULL);
    if (dlg.exec()) {
      for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
           iter != treeSet.end(); ++iter) {
        (*iter)->changeRadius(dlg.getAddValue(), dlg.getMulValue());
      }
      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::changeSelectedSwcNodeSize()
{
  std::set<Swc_Tree_Node*> nodeSet = m_doc->getSelectedSwcNodeSet();

  if (!nodeSet.empty()) {
    SwcSizeDialog dlg(NULL);
    if (dlg.exec()) {
      for (std::set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
           iter != nodeSet.end(); ++iter) {
        SwcTreeNode::changeRadius(*iter, dlg.getAddValue(), dlg.getMulValue());
      }

      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::transformSelectedPuncta()
{
  std::set<ZPunctum*> punctaSet =
      m_doc->getSelectedObjectSet<ZPunctum>(ZStackObject::EType::PUNCTUM);
  if (!punctaSet.empty()) {
    SwcSkeletonTransformDialog dlg(NULL);
    dlg.setWindowTitle("Transform Puncta");
    if (dlg.exec()) {
      double dx = dlg.getTranslateValue(SwcSkeletonTransformDialog::X);
      double dy = dlg.getTranslateValue(SwcSkeletonTransformDialog::Y);
      double dz = dlg.getTranslateValue(SwcSkeletonTransformDialog::Z);

      if (dlg.isTranslateFirst()) {
        for (std::set<ZPunctum*>::iterator iter = punctaSet.begin();
             iter != punctaSet.end(); ++iter) {
          (*iter)->setCenter(
                (*iter)->x() + dx, (*iter)->y() + dy, (*iter)->z() + dz);
        }
      }

      for (std::set<ZPunctum*>::iterator iter = punctaSet.begin();
           iter != punctaSet.end(); ++iter) {
        (*iter)->setCenter(
              (*iter)->x() * dlg.getScaleValue(SwcSkeletonTransformDialog::X),
              (*iter)->y() * dlg.getScaleValue(SwcSkeletonTransformDialog::Y),
              (*iter)->z() * dlg.getScaleValue(SwcSkeletonTransformDialog::Z));
      }

      if (!dlg.isTranslateFirst()) {
        for (std::set<ZPunctum*>::iterator iter = punctaSet.begin();
             iter != punctaSet.end(); ++iter) {
          (*iter)->setCenter(
                (*iter)->x() + dx, (*iter)->y() + dy, (*iter)->z() + dz);
        }
      }
    }
    m_doc->notifyPunctumModified();
  }
}

void Z3DWindow::setUnselectPunctaVisible(bool on)
{
  ZStackDocAccessor::SetObjectVisible(
        m_doc.get(), ZStackObject::EType::PUNCTUM,
        [](const ZStackObject *obj) {
          return !obj->isSelected();
        }, on
  );
  /*
  m_doc->setVisible(ZStackObject::EType::PUNCTUM, on);
  std::set<ZPunctum*> punctaSet =
      m_doc->getSelectedObjectSet<ZPunctum>(ZStackObject::EType::PUNCTUM);
  if (!punctaSet.empty()) {
    for (std::set<ZPunctum*>::iterator iter = punctaSet.begin();
         iter != punctaSet.end(); ++iter) {
      ZPunctum *punctum = *iter;
      punctum->setVisible(!on);
      m_doc->bufferObjectModified(
            punctum, ZStackObjectInfo::STATE_VISIBITLITY_CHANGED);
    }
    m_doc->processObjectModified();
  }
  */
}

void Z3DWindow::setSelectPunctaVisible(bool on)
{
  ZStackDocAccessor::SetObjectVisible(
        m_doc.get(), ZStackObject::EType::PUNCTUM,
        [](const ZStackObject *obj) {
          return obj->isSelected();
        }, on
  );
  /*
  std::set<ZPunctum*> punctaSet =
      m_doc->getSelectedObjectSet<ZPunctum>(ZStackObject::EType::PUNCTUM);
  if (!punctaSet.empty()) {
    for (std::set<ZPunctum*>::iterator iter = punctaSet.begin();
         iter != punctaSet.end(); ++iter) {
      ZPunctum *punctum = *iter;
      punctum->setVisible(on);
      m_doc->bufferObjectModified(
            punctum, ZStackObjectInfo::STATE_VISIBITLITY_CHANGED);
    }
    m_doc->processObjectModified();
  }*/
}

void Z3DWindow::hideSelectedPuncta()
{
  setSelectPunctaVisible(false);
}

void Z3DWindow::hideUnselectedPuncta()
{
  setUnselectPunctaVisible(false);
}

void Z3DWindow::showUnselectedPuncta()
{
  setUnselectPunctaVisible(true);
}

void Z3DWindow::showSelectedPuncta()
{
  setSelectPunctaVisible(true);
}

void Z3DWindow::changeSelectedPunctaColor()
{
  std::set<ZPunctum*> punctaSet =
      m_doc->getSelectedObjectSet<ZPunctum>(ZStackObject::EType::PUNCTUM);
  if (!punctaSet.empty()) {
    QColorDialog dlg;

    if (dlg.exec()) {
      for (std::set<ZPunctum*>::iterator iter = punctaSet.begin();
           iter != punctaSet.end(); ++iter) {
        ZPunctum *punctum = *iter;
        punctum->setColor(dlg.currentColor());
        m_doc->bufferObjectModified(punctum);
      }
      m_doc->processObjectModified();

//      m_doc->notifyPunctumModified();
    }
  }
}

void Z3DWindow::addPunctaSelection()
{
  ZFlyEmBody3dDoc *doc = getDocument<ZFlyEmBody3dDoc>();
  if (doc != NULL) {
    QString filterString = QInputDialog::getText(
          this, "Add Puncta Selection", "Condition");
    if (!filterString.isEmpty()) {
      doc->addSynapseSelection(filterString);
    }
  }
}

void Z3DWindow::transformAllPuncta()
{
  QList<ZPunctum*> punctaSet = m_doc->getPunctumList();
  if (!punctaSet.empty()) {
    SwcSkeletonTransformDialog dlg(NULL);
    dlg.setWindowTitle("Transform Puncta");
    if (dlg.exec()) {
      double dx = dlg.getTranslateValue(SwcSkeletonTransformDialog::X);
      double dy = dlg.getTranslateValue(SwcSkeletonTransformDialog::Y);
      double dz = dlg.getTranslateValue(SwcSkeletonTransformDialog::Z);

      if (dlg.isTranslateFirst()) {
        for (QList<ZPunctum*>::iterator iter = punctaSet.begin();
             iter != punctaSet.end(); ++iter) {
          ZPunctum *punctum = *iter;
          punctum->translate(dx, dy, dz);
        }
      }

      for (QList<ZPunctum*>::iterator iter = punctaSet.begin();
           iter != punctaSet.end(); ++iter) {
        (*iter)->setCenter(
              (*iter)->x() * dlg.getScaleValue(SwcSkeletonTransformDialog::X),
              (*iter)->y() * dlg.getScaleValue(SwcSkeletonTransformDialog::Y),
              (*iter)->z() * dlg.getScaleValue(SwcSkeletonTransformDialog::Z));
      }

      if (!dlg.isTranslateFirst()) {
        for (QList<ZPunctum*>::iterator iter = punctaSet.begin();
             iter != punctaSet.end(); ++iter) {
          ZPunctum *punctum = *iter;
          punctum->translate(dx, dy, dz);
        }
      }
      m_doc->notifyPunctumModified();
    }
  }
}

void Z3DWindow::convertPunctaToSwc()
{
  QList<ZPunctum*> punctaSet = m_doc->getPunctumList();
  if (!punctaSet.empty()) {
    ZSwcTree *tree = new ZSwcTree();
    for (QList<ZPunctum*>::iterator iter = punctaSet.begin();
         iter != punctaSet.end(); ++iter) {
      ZPoint pos((*iter)->x(), (*iter)->y(), (*iter)->z());
      Swc_Tree_Node *tn = SwcTreeNode::MakePointer(pos, (*iter)->radius());
      tree->addRegularRoot(tn);
    }

    m_doc->beginObjectModifiedMode(ZStackDoc::EObjectModifiedMode::CACHE);
    m_doc->addObject(tree, false);
    m_doc->removeSelectedPuncta();
    m_doc->endObjectModifiedMode();
    m_doc->processObjectModified();

//    m_doc->notifyPunctumModified();
//    m_doc->notifySwcModified();
  }
}

void Z3DWindow::transformSelectedSwc()
{
  std::set<ZSwcTree*> treeSet =
      m_doc->getSelectedObjectSet<ZSwcTree>(ZStackObject::EType::SWC);

  if (!treeSet.empty()) {
    SwcSkeletonTransformDialog dlg(NULL);
    if (SwcTreeNode::clipboard().size() >= 2) {
      Swc_Tree_Node node[2];
      for (size_t i = 0; i < 2; ++i) {
        SwcTreeNode::paste(node + i, i);
      }

      ZPoint offset = SwcTreeNode::center(node + 1) - SwcTreeNode::center(node);
      dlg.setTranslateValue(offset.x(), offset.y(), offset.z());
    } else {
      ZIntPoint offset = getDocument()->getStackOffset();
      dlg.setTranslateValue(-offset[0], -offset[1], -offset[2]);
    }
    if (dlg.exec()) {
      for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
           iter != treeSet.end(); ++iter) {
        if (dlg.isTranslateFirst()) {
          (*iter)->translate(dlg.getTranslateValue(SwcSkeletonTransformDialog::X),
                             dlg.getTranslateValue(SwcSkeletonTransformDialog::Y),
                             dlg.getTranslateValue(SwcSkeletonTransformDialog::Z));
        }

        (*iter)->scale(dlg.getScaleValue(SwcSkeletonTransformDialog::X),
                       dlg.getScaleValue(SwcSkeletonTransformDialog::Y),
                       dlg.getScaleValue(SwcSkeletonTransformDialog::Z));

        if (!dlg.isTranslateFirst()) {
          (*iter)->translate(dlg.getTranslateValue(SwcSkeletonTransformDialog::X),
                             dlg.getTranslateValue(SwcSkeletonTransformDialog::Y),
                             dlg.getTranslateValue(SwcSkeletonTransformDialog::Z));
        }
      }
      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::groupSelectedSwc()
{
  m_doc->executeGroupSwcCommand();
}

void Z3DWindow::showPuncta(bool on)
{
  m_view->setLayerVisible(neutu3d::ERendererLayer::PUNCTA, on);

  emit showingPuncta(on);
}

void Z3DWindow::showTodo(bool on)
{
  m_view->setLayerVisible(neutu3d::ERendererLayer::TODO, on);
  emit showingTodo(on);
}

void Z3DWindow::activateTodoAction(bool on)
{
  if (on) {
    getCanvas()->getInteractionEngine()->enterMarkTodo();
  } else {
    getCanvas()->getInteractionEngine()->exitMarkTodo();
  }
}

/*
void Z3DWindow::activateTosplitAction(bool on)
{
  activateTodoAction(on);
}
*/

void Z3DWindow::activateBookmarkAction(bool on)
{
  if (on) {
    getCanvas()->getInteractionEngine()->enterMarkBookmark();
  } else {
    getCanvas()->getInteractionEngine()->exitMarkBookmark();
  }
}

void Z3DWindow::activateLocateAction(bool on)
{
  if (on) {
    getCanvas()->getInteractionEngine()->enterLocateMode();
  } else {
    getCanvas()->getInteractionEngine()->exitLocateMode();
  }
}

void Z3DWindow::showSeletedSwcNodeDist()
{
  getDocument()->showSeletedSwcNodeDist();
}

void Z3DWindow::showSeletedSwcNodeLength()
{
  std::set<Swc_Tree_Node*> nodeSet = m_doc->getSelectedSwcNodeSet();
  double length = SwcTreeNode::segmentLength(nodeSet);

  InformationDialog dlg;

  std::ostringstream textStream;

  textStream << "<p>Overall length of selected branches: " << length << "</p>";

  if (nodeSet.size() == 2) {
    std::set<Swc_Tree_Node*>::const_iterator iter = nodeSet.begin();
    Swc_Tree_Node *tn1 = *iter;
    ++iter;
    Swc_Tree_Node *tn2 = *iter;
    textStream << "<p>Straight line distance between the two selected nodes: "
               << SwcTreeNode::distance(tn1, tn2) << "</p>";
  }

  dlg.setText(textStream.str());
  dlg.exec();
}

void Z3DWindow::showSelectedSwcInfo()
{
  std::set<ZSwcTree*> treeSet =
      m_doc->getSelectedObjectSet<ZSwcTree>(ZStackObject::EType::SWC);

  InformationDialog dlg;

  std::ostringstream textStream;

  if (!treeSet.empty()) {
    for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
         iter != treeSet.end(); ++iter) {
      textStream << "<p><font color=\"blue\">" + (*iter)->getSource() + "</font></p>";
      textStream << "<p>Overall length: " << (*iter)->length() << "</p>";
      std::set<int> typeList = (*iter)->typeSet();
      if (typeList.size() > 1) {
        textStream << "<p>Typed branch length: ";
        textStream << "<ul>";
        for (std::set<int>::const_iterator typeIter = typeList.begin();
             typeIter != typeList.end(); ++typeIter) {
          textStream << "<li>Type " << *typeIter << ": " << (*iter)->length(*typeIter)
                    << "</li>";
        }
        textStream << "</ul>";
        textStream << "</p>";
      }
      textStream << "<p>Lateral-vertical ratio: "
                 << ZSwcGlobalFeatureAnalyzer::computeLateralVerticalRatio(*(*iter))
                 << "</p>";
    }
  }

  dlg.setText(textStream.str());
  dlg.exec();
}

void Z3DWindow::refreshTraceMask()
{
  getDocument()->refreshTraceMask();
}

void Z3DWindow::changeSelectedSwcColor()
{
  std::set<ZSwcTree*> treeSet =
      m_doc->getSelectedObjectSet<ZSwcTree>(ZStackObject::EType::SWC);
  if (!treeSet.empty()) {
    QColorDialog dlg;
    dlg.setCurrentColor((*treeSet.begin())->getColor());

    if (dlg.exec()) {
      for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
           iter != treeSet.end(); ++iter) {
        (*iter)->setColor(dlg.currentColor().red(),
                          dlg.currentColor().green(),
                          dlg.currentColor().blue());
      }
      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::changeSelectedSwcAlpha()
{
  std::set<ZSwcTree*> treeSet =
      m_doc->getSelectedObjectSet<ZSwcTree>(ZStackObject::EType::SWC);
  if (!treeSet.empty()) {
    ZAlphaDialog dlg;
    if (dlg.exec()) {
      int alpha = dlg.getAlpha();
      for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
           iter != treeSet.end(); ++iter) {
        (*iter)->setAlpha(alpha);
      }
      m_doc->notifySwcModified();
    }
  }
}

void Z3DWindow::notifyCameraRotation()
{
  emit cameraRotated();
}



void Z3DWindow::startBodySplit()
{
  ZFlyEmBody3dDoc *doc = getDocument<ZFlyEmBody3dDoc>();
  if (doc != NULL) {
    if (getBodyEnv()) {
      if (getBodyEnv()->cleaving()) {
        doc->notify(
              ZWidgetMessageFactory(
                "WARNING: You will not be able to undo previous cleaving after splitting.").
              as(neutu::EMessageType::WARNING));
      }
    }

    doc->activateSplitForSelected();
  }
}

void Z3DWindow::test()
{

#if 0
  const NeutubeConfig &config = NeutubeConfig::getInstance();

  UNUSED_PARAMETER(&config);

  ZMovieMaker director;
  ZMovieScript script;
#endif

  emit testing();

  /*
  std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();
  treeSet->clear();
  m_doc->requestRedrawSwc();

  QList<ZSwcTree*> *treeList =  m_doc->swcList();
  std::vector<ZMovieActor*> cast(treeList->size());
  std::vector<ZSwcMovieActor> swcCast(treeList->size());

  int index = 0;
  for (QList<ZSwcTree*>::iterator iter = treeList->begin();
       iter != treeList->end(); ++iter, ++index) {
    swcCast[index].setActor(*iter);
    swcCast[index].setMovingOffset((index + 1) * 10, 0, 0);
    cast[index] = &(swcCast[index]);
  }
*/
/*
  ZMovieScene scene(m_doc.get(), this);

  for (index = 0; index < 5; ++index) {
    std::ostringstream stream;
    stream << config.getPath(NeutubeConfig::DATA) + "/test/";
    stream << std::setw(3) << std::setfill('0') << index << ".tif";

    for (std::vector<ZMovieActor*>::iterator iter = cast.begin();
         iter != cast.end(); ++iter) {
      (*iter)->perform();
    }
    scene.saveToImage(stream.str(), 1024, 1024);
  }
*/
  /*
  for (QList<ZSwcTree*>::iterator iter = treeList->begin();
       iter != treeList->end(); ++iter, ++index) {

    scene.saveToImage(stream.str(), 1024, 1024);
    //takeScreenShot(stream.str().c_str(),
    //               1024, 1024, MonoView);
    (*iter)->setVisible(false);
    getInteractionHandler()->getTrackball()->rotate(glm::vec3(0,1,0), 0.5);
    getInteractionHandler()->getTrackball()->zoom(1.1);
  }
*/

  /*
  std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();

  for (std::set<ZSwcTree*>::iterator iter = treeSet->begin();
       iter != treeSet->end(); ++iter) {
    (*iter)->setVisible(false);
  }
  treeSet->clear();
  */


}

void Z3DWindow::viewDetail(bool on)
{
  if (on) {
    getCanvas()->getInteractionEngine()->enterDetailMode();
  } else {
    getCanvas()->getInteractionEngine()->exitDetailMode();
  }
}

void Z3DWindow::viewDataExternally(bool on)
{
#if 0
  ZBrowserOpener *bo = ZGlobal::GetInstance().getBrowserOpener();
  bo->updateChromeBrowser();
  if (bo->getBrowserPath().isEmpty()) {
    ZDialogFactory::Warn(
          "Chrome Not Found",
          "The path to Google Chrome has not be set correctly. "
          "The default browser will be used.", this);
  }
#endif
  if (on) {
    getCanvas()->getInteractionEngine()->enterBrowseMode();
  } else {
    getCanvas()->getInteractionEngine()->exitBrowseMode();
  }
}

void Z3DWindow::breakSelectedSwc()
{
  m_doc->executeBreakForestCommand();
  //Need to change to m_doc->executeBreakSwcCommand();
#if 0
  std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();

  if (!treeSet->empty()) {
    for (std::set<ZSwcTree*>::iterator iter = treeSet->begin();
         iter != treeSet->end(); ++iter) {
      Swc_Tree_Node *root = (*iter)->firstRegularRoot();
      if (root != NULL) {
        root = SwcTreeNode::nextSibling(root);
        while (root != NULL) {
          Swc_Tree_Node *sibling = SwcTreeNode::nextSibling(root);
          SwcTreeNode::detachParent(root);
          ZSwcTree *tree = new ZSwcTree;
          tree->setDataFromNode(root);
          /*
          std::ostringstream stream;
          stream << (*iter)->source().c_str() << "-" << m_doc->swcList()->size() + 1;
          tree->setSource(stream.str());
          */
          m_doc->addSwcTree(tree, false);
          root = sibling;
        }
      }
    }
    m_doc->notifySwcModified();
  }
#endif

}

void Z3DWindow::saveSelectedSwc()
{
  std::set<ZSwcTree*> treeSet =
      m_doc->getSelectedObjectSet<ZSwcTree>(ZStackObject::EType::SWC);

  QString fileName = "";

  if (!treeSet.empty()) {
    if (!(*treeSet.begin())->getSource().empty()) {
      if ((*treeSet.begin())->getSource()[0] != '#') {
        fileName = QString((*treeSet.begin())->getSource().c_str());
      }
    }

    if (fileName.isEmpty()) {
      ZString stackSource = m_doc->stackSourcePath();
      if (!stackSource.empty()) {
        fileName = stackSource.changeExt("Edit.swc").c_str();
      }
    }

    if (fileName.isEmpty()) {
      fileName = "untitled.swc";
    }

    if (GET_APPLICATION_NAME == "Biocytin") {
      ZStackFrame *frame = m_doc->getParentFrame();
      if (frame != NULL) {
        fileName = m_doc->getParentFrame()->swcFilename;
      }
      //fileName =ZBiocytinFileNameParser::getSwcEditPath(fileName.toStdString()).c_str();
    }

    fileName =
        QFileDialog::getSaveFileName(this, tr("Save SWC"), fileName,
                                     tr("SWC File"), nullptr);

    if (!fileName.isEmpty()) {
      if (!fileName.endsWith(".swc", Qt::CaseInsensitive)) {
        fileName += ".swc";
      }

      if (treeSet.size() > 1) {
        ZSwcTree tree;

        for (std::set<ZSwcTree*>::iterator iter = treeSet.begin();
             iter != treeSet.end(); ++iter) {
          tree.merge((*iter)->cloneData(), true);
        }

        tree.resortId();
        tree.save(fileName.toStdString().c_str());
      } else {
        ZSwcTree *tree = *(treeSet.begin());
        tree->resortId();
        tree->save(fileName.toStdString().c_str());
        tree->setSource(fileName.toStdString().c_str());
        getDocument()->notifySwcModified();
      }
    } //!fileName.isEmpty()
  } //!treeSet.empty()
}

void Z3DWindow::convertSelectedChainToSwc()
{
  std::set<ZLocsegChain*> chainSet =
      m_doc->getSelectedObjectSet<ZLocsegChain>(ZStackObject::EType::LOCSEG_CHAIN);

  m_doc->beginObjectModifiedMode(ZStackDoc::EObjectModifiedMode::CACHE);
  for (std::set<ZLocsegChain*>::iterator iter = chainSet.begin();
       iter != chainSet.end(); ++iter) {
    Swc_Tree_Node *tn = TubeModel::createSwc((*iter)->data());
    if (tn != NULL) {
      ZSwcTree *tree = new ZSwcTree;
      tree->setDataFromNode(tn);
      m_doc->addObject(tree, false);
    }
  }
  //chainSet->clear();

  m_doc->executeRemoveTubeCommand();
  m_doc->endObjectModifiedMode();

  m_doc->processObjectModified();

//  m_doc->notifySwcModified();
//  m_doc->notifyChainModified();
}

bool Z3DWindow::hasSwc() const
{
  return m_doc->hasSwc();
}

bool Z3DWindow::hasSelectedSwc() const
{
  return m_doc->hasSelectedSwc();
}

bool Z3DWindow::hasSelectedSwcNode() const
{
  return !m_doc->hasSelectedSwcNode();
}

bool Z3DWindow::hasMultipleSelectedSwcNode() const
{
  return m_doc->hasMultipleSelectedSwcNode();
}

void Z3DWindow::notifyUser(const QString &message)
{
  statusBar()->showMessage(message);
}
#if 0
void Z3DWindow::addStrokeFrom3dPaint(ZStroke2d *stroke)
{
  bool success;


  QList<ZStroke2d*> strokeList;

  for (size_t i = 0; i < stroke->getPointNumber(); ++i) {
    double x = 0;
    double y = 0;
    stroke->getPoint(&x, &y, i);
    glm::vec3 fpos = m_volumeRaycaster->get3DPosition(
          x, y, m_canvas->width(), m_canvas->height(), success);

    /*
    ZLineSegment seg = m_volumeRaycaster->getScreenRay(
          x, y, m_canvas->width(), m_canvas->height(), success);
*/

    if (success) {
      //ZObject3d *obj = ZVoxelGraphics::createLineObject(seg);

#ifdef _DEBUG_2
      std::cout << fpos << std::endl;
#endif
      //for (size_t i = 0; i < obj->size(); ++i) {
        ZStroke2d *strokeData = new ZStroke2d;
        strokeData->setWidth(stroke->getWidth());
        strokeData->setLabel(stroke->getLabel());

        strokeData->append(fpos[0], fpos[1]);
        strokeData->setZ(iround(fpos[2]));
        //strokeData->append(obj->x(i), obj->y(i));
        //strokeData->setZ(obj->z(i));

        if (!strokeData->isEmpty()) {
          strokeList.append(strokeData);
        } else {
          delete strokeData;
        }
      //}

      //delete obj;
    }
  }

  m_doc->executeAddStrokeCommand(strokeList);
}
#endif

void Z3DWindow::addStrokeFrom3dPaint(ZStroke2d *stroke)
{
  //bool success = false;

  ZObject3d *baseObj = stroke->toObject3d();

  double x = 0.0;
  double y = 0.0;

  stroke->getPoint(&x, &y, stroke->getPointNumber() / 2);
  /*
  ZLineSegment seg = m_volumeRaycaster->getScreenRay(
        iround(x), iround(y), m_canvas->width(), m_canvas->height());
*/
  ZObject3d *obj = new ZObject3d;
  for (size_t i = 0; i < baseObj->size(); ++i) {
    ZLineSegment seg = getVolumeFilter()->getScreenRay(
          baseObj->getX(i), baseObj->getY(i),
          getCanvas()->width(), getCanvas()->height());
    ZPoint slope = seg.getEndPoint() - seg.getStartPoint();
    //if (success) {
//      ZIntCuboid box = m_doc->stackRef()->getBoundBox();
//      ZIntCuboid box = m_volumeBoundBox;
#if 0
      ZCuboid rbox(box.getFirstCorner().getX(), box.getFirstCorner().getY(),
                   box.getFirstCorner().getZ(),
                   box.getLastCorner().getX(), box.getLastCorner().getY(),
                   box.getLastCorner().getZ());
#endif
      const auto& volumeBound = getVolumeFilter()->axisAlignedBoundBox();
      ZCuboid rbox;
      rbox.setFirstCorner(volumeBound.minCorner().x, volumeBound.minCorner().y, volumeBound.minCorner().z);
      rbox.setLastCorner(volumeBound.maxCorner().x, volumeBound.maxCorner().y, volumeBound.maxCorner().z);

      if (getVolumeFilter()->isSubvolume()) {
        const auto& zoomInBound = getVolumeFilter()->zoomInBound();
        rbox.setFirstCorner(zoomInBound.minCorner().x, zoomInBound.minCorner().y, zoomInBound.minCorner().z);
        rbox.setLastCorner(zoomInBound.maxCorner().x, zoomInBound.maxCorner().y, zoomInBound.maxCorner().z);
      }

      ZLineSegment stackSeg;
      if (rbox.intersectLine(seg.getStartPoint(), slope, &stackSeg)) {
        ZObject3d *scanLine = ZVoxelGraphics::createLineObject(stackSeg);
        obj->append(*scanLine);
#ifdef _DEBUG_2
        scanLine->print();
        obj->print();
        break;
#endif
        delete scanLine;
      }
    //}
  }
  obj->setLabel(stroke->getLabel());
  ZLabelColorTable colorTable;
  obj->setColor(colorTable.getColor(obj->getLabel()));

  delete baseObj;

  if (!obj->isEmpty()) {
    obj->setRole(ZStackObjectRole::ROLE_SEED);
    m_doc->executeAddObjectCommand(obj);
  } else {
    delete obj;
  }
}

void Z3DWindow::selectSwcNodeFromStroke(const ZStroke2d *stroke)
{
  if (hasSwc() && stroke != NULL) {
    ZObject3d *ptArray = stroke->toObject3d();
    if (ptArray != NULL) {
      getSwcFilter()->selectSwcNode(*ptArray);
    }
  }
}

void Z3DWindow::checkSelectedTodo()
{
  ZFlyEmBody3dDoc *doc = getDocument<ZFlyEmBody3dDoc>();
  if (doc != NULL) {
    doc->checkSelectedTodoItem();
  }
}

void Z3DWindow::uncheckSelectedTodo()
{
  ZFlyEmBody3dDoc *doc = getDocument<ZFlyEmBody3dDoc>();
  if (doc != NULL) {
    doc->uncheckSelectedTodoItem();
  }
}

void Z3DWindow::processMessage(const ZWidgetMessage &msg)
{
  if (msg.hasTarget(ZWidgetMessage::TARGET_CUSTOM_AREA)) {
    m_view->dump(msg.toPlainString());
  }

  if (msg.hasTarget(ZWidgetMessage::TARGET_DIALOG)) {
    ZDialogFactory::PromptMessage(msg, this);
  }

  neutu::LogMessage(msg);

#if 0
  if (msg.getTarget() == ZWidgetMessage::TARGET_CUSTOM_AREA) {
    m_view->dump(msg.toPlainString());
  } else if (msg.getTarget() == ZWidgetMessage::TARGET_DIALOG) {
    ZDialogFactory::PromptMessage(msg, this);
  }/* else {
    emit messageGenerated(msg);
  }*/
#endif
}

void Z3DWindow::setMeshOpacity(double opacity)
{
  if (m_meshOpacitySpinBox != NULL) {
    if (opacity != m_meshOpacitySpinBox->value()) {
      m_meshOpacitySpinBox->blockSignals(true);
      m_meshOpacitySpinBox->setValue(opacity);
      m_meshOpacitySpinBox->blockSignals(false);
    }
  }

  if (getMeshFilter() != NULL) {
    if (opacity != getMeshFilter()->opacity()) {
      getMeshFilter()->setOpacityQuitely(opacity);
    }
  }
}

void Z3DWindow::locateWithRay(int x, int y)
{
  std::vector<ZPoint> intersection = getRayIntersection(x, y);
  if (!intersection.empty()) {
    ZPoint &pt = intersection.front();
    m_view->gotoPosition(pt.x(), pt.y(), pt.z());
  }
}

void Z3DWindow::browseWithRay(int x, int y)
{
  uint64_t bodyId = 0;

  std::vector<ZPoint> intersection = getRayIntersection(x, y, &bodyId);
  if (!intersection.empty()) {
    ZPoint &pt = intersection.front();
    if (intersection.size() > 1) {
      pt += intersection[1];
      pt *= 0.5;
    }

    emit messageGenerated(
          ZWidgetMessage(
            QString("Checking (%1, %2, %3)").
            arg(iround(pt.x())).arg(iround(pt.y())).arg(iround(pt.z()))));
#if defined(_NEU3_)
    emit browsing(pt.x(), pt.y(), pt.z());
#else
    locate2DView(pt, 300);
#endif
  }
}

void Z3DWindow::showDetail(int x, int y)
{
  ZFlyEmBody3dDoc *doc = getDocument<ZFlyEmBody3dDoc>();
  if (doc != NULL) {
    uint64_t bodyId = 0;
    std::vector<ZPoint> intersection = getRayIntersection(x, y, &bodyId);
    bodyId = doc->getMappedId(bodyId);
    if (!intersection.empty()) {
      ZPoint center = intersection[0];
      if (intersection.size() > 1) {
        center += intersection[1];
        center *= 0.5;
      }

      ZIntCuboid range = zgeom::MakeSphereBox(center.toIntPoint(), 256);
      doc->showMoreDetail(bodyId, range);
    }
  }
}


void Z3DWindow::shootTodo(int x, int y)
{
  ZFlyEmBody3dDoc *doc = getDocument<ZFlyEmBody3dDoc>();
  if (doc != NULL) {
    uint64_t bodyId = 0;
    std::vector<ZPoint> intersection = getRayIntersection(x, y, &bodyId);
    uint64_t parentId = doc->getMappedId(bodyId);
    if (parentId > 0) {
      if (!intersection.empty()) {
        ZPoint &pt = intersection.front();
        int cx = iround(pt.x());
        int cy = iround(pt.y());
        int cz = iround(pt.z());
        doc->executeAddTodoCommand(cx, cy, cz, false, parentId);
      }
    }
  }
}

void Z3DWindow::addTodoMarkerFromStroke(const ZStroke2d *stroke)
{
  if (hasSwc() && stroke != NULL) {
    getSwcFilter()->forceNodePicking(true);
    getSwcFilter()->invalidate();
    //m_view->updateNetwork();
    int x = 0;
    int y = 0;
    stroke->getLastPoint(&x, &y);
    shootTodo(x, y);
  }
}

void Z3DWindow::labelSwcNodeFromStroke(const ZStroke2d *stroke)
{
  if (hasSwc() && stroke != NULL) {
    getSwcFilter()->forceNodePicking(true);
    getSwcFilter()->invalidate();
    //m_view->updateNetwork();
    ZObject3d *ptArray = stroke->toObject3d();
    if (ptArray != NULL) {
      QList<Swc_Tree_Node*> nodeArray = getSwcFilter()->pickSwcNode(*ptArray);
      getDocument()->executeChangeSwcNodeType(nodeArray, stroke->getLabel());
      /*
      foreach (Swc_Tree_Node *node, nodeArray) {
        SwcTreeNode::setType(node, stroke->getLabel());
      }
      */
    }
    getSwcFilter()->forceNodePicking(false);
//    m_doc->notifySwcModified();
  }
}

void Z3DWindow::processStroke(ZStroke2d *stroke)
{
#ifdef _DEBUG_2
  addTodoMarkerFromStroke(stroke);
#else
//  stroke->decimate();
//  labelSwcNodeFromStroke(stroke);
  addPolyplaneFrom3dPaint(stroke);
  delete stroke;
#endif
}

ZLineSegment Z3DWindow::getStackSeg(
    const ZLineSegment &seg, const ZCuboid &rbox) const
{
  ZLineSegment stackSeg;

  if (seg.isValid()) {
    ZPoint slope = seg.getEndPoint() - seg.getStartPoint();
    if (rbox.intersectLine(seg.getStartPoint(), slope, &stackSeg)) {
      ZPoint slope2 = stackSeg.getEndPoint() - stackSeg.getStartPoint();
      if (slope.dot(slope2) < 0.0) {
        stackSeg.invert();
      }
      const ZPoint &start = stackSeg.getStartPoint();
      const ZPoint &end = stackSeg.getEndPoint();
      if (end.distanceTo(start) > end.distanceTo(seg.getStartPoint())) {
        stackSeg.setStartPoint(seg.getStartPoint());
      }
    }
  }

  return stackSeg;
}

std::vector<ZPoint> Z3DWindow::shootMesh(const ZMesh *mesh, int x, int y)
{
  std::vector<ZPoint> intersection;
  if (mesh != NULL) {
    glm::dvec3 v1,v2;
    int w = getCanvas()->width();
    int h = getCanvas()->height();
    getMeshFilter()->rayUnderScreenPoint(v1, v2, x, y, w, h);
#ifdef _DEBUG_
    std::cout << "Segment start: " << v1.x << " " << v1.y << " " << v1.z << std::endl;
    std::cout << "Segment end: " << v2.x << " " << v2.y << " " << v2.z << std::endl;
#endif
    const ZBBox<glm::dvec3> &boundBox = m_view->boundBox();
    ZCuboid rbox;
    rbox.setFirstCorner(
          boundBox.minCorner().x, boundBox.minCorner().y, boundBox.minCorner().z);
    rbox.setLastCorner(
          boundBox.maxCorner().x, boundBox.maxCorner().y, boundBox.maxCorner().z);

    ZLineSegment seg(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z);
    ZLineSegment stackSeg = getStackSeg(seg, rbox);
/*
    ZLineSegment Z3DWindow::getStackSeg(
        const ZLineSegment &seg, const ZCuboid &rbox) const

    ZPoint slope = seg.getEndPoint() - seg.getStartPoint();
    ZLineSegment stackSeg;
    if (rbox.intersectLine(seg.getStartPoint(), slope, &stackSeg)) {
      ZPoint slope2 = stackSeg.getEndPoint() - stackSeg.getStartPoint();
      if (slope.dot(slope2) < 0.0) {
        stackSeg.invert();
      }
    }
*/
#ifdef _DEBUG_
    std::cout << "Segment start: " << stackSeg.getStartPoint().toString() << std::endl;
    std::cout << "Segment end: " << stackSeg.getEndPoint().toString() << std::endl;
#endif

    intersection = mesh->intersectLineSeg(
          stackSeg.getStartPoint(), stackSeg.getEndPoint());
  }

  return intersection;
}

std::vector<ZPoint> Z3DWindow::getRayIntersection(int x, int y, uint64_t *id)
{
  getCanvas()->getGLFocus();
  std::vector<ZPoint> intersection;
  ZStackDoc *doc = getDocument();

  neutu::assign<uint64_t>(id, 0);

  if (doc != NULL) {
    bool hit = false;
    if (getMeshFilter()) {
      ZMesh *mesh = getMeshFilter()->hitMesh(x, y);
      if (mesh != NULL) {
        intersection = shootMesh(mesh, x, y);
        if (!intersection.empty()) {
          neutu::assign(id, mesh->getLabel());
          hit = true;
        }
      }
    }

    if (!hit) {
      if (hasSwc()) {
        getSwcFilter()->forceNodePicking(true);
        getSwcFilter()->invalidate();
        //m_view->updateNetwork();
        Swc_Tree_Node *tn = getSwcFilter()->pickSwcNode(x, y);
        if (tn != NULL) {
          ZSwcTree *tree = getDocument()->nodeToSwcTree(tn);
          if (tree != NULL) {
            neutu::assign(id, tree->getLabel());
            glm::dvec3 v1,v2;
            int w = getCanvas()->width();
            int h = getCanvas()->height();
            getSwcFilter()->rayUnderScreenPoint(v1, v2, x, y, w, h);
            ZPoint lineStart(v1.x, v1.y, v1.z);
            glm::dvec3 norm = v2 - v1;
            ZPoint lineNorm(norm.x, norm.y, norm.z);
            intersection = zgeom::LineShpereIntersection(
                  lineStart, lineNorm, SwcTreeNode::center(tn), SwcTreeNode::radius(tn));
          }
        }
        getSwcFilter()->forceNodePicking(false);
      }
    }
  }

  return intersection;
}

ZCuboid Z3DWindow::getRayBoundbox() const
{
  ZCuboid rbox;

  if (m_doc->hasStack()) {
    const auto& volumeBound = getVolumeFilter()->axisAlignedBoundBox();

    rbox.setFirstCorner(volumeBound.minCorner().x, volumeBound.minCorner().y, volumeBound.minCorner().z);
    rbox.setLastCorner(volumeBound.maxCorner().x, volumeBound.maxCorner().y, volumeBound.maxCorner().z);

    if (getVolumeFilter()->isSubvolume()) {
      const auto& zoomInBound = getVolumeFilter()->zoomInBound();
      rbox.setFirstCorner(zoomInBound.minCorner().x, zoomInBound.minCorner().y, zoomInBound.minCorner().z);
      rbox.setLastCorner(zoomInBound.maxCorner().x, zoomInBound.maxCorner().y, zoomInBound.maxCorner().z);
    } else {
      ZIntCuboid cutBox = getVolumeFilter()->cutBox();
      //      cutBox.translate(m_doc->getStackOffset());
      rbox = misc::CutBox(rbox, cutBox);
    }
  } else {
    const ZBBox<glm::dvec3> &boundBox = m_view->boundBox();
    rbox.setFirstCorner(
          boundBox.minCorner().x, boundBox.minCorner().y, boundBox.minCorner().z);
    rbox.setLastCorner(
          boundBox.maxCorner().x, boundBox.maxCorner().y, boundBox.maxCorner().z);
  }

  return rbox;
}

ZLineSegment Z3DWindow::getRaySegment(int x, int y, std::string &source) const
{
  ZLineSegment stackSeg;

  int w = getCanvas()->width();
  int h = getCanvas()->height();
  ZCuboid rbox = getRayBoundbox();

  if (m_doc->hasStack()) {
    ZLineSegment seg = getVolumeFilter()->getScreenRay(
          iround(x), iround(y), w, h);
    stackSeg = getStackSeg(seg, rbox);
    source = "";
  } else if (m_doc->hasMesh()){
//    QList<ZMesh*> meshList = m_doc->getMeshList();
//    ZMesh *mesh = meshList.front();

    ZMesh *mesh = ZFlyEmBody3dDocHelper::GetMeshForSplit(m_doc.get());

    if (mesh != NULL) {
#if defined(_NEU3_)
      uint64_t bodyId =
          ZStackObjectSourceFactory::ExtractIdFromFlyEmBodySource(mesh->getSource());
      if (bodyId > 0) {
        source = ZStackObjectSourceFactory::MakeFlyEmSeedSource(bodyId);
      }
#endif

      ZLineSegment seg = getMeshFilter()->getScreenRay(
            iround(x), iround(y), w, h);
      stackSeg = getStackSeg(seg, rbox);

      if (stackSeg.isValid()) {
        std::vector<ZPoint> ptArray = mesh->intersectLineSeg(
              stackSeg.getStartPoint(), stackSeg.getEndPoint());
        if (ptArray.size() >= 2) {
          stackSeg.setStartPoint(ptArray[0]);
          stackSeg.setEndPoint(ptArray[1]);
          //          ZVoxelGraphics::addLineObject(
          //                processedObj, ptArray[0].toIntPoint(), ptArray[1].toIntPoint());
        } else {
          stackSeg.set(ZPoint(0, 0, 0), ZPoint(0, 0, 0));
        }
      }
    }
  }

  return stackSeg;
}

std::string Z3DWindow::updatePolyLinePairList(
    const ZStroke2d *stroke,
    std::vector<std::pair<ZIntPointArrayPtr, ZIntPointArrayPtr> > &polylinePairList)
{
  std::string source;

  ZIntPointArrayPtr polyline1;
  ZIntPointArrayPtr polyline2;
  for (size_t i = 0; i < stroke->getPointNumber(); ++i) {
    double x = 0.0;
    double y = 0.0;
    stroke->getPoint(&x, &y, i);

    ZLineSegment stackSeg = getRaySegment(iround(x), iround(y), source);

    if (stackSeg.isValid()) {
      if (!polyline1) {
        polyline1 = ZIntPointArray::MakePointer();
        polyline2 = ZIntPointArray::MakePointer();
        polylinePairList.emplace_back(polyline1, polyline2);
      }
      polyline1->push_back(ZIntPoint(stackSeg.getStartPoint().toIntPoint()));
      polyline2->push_back(ZIntPoint(stackSeg.getEndPoint().toIntPoint()));
    } else {
      polyline1.reset();
      polyline2.reset();
    }
  }

  return source;
}

ZObject3d *Z3DWindow::createPolyplaneFrom3dPaintForMesh(ZStroke2d *stroke)
{
  ZObject3d *obj = NULL;
  if (m_doc->hasMesh() && getMeshFilter()) {
    std::vector<ZStroke2dPtr> strokeList;
    ZStroke2dPtr subStroke = ZStroke2dPtr(new ZStroke2d);

    std::vector<std::pair<int, int> > ptArray;
    for (size_t i = 0; i < stroke->getPointNumber(); ++i) {
      double x = 0.0;
      double y = 0.0;
      stroke->getPoint(&x, &y, i);

      ptArray.emplace_back(iround(x), iround(y));
    }
    getCanvas()->getGLFocus();
    std::vector<bool> hitTest = getMeshFilter()->hitObject(ptArray);

    for (size_t i = 0; i < hitTest.size(); ++i) {
      bool hit = hitTest[i];

      double x = 0.0;
      double y = 0.0;
      stroke->getPoint(&x, &y, i);

      if (hit) {
        if (subStroke->isEmpty()) {
          strokeList.push_back(subStroke);
        }
        subStroke->append(x, y);
      } else {
        if (!subStroke->isEmpty()) {
          subStroke = ZStroke2dPtr(new ZStroke2d);
        }
      }
    }

    std::vector<std::pair<ZIntPointArrayPtr, ZIntPointArrayPtr> > polylinePairList;

    std::string source;
    for (size_t i = 0; i < strokeList.size(); ++i) {
      ZStroke2dPtr subStroke = strokeList[i];
      subStroke->decimate();
      source = updatePolyLinePairList(subStroke.get(), polylinePairList);
    }

    obj = ZVoxelGraphics::createPolylineObject(polylinePairList);
    if (obj != NULL) {
      obj->setSource(source);
    }
  }

  return obj;
}

ZObject3d *Z3DWindow::createPolyplaneFrom3dPaintForVolume(ZStroke2d *stroke)
{
  ZObject3d *obj = NULL;

  if (m_doc->hasStack()) {
    std::vector<std::pair<ZIntPointArrayPtr, ZIntPointArrayPtr> > polylinePairList;
    std::string source = updatePolyLinePairList(stroke, polylinePairList);


    obj = ZVoxelGraphics::createPolylineObject(polylinePairList);

    if (obj != NULL) {
      ZObject3d *processedObj = NULL;

      const ZStack *stack = NULL;
      int xIntv = 0;
      int yIntv = 0;
      int zIntv = 0;

      if (getDocument()->hasSparseStack()) {
        ZStackDocHelper docHelper;
        stack = docHelper.getSparseStack(getDocument());
//        stack = getDocument()->getSparseStack()->getStack();
        ZIntPoint dsIntv = stack->getDsIntv();
        //        ZIntPoint dsIntv = getDocument()->getSparseStack()->getDownsampleInterval();
        xIntv = dsIntv.getX();
        yIntv = dsIntv.getY();
        zIntv = dsIntv.getZ();
      } else {
        stack = getDocument()->getStack();
      }

      processedObj = new ZObject3d;
      for (size_t i = 0; i < obj->size(); ++i) {
        int x = obj->getX(i) / (xIntv + 1) - stack->getOffset().getX();
        int y = obj->getY(i) / (yIntv + 1) - stack->getOffset().getY();
        int z = obj->getZ(i) / (zIntv + 1) - stack->getOffset().getZ();
        int v = 0;
        for (int dz = -1; dz <= 1; ++dz) {
          for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
              v = stack->getIntValueLocal(x + dx, y + dy, z + dz);
              if (v > 0) {
                break;
              }
            }
          }
        }
        if (v > 0) {
          processedObj->append(obj->getX(i), obj->getY(i), obj->getZ(i));
        }
      }
      delete obj;
      obj = processedObj;
    }

    if (obj != NULL) {
      obj->setSource(source);
    }
  }

  return obj;
}

void Z3DWindow::addPolyplaneFrom3dPaint(ZStroke2d *stroke)
{
  ZObject3d* obj = NULL;
  if (m_doc->hasStack()) {
    obj = createPolyplaneFrom3dPaintForVolume(stroke);
  } else {
    obj = createPolyplaneFrom3dPaintForMesh(stroke);
  }

  KINFO << "Paint stroke in 3D";

  if (obj != NULL) {
    if (!obj->isEmpty()) {
      obj->setLabel(stroke->getLabel());

      KINFO << QString("Add seed from 3D: %1").arg(obj->getLabel());

      ZLabelColorTable colorTable;
      obj->setColor(colorTable.getColor(obj->getLabel()));
      obj->setRole(ZStackObjectRole::ROLE_SEED |
                   ZStackObjectRole::ROLE_3DGRAPH_DECORATOR);
      m_doc->executeAddObjectCommand(obj, false);
      //m_doc->notifyVolumeModified();
    } else {
      delete obj;
    }
  }
}

void Z3DWindow::help()
{
  m_helpDlg->setSource((GET_CONFIG_DIR + "/doc/shortcut_3d.html").c_str());
  m_helpDlg->show();
  m_helpDlg->raise();
}

void Z3DWindow::diagnose()
{
  getDocument()->diagnose();

  emit diagnosing();
}

void Z3DWindow::markSwcSoma()
{
  ZMarkSwcSomaDialog dlg;
  if (dlg.exec()) {
    QList<ZSwcTree*> trees = m_doc->getSwcList();
    for (int i=0; i<trees.size(); ++i) {
      trees.at(i)->markSoma(dlg.getRadiusThre(), dlg.getSomaType(), dlg.getOtherType());
    }
    m_doc->notifySwcModified();
  }
}

void Z3DWindow::setBackgroundColor(
    const glm::vec3 &color1, const glm::vec3 &color2)
{
  getCompositor()->setBackgroundFirstColor(color1);
  getCompositor()->setBackgroundSecondColor(color2);
}

Z3DWindow* Z3DWindow::Make(
    ZStackDoc* doc, QWidget *parent, Z3DView::EInitMode mode)
{
  return Make(ZSharedPointer<ZStackDoc>(doc), parent, mode);
}

Z3DWindow* Z3DWindow::Open(
    ZStackDoc* doc, QWidget *parent, Z3DView::EInitMode mode)
{
  return Open(ZSharedPointer<ZStackDoc>(doc), parent, mode);
}

Z3DWindow* Z3DWindow::Make(
    ZSharedPointer<ZStackDoc> doc, QWidget *parent, Z3DView::EInitMode mode)
{
  ZWindowFactory factory;
  factory.setParentWidget(parent);
  return factory.make3DWindow(doc, mode);
}

Z3DWindow* Z3DWindow::Open(
    ZSharedPointer<ZStackDoc> doc, QWidget *parent, Z3DView::EInitMode mode)
{
  Z3DWindow *window = Make(doc, parent, mode);
  window->show();
  window->raise();

  return window;
}


Z3DGeometryFilter* Z3DWindow::getFilter(neutu3d::ERendererLayer layer) const
{
  return m_view->getFilter(layer);
}

Z3DBoundedFilter* Z3DWindow::getBoundedFilter(neutu3d::ERendererLayer layer) const
{
  return m_view->getBoundedFilter(layer);
}

void Z3DWindow::updateCuttingBox()
{
  if (m_cuttingStackBound) {
    if (getDocument()->hasStack()) {
      m_view->setCutBox(
            neutu3d::ERendererLayer::SWC, getDocument()->getStack()->getBoundBox());
    }
  } else {
    m_view->resetCutBox(neutu3d::ERendererLayer::SWC);
  }
}

void Z3DWindow::setCutBox(neutu3d::ERendererLayer layer, const ZIntCuboid &box)
{
  m_view->setCutBox(layer, box);
}

void Z3DWindow::resetCutBox(neutu3d::ERendererLayer layer)
{
  m_view->resetCutBox(layer);
}

void Z3DWindow::setZScale(double s)
{
  m_view->setZScale(s);
}

bool Z3DWindow::isLayerVisible(neutu3d::ERendererLayer layer) const
{
  return m_view->isLayerVisible(layer);
}

void Z3DWindow::setLayerVisible(neutu3d::ERendererLayer layer, bool visible)
{
  m_view->setLayerVisible(layer, visible);
}

void Z3DWindow::setOpacity(neutu3d::ERendererLayer layer, double opacity)
{
  if (layer == neutu3d::ERendererLayer::MESH) {
    setMeshOpacity(opacity);
  } else {
    m_view->setOpacity(layer, opacity);
  }
}

void Z3DWindow::setOpacityQuietly(
    neutu3d::ERendererLayer layer, double opacity)
{
  m_view->setOpacityQuietly(layer, opacity);
}

void Z3DWindow::setFront(neutu3d::ERendererLayer layer, bool on)
{
  m_view->setFront(layer, on);
}

void Z3DWindow::setColorMode(
    neutu3d::ERendererLayer layer, const std::string &mode)
{
  switch (layer) {
  case neutu3d::ERendererLayer::MESH:
    getMeshFilter()->setColorMode(mode);
    break;
  case neutu3d::ERendererLayer::SWC:
    getSwcFilter()->setColorMode(mode);
    break;
  case neutu3d::ERendererLayer::PUNCTA:
    getPunctaFilter()->setColorMode(mode);
    break;
  default:
    break;
  }
}

void Z3DWindow::gotoPosition(const ZCuboid& bound)
{
  ZBBox<glm::dvec3> bd(glm::dvec3(bound.firstCorner().x(),
                                  bound.firstCorner().y(),
                                  bound.firstCorner().z()),
                       glm::dvec3(bound.lastCorner().x(),
                                  bound.lastCorner().y(),
                                  bound.lastCorner().z()));
  m_view->gotoPosition(bd);
}

void Z3DWindow::gotoPosition(const ZPoint &position, double radius)
{
  m_view->gotoPosition(position.x(), position.y(), position.z(), radius);
}

bool Z3DWindow::isProjectedInRectRoi(const ZIntPoint &pt) const
{
  QPointF screenPos = m_view->getScreenProjection(
        pt.getX(), pt.getY(), pt.getZ(), neutu3d::ERendererLayer::SWC);

  return getRectRoi().contains(screenPos.x(), screenPos.y());
}

void Z3DWindow::deleteSelected()
{
  ZFlyEmBody3dDoc *doc = getDocument<ZFlyEmBody3dDoc>();
  if (doc != NULL) {
    doc->executeRemoveTodoCommand();
  }
}

void Z3DWindow::saveSplitTask()
{
  emit savingSplitTask();
}

void Z3DWindow::deleteSplitSeed()
{
  emit deletingSplitSeed();
}

void Z3DWindow::deleteSelectedSplitSeed()
{
  emit deletingSelectedSplitSeed();
}

void Z3DWindow::cropSwcInRoi()
{
  ZFlyEmBody3dDoc *doc = getDocument<ZFlyEmBody3dDoc>();
  if (doc != NULL) {
    if (doc->isDvidMutable()) {
      if (doc->getTag() == neutu::Document::ETag::FLYEM_BODY_3D &&
          doc->showingCoarseOnly()) {
        //    m_doc->executeDeleteSwcNodeCommand();
        if (ZDialogFactory::Ask("Cropping", "Do you want to crop the body?", this)) {
          emit croppingSwcInRoi();
        }
      } else {
        QMessageBox::warning(
              this, "Action Failed", "Cropping only works in coarse body view.");
      }
    }
  } else {
    selectSwcTreeNodeInRoi(false);
    m_doc->executeDeleteSwcNodeCommand();
  }
}

void Z3DWindow::selectSwcTreeNodeInRoi(bool appending)
{
  if (hasRectRoi()) {
    QList<ZSwcTree*> treeList = m_doc->getSwcList();

    ZRect2d rect = getRectRoi();

    for (QList<ZSwcTree*>::const_iterator iter = treeList.begin();
         iter != treeList.end(); ++iter) {
      ZSwcTree *tree = *iter;
      tree->recordSelection();
      if (!appending) {
        tree->deselectAllNode();
      }

      ZSwcTree::DepthFirstIterator nodeIter(tree);
      while (nodeIter.hasNext()) {
        Swc_Tree_Node *tn = nodeIter.next();
        if (SwcTreeNode::isRegular(tn)) {
          const QPointF &pt = m_view->getScreenProjection(
                SwcTreeNode::x(tn), SwcTreeNode::y(tn), SwcTreeNode::z(tn),
                neutu3d::ERendererLayer::SWC);
          if (rect.contains(pt.x(), pt.y())) {
            tree->selectNode(tn, true);
          }
        }
      }
    }

    for (QList<ZSwcTree*>::const_iterator iter = treeList.begin();
         iter != treeList.end(); ++iter) {
      ZSwcTree *tree = *iter;
      tree->processSelection();
    }

    m_doc->notifySwcTreeNodeSelectionChanged();
    removeRectRoi();
  }
}

void Z3DWindow::selectSwcTreeNodeTreeInRoi(bool appending)
{
  if (hasRectRoi()) {
    QList<ZSwcTree*> treeList = m_doc->getSwcList();

    ZRect2d rect = getRectRoi();

    for (QList<ZSwcTree*>::const_iterator iter = treeList.begin();
         iter != treeList.end(); ++iter) {
      ZSwcTree *tree = *iter;
      tree->recordSelection();
      if (!appending) {
        tree->deselectAllNode();
      }

      ZSwcTree::RegularRootIterator rootIter(tree);
      while (rootIter.hasNext()) {
        Swc_Tree_Node *root = rootIter.next();
        bool treeInRoi = true;
        ZSwcTree::DownstreamIterator dsIter(root);
        while (dsIter.hasNext()) {
          Swc_Tree_Node *tn = dsIter.next();
          const QPointF &pt = m_view->getScreenProjection(
                SwcTreeNode::x(tn), SwcTreeNode::y(tn), SwcTreeNode::z(tn),
                neutu3d::ERendererLayer::SWC);
          if (!rect.contains(pt.x(), pt.y())) {
            treeInRoi = false;
            break;
          }
        }

        if (treeInRoi) {
          dsIter.restart();
          while (dsIter.hasNext()) {
            Swc_Tree_Node *tn = dsIter.next();
            tree->selectNode(tn, true);
          }
        }
      }

      tree->processSelection();
    }

    m_doc->notifySwcTreeNodeSelectionChanged();
    removeRectRoi();
  }
}

void Z3DWindow::selectTerminalBranchInRoi(bool appending)
{
  if (hasRectRoi()) {
    QList<ZSwcTree*> treeList = m_doc->getSwcList();

    ZRect2d rect = getRectRoi();

    for (QList<ZSwcTree*>::const_iterator iter = treeList.begin();
         iter != treeList.end(); ++iter) {
      ZSwcTree *tree = *iter;
      tree->recordSelection();
      if (!appending) {
        tree->deselectAllNode();
      }

      ZSwcTree::TerminalIterator termIter(tree);
      while (termIter.hasNext()) {
        Swc_Tree_Node *terminal = termIter.next();
        Swc_Tree_Node *tn = terminal;
        bool treeInRoi = true;
        while (SwcTreeNode::isRegular(tn) && !SwcTreeNode::isBranchPoint(tn)) {
          const QPointF &pt = m_view->getScreenProjection(
                SwcTreeNode::x(tn), SwcTreeNode::y(tn), SwcTreeNode::z(tn),
                neutu3d::ERendererLayer::SWC);
          if (!rect.contains(pt.x(), pt.y())) {
            treeInRoi = false;
            break;
          }
          tn = SwcTreeNode::parent(tn);
        }

        if (treeInRoi) {
          tn = terminal;
          while (SwcTreeNode::isRegular(tn) && !SwcTreeNode::isBranchPoint(tn)) {
            tree->selectNode(tn, true);
            tn = SwcTreeNode::parent(tn);
          }
        }
      }

      tree->processSelection();
    }

    m_doc->notifySwcTreeNodeSelectionChanged();
    removeRectRoi();
  }
}


void Z3DWindow::removeRectRoi()
{
  getCanvas()->getInteractionEngine()->removeRectDecoration();
}

QDockWidget* Z3DWindow::getSettingsDockWidget()
{
    return m_settingsDockWidget;
}

QDockWidget* Z3DWindow::getObjectsDockWidget()
{
    return m_objectsDockWidget;
}

ZROIWidget* Z3DWindow::getROIsDockWidget()
{
    return m_roiDockWidget;
}

void Z3DWindow::setButtonStatus(int index, bool v)
{
    m_buttonStatus[index] = v;

}

bool Z3DWindow::getButtonStatus(int index)
{
    return m_buttonStatus[index];
}
