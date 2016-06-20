#include "z3dgl.h"
#include <QtGui>
#ifdef _QT5_
#include <QtWidgets>
#include <QtConcurrent>
#else
#include <QtConcurrentRun>
#endif

#include "zstackpresenter.h"
#include "zstackframe.h"
#include "zstackview.h"
#include "tz_rastergeom.h"
#include "zlocalneuroseg.h"
#include "zlocsegchain.h"
#include "widgets/zimagewidget.h"
#include "zstack.hxx"
#include "zstackdoc.h"
#include "swctreenode.h"
#include "dialogs/channeldialog.h"
#include "neutubeconfig.h"
#include "zcursorstore.h"
#include "zstroke2d.h"
#include "tz_geo3d_utils.h"
#include "zstackdocmenufactory.h"
#include "zinteractionevent.h"
#include "zellipse.h"
#include "zstackoperator.h"
#include "zrect2d.h"
#include "zstackobjectsource.h"
#include "zstackobjectsourcefactory.h"
#include "zstackmvc.h"
#if defined (_FLYEM_)
#include "dvid/zdvidlabelslice.h"
#include "dvid/zdvidsparsestack.h"
#endif
#include "zkeyoperationconfig.h"
#include "zstackfactory.h"

/*
ZStackPresenter::ZStackPresenter(ZStackFrame *parent) : QObject(parent)
{
  init();
}
*/

ZStackPresenter::ZStackPresenter(QWidget *parent) : QObject(parent)
{
  init();
}

ZStackPresenter::~ZStackPresenter()
{
  clearData();

  for (QMap<EObjectRole, ZStackObject*>::iterator iter =
       m_activeObjectMap.begin(); iter != m_activeObjectMap.end(); ++iter) {
    delete iter.value();
  }
  m_activeObjectMap.clear();
  m_activeDecorationList.clear();

  delete m_swcNodeContextMenu;
  delete m_strokePaintContextMenu;
  delete m_stackContextMenu;
  delete m_keyConfig;
  delete m_actionFactory;
  delete m_menuFactory;
}

ZStackPresenter* ZStackPresenter::Make(QWidget *parent)
{
  ZStackPresenter *presenter = new ZStackPresenter(parent);
  presenter->configKeyMap();

  return presenter;
}

void ZStackPresenter::init()
{
  m_showObject = true;
//  m_isStrokeOn = false;
  m_skipMouseReleaseEvent = 0;
  m_zOrder = 2;

  initInteractiveContext();

  m_greyScale.resize(5, 1.0);
  m_greyOffset.resize(5, 0.0);

  m_objStyle = ZStackObject::BOUNDARY;
  m_threshold = -1;
  m_mouseLeftButtonPressed = false;
  m_mouseRightButtonPressed = false;

  for (int i = 0; i < 3; i++) {
    m_mouseLeftReleasePosition[i] = -1;
    m_mouseRightReleasePosition[i] = -1;
    m_mouseLeftPressPosition[i] = -1;
    m_mouseRightPressPosition[i] = -1;
    m_mouseLeftDoubleClickPosition[i] = -1;
    m_mouseMovePosition[i] = -1;
  }

  m_cursorRadius = 10;

  ZStroke2d *stroke = new ZStroke2d;
  stroke->setVisible(false);
  stroke->setFilled(true);
  stroke->setPenetrating(true);
  stroke->hideStart(true);
  stroke->setTarget(ZStackObject::TARGET_OBJECT_CANVAS);
  addActiveObject(ROLE_STROKE, stroke);

  stroke = new ZStroke2d;
  stroke->setVisible(false);
  stroke->setFilled(false);
  stroke->setPenetrating(true);
  stroke->hideStart(true);
  stroke->setTarget(ZStackObject::TARGET_WIDGET);
  addActiveObject(ROLE_SWC, stroke);

  stroke = new ZStroke2d;
  stroke->setVisible(false);
  stroke->setFilled(false);
  stroke->setPenetrating(true);
  stroke->hideStart(true);
  stroke->setTarget(ZStackObject::TARGET_WIDGET);
  addActiveObject(ROLE_SYNAPSE, stroke);

  stroke = new ZStroke2d;
  stroke->setVisible(false);
  stroke->setFilled(false);
  stroke->setPenetrating(true);
  stroke->hideStart(true);
  stroke->setWidth(10.0);
  stroke->setTarget(ZStackObject::TARGET_WIDGET);
  addActiveObject(ROLE_BOOKMARK, stroke);

  /*
  m_stroke.setVisible(false);
  m_stroke.setPenetrating(true);
  m_stroke.hideStart(true);
  m_swcStroke.setPenetrating(true);
  m_swcStroke.setVisible(false);
  */

//  m_activeDecorationList.append(&m_stroke);
//  m_activeDecorationList.append(&m_swcStroke);

  m_highlightDecoration.setRadius(5.0);
  m_highlightDecoration.setColor(QColor(255, 255, 255, 160));
  m_highlightDecoration.setVisualEffect(NeuTube::Display::Sphere::VE_FORCE_FILL);
  m_highlightDecorationList.append(&m_highlightDecoration);
  m_highlight = false;

  m_swcNodeContextMenu = NULL;
  m_strokePaintContextMenu = NULL;
  m_stackContextMenu = NULL;
  m_bodyContextMenu = NULL;
    m_contextMenu = NULL;

  createActions();

  //m_leftButtonReleaseMapper.setContext(&m_interactiveContext);
  //m_moveMapper.setContext(&m_interactiveContext);
  m_mouseEventProcessor.setInteractiveContext(&m_interactiveContext);

  m_keyConfig = NULL;

  m_menuFactory = NULL;
  m_actionFactory = new ZActionFactory;

  /*
  ZKeyOperationConfig::Configure(m_activeStrokeOperationMap,
                                 ZKeyOperation::OG_ACTIVE_STROKE);
  ZKeyOperationConfig::Configure(
        m_swcKeyOperationMap, ZKeyOperation::OG_SWC_TREE_NODE);
  ZKeyOperationConfig::Configure(
        m_stackKeyOperationMap, ZKeyOperation::OG_STACK);
        */
}

void ZStackPresenter::addActiveObject(EObjectRole role, ZStackObject *obj)
{
  if (m_activeObjectMap.contains(role)) {
    m_activeDecorationList.removeOne(obj);
    delete m_activeObjectMap[role];
  }

  m_activeObjectMap[role] = obj;
  m_activeDecorationList.append(obj);
}

ZKeyOperationConfig* ZStackPresenter::getKeyConfig()
{
  if (m_keyConfig == NULL) {
    m_keyConfig = new ZKeyOperationConfig;
  }

  return m_keyConfig;
}

ZStackDocMenuFactory* ZStackPresenter::getMenuFactory()
{
  if (m_menuFactory == NULL) {
    m_menuFactory = new ZStackDocMenuFactory;
    m_menuFactory->setAdminState(NeuTube::IsAdminUser());
  }

  return m_menuFactory;
}

void ZStackPresenter::configKeyMap()
{
  ZKeyOperationConfig *config = getKeyConfig();
  config->configure(
        m_activeStrokeOperationMap, ZKeyOperation::OG_ACTIVE_STROKE);
  config->configure(
        m_swcKeyOperationMap, ZKeyOperation::OG_SWC_TREE_NODE);
  config->configure(
        m_stackKeyOperationMap, ZKeyOperation::OG_STACK);
  config->configure(m_objectKeyOperationMap, ZKeyOperation::OG_STACK_OBJECT);
}

QAction* ZStackPresenter::getAction(ZActionFactory::EAction item) const
{
  QAction *action = NULL;

  if (action == NULL) {
    const_cast<ZStackPresenter&>(*this).makeAction(item);
    if (m_actionMap.contains(item)) {
      action = m_actionMap[item];
    }
    if (action == NULL) {
      action = buddyDocument()->getAction(item);
    }
  }

  return action;
}

void ZStackPresenter::makeAction(ZActionFactory::EAction item)
{
  if (!m_actionMap.contains(item)) {
    QAction *action = m_actionFactory->makeAction(item, this);
    m_actionMap[item] = action;

    //Additional behaviors
    if (action != NULL) {
      switch (item) {
      case ZActionFactory::ACTION_DELETE_SELECTED:
        connect(action, SIGNAL(triggered()), this, SLOT(deleteSelected()));
        break;
      case ZActionFactory::ACTION_FIT_ELLIPSE:
        connect(action, SIGNAL(triggered()), this, SLOT(fitEllipse()));
        break;

        //Puncta actions
      case ZActionFactory::ACTION_PUNCTA_MARK:
        connect(action, SIGNAL(triggered()), this, SLOT(markPuncta()));
        break;
      case ZActionFactory::ACTION_PUNCTA_ENLARGE:
        connect(action, SIGNAL(triggered()), this, SLOT(enlargePuncta()));
        break;
      case ZActionFactory::ACTION_PUNCTA_NARROW:
        connect(action, SIGNAL(triggered()), this, SLOT(narrowPuncta()));
        break;
      case ZActionFactory::ACTION_PUNCTA_MEANSHIFT:
        connect(action, SIGNAL(triggered()), this, SLOT(meanshiftPuncta()));
        break;
      case ZActionFactory::ACTION_PUNCTA_MEANSHIFT_ALL:
        connect(action, SIGNAL(triggered()), this, SLOT(meanshiftAllPuncta()));
        break;

        //SWC actions
      case ZActionFactory::ACTION_ADD_SWC_NODE:
        connect(action, SIGNAL(triggered()), this, SLOT(trySwcAddNodeMode()));
        break;
      case ZActionFactory::ACTION_TOGGLE_SWC_SKELETON:
        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(toggleSwcSkeleton(bool)));
        break;
      case ZActionFactory::ACTION_LOCATE_SELECTED_SWC_NODES_IN_3D:
        connect(action, SIGNAL(triggered()),
                getParentFrame(), SLOT(locateSwcNodeIn3DView()));
        break;
      case ZActionFactory::ACTION_CONNECT_TO_SWC_NODE:
        connect(action, SIGNAL(triggered()),
                this, SLOT(enterSwcConnectMode()));
        m_singleSwcNodeActionActivator.registerAction(action, true);
        break;
      case ZActionFactory::ACTION_EXTEND_SWC_NODE:
        connect(action, SIGNAL(triggered()), this, SLOT(enterSwcExtendMode()));
        m_singleSwcNodeActionActivator.registerAction(action, true);
        break;
      case ZActionFactory::ACTION_MOVE_SWC_NODE:
        connect(action, SIGNAL(triggered()), this, SLOT(enterSwcMoveMode()));
        break;
      case ZActionFactory::ACTION_LOCK_SWC_NODE_FOCUS:
        connect(action, SIGNAL(triggered()),
                this, SLOT(lockSelectedSwcNodeFocus()));
        break;
      case ZActionFactory::ACTION_CHANGE_SWC_NODE_FOCUS:
        connect(action, SIGNAL(triggered()),
                this, SLOT(changeSelectedSwcNodeFocus()));
        break;
      case ZActionFactory::ACTION_ESTIMATE_SWC_NODE_RADIUS:
        connect(action, SIGNAL(triggered()),
                this, SLOT(estimateSelectedSwcRadius()));
        break;

        //Trace actions
      case ZActionFactory::ACTION_TRACE:
        connect(action, SIGNAL(triggered()), this, SLOT(traceTube()));
        break;
      case ZActionFactory::ACTION_FITSEG:
        connect(action, SIGNAL(triggered()), this, SLOT(fitSegment()));
        break;
      case ZActionFactory::ACTION_DROPSEG:
        connect(action, SIGNAL(triggered()), this, SLOT(dropSegment()));
        break;

        //stroke actions
      case ZActionFactory::ACTION_PAINT_STROKE:
        connect(action, SIGNAL(triggered()), this, SLOT(tryPaintStrokeMode()));
        break;
      case ZActionFactory::ACTION_ADD_SPLIT_SEED:
        connect(action, SIGNAL(triggered()), this, SLOT(tryPaintStrokeMode()));
        break;
      case ZActionFactory::ACTION_ERASE_STROKE:
        connect(action, SIGNAL(triggered()),
                this, SLOT(tryEraseStrokeMode()));
        break;

        //Body actions
      case ZActionFactory::ACTION_BODY_ANNOTATION:
        connect(action, SIGNAL(triggered()),
                this, SLOT(notifyBodyAnnotationTriggered()));
        break;
      case ZActionFactory::ACTION_BODY_SPLIT_START:
        connect(action, SIGNAL(triggered()),
                this, SLOT(notifyBodySplitTriggered()));
        break;
      case ZActionFactory::ACTION_BODY_CHECKIN:
        connect(action, SIGNAL(triggered()),
                this, SLOT(notifyBodyCheckinTriggered()));
        break;
      case ZActionFactory::ACTION_BODY_FORCE_CHECKIN:
        connect(action, SIGNAL(triggered()),
                this, SLOT(notifyBodyForceCheckinTriggered()));
        break;
      case ZActionFactory::ACTION_BODY_CHECKOUT:
        connect(action, SIGNAL(triggered()),
                this, SLOT(notifyBodyCheckoutTriggered()));
        break;
      case ZActionFactory::ACTION_BODY_DECOMPOSE:
        connect(action, SIGNAL(triggered()),
                this, SLOT(notifyBodyDecomposeTriggered()));
        break;
      case ZActionFactory::ACTION_BOOKMARK_CHECK:
        connect(action, SIGNAL(triggered()), this, SLOT(checkingBookmark()));
        break;
      case ZActionFactory::ACTION_BOOKMARK_UNCHECK:
        connect(action, SIGNAL(triggered()), this, SLOT(uncheckingBookmark()));
        break;
      case ZActionFactory::ACTION_SELECT_ALL_SWC_NODE:
        connect(action, SIGNAL(triggered()),
                buddyDocument(), SLOT(selectAllSwcTreeNode()));
        break;
      default:
        break;
      }
    }
  }
}

void ZStackPresenter::clearData()
{
  foreach(ZStackObject *decoration, m_decorationList) {
    delete decoration;
  }

  m_decorationList.clear();
}

void ZStackPresenter::initInteractiveContext()
{
  if (NeutubeConfig::getInstance().getMainWindowConfig().isTracingOff()) {
    m_interactiveContext.setTraceMode(ZInteractiveContext::TRACE_OFF);
  } else {
    m_interactiveContext.setTraceMode(ZInteractiveContext::TRACE_TUBE);
  }

  m_interactiveContext.setTubeEditMode(ZInteractiveContext::TUBE_EDIT_OFF);
}

void ZStackPresenter::createTraceActions()
{
  {
    QAction *action =
        ZActionFactory::MakeAction(ZActionFactory::ACTION_TRACE, this);
    connect(action, SIGNAL(triggered()), this, SLOT(traceTube()));
  }

  {
    QAction *action =
        ZActionFactory::MakeAction(ZActionFactory::ACTION_FITSEG, this);
    connect(action, SIGNAL(triggered()), this, SLOT(fitSegment()));
  }

  {
    QAction *action =
        ZActionFactory::MakeAction(ZActionFactory::ACTION_DROPSEG, this);
    connect(action, SIGNAL(triggered()), this, SLOT(dropSegment()));
  }
}

void ZStackPresenter::createPunctaActions()
{
  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_PUNCTA_MARK, this);
    connect(action, SIGNAL(triggered()), this, SLOT(markPuncta()));
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_PUNCTA_ENLARGE, this);
    connect(action, SIGNAL(triggered()), this, SLOT(enlargePuncta()));
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_PUNCTA_NARROW, this);
    connect(action, SIGNAL(triggered()), this, SLOT(narrowPuncta()));
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_PUNCTA_MEANSHIFT, this);
    connect(action, SIGNAL(triggered()), this, SLOT(meanshiftPuncta()));
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_PUNCTA_MEANSHIFT_ALL, this);
    connect(action, SIGNAL(triggered()), this, SLOT(meanshiftAllPuncta()));
  }
}

//Doesn't work while connecting doc slots directly for unknown reason
void ZStackPresenter::createDocDependentActions()
{
  assert(buddyDocument());
#if 0
  m_selectSwcConnectionAction = new QAction("Select Connection", this);
  connect(m_selectSwcConnectionAction, SIGNAL(triggered()), this,
          SLOT(selectSwcNodeConnection()));

  m_selectSwcNodeUpstreamAction = new QAction("Select Upstream", this);
  connect(m_selectSwcNodeUpstreamAction, SIGNAL(triggered()), this,
          SLOT(selectUpstreamNode()));

  m_selectSwcNodeBranchAction = new QAction("Select Branch", this);
  connect(m_selectSwcNodeBranchAction, SIGNAL(triggered()), this,
          SLOT(selectBranchNode()));

  m_selectSwcNodeTreeAction = new QAction("Select Tree", this);
  connect(m_selectSwcNodeTreeAction, SIGNAL(triggered()), this,
          SLOT(selectTreeNode()));

  m_selectAllConnectedSwcNodeAction = new QAction("Select All Connected Nodes", this);
  connect(m_selectAllConnectedSwcNodeAction, SIGNAL(triggered()), this,
          SLOT(selectConnectedNode()));

  m_selectAllSwcNodeAction = new QAction("Select All Nodes", this);
  connect(m_selectAllSwcNodeAction, SIGNAL(triggered()), this,
          SLOT(selectAllSwcTreeNode()));
#endif
}

void ZStackPresenter::createSwcActions()
{ 
  {
    QAction *action =
        ZActionFactory::MakeAction(ZActionFactory::ACTION_ADD_SWC_NODE, this);
    connect(action, SIGNAL(triggered()),
            this, SLOT(trySwcAddNodeMode()));
    m_actionMap[ZActionFactory::ACTION_ADD_SWC_NODE] = action;
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_TOGGLE_SWC_SKELETON, this);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(toggleSwcSkeleton(bool)));
    m_actionMap[ZActionFactory::ACTION_TOGGLE_SWC_SKELETON] = action;
  }


  if (getParentFrame() != NULL) {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_LOCATE_SELECTED_SWC_NODES_IN_3D, this);
    connect(action, SIGNAL(triggered()),
            getParentFrame(), SLOT(locateSwcNodeIn3DView()));
    m_actionMap[ZActionFactory::ACTION_LOCATE_SELECTED_SWC_NODES_IN_3D] = action;
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_CONNECT_TO_SWC_NODE, parent());
    connect(action, SIGNAL(triggered()),
            this, SLOT(enterSwcConnectMode()));
    m_actionMap[ZActionFactory::ACTION_CONNECT_TO_SWC_NODE] = action;
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_EXTEND_SWC_NODE, parent());
    connect(action, SIGNAL(triggered()), this, SLOT(enterSwcExtendMode()));
    m_actionMap[ZActionFactory::ACTION_EXTEND_SWC_NODE] = action;
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_MOVE_SWC_NODE, this->parent());
    connect(action, SIGNAL(triggered()), this, SLOT(enterSwcMoveMode()));
    m_actionMap[ZActionFactory::ACTION_MOVE_SWC_NODE] = action;
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_LOCK_SWC_NODE_FOCUS, this);
    connect(action, SIGNAL(triggered()),
            this, SLOT(lockSelectedSwcNodeFocus()));
    m_actionMap[ZActionFactory::ACTION_LOCK_SWC_NODE_FOCUS] = action;
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_CHANGE_SWC_NODE_FOCUS, parent());
    connect(action, SIGNAL(triggered()),
            this, SLOT(changeSelectedSwcNodeFocus()));
    m_actionMap[ZActionFactory::ACTION_CHANGE_SWC_NODE_FOCUS] = action;
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_ESTIMATE_SWC_NODE_RADIUS, this);
    connect(action, SIGNAL(triggered()),
            this, SLOT(estimateSelectedSwcRadius()));
    m_actionMap[ZActionFactory::ACTION_ESTIMATE_SWC_NODE_RADIUS] = action;
  }

  /*
  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ZActionFactory::ACTION_EXTEND_SWC_NODE], true);
  m_singleSwcNodeActionActivator.registerAction(
        m_actionMap[ZActionFactory::ACTION_CONNECT_TO_SWC_NODE], true);
        */
}

void ZStackPresenter::createStrokeActions()
{
  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_PAINT_STROKE, this);
    connect(action, SIGNAL(triggered()), this, SLOT(tryPaintStrokeMode()));
    m_actionMap[ZActionFactory::ACTION_PAINT_STROKE] = action;
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_ADD_SPLIT_SEED, this);
    connect(action, SIGNAL(triggered()),
            this, SLOT(tryPaintStrokeMode()));
    m_actionMap[ZActionFactory::ACTION_ADD_SPLIT_SEED] = action;
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_ERASE_STROKE, this);
    connect(action, SIGNAL(triggered()),
            this, SLOT(tryEraseStrokeMode()));
    m_actionMap[ZActionFactory::ACTION_ERASE_STROKE] = action;
  }
}

void ZStackPresenter::createMiscActions()
{
  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_SHOW_ORTHO, this);
    connect(action, SIGNAL(triggered()), this, SLOT(notifyOrthoViewTriggered()));
    m_actionMap[ZActionFactory::ACTION_SHOW_ORTHO] = action;
  }

}

void ZStackPresenter::createBodyActions()
{
  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_BODY_SPLIT_START, this);
    connect(action, SIGNAL(triggered()), this, SLOT(notifyBodySplitTriggered()));
    m_actionMap[ZActionFactory::ACTION_BODY_SPLIT_START] = action;
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_BODY_ANNOTATION, this);
    connect(action, SIGNAL(triggered()),
            this, SLOT(notifyBodyAnnotationTriggered()));
    m_actionMap[ZActionFactory::ACTION_BODY_ANNOTATION] = action;
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_BODY_CHECKIN, this);
    connect(action, SIGNAL(triggered()),
            this, SLOT(notifyBodyCheckinTriggered()));
    m_actionMap[ZActionFactory::ACTION_BODY_CHECKIN] = action;
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_BODY_FORCE_CHECKIN, this);
    connect(action, SIGNAL(triggered()),
            this, SLOT(notifyBodyForceCheckinTriggered()));
    m_actionMap[ZActionFactory::ACTION_BODY_FORCE_CHECKIN] = action;
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_BODY_CHECKOUT, this);
    connect(action, SIGNAL(triggered()),
            this, SLOT(notifyBodyCheckoutTriggered()));
    m_actionMap[ZActionFactory::ACTION_BODY_CHECKOUT] = action;
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_BODY_DECOMPOSE, this);
    connect(action, SIGNAL(triggered()),
            this, SLOT(notifyBodyDecomposeTriggered()));
    m_actionMap[ZActionFactory::ACTION_BODY_DECOMPOSE] = action;
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_BODY_MERGE, this);
    connect(action, SIGNAL(triggered()),
            this, SLOT(notifyBodyMergeTriggered()));
    m_actionMap[ZActionFactory::ACTION_BODY_MERGE] = action;
  }

//  action = new QAction(tr("Add split seed"), this);
//  connect(action, SIGNAL(triggered()), this, SLOT());
//  m_actionMap[ACTION_ADD_SPLIT_SEED] = action;
}

void ZStackPresenter::highlight(int x, int y, int z)
{
  m_highlightDecoration.setCenter(x, y, z);
}

void ZStackPresenter::createMainWindowActions()
{
  QAction *action = getParentFrame()->getBodySplitAction();
  if (action != NULL) {
    m_actionMap[ZActionFactory::ACTION_SPLIT_DATA] = action;
  }
}

void ZStackPresenter::createActions()
{
  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_DELETE_SELECTED, this);
    connect(action, SIGNAL(triggered()), this, SLOT(deleteSelected()));
    m_actionMap[ZActionFactory::ACTION_DELETE_SELECTED] = action;
  }


  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_FIT_ELLIPSE, this);
    connect(action, SIGNAL(triggered()), this, SLOT(fitEllipse()));
  }

/*
  m_frontAction = new QAction(tr("Bring to front"), this);
  connect(m_frontAction, SIGNAL(triggered()),
          this, SLOT(bringSelectedToFront()));

  m_backAction = new QAction(tr("Send to back"), this);
  connect(m_backAction, SIGNAL(triggered()),
          this, SLOT(sendSelectedToBack()));
*/
  createPunctaActions();
  createSwcActions();
  createTraceActions();
  //createTubeActions();
  createStrokeActions();
  createBodyActions();
  createMiscActions();
}

void ZStackPresenter::createSwcNodeContextMenu()
{
  if (m_swcNodeContextMenu == NULL) {
//    ZStackDocMenuFactory menuFactory;
    getMenuFactory()->setSingleSwcNodeActionActivator(&m_singleSwcNodeActionActivator);
    m_swcNodeContextMenu = getMenuFactory()->makeSwcNodeContextMenu(
          this, getParentWidget(), NULL);
    getMenuFactory()->makeSwcNodeContextMenu(
          buddyDocument(), getParentWidget(), m_swcNodeContextMenu);
    m_swcNodeContextMenu->addSeparator();
    m_swcNodeContextMenu->addAction(
          getAction(ZActionFactory::ACTION_ADD_SWC_NODE));
//          m_actionMap[ZActionFactory::ACTION_ADD_SWC_NODE]);
    m_swcNodeContextMenu->addAction(
          getAction(ZActionFactory::ACTION_LOCATE_SELECTED_SWC_NODES_IN_3D));
//          m_actionMap[ZActionFactory::ACTION_LOCATE_SELECTED_SWC_NODES_IN_3D]);
  }
}

QMenu* ZStackPresenter::getSwcNodeContextMenu()
{
  if (m_swcNodeContextMenu == NULL) {
    createSwcNodeContextMenu();
  }

  buddyDocument()->updateSwcNodeAction();
  m_singleSwcNodeActionActivator.update(buddyDocument());

  return m_swcNodeContextMenu;
}

void ZStackPresenter::createStrokeContextMenu()
{
  if (m_strokePaintContextMenu == NULL) {
//    ZStackDocMenuFactory menuFactory;
    m_strokePaintContextMenu =
        getMenuFactory()->makeSrokePaintContextMenu(this, getParentWidget(), NULL);
  }
}

QMenu* ZStackPresenter::getStrokeContextMenu()
{
  if (m_strokePaintContextMenu == NULL) {
    createStrokeContextMenu();
  }

  return m_strokePaintContextMenu;
}
#if defined(_FLYEM_)
#include "flyem/zflyemproofdocmenufactory.h"
#endif
void ZStackPresenter::createBodyContextMenu()
{
#if defined(_FLYEM_)
  if (dynamic_cast<ZFlyEmProofDocMenuFactory*>(getMenuFactory()) != NULL) {
    std::cout << "ZFlyEmProofDocMenuFactory" << std::endl;
  }
#endif

  if (m_bodyContextMenu == NULL) {
//    ZStackDocMenuFactory menuFactory;
    m_bodyContextMenu =
        getMenuFactory()->makeBodyContextMenu(this, getParentWidget(), NULL);
  }
}

QMenu* ZStackPresenter::getBodyContextMenu()
{
  if (m_bodyContextMenu == NULL) {
    createBodyContextMenu();
  }

  return m_bodyContextMenu;
}

void ZStackPresenter::createStackContextMenu()
{
  if (m_stackContextMenu == NULL) {
//    ZStackDocMenuFactory menuFactory;
    m_stackContextMenu =
        getMenuFactory()->makeStackContextMenu(this, getParentWidget(), NULL);
  }
}

QMenu* ZStackPresenter::getStackContextMenu()
{
  if (m_stackContextMenu == NULL) {
    createStackContextMenu();
  }

  return m_stackContextMenu;
}

QMenu* ZStackPresenter::getContextMenu()
{
  return getStackContextMenu();
}

ZStackObject* ZStackPresenter::getFirstOnActiveObject() const
{
  for (QMap<EObjectRole, ZStackObject*>::const_iterator iter =
       m_activeObjectMap.begin(); iter != m_activeObjectMap.end(); ++iter) {
    if (iter.value()->isVisible()) {
      return iter.value();
    }
  }

  return NULL;
}

void ZStackPresenter::turnOnActiveObject(EObjectRole role, bool refreshing)
{
  turnOffActiveObject();
  ZStackObject *obj = getActiveObject(role);
  if (obj != NULL) {
    ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(obj);
    if (stroke != NULL) {
      const ZMouseEvent& event = m_mouseEventProcessor.getLatestMouseEvent();
      ZPoint currentStackPos = event.getPosition(NeuTube::COORD_STACK);
      currentStackPos.shiftSliceAxis(getSliceAxis());

      stroke->setLast(currentStackPos.x(), currentStackPos.y());
    }

    switch (role) {
    case ROLE_SWC:
      if (buddyDocument()->getTag() == NeuTube::Document::FLYEM_ROI) {
        obj->useCosmeticPen(true);
      } else {
        obj->useCosmeticPen(false);
      }
      break;
    default:
      break;
    }
    obj->setVisible(true);
    if (refreshing) {
      buddyView()->paintActiveDecoration();
    }
  }
}
/*
void ZStackPresenter::turnOnStroke()
{
  if (buddyDocument()->getTag() == NeuTube::Document::FLYEM_ROI) {
    m_stroke.useCosmeticPen(true);
  } else {
    m_stroke.useCosmeticPen(false);
  }
  m_stroke.setVisible(true);
//  m_isStrokeOn = true;
  buddyView()->paintActiveDecoration();
}
*/

void ZStackPresenter::turnOffActiveObject()
{
  for (QMap<EObjectRole, ZStackObject*>::iterator iter =
       m_activeObjectMap.begin(); iter != m_activeObjectMap.end(); ++iter) {
    iter.value()->setVisible(false);
  }
}

void ZStackPresenter::turnOffActiveObject(EObjectRole role)
{
  ZStackObject *obj = getActiveObject(role);
  if (obj != NULL) {
    obj->setVisible(false);
    buddyView()->paintActiveDecoration();
  }
}

/*
void ZStackPresenter::turnOffStroke()
{
  m_stroke.toggleLabel(false);
  m_stroke.clear();
//  m_isStrokeOn = false;
  m_stroke.setVisible(false);
  buddyView()->paintActiveDecoration();
}
*/


ZStackDoc* ZStackPresenter::buddyDocument() const
{
  if (getParentFrame() != NULL) {
    return getParentFrame()->document().get();
  } else if (getParentMvc() != NULL) {
    return getParentMvc()->getDocument().get();
  }

  return NULL;
}

ZSharedPointer<ZStackDoc> ZStackPresenter::getSharedBuddyDocument() const
{
  if (getParentFrame() != NULL) {
    return getParentFrame()->document();
  } else if (getParentMvc() != NULL) {
    return getParentMvc()->getDocument();
  }

  return ZSharedPointer<ZStackDoc>();
}

ZStackView* ZStackPresenter::buddyView() const
{
  if (getParentFrame() != NULL) {
    return getParentFrame()->view();
  } else if (getParentMvc() != NULL) {
    return getParentMvc()->getView();
  }

  return NULL;
}

void ZStackPresenter::addPunctaEditFunctionToRightMenu()
{
  updateRightMenu(getAction(ZActionFactory::ACTION_PUNCTA_ENLARGE), false);
  updateRightMenu(getAction(ZActionFactory::ACTION_PUNCTA_ENLARGE), false);
  updateRightMenu(getAction(ZActionFactory::ACTION_PUNCTA_MEANSHIFT), false);
  updateRightMenu(getAction(ZActionFactory::ACTION_PUNCTA_MEANSHIFT_ALL), false);
  updateRightMenu(getAction(ZActionFactory::ACTION_DELETE_SELECTED), false);
  //updateRightMenu(m_deleteAllPunctaAction, false);
}

void ZStackPresenter::prepareView()
{
  createDocDependentActions();
  if (NeutubeConfig::getInstance().getMainWindowConfig().isTracingOn()) {
    updateLeftMenu(getAction(ZActionFactory::ACTION_TRACE));
  } else {
    updateLeftMenu(NULL);
  }
  buddyView()->rightMenu()->clear();
  //m_interactiveContext.setView(buddyView()->imageWidget()->projectRegion(),
  //                             buddyView()->imageWidget()->viewPort());
  m_mouseEventProcessor.setImageWidget(buddyView()->imageWidget());
  m_mouseEventProcessor.setDocument(getSharedBuddyDocument());

  setSliceAxis(buddyView()->getSliceAxis());

//  m_swcKeyMapper.setTag(buddyDocument()->getTag());
}

void ZStackPresenter::updateLeftMenu(QAction *action, bool clear)
{
  if (clear == true) {
    buddyView()->leftMenu()->clear();
  }
  if (action != NULL) {
    buddyView()->leftMenu()->addAction(action);
  }
}

void ZStackPresenter::updateLeftMenu()
{
  bool traceOnFlag = false;
  if (interactiveContext().tracingTube()) {
    updateLeftMenu(getAction(ZActionFactory::ACTION_TRACE), true);
    traceOnFlag = true;
  } else if (interactiveContext().fittingSegment()) {
    updateLeftMenu(getAction(ZActionFactory::ACTION_FITSEG), true);
    updateLeftMenu(getAction(ZActionFactory::ACTION_DROPSEG), false);
    updateLeftMenu(getAction(ZActionFactory::ACTION_FIT_ELLIPSE), false);
    traceOnFlag = true;
  }

  if (interactiveContext().markPuncta()) {
    if (traceOnFlag) {
      updateLeftMenu(getAction(ZActionFactory::ACTION_PUNCTA_MARK), false);
    } else {
      updateLeftMenu(getAction(ZActionFactory::ACTION_PUNCTA_MARK), true);
    }
  }
}

void ZStackPresenter::updateRightMenu(QAction *action, bool clear)
{
  if (clear == true) {
    buddyView()->rightMenu()->clear();
  }

  buddyView()->rightMenu()->addAction(action);
}

void ZStackPresenter::updateRightMenu(QMenu *submenu, bool clear)
{
  if (clear == true) {
    buddyView()->rightMenu()->clear();
  }

  buddyView()->rightMenu()->addMenu(submenu);
}

void ZStackPresenter::updateView() const
{
  buddyView()->redraw();
}

/*
int ZStackPresenter::zoomRatio() const
{
  return buddyView()->imageWidget()->zoomRatio();
}
*/

bool ZStackPresenter::hasObjectToShow() const
{
  if (m_showObject == true) {
    if (m_decorationList.size() > 0) {
      return true;
    }
  }

  return false;
}

const QPointF ZStackPresenter::stackPositionFromMouse(MouseButtonAction mba)
{
  int x, y;
  switch (mba) {
  case LEFT_RELEASE:
    x = m_mouseLeftReleasePosition[0];
    y = m_mouseLeftReleasePosition[1];
    break;
  case RIGHT_RELEASE:
    x = m_mouseRightReleasePosition[0];
    y = m_mouseRightReleasePosition[1];
    break;
  case LEFT_PRESS:
    x = m_mouseLeftPressPosition[0];
    y = m_mouseLeftPressPosition[1];
    break;
  case RIGHT_PRESS:
    x = m_mouseRightPressPosition[0];
    y = m_mouseRightPressPosition[1];
    break;
  case LEFT_DOUBLE_CLICK:
    x = m_mouseLeftDoubleClickPosition[0];
    y = m_mouseLeftDoubleClickPosition[1];
    break;
  case MOVE:
    x = m_mouseMovePosition[0];
    y = m_mouseMovePosition[1];
    break;
  default:
    x = 0;
    y = 0;
  }

  return buddyView()->imageWidget()->canvasCoordinate(QPoint(x, y));
}

ZPoint ZStackPresenter::getLastMousePosInStack()
{
  return m_mouseEventProcessor.getLatestStackPosition();
}

const Swc_Tree_Node* ZStackPresenter::getSelectedSwcNode() const
{
  std::set<Swc_Tree_Node*> nodeSet =
      buddyDocument()->getSelectedSwcNodeSet();
  if (!nodeSet.empty()) {
    return *(nodeSet.begin());
  }

  return NULL;
}

bool ZStackPresenter::isPointInStack(double x, double y)
{
  return (x >= 0) && (x < buddyDocument()->getStack()->width()) &&
      (y >= 0) && (y < buddyDocument()->getStack()->height());
}

int ZStackPresenter::getSliceIndex() const {
  int sliceIndex = -1;
  if (!m_interactiveContext.isProjectView()) {
    sliceIndex = buddyView()->sliceIndex();
  }

  return sliceIndex;
}

void ZStackPresenter::processMouseReleaseEvent(QMouseEvent *event)
{
#ifdef _DEBUG_
  std::cout << event->button() << " released: " << event->buttons() << std::endl;
#endif

  if (m_skipMouseReleaseEvent) {
    if (event->buttons() == Qt::NoButton) {
      m_skipMouseReleaseEvent = 0;
      this->interactiveContext().restoreExploreMode();
//      buddyView()->notifyViewPortChanged();
    }

//    --m_skipMouseReleaseEvent;
//    if (m_skipMouseReleaseEvent == 0) {

//    }
    return;
  }

  if (event->buttons() != Qt::NoButton) {
    m_skipMouseReleaseEvent = 1;
    return;
  }

  const ZMouseEvent& mouseEvent =
      m_mouseEventProcessor.process(
        event, ZMouseEvent::ACTION_RELEASE, getSliceIndex());

  if (mouseEvent.isNull()) {
    return;
  }

  ZStackOperator op = m_mouseEventProcessor.getOperator();  
  process(op);

  if (isContextMenuOn()) {
    m_interactiveContext.blockContextMenu();
  }
}

/*
void ZStackPresenter::setViewPortCenter(int x, int y, int z)
{
  buddyView()->imageWidget()->setViewPortOffset(
        x - buddyView()->imageWidget()->viewPort().width() / 2,
        y - buddyView()->imageWidget()->viewPort().height() / 2);
  buddyView()->setSliceIndex(z);
  buddyView()->updateImageScreen(ZStackView::UPDATE_QUEUED);
}
*/

/*
void ZStackPresenter::moveImage(int mouseX, int mouseY)
{
  moveImageToMouse(m_grabPosition.x(), m_grabPosition.y(), mouseX, mouseY);
}
*/

void ZStackPresenter::moveImageToMouse(
    double srcX, double srcY, int mouseX, int mouseY)
{
//  QPointF targetPosition =
//      buddyView()->imageWidget()->canvasCoordinate(QPoint(mouseX, mouseY));

  /* Dist from target to the viewport start in the world coordinate */
  double distX = double(mouseX) * buddyView()->imageWidget()->viewPort().width()
      / buddyView()->imageWidget()->projectSize().width();
  double distY = double(mouseY) * buddyView()->imageWidget()->viewPort().height()
      / buddyView()->imageWidget()->projectSize().height();

  /* the grabbed position keeps constant in the world coordinate */
  double x = srcX - distX;
  double y = srcY - distY;

//  std::cout << srcX << " " << srcY << std::endl;
//  qDebug() << "target pos: " << targetPosition;

//  QPointF oldViewportOffset = buddyView()->imageWidget()->viewPort().topLeft();

//  QPointF newOffset = oldViewportOffset + targetPosition - QPointF(srcX, srcY);

  moveViewPortTo(iround(x), iround(y));

  /*
  int x, y;

  x = srcX
      - double(mouseX) * buddyView()->imageWidget()->viewPort().width()
      / buddyView()->imageWidget()->projectSize().width();
  y = srcY
      - double(mouseY) * buddyView()->imageWidget()->viewPort().height()
      / buddyView()->imageWidget()->projectSize().height();

  moveViewPortTo(iround(x), iround(y));
  */
}

void ZStackPresenter::moveViewPort(int dx, int dy)
{
  buddyView()->imageWidget()->moveViewPort(dx, dy);
  buddyView()->updateImageScreen(ZStackView::UPDATE_QUEUED);
}

void ZStackPresenter::moveViewPortTo(int x, int y)
{
  buddyView()->setViewPortOffset(x, y);
//  buddyView()->updateImageScreen(ZStackView::UPDATE_QUEUED);
}

void ZStackPresenter::increaseZoomRatio()
{
  buddyView()->increaseZoomRatio();
}

void ZStackPresenter::decreaseZoomRatio()
{
  buddyView()->decreaseZoomRatio();
  //buddyView()->updateImageScreen();
  /*
  m_interactiveContext.setView(buddyView()->imageWidget()->projectRegion(),
                               buddyView()->imageWidget()->viewPort());
                               */
}

void ZStackPresenter::increaseZoomRatio(int x, int y)
{
  buddyView()->increaseZoomRatio(x, y);
}

void ZStackPresenter::decreaseZoomRatio(int x, int y)
{
  buddyView()->decreaseZoomRatio(x, y);
}

void ZStackPresenter::processMouseMoveEvent(QMouseEvent *event)
{
#ifdef _DEBUG_2
  std::cout << "Recorder address: " << &(m_mouseEventProcessor.getRecorder())
            << std::endl;
#endif

  const ZMouseEvent &mouseEvent = m_mouseEventProcessor.process(
        event, ZMouseEvent::ACTION_MOVE, getSliceIndex());

  if (mouseEvent.isNull()) {
    return;
  }

#ifdef _DEBUG_2
  std::cout << "Recorder address: " << &(m_mouseEventProcessor.getRecorder())
            << std::endl;
#endif

  //mouseEvent.set(event, ZMouseEvent::ACTION_MOVE, m_mouseMovePosition[2]);
  ZStackOperator op = m_mouseEventProcessor.getOperator();

  process(op);
}

QPointF ZStackPresenter::mapFromWidgetToStack(const QPoint &pos)
{
  return buddyView()->imageWidget()->canvasCoordinate(pos) +
      QPoint(buddyDocument()->getStackOffset().getX(),
             buddyDocument()->getStackOffset().getY());
}

QPointF ZStackPresenter::mapFromGlobalToStack(const QPoint &pos)
{
  return mapFromWidgetToStack(buddyView()->imageWidget()->mapFromGlobal(pos));
}

bool ZStackPresenter::isContextMenuOn()
{
  if (getSwcNodeContextMenu()->isVisible()) {
    return true;
  }

  if (getStrokeContextMenu()->isVisible()) {
    return true;
  }

  if (getStackContextMenu()->isVisible()) {
    return true;
  }

  return false;
}

void ZStackPresenter::processMousePressEvent(QMouseEvent *event)
{
  if (event->buttons() == (Qt::LeftButton | Qt::RightButton)) {
    m_skipMouseReleaseEvent = 2;
  } else {
    m_skipMouseReleaseEvent = 0;
  }

#ifdef _DEBUG_
  std::cout << "Pressed mouse buttons: " << event->buttons() << std::endl;
#endif

  const ZMouseEvent &mouseEvent = m_mouseEventProcessor.process(
        event, ZMouseEvent::ACTION_PRESS, buddyView()->sliceIndex());
  if (mouseEvent.isNull()) {
    return;
  }

  ZStackOperator op = m_mouseEventProcessor.getOperator();
  process(op);

  m_interactiveContext.setExitingEdit(false);

//  ZStackOperator op =m_mouseEventProcessor.getOperator();
//  switch (op.getOperation()) {
//  case ZStackOperator::OP_STROKE_START_PAINT:
//    m_stroke.set(zevent.getStackPosition().x(), zevent.getStackPosition().y());
//    break;
//  default:
//    break;
//  }

  if (event->button() == Qt::RightButton) {
    m_mouseRightButtonPressed = true;
  }
}

#define REMOVE_SELECTED_OBJECT(objtype, list, iter)	\
QMutableListIterator<objtype*> iter(list);	\
    while (iter.hasNext()) {	\
      if (iter.next()->isSelected()) {	\
        iter.remove();	\
      }	\
}

bool ZStackPresenter::isOperatable(ZStackOperator::EOperation op)
{
  return ZStackOperator::IsOperable(op, buddyDocument());
#if 0
  bool opable = true;
  switch (op) {
  case ZStackOperator::OP_NULL:
    opable = false;
    break;
  case ZStackOperator::OP_SWC_DELETE_NODE:
  case ZStackOperator::OP_SWC_MOVE_NODE_LEFT:
  case ZStackOperator::OP_SWC_MOVE_NODE_LEFT_FAST:
  case ZStackOperator::OP_SWC_MOVE_NODE_RIGHT:
  case ZStackOperator::OP_SWC_MOVE_NODE_RIGHT_FAST:
  case ZStackOperator::OP_SWC_MOVE_NODE_UP:
  case ZStackOperator::OP_SWC_MOVE_NODE_UP_FAST:
  case ZStackOperator::OP_SWC_MOVE_NODE_DOWN:
  case ZStackOperator::OP_SWC_MOVE_NODE_DOWN_FAST:
  case ZStackOperator::OP_SWC_CONNECT_NODE:
  case ZStackOperator::OP_SWC_CONNECT_NODE_SMART:
  case ZStackOperator::OP_SWC_CONNECT_ISOLATE:
  case ZStackOperator::OP_SWC_ZOOM_TO_SELECTED_NODE:
  case ZStackOperator::OP_SWC_MOVE_NODE:
  case ZStackOperator::OP_SWC_CHANGE_NODE_FOCUS:
  case ZStackOperator::OP_SWC_SELECT_CONNECTION:
  case ZStackOperator::OP_SWC_SELECT_FLOOD:
    if (buddyDocument()->getSelectedSwcNodeList().isEmpty()) {
      opable = false;
    }
    break;
  case ZStackOperator::OP_SWC_EXTEND:
  case ZStackOperator::OP_SWC_SMART_EXTEND:
  case ZStackOperator::OP_SWC_RESET_BRANCH_POINT:
  case ZStackOperator::OP_SWC_CONNECT_TO:
  case ZStackOperator::OP_SWC_LOCATE_FOCUS:
  case ZStackOperator::OP_SWC_ENTER_EXTEND_NODE:
    if (buddyDocument()->getSelectedSwcNodeList().size() != 1) {
      opable = false;
    }
    break;
  case ZStackOperator::OP_SWC_BREAK_NODE:
  case ZStackOperator::OP_SWC_INSERT_NODE:
    if (buddyDocument()->getSelectedSwcNodeList().size() <= 1) {
      opable = false;
    }
    break;
  case ZStackOperator::OP_SWC_ENTER_ADD_NODE:
    if (buddyDocument()->getTag() != NeuTube::Document::NORMAL &&
        buddyDocument()->getTag() != NeuTube::Document::BIOCYTIN_STACK &&
        buddyDocument()->getTag() != NeuTube::Document::FLYEM_ROI) {
      opable = false;
    }
    break;
  case ZStackOperator::OP_SWC_DECREASE_NODE_SIZE:
  case ZStackOperator::OP_SWC_INCREASE_NODE_SIZE:
    if (buddyDocument()->getSelectedSwcNodeList().isEmpty() || isStrokeOn()) {
      opable = false;
    }
    break;
  default:
    break;
  }

  return opable;
#endif
}

bool ZStackPresenter::processKeyPressEventForStack(QKeyEvent *event)
{
  bool taken = false;

  ZStackOperator::EOperation opId =
      m_stackKeyOperationMap.getOperation(event->key(), event->modifiers());
  if (isOperatable(opId)) {
    ZStackOperator op;
    op.setOperation(opId);
    if (!op.isNull()) {
      taken = true;
      process(op);
    }
  }

  return taken;
}

bool ZStackPresenter::processKeyPressEventForActiveStroke(QKeyEvent *event)
{
  bool taken = false;

  if (isActiveObjectOn()) {
    ZStackOperator::EOperation opId =
        m_activeStrokeOperationMap.getOperation(
          event->key(), event->modifiers());

    ZStackOperator op;
    op.setOperation(opId);
    if (!op.isNull()) {
      taken = true;
      process(op);
    }
  }

  return taken;
}

bool ZStackPresenter::processKeyPressEventForSwc(QKeyEvent *event)
{
  bool taken = false;

  ZStackOperator::EOperation opId =
      m_swcKeyOperationMap.getOperation(event->key(), event->modifiers());
  if (isOperatable(opId)) {
    ZStackOperator op;
    op.setOperation(opId);
    if (!op.isNull()) {
      taken = true;
      process(op);
    }
  }

  return taken;
}

bool ZStackPresenter::processKeyPressEventForObject(QKeyEvent *event)
{
  bool taken = false;

  ZStackOperator::EOperation opId =
      m_objectKeyOperationMap.getOperation(event->key(), event->modifiers());
  if (isOperatable(opId)) {
    ZStackOperator op;
    op.setOperation(opId);
    if (!op.isNull()) {
      taken = true;
      process(op);
    }
  }

  return taken;
}

bool ZStackPresenter::processKeyPressEventForStroke(QKeyEvent *event)
{
  bool taken = false;
  switch (event->key()) {
  case Qt::Key_R:
    if (event->modifiers() == Qt::ShiftModifier) {
      tryDrawRectMode();
      taken = true;
    } else {
      QAction *action = getAction(ZActionFactory::ACTION_PAINT_STROKE);
      if (action->isEnabled()) {
//        if (isStrokeOn()) {
        if (interactiveContext().strokeEditMode() ==
            ZInteractiveContext::STROKE_DRAW) {
          exitStrokeEdit();
        } else {
          action->trigger();
        }
        taken = true;
      }
    }
    break;
  case Qt::Key_0:
  case Qt::Key_1:
  case Qt::Key_2:
  case Qt::Key_3:
#if defined(_FLYEM_)
  case Qt::Key_4:
  case Qt::Key_5:
  case Qt::Key_6:
  case Qt::Key_7:
  case Qt::Key_8:
  case Qt::Key_9:
#endif
    if (m_interactiveContext.strokeEditMode() ==
        ZInteractiveContext::STROKE_DRAW) {
      ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_STROKE);
      stroke->setLabel(event->key() - Qt::Key_0);
      buddyView()->paintActiveDecoration();
      taken = true;
    }
    break;
  case Qt::Key_QuoteLeft:
    if (m_interactiveContext.strokeEditMode() ==
        ZInteractiveContext::STROKE_DRAW) {
      ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_STROKE);
      stroke->setLabel(255);
      buddyView()->paintActiveDecoration();
      taken = true;
    }
    break;
  case Qt::Key_E:
    if (event->modifiers() == Qt::ControlModifier) {
      QAction *action = getAction(ZActionFactory::ACTION_ERASE_STROKE);
      if (action->isEnabled()) {
        action->trigger();
        taken = true;
      }
    }
    break;
  default:
    break;
  }

  return taken;
}

void ZStackPresenter::setZoomRatio(double ratio)
{
  //m_zoomRatio = ratio;
  //CLIP_VALUE(m_zoomRatio, 1, 16);
  //buddyView()->imageWidget()->setZoomRatio(m_zoomRatio);
  buddyView()->imageWidget()->setZoomRatio(ratio);
}

bool ZStackPresenter::estimateActiveStrokeWidth()
{
  //Automatic adjustment
  bool succ = false;
  for (QMap<EObjectRole, ZStackObject*>::iterator iter =
       m_activeObjectMap.begin(); iter != m_activeObjectMap.end(); ++iter) {
    ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(iter.value());
    if (stroke != NULL) {
      if (stroke->isVisible()) {
        int x = 0;
        int y = 0;
        int width = stroke->getWidth();
        stroke->getLastPoint(&x, &y);

        Swc_Tree_Node tn;
        x -= buddyDocument()->getStack()->getOffset().getX();
        y -= buddyDocument()->getStack()->getOffset().getY();

        SwcTreeNode::setNode(
              &tn, 1, 2, x, y, buddyView()->sliceIndex(), width / 2.0, -1);

        if (SwcTreeNode::fitSignal(&tn, buddyDocument()->getStack()->c_stack(),
                                   buddyDocument()->getStackBackground())) {
          stroke->setWidth(SwcTreeNode::radius(&tn) * 2.0);

          succ = true;
        }
        break;
      }
    }
  }

  return succ;
}

bool ZStackPresenter::processKeyPressEvent(QKeyEvent *event)
{
  bool processed = true;

  if (processKeyPressEventForActiveStroke(event)) {
    return processed;
  }

  if (processKeyPressEventForSwc(event)) {
    return processed;
  }

  if (processKeyPressEventForStroke(event)) {
    return processed;
  }

  if (processKeyPressEventForObject(event)) {
    return processed;
  }

  if (processKeyPressEventForStack(event)) {
    return processed;
  }

  switch (event->key()) {
  /*
  case Qt::Key_Backspace:
  case Qt::Key_Delete:
    buddyDocument()->executeRemoveSelectedObjectCommand();
    break;
    */
  case Qt::Key_P:
    if (event->modifiers() == Qt::ControlModifier) {
      if (interactiveContext().isProjectView()) {
        interactiveContext().setViewMode(ZInteractiveContext::VIEW_NORMAL);
      } else {
        interactiveContext().setViewMode(ZInteractiveContext::VIEW_PROJECT);
      }
      updateView();
      emit(viewModeChanged());
    } else if (event->modifiers() == Qt::ShiftModifier) {
      buddyDocument()->pushSelectedLocsegChain();
      updateView();
    }
    break;

  case Qt::Key_Equal:
  case Qt::Key_Up:
    if (event->modifiers() == Qt::NoModifier) {
      increaseZoomRatio();
    }
    break;
  case Qt::Key_2:
    if (m_interactiveContext.strokeEditMode() !=
        ZInteractiveContext::STROKE_DRAW) {
      increaseZoomRatio();
    }
    break;
  case Qt::Key_Minus:
  case Qt::Key_Down:
    if (event->modifiers() == Qt::NoModifier) {
      decreaseZoomRatio();
    }
    break;
  case Qt::Key_1:
    if (m_interactiveContext.strokeEditMode() !=
        ZInteractiveContext::STROKE_DRAW) {
      decreaseZoomRatio();
    }
    break;
  case Qt::Key_W:
    if (event->modifiers() == Qt::ShiftModifier) {
      moveViewPort(0, 10);
    } else {
      moveViewPort(0, 1);
    }
    break;
  case Qt::Key_A:
    if (event->modifiers() == Qt::ShiftModifier) {
      moveViewPort(10, 0);
    } else {
      moveViewPort(1, 0);
    }
    break;

  case Qt::Key_S:
    if (event->modifiers() == Qt::ShiftModifier) {
      moveViewPort(0, -10);
    } else if (event->modifiers() == Qt::NoModifier) {
      moveViewPort(0, -1);
    } else if (event->modifiers() == Qt::ControlModifier) {
      if (getParentFrame() != NULL) {
        buddyDocument()->saveSwc(getParentFrame());
      }
    }
    break;

  case Qt::Key_D:
    if (event->modifiers() == Qt::ShiftModifier) {
      moveViewPort(-10, 0);
    } else {
      moveViewPort(-1, 0);
    }
    break;

  case Qt::Key_Left:
    if (!interactiveContext().isProjectView()) {
      int step = -1;

      if (event->modifiers() & Qt::ShiftModifier) {
        step = -5;
      }
      buddyView()->stepSlice(step);
    }
    break;

  case Qt::Key_Right:
    if (!interactiveContext().isProjectView()) {
      int step = 1;

      if (event->modifiers() & Qt::ShiftModifier) {
        step = 5;
      }
      buddyView()->stepSlice(step);
    }
    break;


  case Qt::Key_M:
    if (m_interactiveContext.isStackSliceView()) {
      if (interactiveContext().markPuncta() && buddyDocument()->hasStackData() &&
          (!buddyDocument()->getStack()->isVirtual())) {
        QPointF dataPos = stackPositionFromMouse(MOVE);
        buddyDocument()->markPunctum(dataPos.x(), dataPos.y(),
                                     buddyView()->sliceIndex());
      }
    } else {
      processed = false;
    }
    break;

  case Qt::Key_Escape:
    enterSwcSelectMode();
//    m_interactiveContext.setSwcEditMode(ZInteractiveContext::SWC_EDIT_SELECT);
    m_interactiveContext.setTubeEditMode(ZInteractiveContext::TUBE_EDIT_OFF);
    //turnOffStroke();
    exitStrokeEdit();
    updateCursor();
    break;
  case Qt::Key_Comma:
    if (isActiveObjectOn()) {
      ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(getFirstOnActiveObject());
      if (stroke != NULL) {
        stroke->addWidth(-1.0);
      }
      buddyView()->paintActiveDecoration();
    }
    break;
  case Qt::Key_Period:
    if (isActiveObjectOn()) {
      if (event->modifiers() == Qt::NoModifier) {
        ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(getFirstOnActiveObject());
        if (stroke != NULL) {
          stroke->addWidth(1.0);
        }
        buddyView()->paintActiveDecoration();
      } else if (event->modifiers() == Qt::ShiftModifier) {
        if (estimateActiveStrokeWidth()) {
          buddyView()->paintActiveDecoration();
        }
      }
    }
    break;
  case Qt::Key_Space:
    if (GET_APPLICATION_NAME == "FlyEM") {
      //if (buddyDocument()->getTag() == NeuTube::Document::FLYEM_SPLIT ||
       //   buddyDocument()->getTag() == NeuTube::Document::FLYEM_PROOFREAD ||
         // buddyDocument()->getTag() == NeuTube::Document::SEGMENTATION_TARGET) {
        if (event->modifiers() == Qt::ShiftModifier) {
          qDebug() << "Starting watershed ...";
          buddyDocument()->runSeededWatershed();
        } else {
          buddyDocument()->runLocalSeededWatershed();
        }
      //}
    }
    break;
  case Qt::Key_Z:
    if (getParentMvc() != NULL) {
      if (event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier)) {
        buddyDocument()->getAction(ZActionFactory::ACTION_REDO)->trigger();
//        buddyDocument()->undoStack()->redo();
      } else if (event->modifiers() == Qt::ControlModifier) {
        buddyDocument()->getAction(ZActionFactory::ACTION_UNDO)->trigger();
//        buddyDocument()->undoStack()->undo();
      }
    }
    break;
    /*
  case Qt::Key_F:
    toggleObjectVisible();
    break;
    */
  default:
    processed = false;
    break;
  }

  if (!processed) {
    processed = customKeyProcess(event);
  }

  return processed;
}

bool ZStackPresenter::customKeyProcess(QKeyEvent */*event*/)
{
  return false;
}

void ZStackPresenter::processMouseDoubleClickEvent(QMouseEvent *event)
{
  const ZMouseEvent &mouseEvent = m_mouseEventProcessor.process(
        event, ZMouseEvent::ACTION_DOUBLE_CLICK, getSliceIndex());

  if (mouseEvent.isNull()) {
    return;
  }

  ZStackOperator op = m_mouseEventProcessor.getOperator();
  if (op.getMouseEventRecorder() == NULL) {
    return;
  }

  process(op);

//  ZPoint currentStackPos = op.getMouseEventRecorder()->getLatestMouseEvent().
//      getPosition(ZMouseEvent::COORD_STACK);
  /*
  ZPoint currentRawStackPos = op.getMouseEventRecorder()->getLatestMouseEvent().
      getPosition(NeuTube::COORD_RAW_STACK);

  ZInteractionEvent interactionEvent;
*/

}

void ZStackPresenter::setObjectVisible(bool v)
{
  if (m_showObject != v) {
    m_showObject = v;
    if (v) {
      ZStackDoc::ActiveViewObjectUpdater updater(getSharedBuddyDocument());
      updater.exclude(ZStackObject::TYPE_DVID_TILE_ENSEMBLE);
      updater.update(buddyView()->getViewParameter(NeuTube::COORD_STACK));
    }

    buddyView()->paintObject();
    if (m_showObject) {
      emit objectVisibleTurnedOn();
    }
  }
}

void ZStackPresenter::toggleObjectVisible()
{
  setObjectVisible(!isObjectVisible());
}

void ZStackPresenter::setObjectStyle(ZStackObject::EDisplayStyle style)
{
  if (m_objStyle != style) {
    m_objStyle = style;
    buddyView()->redrawObject();
//    updateView();
  }
}

bool ZStackPresenter::isObjectVisible()
{
  return m_showObject;
}

void ZStackPresenter::removeLastDecoration(ZStackObject *obj)
{
  if (!m_decorationList.isEmpty()) {
    if (obj == NULL) {
      delete m_decorationList.takeLast();
      updateView();
    } else {
      if (obj == m_decorationList.last()) {
        m_decorationList.removeLast();
        updateView();
      } else if (obj == m_decorationList.first()) {
        m_decorationList.removeFirst();
        updateView();
      }
    }
  }
}

void ZStackPresenter::removeDecoration(ZStackObject *obj, bool redraw)
{
  for (int i = 0; i < m_decorationList.size(); i++) {
    if (m_decorationList.at(i) == obj) {
      m_decorationList.removeAt(i);
      break;
    }
  }

  if (redraw == true) {
    updateView();
  }
}

void ZStackPresenter::removeAllDecoration()
{
  m_decorationList.clear();
}

void ZStackPresenter::addDecoration(ZStackObject *obj, bool tail)
{
  if (tail == true) {
    m_decorationList.append(obj);
  } else {
    m_decorationList.prepend(obj);
  }
}



void ZStackPresenter::setStackBc(double scale, double offset, int c)
{
  if (c >= 0 && c < (int) m_greyScale.size()) {
    m_greyScale[c] = scale;
    m_greyOffset[c] = offset;
  }
}

void ZStackPresenter::optimizeStackBc()
{
  if (buddyDocument() != NULL) {
    ZStack *stack = buddyDocument()->getStack();
    if (stack != NULL) {
      if (!stack->isVirtual()) {
        if (stack->kind() != GREY) {
          double scale, offset;
          m_greyScale.resize(stack->channelNumber());
          m_greyOffset.resize(stack->channelNumber());
          for (int i=0; i<stack->channelNumber(); i++) {
            stack->bcAdjustHint(&scale, &offset, i);
            setStackBc(scale, offset, i);
          }
        }
      }
    }
  }
}

void ZStackPresenter::autoThreshold()
{
  QString id = "ZStackDoc::autoThreshold";
  if (!m_futureMap.isAlive(id)) {
    QFuture<void> future =
        QtConcurrent::run(buddyDocument(), &ZStackDoc::autoThreshold);
    m_futureMap[id] = future;
  }
  //buddyView()->setThreshold(buddyDocument()->autoThreshold());
}

void ZStackPresenter::binarizeStack()
{
  buddyDocument()->binarize(buddyView()->getIntensityThreshold());
}

void ZStackPresenter::solidifyStack()
{
  buddyDocument()->bwsolid();
}

/*
void ZStackPresenter::autoTrace()
{
  buddyDocument()->executeAutoTraceCommand(false);
}
*/

void ZStackPresenter::traceTube()
{
  //QPointF dataPos = stackPositionFromMouse(LEFT_RELEASE);
  buddyView()->setScreenCursor(Qt::BusyCursor);
  const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
        Qt::LeftButton, ZMouseEvent::ACTION_RELEASE);
  ZPoint pt = event.getStackPosition();
  if (buddyDocument()->getStack()->channelNumber() == 1) {
    if (event.getRawStackPosition().z() < 0) {
      buddyDocument()->executeTraceSwcBranchCommand(pt.x(), pt.y());
    } else {
      buddyDocument()->executeTraceSwcBranchCommand(pt.x(), pt.y(), pt.z());
    }

    /*
    buddyDocument()->executeTraceSwcBranchCommand(
          dataPos.x(), dataPos.y(), m_mouseLeftReleasePosition[2]);
          */
#if 0
    QUndoCommand *traceTubeCommand = new ZStackDocTraceTubeCommand(buddyDocument(),
                                                                   dataPos.x(), dataPos.y(),
                                                                   m_mouseLeftReleasePosition[2]);
    buddyDocument()->pushUndoCommand(traceTubeCommand);
#endif
  } else if (buddyDocument()->getStack()->channelNumber() > 1) {
    ChannelDialog dlg(NULL, buddyDocument()->getStack()->channelNumber());
    if (dlg.exec() == QDialog::Accepted) {
      int channel = dlg.channel();
      buddyDocument()->executeTraceTubeCommand(
            pt.x(), pt.y(), pt.z(), channel);
#if 0
      QUndoCommand *traceTubeCommand = new ZStackDocTraceTubeCommand(buddyDocument(),
                                                                     dataPos.x(), dataPos.y(),
                                                                     m_mouseLeftReleasePosition[2],
                                                                     channel);
      buddyDocument()->pushUndoCommand(traceTubeCommand);
#endif
    }
  }
  //buddyView()->setImageWidgetCursor(Qt::CrossCursor);
  updateCursor();
  //updateView();
}

void ZStackPresenter::fitSegment()
{
  QPointF dataPos = stackPositionFromMouse(LEFT_RELEASE);
  /*
  buddyDocument()->fitseg(dataPos.x(), dataPos.y(),
                          m_mouseLeftReleasePosition[2]);
                          */
  if (buddyDocument()->getStack()->depth() == 1) {
    buddyDocument()->fitRect(dataPos.x(), dataPos.y(),
            m_mouseLeftReleasePosition[2]);
  } else {
    buddyDocument()->fitseg(dataPos.x(), dataPos.y(),
                             m_mouseLeftReleasePosition[2]);
  }
}

void ZStackPresenter::fitEllipse()
{
  QPointF dataPos = stackPositionFromMouse(LEFT_RELEASE);
  buddyDocument()->fitEllipse(dataPos.x(), dataPos.y(),
                              m_mouseLeftReleasePosition[2]);
}

void ZStackPresenter::markPuncta()
{
  const ZMouseEvent& event = m_mouseEventProcessor.getMouseEvent(
        Qt::LeftButton, ZMouseEvent::ACTION_RELEASE);
  ZPoint currentStackPos = event.getPosition(NeuTube::COORD_STACK);
  buddyDocument()->markPunctum(currentStackPos.x(), currentStackPos.y(),
                               currentStackPos.z());

  /*
   *   QPoint pos(event.getPosition().getX(),
             event.getPosition().getY());

  QPointF dataPos = stackPositionFromMouse(LEFT_RELEASE);
  buddyDocument()->markPunctum(dataPos.x(), dataPos.y(),
                          m_mouseLeftReleasePosition[2]);
                          */
}

void ZStackPresenter::dropSegment()
{
  QPointF dataPos = stackPositionFromMouse(LEFT_RELEASE);
  buddyDocument()->dropseg(dataPos.x(), dataPos.y(),
                           m_mouseLeftReleasePosition[2]);
}


QStringList ZStackPresenter::toStringList() const
{
  QStringList list;
  list.append(QString("Number of decorations: %1")
              .arg(m_decorationList.size()));

  return list;
}

void ZStackPresenter::deleteSelected()
{
  buddyDocument()->executeRemoveSelectedObjectCommand();
#if 0
  QUndoCommand *removeselectedcommand =
      new ZStackDocRemoveSelectedObjectCommand(buddyDocument());
  buddyDocument()->pushUndoCommand(removeselectedcommand);
  //m_parent->undoStack()->push(removeselectedcommand);
  //buddyDocument()->removeSelectedObject(true);
  //updateView();
#endif
}

/*
void ZStackPresenter::deleteAllPuncta()
{
  buddyDocument()->deleteObject(ZStackObject::TYPE_PUNCTUM);
  updateView();
}
*/

void ZStackPresenter::enlargePuncta()
{
  buddyDocument()->expandSelectedPuncta();
  updateView();
}

void ZStackPresenter::narrowPuncta()
{
  buddyDocument()->shrinkSelectedPuncta();
  updateView();
}

void ZStackPresenter::meanshiftPuncta()
{
  buddyDocument()->meanshiftSelectedPuncta();
  updateView();
}

void ZStackPresenter::meanshiftAllPuncta()
{
  buddyDocument()->meanshiftAllPuncta();
  updateView();
}

void ZStackPresenter::updateStackBc()
{
  optimizeStackBc();
}



void ZStackPresenter::enterMouseCapturingMode()
{
  this->interactiveContext().setExploreMode(ZInteractiveContext::EXPLORE_CAPTURE_MOUSE);
}

void ZStackPresenter::enterSwcConnectMode()
{
  if (getParentFrame() != NULL) {
    getParentFrame()->notifyUser(
          "Connecting mode is on. Click on the target node to connect.");
    this->interactiveContext().setSwcEditMode(
          ZInteractiveContext::SWC_EDIT_CONNECT);
    updateCursor();
  }
}

void ZStackPresenter::updateSwcExtensionHint()
{
  if (isActiveObjectOn(ROLE_SWC)) {
    const Swc_Tree_Node *tn = getSelectedSwcNode();
    if (tn != NULL) {
      ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_SWC);
      stroke->set(SwcTreeNode::x(tn), SwcTreeNode::y(tn));
      stroke->setWidth(SwcTreeNode::radius(tn) * 2.0);
      QPointF pos = mapFromGlobalToStack(QCursor::pos());
      stroke->append(pos.x(), pos.y());
    }
  }
}

void ZStackPresenter::notifyUser(const QString &msg)
{
  if (getParentFrame() != NULL) {
    getParentFrame()->notifyUser(msg);
  }
}

bool ZStackPresenter::enterSwcExtendMode()
{
  bool succ = false;
  if (buddyDocument()->getSelectedSwcNodeNumber() == 1) {
    const Swc_Tree_Node *tn = getSelectedSwcNode();
    if (tn != NULL) {
      notifyUser("Left click to extend. Path calculation is off when 'Cmd/Ctrl' is held."
                 "Right click to exit extending mode.");

//      m_stroke.setFilled(false);
      QPointF pos = mapFromGlobalToStack(QCursor::pos());

      ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_SWC);
      stroke->set(SwcTreeNode::x(tn), SwcTreeNode::y(tn));
      stroke->append(pos.x(), pos.y());
      //m_stroke.set(SwcTreeNode::x(tn), SwcTreeNode::y(tn));
      stroke->setWidth(SwcTreeNode::radius(tn) * 2.0);

      turnOnActiveObject(ROLE_SWC);
//      turnOnStroke();
//      m_stroke.setTarget(ZStackObject::TARGET_WIDGET);
      interactiveContext().setSwcEditMode(ZInteractiveContext::SWC_EDIT_EXTEND);
      updateCursor();
      succ = true;
    }
  }

  return succ;
}

void ZStackPresenter::exitSwcExtendMode()
{
  if (interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_EXTEND ||
      interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_SMART_EXTEND) {
    interactiveContext().setSwcEditMode(ZInteractiveContext::SWC_EDIT_SELECT);
    enterSwcSelectMode();
    notifyUser("Exit extending mode");
  }
}

void ZStackPresenter::enterSwcMoveMode()
{
  const Swc_Tree_Node *tn = getSelectedSwcNode();
  if (tn != NULL) {
    interactiveContext().setSwcEditMode(
          ZInteractiveContext::SWC_EDIT_MOVE_NODE);
    notifyUser("Hold the Shift key and then move the mouse with left button pressed "
               "to move the selected nodes. Right click to exit moving mode.");
    updateCursor();
  }
}

void ZStackPresenter::enterSwcAddNodeMode(double x, double y)
{
  interactiveContext().setSwcEditMode(ZInteractiveContext::SWC_EDIT_ADD_NODE);
  ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_SWC);
  if (buddyDocument()->getTag() == NeuTube::Document::FLYEM_ROI) {
    stroke->setWidth(
          20.0 + imax2(buddyDocument()->getStack()->width(),
                       buddyDocument()->getStack()->height()) / 200);
  } else {
    stroke->setWidth(6.0);
  }
//  buddyDocument()->mapToDataCoord(&x, &y, NULL);
  stroke->set(x, y);

  turnOnActiveObject(ROLE_SWC);
//  m_stroke.setEraser(false);
//  m_stroke.setFilled(false);
//  m_stroke.setTarget(ZStackObject::TARGET_WIDGET);
//  turnOnStroke();
  //buddyView()->paintActiveDecoration();
  updateCursor();
}


void ZStackPresenter::toggleSwcSkeleton(bool state)
{
  buddyDocument()->showSwcFullSkeleton(state);
}

void ZStackPresenter::trySwcAddNodeMode()
{
  QPointF pos = mapFromGlobalToStack(QCursor::pos());
  trySwcAddNodeMode(pos.x(), pos.y());
}

void ZStackPresenter::trySwcAddNodeMode(double x, double y)
{
  if (interactiveContext().strokeEditMode() !=
      ZInteractiveContext::STROKE_DRAW) {
    enterSwcAddNodeMode(x, y);
  }
}

void ZStackPresenter::tryPaintStrokeMode()
{
  QPointF pos = mapFromGlobalToStack(QCursor::pos());
  tryDrawStrokeMode(pos.x(), pos.y(), false);
}

void ZStackPresenter::tryDrawRectMode()
{
  QPointF pos = mapFromGlobalToStack(QCursor::pos());
  tryDrawRectMode(pos.x(), pos.y());
}

void ZStackPresenter::tryEraseStrokeMode()
{
  QPointF pos = mapFromGlobalToStack(QCursor::pos());
  tryDrawStrokeMode(pos.x(), pos.y(), true);
}

void ZStackPresenter::tryDrawStrokeMode(double x, double y, bool isEraser)
{
  if (GET_APPLICATION_NAME == "Biocytin" ||
      GET_APPLICATION_NAME == "FlyEM") {
    if ((interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_OFF ||
        interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_SELECT) &&
        interactiveContext().tubeEditMode() == ZInteractiveContext::TUBE_EDIT_OFF &&
        interactiveContext().isRectEditModeOff()) {
      if (isEraser) {
        enterEraseStrokeMode(x, y);
      } else {
        enterDrawStrokeMode(x, y);
      }
    }
  }
}

void ZStackPresenter::tryDrawRectMode(double x, double y)
{
  if (GET_APPLICATION_NAME == "FlyEM") {
    if ((interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_OFF ||
         interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_SELECT) &&
        interactiveContext().tubeEditMode() == ZInteractiveContext::TUBE_EDIT_OFF &&
        interactiveContext().isStrokeEditModeOff()) {
      enterDrawRectMode(x, y);
    }
  } else if (buddyDocument()->getTag() == NeuTube::Document::BIOCYTIN_PROJECTION) {
    if ((interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_OFF ||
         interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_SELECT) &&
        interactiveContext().tubeEditMode() == ZInteractiveContext::TUBE_EDIT_OFF &&
        interactiveContext().isStrokeEditModeOff()) {
      enterDrawRectMode(x, y);
    }
  }
}

void ZStackPresenter::enterDrawStrokeMode(double x, double y)
{
//  buddyDocument()->mapToDataCoord(&x, &y, NULL);

  ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_STROKE);
  stroke->set(x, y);
  stroke->setEraser(false);
  turnOnActiveObject(ROLE_STROKE);
//  m_stroke.setFilled(true);
//  m_stroke.setTarget(ZStackObject::TARGET_OBJECT_CANVAS);
//  turnOnStroke();
  //buddyView()->paintActiveDecoration();
  interactiveContext().setStrokeEditMode(ZInteractiveContext::STROKE_DRAW);
  updateCursor();
}

void ZStackPresenter::enterDrawRectMode(double /*x*/, double /*y*/)
{
//  buddyDocument()->mapToDataCoord(&x, &y, NULL);
  interactiveContext().setRectEditMode(ZInteractiveContext::RECT_DRAW);
  updateCursor();
}

void ZStackPresenter::enterEraseStrokeMode(double x, double y)
{
//  buddyDocument()->mapToDataCoord(&x, &y, NULL);
  ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_STROKE);
  stroke->set(x, y);
//  m_stroke.setFilled(true);
  stroke->setEraser(true);
//  m_stroke.setTarget(ZStackObject::TARGET_OBJECT_CANVAS);
//  turnOnStroke();
  turnOnActiveObject(ROLE_STROKE);
  //buddyView()->paintActiveDecoration();
  interactiveContext().setStrokeEditMode(ZInteractiveContext::STROKE_DRAW);
  updateCursor();
}

void ZStackPresenter::exitStrokeEdit()
{
  turnOffActiveObject(ROLE_STROKE);
//  turnOffStroke();
  interactiveContext().setStrokeEditMode(ZInteractiveContext::STROKE_EDIT_OFF);
  updateCursor();

  m_interactiveContext.setExitingEdit(true);
}

#if 0
void ZStackPresenter::exitSwcEdit()
{
  turnOffActiveObject(ROLE_SWC);
  interactiveContext().setSwcEditMode(ZInteractiveContext::SWC_EDIT_SELECT);
  updateCursor();
  m_interactiveContext.setExitingEdit(true);
}
#endif

void ZStackPresenter::exitRectEdit()
{
  if (interactiveContext().rectEditMode() != ZInteractiveContext::RECT_EDIT_OFF) {
    interactiveContext().setRectEditMode(ZInteractiveContext::RECT_EDIT_OFF);
    updateCursor();

    m_interactiveContext.setExitingEdit(true);

    emit exitingRectEdit();
  }
}

void ZStackPresenter::exitBookmarkEdit()
{
  turnOffActiveObject(ROLE_BOOKMARK);
  interactiveContext().setBookmarkEditMode(ZInteractiveContext::BOOKMARK_EDIT_OFF);
  updateCursor();
  m_interactiveContext.setExitingEdit(true);
}

void ZStackPresenter::exitSynapseEdit()
{
  turnOffActiveObject(ROLE_SYNAPSE);
  interactiveContext().setSynapseEditMode(ZInteractiveContext::SYNAPSE_EDIT_OFF);
  updateCursor();
  m_interactiveContext.setExitingEdit(true);
}

void ZStackPresenter::deleteSwcNode()
{
  buddyDocument()->executeDeleteSwcNodeCommand();
}

void ZStackPresenter::lockSelectedSwcNodeFocus()
{
  this->interactiveContext().setSwcEditMode(
        ZInteractiveContext::SWC_EDIT_LOCK_FOCUS);
  buddyDocument()->executeSwcNodeChangeZCommand(buddyView()->sliceIndex());
  updateCursor();
}

void ZStackPresenter::changeSelectedSwcNodeFocus()
{
  buddyDocument()->executeSwcNodeChangeZCommand(buddyView()->sliceIndex());
}

void ZStackPresenter::estimateSelectedSwcRadius()
{
  buddyDocument()->executeSwcNodeEstimateRadiusCommand();
}

void ZStackPresenter::connectSelectedSwcNode()
{
  buddyDocument()->executeConnectSwcNodeCommand();
}

void ZStackPresenter::breakSelectedSwcNode()
{
  buddyDocument()->executeBreakSwcConnectionCommand();
}

void ZStackPresenter::processSliceChangeEvent(int z)
{
  if (this->interactiveContext().swcEditMode() ==
      ZInteractiveContext::SWC_EDIT_LOCK_FOCUS) {
    buddyDocument()->executeSwcNodeChangeZCommand(z);
  }
}

/*
void ZStackPresenter::bringSelectedToFront()
{
  buddyDocument()->bringChainToFront();
  updateView();
}

void ZStackPresenter::sendSelectedToBack()
{
  buddyDocument()->sendChainToBack();
  updateView();
}
*/
void ZStackPresenter::enterSwcSelectMode()
{
  if (m_interactiveContext.swcEditMode() != ZInteractiveContext::SWC_EDIT_OFF ||
      m_interactiveContext.swcEditMode() != ZInteractiveContext::SWC_EDIT_SELECT) {
    m_interactiveContext.setExitingEdit(true);
  }

  if (buddyDocument()->hasSwc()) {
    notifyUser("Use mouse to select nodes");
  }

  turnOffActiveObject(ROLE_SWC);
  m_interactiveContext.setSwcEditMode(ZInteractiveContext::SWC_EDIT_SELECT);
  updateCursor();
}

void ZStackPresenter::updateCursor()
{
  if (this->interactiveContext().swcEditMode() ==
      ZInteractiveContext::SWC_EDIT_EXTEND ||
      this->interactiveContext().swcEditMode() ==
      ZInteractiveContext::SWC_EDIT_SMART_EXTEND ||
      this->interactiveContext().rectEditMode() ==
      ZInteractiveContext::RECT_DRAW) {
    buddyView()->setScreenCursor(Qt::PointingHandCursor);
  } else if (this->interactiveContext().swcEditMode() ==
      ZInteractiveContext::SWC_EDIT_MOVE_NODE) {
    buddyView()->setScreenCursor(Qt::ClosedHandCursor);
  } else if (this->interactiveContext().swcEditMode() ==
             ZInteractiveContext::SWC_EDIT_LOCK_FOCUS) {
    buddyView()->setScreenCursor(Qt::ClosedHandCursor);
  } else if (interactiveContext().swcEditMode() ==
             ZInteractiveContext::SWC_EDIT_ADD_NODE ||
             interactiveContext().synapseEditMode() ==
             ZInteractiveContext::SYNAPSE_ADD_PRE ||
             interactiveContext().synapseEditMode() ==
             ZInteractiveContext::SYNAPSE_ADD_POST ||
             interactiveContext().synapseEditMode() ==
             ZInteractiveContext::SYNAPSE_MOVE){
    if (buddyDocument()->getTag() == NeuTube::Document::FLYEM_ROI) {
      buddyView()->setScreenCursor(Qt::PointingHandCursor);
    } else {
      buddyView()->setScreenCursor(Qt::PointingHandCursor);
    }
    /*
    buddyView()->setScreenCursor(
          ZCursorStore::getInstance().getSmallCrossCursor());
          */
  } else if (this->interactiveContext().tubeEditMode() ==
             ZInteractiveContext::TUBE_EDIT_HOOK ||
             interactiveContext().swcEditMode() ==
             ZInteractiveContext::SWC_EDIT_CONNECT) {
    buddyView()->setScreenCursor(Qt::SizeBDiagCursor);
  } else if (this->interactiveContext().strokeEditMode() ==
             ZInteractiveContext::STROKE_DRAW) {
    buddyView()->setScreenCursor(Qt::PointingHandCursor);
    /*
    buddyView()->setScreenCursor(
          ZCursorStore::getInstance().getSmallCrossCursor());
          */
    //buddyView()->setScreenCursor(Qt::PointingHandCursor);
  } else if (interactiveContext().bookmarkEditMode() ==
             ZInteractiveContext::BOOKMARK_ADD){
    buddyView()->setScreenCursor(Qt::PointingHandCursor);
  } else {
    buddyView()->setScreenCursor(Qt::CrossCursor);
  }
}

void ZStackPresenter::selectAllSwcTreeNode()
{
  buddyDocument()->selectAllSwcTreeNode();
}

void ZStackPresenter::slotTest()
{
  std::cout << "action triggered" << std::endl;
  buddyDocument()->selectDownstreamNode();
}

void ZStackPresenter::notifyBodySplitTriggered()
{
  emit bodySplitTriggered();
}

void ZStackPresenter::notifyOrthoViewTriggered()
{
  const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
        Qt::RightButton, ZMouseEvent::ACTION_RELEASE);
  ZPoint pt = event.getStackPosition();

//  ZPoint pt = getLastMousePosInStack();

  emit orthoViewTriggered(pt.x(), pt.y(), pt.z());
}

void ZStackPresenter::notifyBodyDecomposeTriggered()
{
  emit bodyDecomposeTriggered();
}

void ZStackPresenter::notifyBodyMergeTriggered()
{
  emit bodyMergeTriggered();
}

void ZStackPresenter::notifyBodyAnnotationTriggered()
{
  emit bodyAnnotationTriggered();
}

void ZStackPresenter::notifyBodyCheckinTriggered()
{
  emit bodyCheckinTriggered();
}

void ZStackPresenter::notifyBodyForceCheckinTriggered()
{
  emit bodyForceCheckinTriggered();
}

void ZStackPresenter::notifyBodyCheckoutTriggered()
{
  emit bodyCheckoutTriggered();
}

void ZStackPresenter::selectDownstreamNode()
{
  buddyDocument()->selectDownstreamNode();
}

void ZStackPresenter::selectSwcNodeConnection(Swc_Tree_Node *lastSelected)
{
  buddyDocument()->selectSwcNodeConnection(lastSelected);
}

void ZStackPresenter::selectUpstreamNode()
{
  buddyDocument()->selectUpstreamNode();
}

void ZStackPresenter::selectBranchNode()
{
  buddyDocument()->selectBranchNode();
}

void ZStackPresenter::selectTreeNode()
{
  buddyDocument()->selectTreeNode();
}

void ZStackPresenter::selectConnectedNode()
{
  buddyDocument()->selectConnectedNode();
}

void ZStackPresenter::processRectRoiUpdate(ZRect2d *rect, bool appending)
{
  buddyDocument()->processRectRoiUpdate(rect, appending);
}

void ZStackPresenter::acceptRectRoi(bool appending)
{
  ZStackObject *obj = buddyDocument()->getObjectGroup().findFirstSameSource(
        ZStackObject::TYPE_RECT2D,
        ZStackObjectSourceFactory::MakeRectRoiSource());
  ZRect2d *rect = dynamic_cast<ZRect2d*>(obj);
  if (rect != NULL) {
    rect->setColor(QColor(255, 255, 255));
    processRectRoiUpdate(rect, appending);
  }

//  exitRectEdit();
}

void ZStackPresenter::processEvent(ZInteractionEvent &event)
{
  switch (event.getEvent()) {
  case ZInteractionEvent::EVENT_SWC_NODE_SELECTED:
    if (buddyDocument()->getSelectedSwcNodeNumber() != 1) {
      exitSwcExtendMode();
    }
    buddyView()->redrawObject();
    break;
  case ZInteractionEvent::EVENT_VIEW_PROJECTION:
  case ZInteractionEvent::EVENT_VIEW_SLICE:
    updateView();
    emit(viewModeChanged());
    break;
  case ZInteractionEvent::EVENT_ACTIVE_DECORATION_UPDATED:
    buddyView()->paintActiveDecoration();
    break;
  case ZInteractionEvent::EVENT_STROKE_SELECTED:
  case ZInteractionEvent::EVENT_ALL_OBJECT_DESELCTED:
  case ZInteractionEvent::EVENT_OBJ3D_SELECTED:
  case ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED:
  case ZInteractionEvent::EVENT_OBJECT_SELECTED:
  case ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED_IN_LABEL_SLICE:
    buddyView()->redrawObject();
    buddyDocument()->notifySelectorChanged();
    if (event.getEvent() ==
        ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED_IN_LABEL_SLICE) {
      emit labelSliceSelectionChanged();
    }
    break;
  default:
    break;
  }
  notifyUser(event.getMessage());
  event.setEvent(ZInteractionEvent::EVENT_NULL);
}

void ZStackPresenter::setViewMode(ZInteractiveContext::ViewMode mode)
{
  m_interactiveContext.setViewMode(mode);
  emit viewModeChanged();
}

void ZStackPresenter::processCustomOperator(
    const ZStackOperator &/*op*/, ZInteractionEvent */*e*/)
{

}

bool ZStackPresenter::hasDrawable(ZStackObject::ETarget target) const
{
  for (QList<ZStackObject*>::const_iterator iter = m_decorationList.begin();
       iter != m_decorationList.end(); ++iter) {
    const ZStackObject *obj = *iter;
    if (obj->getTarget() == target) {
      return true;
    }
  }

  return false;
}

NeuTube::EAxis ZStackPresenter::getSliceAxis() const
{
  return buddyView()->getSliceAxis();
}
#if defined (_FLYEM_)
static void SyncDvidLabelSliceSelection(
    ZStackDoc *doc, ZDvidLabelSlice *labelSlice)
{
  QList<ZDvidLabelSlice*> sliceList = doc->getObjectList<ZDvidLabelSlice>();
  for (QList<ZDvidLabelSlice*>::iterator iter = sliceList.begin();
       iter != sliceList.end(); ++iter) {
    ZDvidLabelSlice *buddySlice = *iter;
    if (buddySlice != labelSlice) {
      const std::set<uint64_t> &selectedSet =
          labelSlice->getSelectedOriginal();
      buddySlice->setSelection(selectedSet, NeuTube::BODY_LABEL_ORIGINAL);
    }
  }
}
#endif
void ZStackPresenter::process(ZStackOperator &op)
{
  ZInteractionEvent interactionEvent;
  const ZMouseEvent& event = m_mouseEventProcessor.getLatestMouseEvent();
  QPoint currentWidgetPos(event.getPosition().getX(),
             event.getPosition().getY());
  ZPoint currentStackPos = event.getPosition(NeuTube::COORD_STACK);
  ZPoint currentRawStackPos = event.getPosition(NeuTube::COORD_RAW_STACK);

  buddyDocument()->getObjectGroup().resetSelection();

  switch (op.getOperation()) {
  case ZStackOperator::OP_IMAGE_MOVE_DOWN:
    moveViewPort(0, -1);
    break;
  case ZStackOperator::OP_IMAGE_MOVE_DOWN_FAST:
    moveViewPort(0, -10);
    break;
  case ZStackOperator::OP_IMAGE_MOVE_UP:
    moveViewPort(0, 1);
    break;
  case ZStackOperator::OP_IMAGE_MOVE_UP_FAST:
    moveViewPort(0, 10);
    break;
  case ZStackOperator::OP_IMAGE_MOVE_LEFT:
    moveViewPort(1, 0);
    break;
  case ZStackOperator::OP_IMAGE_MOVE_LEFT_FAST:
    moveViewPort(10, 0);
    break;
  case ZStackOperator::OP_IMAGE_MOVE_RIGHT:
    moveViewPort(-1, 0);
    break;
  case ZStackOperator::OP_IMAGE_MOVE_RIGHT_FAST:
    moveViewPort(-10, 0);
    break;
  case ZStackOperator::OP_STACK_DEC_SLICE:
    buddyView()->setSliceIndex(buddyView()->sliceIndex() - 1);
    break;
  case ZStackOperator::OP_STACK_INC_SLICE:
    buddyView()->setSliceIndex(buddyView()->sliceIndex() + 1);
    break;
  case ZStackOperator::OP_STACK_DEC_SLICE_FAST:
    buddyView()->setSliceIndex(buddyView()->sliceIndex() - 10);
    break;
  case ZStackOperator::OP_STACK_INC_SLICE_FAST:
    buddyView()->setSliceIndex(buddyView()->sliceIndex() + 10);
    break;
  case ZStackOperator::OP_SWC_ENTER_ADD_NODE:
  {
//    QPointF pos = mapFromGlobalToStack(QCursor::pos());
    trySwcAddNodeMode(currentStackPos.x(), currentStackPos.y());
  }
    break;
  case ZStackOperator::OP_SWC_ENTER_EXTEND_NODE:
    enterSwcExtendMode();
    break;
  case ZStackOperator::OP_SWC_DELETE_NODE:
    buddyDocument()->executeDeleteSwcNodeCommand();
    if (m_interactiveContext.swcEditMode() ==
        ZInteractiveContext::SWC_EDIT_EXTEND) {
      exitSwcExtendMode();
    }
    break;
  case ZStackOperator::OP_SWC_SELECT_SINGLE_NODE:
    buddyDocument()->recordSwcTreeNodeSelection();
    buddyDocument()->deselectAllSwcTreeNodes();
    buddyDocument()->selectHitSwcTreeNode(op.getHitObject<ZSwcTree>());
    buddyDocument()->notifySwcTreeNodeSelectionChanged();

    if (buddyDocument()->getSelectedSwcNodeNumber() == 1) {
      if (buddyDocument()->getTag() != NeuTube::Document::BIOCYTIN_PROJECTION) {
        if (NeutubeConfig::getInstance().getApplication() == "Biocytin" ||
            buddyDocument()->getTag() == NeuTube::Document::FLYEM_PROOFREAD) {
          enterSwcExtendMode();
        }
      }
    }
//    interactionEvent.setEvent(ZInteractionEvent::EVENT_SWC_NODE_SELECTED);
    break;
  case ZStackOperator::OP_SWC_DESELECT_ALL_NODE:
    buddyDocument()->deselectAllSwcTreeNodes();
    break;
  case ZStackOperator::OP_SWC_DESELECT_SINGLE_NODE:
    buddyDocument()->deselectHitSwcTreeNode(op.getHitObject<ZSwcTree>());
    //buddyDocument()->setSwcTreeNodeSelected(op.getHitSwcNode(), false);
    break;
  case ZStackOperator::OP_SWC_SELECT_MULTIPLE_NODE:
    buddyDocument()->selectHitSwcTreeNode(op.getHitObject<ZSwcTree>(), true);
    //buddyDocument()->setSwcTreeNodeSelected(op.getHitSwcNode(), true);
    interactionEvent.setEvent(ZInteractionEvent::EVENT_SWC_NODE_SELECTED);
    break;
  case ZStackOperator::OP_SWC_SELECT_CONNECTION:
    buddyDocument()->selectHitSwcTreeNodeConnection(
          op.getHitObject<ZSwcTree>());
    //buddyDocument()->setSwcTreeNodeSelected(op.getHitSwcNode(), true);
    //buddyDocument()->selectSwcNodeConnection(op.getHitSwcNode());
    interactionEvent.setEvent(ZInteractionEvent::EVENT_SWC_NODE_SELECTED);
    break;
  case ZStackOperator::OP_SWC_SELECT_FLOOD:
    buddyDocument()->selectHitSwcTreeNodeFloodFilling(
          op.getHitObject<ZSwcTree>());
    /*
    buddyDocument()->selectSwcTreeNode(op.getHitSwcNode(), true);
    buddyDocument()->selectSwcNodeFloodFilling(op.getHitSwcNode());
    */
    interactionEvent.setEvent(ZInteractionEvent::EVENT_SWC_NODE_SELECTED);
    break;
  case ZStackOperator::OP_SWC_BREAK_NODE:
    buddyDocument()->executeBreakSwcConnectionCommand();
    break;
  case ZStackOperator::OP_SWC_CONNECT_TO:
  {
    if (buddyDocument()->hasSelectedSwcNode()) {
      std::set<Swc_Tree_Node*> nodeSet =
          buddyDocument()->getSelectedSwcNodeSet();
      Swc_Tree_Node *prevNode = *(nodeSet.begin());
      if (prevNode != NULL) {
        Swc_Tree_Node *tn = op.getHitObject<Swc_Tree_Node>();
        if (tn != NULL) {
          if (buddyDocument()->executeConnectSwcNodeCommand(prevNode, tn)) {
            enterSwcSelectMode();
            //status = MOUSE_COMMAND_EXECUTED;
          }
        }
      }
    }
  }
    break;
  case ZStackOperator::OP_SWC_EXTEND:
  {
    ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_SWC);
    if (buddyDocument()->executeSwcNodeExtendCommand(
          m_mouseEventProcessor.getLatestStackPosition(),
          stroke->getWidth() / 2.0)) {
      interactionEvent.setEvent(ZInteractionEvent::EVENT_SWC_NODE_EXTENDED);
    }
  }
    break;
  case ZStackOperator::OP_SWC_SMART_EXTEND:
    if (buddyDocument()->hasStackData()) {
      ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_SWC);
      if (buddyDocument()->executeSwcNodeSmartExtendCommand(
            m_mouseEventProcessor.getLatestStackPosition(),
            stroke->getWidth() / 2.0)) {
        //status = MOUSE_COMMAND_EXECUTED;
        interactionEvent.setEvent(ZInteractionEvent::EVENT_SWC_NODE_EXTENDED);
      }
    } else {
      ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_SWC);
      if (buddyDocument()->executeSwcNodeExtendCommand(
            m_mouseEventProcessor.getLatestStackPosition(),
            stroke->getWidth() / 2.0)) {
        interactionEvent.setEvent(ZInteractionEvent::EVENT_SWC_NODE_EXTENDED);
      }
    }
    break;
  case ZStackOperator::OP_SWC_CONNECT_NODE_SMART:
    if (buddyDocument()->hasStackData()) {
      if (buddyDocument()->executeSmartConnectSwcNodeCommand()) {
        interactionEvent.setEvent(ZInteractionEvent::EVENT_SWC_NODE_ADDED);
      }
    }
    break;
  case ZStackOperator::OP_SWC_SET_AS_ROOT:
    buddyDocument()->executeSetRootCommand();
    break;
  case ZStackOperator::OP_SWC_ADD_NODE:
  {
    ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_SWC);
    if (buddyDocument()->executeAddSwcNodeCommand(
          m_mouseEventProcessor.getLatestStackPosition(),
          stroke->getWidth() / 2.0)) {
      //status = MOUSE_COMMAND_EXECUTED;
      if (buddyDocument()->getTag() == NeuTube::Document::FLYEM_ROI) {
        buddyDocument()->selectSwcTreeNode(
              m_mouseEventProcessor.getLatestStackPosition(), false);
        enterSwcExtendMode();
      }
      interactionEvent.setEvent(ZInteractionEvent::EVENT_SWC_NODE_ADDED);
    }
  }
    break;
  case ZStackOperator::OP_SWC_ZOOM_TO_SELECTED_NODE:
    buddyDocument()->notifyZoomingToSelectedSwcNode();
    /*
    if (getParentFrame() != NULL) {
      getParentFrame()->zoomToSelectedSwcNodes();
    }
    */
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_UP:
    buddyDocument()->executeMoveSwcNodeCommand(0, -1.0, 0);
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_UP_FAST:
    buddyDocument()->executeMoveSwcNodeCommand(0, -10.0, 0);
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_LEFT:
    buddyDocument()->executeMoveSwcNodeCommand(-1.0, 0, 0);
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_LEFT_FAST:
    buddyDocument()->executeMoveSwcNodeCommand(-10.0, 0, 0);
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_DOWN:
    buddyDocument()->executeMoveSwcNodeCommand(0, 1.0, 0);
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_DOWN_FAST:
    buddyDocument()->executeMoveSwcNodeCommand(0, 10.0, 0);
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_RIGHT:
    buddyDocument()->executeMoveSwcNodeCommand(1.0, 0, 0);
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_RIGHT_FAST:
    buddyDocument()->executeMoveSwcNodeCommand(10.0, 0, 0);
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE:
    enterSwcMoveMode();
    break;
  case ZStackOperator::OP_SWC_INSERT_NODE:
    buddyDocument()->executeInsertSwcNode();
    break;
  case ZStackOperator::OP_SWC_INCREASE_NODE_SIZE:
//    if (isStrokeOff()) {
      buddyDocument()->executeSwcNodeChangeSizeCommand(0.5);
//    }
    break;
  case ZStackOperator::OP_SWC_DECREASE_NODE_SIZE:
//    if (isStrokeOff()) {
      buddyDocument()->executeSwcNodeChangeSizeCommand(-0.5);
//    }
    break;
  case ZStackOperator::OP_SWC_CONNECT_NODE:
    if (buddyDocument()->hasSelectedSwcNode()) {
      if (buddyDocument()->getSelectedSwcNodeNumber() == 1) {
        enterSwcConnectMode();
        //taken = true;
      } else {
        buddyDocument()->executeConnectSwcNodeCommand();
      }
    }
    break;
  case ZStackOperator::OP_SWC_CONNECT_ISOLATE:
    buddyDocument()->executeConnectIsolatedSwc();
    break;
  case ZStackOperator::OP_SWC_SELECT_ALL_NODE:
    getAction(ZActionFactory::ACTION_SELECT_ALL_SWC_NODE)->trigger();
//    m_selectAllSwcNodeAction->trigger();
    break;
  case ZStackOperator::OP_RESTORE_EXPLORE_MODE:
    this->interactiveContext().restoreExploreMode();
    buddyView()->processViewChange(false, false);
//    buddyView()->notifyViewChanged();
    buddyView()->redraw();
//    buddyView()->notifyViewPortChanged();
    break;
  case ZStackOperator::OP_SHOW_CONTEXT_MENU:
    buddyView()->showContextMenu(getContextMenu(), currentWidgetPos);
    //status = CONTEXT_MENU_POPPED;
    break;
  case ZStackOperator::OP_SHOW_SWC_CONTEXT_MENU:
    if (getSwcNodeContextMenu()->isHidden()) {
      buddyView()->showContextMenu(getSwcNodeContextMenu(), currentWidgetPos);
    }
    //status = CONTEXT_MENU_POPPED;
    break;
  case ZStackOperator::OP_SWC_CHANGE_NODE_FOCUS:
    changeSelectedSwcNodeFocus();
    break;
  case ZStackOperator::OP_SHOW_STROKE_CONTEXT_MENU:
    buddyView()->showContextMenu(getStrokeContextMenu(), currentWidgetPos);
    //status = CONTEXT_MENU_POPPED;
    break;
  case ZStackOperator::OP_SHOW_TRACE_CONTEXT_MENU:
    if (buddyDocument()->hasTracable()) {
      if (m_interactiveContext.isContextMenuActivated()) {
        if (buddyView()->popLeftMenu(currentWidgetPos)) {
          m_interactiveContext.blockContextMenu();
          //status = CONTEXT_MENU_POPPED;
        }
      } else {
        m_interactiveContext.blockContextMenu(false);
      }
    }
    break;
  case ZStackOperator::OP_SHOW_PUNCTA_CONTEXT_MENU:
    buddyView()->rightMenu()->clear();
    addPunctaEditFunctionToRightMenu();
    buddyView()->popRightMenu(currentWidgetPos);
    break;
  case ZStackOperator::OP_SHOW_BODY_CONTEXT_MENU:
    buddyView()->showContextMenu(getBodyContextMenu(), currentWidgetPos);
    break;
  case ZStackOperator::OP_EXIT_EDIT_MODE:
    turnOffActiveObject();
    /*
    if (isStrokeOn()) {
      turnOffStroke();
    }
    */
    exitStrokeEdit();
    exitRectEdit();
    exitBookmarkEdit();
    exitSynapseEdit();
    enterSwcSelectMode();
    break;
  case ZStackOperator::OP_PUNCTA_SELECT_SINGLE:
    buddyDocument()->deselectAllPuncta();
    buddyDocument()->setSelected(op.getHitObject<ZPunctum>(), true);
    interactionEvent.setEvent(
          ZInteractionEvent::EVENT_OBJECT_SELECTED);
    //buddyDocument()->selectPuncta(op.getPunctaIndex());
    break;
  case ZStackOperator::OP_PUNCTA_SELECT_MULTIPLE:
    buddyDocument()->setSelected(op.getHitObject<ZPunctum>(), true);
    interactionEvent.setEvent(
          ZInteractionEvent::EVENT_OBJECT_SELECTED);
    //buddyDocument()->selectPuncta(op.getPunctaIndex());
    break;
  case ZStackOperator::OP_SHOW_PUNCTA_MENU:
    if (m_interactiveContext.isContextMenuActivated()) {
      if (buddyView()->popLeftMenu(currentWidgetPos)) {
        m_interactiveContext.blockContextMenu();
      }
    } else {
      m_interactiveContext.blockContextMenu(false);
    }
    break;
  case ZStackOperator::OP_DESELECT_ALL:
    buddyDocument()->deselectAllObject();
    interactionEvent.setEvent(ZInteractionEvent::EVENT_ALL_OBJECT_DESELCTED);
    break;
  case ZStackOperator::OP_OBJECT_SELECT_SINGLE:
    buddyDocument()->deselectAllObject(false);
    if (op.getHitObject<ZStackObject>() != NULL) {
      buddyDocument()->setSelected(op.getHitObject<ZStackObject>(), true);
      interactionEvent.setEvent(
            ZInteractionEvent::EVENT_OBJECT_SELECTED);
    }
    break;
  case ZStackOperator::OP_BOOKMARK_SELECT_SIGNLE:
    buddyDocument()->deselectAllObject(false);
//    buddyDocument()->deselectAllObject(ZStackObject::TYPE_FLYEM_BOOKMARK);
    if (op.getHitObject<ZStackObject>() != NULL) {
      buddyDocument()->setSelected(op.getHitObject<ZStackObject>(), true);
      interactionEvent.setEvent(
            ZInteractionEvent::EVENT_OBJECT_SELECTED);
    }
    break;
  case ZStackOperator::OP_OBJECT_SELECT_MULTIPLE:
    if (op.getHitObject<ZStackObject>() != NULL) {
      buddyDocument()->toggleSelected(op.getHitObject<ZStackObject>());
//      buddyDocument()->setSelected(op.getHitObject<ZStackObject>(), true);
      interactionEvent.setEvent(
            ZInteractionEvent::EVENT_OBJECT_SELECTED);
    }
    break;
  case ZStackOperator::OP_STROKE_SELECT_SINGLE:
    buddyDocument()->deselectAllObject();
    if (op.getHitObject<ZStroke2d>() != NULL) {
      buddyDocument()->setSelected(op.getHitObject<ZStroke2d>(), true);
      interactionEvent.setEvent(
            ZInteractionEvent::EVENT_STROKE_SELECTED);
    }
    break;
  case ZStackOperator::OP_OBJECT_TOGGLE_VISIBILITY:
    toggleObjectVisible();
    break;
  case ZStackOperator::OP_OBJECT3D_SCAN_SELECT_SINGLE:
    buddyDocument()->deselectAllObject();
    if (op.getHitObject<ZObject3dScan>() != NULL) {
      buddyDocument()->setSelected(op.getHitObject<ZObject3dScan>());
      interactionEvent.setEvent(ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED);
    }
#if defined (_FLYEM_)
    else if (op.getHitObject<ZDvidLabelSlice>() != NULL) {
      ZDvidLabelSlice *labelSlice =  op.getHitObject<ZDvidLabelSlice>();
      labelSlice->recordSelection();
      labelSlice->selectHit();
      labelSlice->processSelection();
      SyncDvidLabelSliceSelection(buddyDocument(), labelSlice);
      interactionEvent.setEvent(
            ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED_IN_LABEL_SLICE);
    }
#endif
    break;
  case ZStackOperator::OP_OBJECT3D_SCAN_SELECT_MULTIPLE:
    if (op.getHitObject<ZObject3dScan>() != NULL) {
      buddyDocument()->setSelected(op.getHitObject<ZObject3dScan>());
      interactionEvent.setEvent(ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED);
    }
#if defined (_FLYEM_)
    else if (op.getHitObject<ZDvidLabelSlice>() != NULL) {
      ZDvidLabelSlice *labelSlice =  op.getHitObject<ZDvidLabelSlice>();
      labelSlice->recordSelection();
      labelSlice->selectHit(true);
      labelSlice->processSelection();
      SyncDvidLabelSliceSelection(buddyDocument(), labelSlice);
      interactionEvent.setEvent(
            ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED_IN_LABEL_SLICE);
    }
#endif
    break;
  case ZStackOperator::OP_OBJECT3D_SCAN_TOGGLE_SELECT:
    if (op.getHitObject<ZObject3dScan>() != NULL) {
      buddyDocument()->toggleSelected(op.getHitObject<ZObject3dScan>());
      interactionEvent.setEvent(ZInteractionEvent::EVENT_OBJ3D_SELECTED);
    } else {
#if defined (_FLYEM_)
      ZDvidLabelSlice *labelSlice =  op.getHitObject<ZDvidLabelSlice>();
      if (labelSlice != NULL) {
        labelSlice->recordSelection();
        labelSlice->toggleHitSelection(true);
        labelSlice->processSelection();
        SyncDvidLabelSliceSelection(buddyDocument(), labelSlice);
        interactionEvent.setEvent(
              ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED_IN_LABEL_SLICE);
      }
#endif
    }
    break;
  case ZStackOperator::OP_OBJECT3D_SCAN_TOGGLE_SELECT_SINGLE:
  {
    if (!op.getHitObject()->isSelected()) {
      ZObject3dScan *obj = op.getHitObject<ZObject3dScan>();
      if (obj != NULL) {
        buddyDocument()->deselectAllObject();

        buddyDocument()->setSelected(obj);

        notifyUser(QString("%1 (%2)").
                   arg(obj->getSource().c_str()).arg(obj->getLabel()));
        interactionEvent.setEvent(ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED);
      }
#if defined (_FLYEM_)
      else if (op.getHitObject<ZDvidLabelSlice>() != NULL) {
        buddyDocument()->deselectAllObject(false);
        buddyDocument()->deselectAllSwcTreeNodes();
//        op.getHitObject<ZDvidLabelSlice>()->clearSelection();

        ZDvidLabelSlice *labelSlice =  op.getHitObject<ZDvidLabelSlice>();
        labelSlice->recordSelection();
        op.getHitObject<ZDvidLabelSlice>()->toggleHitSelection(false);
        labelSlice->processSelection();
        SyncDvidLabelSliceSelection(buddyDocument(), labelSlice);
        interactionEvent.setEvent(
              ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED_IN_LABEL_SLICE);
      }
#endif
    } else {
      buddyDocument()->deselectAllObject();
      interactionEvent.setEvent(ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED);
    }

  }
    break;
  case ZStackOperator::OP_OBJECT3D_SELECT_SINGLE:
    buddyDocument()->deselectAllObject();
    buddyDocument()->setSelected(op.getHitObject<ZObject3d>());
    interactionEvent.setEvent(ZInteractionEvent::EVENT_OBJ3D_SELECTED);
    break;
  case ZStackOperator::OP_OBJECT3D_SELECT_MULTIPLE:
    buddyDocument()->setSelected(op.getHitObject<ZObject3d>());
    interactionEvent.setEvent(ZInteractionEvent::EVENT_OBJ3D_SELECTED);
    break;
  case ZStackOperator::OP_STROKE_SELECT_MULTIPLE:
    if (op.getHitObject<ZStroke2d>() != NULL) {
      buddyDocument()->setSelected(op.getHitObject<ZStroke2d>(), true);
      interactionEvent.setEvent(
            ZInteractionEvent::EVENT_STROKE_SELECTED);
    }
    break;
  case ZStackOperator::OP_STROKE_ADD_NEW:
    acceptActiveStroke();
    LINFO() << "Add painted mask stroke";
    break;
  case ZStackOperator::OP_STROKE_START_PAINT:
  {
    LINFO() << "Start painting mask";
    ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_STROKE);
    stroke->set(currentStackPos.x(), currentStackPos.y());
          //m_mouseEventProcessor.getLatestStackPosition().x(),
            //    m_mouseEventProcessor.getLatestStackPosition().y());
  }
    break;
  case ZStackOperator::OP_MOVE_OBJECT:
  {
    ZPoint offset = op.getMouseEventRecorder()->
        getPositionOffset(NeuTube::COORD_STACK);

    buddyDocument()->executeMoveObjectCommand(offset.x(), offset.y(),
                                              0,
                                              1,1,1,1,1,1);
  }
    break;
  case ZStackOperator::OP_MOVE_IMAGE:
  {
    Qt::MouseButtons grabButton = op.getPressedButtons();
    if (grabButton == Qt::NoButton || (grabButton & Qt::LeftButton)) {
      grabButton = Qt::LeftButton;
    }
    ZPoint grabPosition = op.getMouseEventRecorder()->getPosition(
          grabButton, ZMouseEvent::ACTION_PRESS, NeuTube::COORD_STACK);
    grabPosition.shiftSliceAxis(getSliceAxis());
    moveImageToMouse(
          grabPosition.x(), grabPosition.y(),
          currentWidgetPos.x(), currentWidgetPos.y());
  }
    break;
  case ZStackOperator::OP_ZOOM_IN:
    increaseZoomRatio();
    break;
  case ZStackOperator::OP_ZOOM_OUT:
    decreaseZoomRatio();
    break;
  case ZStackOperator::OP_ZOOM_IN_GRAB_POS:
  {
    m_interactiveContext.blockContextMenu();
    ZPoint grabPosition = op.getMouseEventRecorder()->getPosition(
          Qt::RightButton, ZMouseEvent::ACTION_PRESS,
          NeuTube::COORD_WIDGET);
    m_interactiveContext.setExploreMode(
          ZInteractiveContext::EXPLORE_ZOOM_IN_IMAGE);
//    buddyView()->blockViewChangeEvent(true);
    increaseZoomRatio(grabPosition.x(), grabPosition.y());
//    buddyView()->blockViewChangeEvent(false);
  }
    break;
  case ZStackOperator::OP_ZOOM_OUT_GRAB_POS:
  {
    m_interactiveContext.blockContextMenu();
    ZPoint grabPosition = op.getMouseEventRecorder()->getPosition(
          Qt::RightButton, ZMouseEvent::ACTION_PRESS,
          NeuTube::COORD_WIDGET);
    m_interactiveContext.setExploreMode(ZInteractiveContext::EXPLORE_ZOOM_OUT_IMAGE);
//    buddyView()->blockViewChangeEvent(true);
    decreaseZoomRatio(grabPosition.x(), grabPosition.y());
//    buddyView()->blockViewChangeEvent(false);
  }
    break;
  case ZStackOperator::OP_EXIT_ZOOM_MODE:
    m_interactiveContext.setExploreMode(ZInteractiveContext::EXPLORE_OFF);
    buddyView()->processViewChange(true, false);
    buddyView()->imageWidget()->update();
//    buddyView()->notifyViewChanged();
    break;
  case ZStackOperator::OP_PAINT_STROKE:
  {
    ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_STROKE);
    stroke->append(currentStackPos.x(), currentStackPos.y());
    buddyView()->paintActiveDecoration();
  }
    break;
  case ZStackOperator::OP_RECT_ROI_INIT:
  {
    ZRect2d *rect = new ZRect2d(currentStackPos.x(), currentStackPos.y(),
                                0, 0);
    rect->setSource(ZStackObjectSourceFactory::MakeRectRoiSource());
    rect->setPenetrating(true);
    rect->setZ(buddyView()->getCurrentZ());
    rect->setColor(255, 128, 128);

#ifdef _DEBUG_
    std::cout << "Adding roi: " << rect << std::endl;
#endif
//    buddyDocument()->removeObject(rect->getSource(), false);

    ZStackObject *obj = buddyDocument()->getObjectGroup().findFirstSameSource(
          ZStackObject::TYPE_RECT2D,
          ZStackObjectSourceFactory::MakeRectRoiSource());
    ZRect2d *oldRect = dynamic_cast<ZRect2d*>(obj);
    if (oldRect != NULL) {
      buddyDocument()->executeRemoveObjectCommand(obj);
    }

    buddyDocument()->addObject(rect, false); //Undo will be handled after roi accepted

//    buddyDocument()->executeAddObjectCommand(rect);
  }
    break;
  case ZStackOperator::OP_RECT_ROI_UPDATE:
  {
    ZStackObject *obj = buddyDocument()->getObjectGroup().findFirstSameSource(
          ZStackObject::TYPE_RECT2D,
          ZStackObjectSourceFactory::MakeRectRoiSource());
    ZRect2d *rect = dynamic_cast<ZRect2d*>(obj);
    if (rect != NULL) {
      ZPoint grabPosition = op.getMouseEventRecorder()->getPosition(
            Qt::LeftButton, ZMouseEvent::ACTION_PRESS, NeuTube::COORD_STACK);
      grabPosition.shiftSliceAxis(getSliceAxis());
      ZPoint shiftedStackPos = currentStackPos;
      shiftedStackPos.shiftSliceAxis(getSliceAxis());

      int x0 = std::min(grabPosition.x(), shiftedStackPos.x());
      int y0 = std::min(grabPosition.y(), shiftedStackPos.y());

      int x1 = std::max(grabPosition.x(), shiftedStackPos.x());
      int y1 = std::max(grabPosition.y(), shiftedStackPos.y());

      rect->setFirstCorner(x0, y0);
      rect->setLastCorner(x1, y1);
      buddyDocument()->processObjectModified(rect);
      buddyDocument()->notifyObjectModified();
    }
  }
    break;
  case ZStackOperator::OP_RECT_ROI_APPEND:
    acceptRectRoi(true);
    exitRectEdit();
    break;
  case ZStackOperator::OP_RECT_ROI_ACCEPT:
    acceptRectRoi(false);
    exitRectEdit();
    break;
  case ZStackOperator::OP_START_MOVE_IMAGE:
    //if (buddyView()->imageWidget()->zoomRatio() > 1) {
    if (buddyView()->isImageMovable()) {
      this->interactiveContext().backupExploreMode();
      this->interactiveContext().
          setExploreMode(ZInteractiveContext::EXPLORE_MOVE_IMAGE);
      //m_grabPosition = buddyView()->screen()->canvasCoordinate(event->pos());
    }
    break;
  case ZStackOperator::OP_OBJECT_TOGGLE_TMP_RESULT_VISIBILITY:
    buddyDocument()->toggleVisibility(ZStackObjectRole::ROLE_TMP_RESULT);
    break;
  case ZStackOperator::OP_TRACK_MOUSE_MOVE:
    buddyView()->setInfo(
          buddyDocument()->rawDataInfo(
            currentRawStackPos.x(), currentRawStackPos.y(),
            currentRawStackPos.z()));

    if (m_interactiveContext.synapseEditMode() ==
        ZInteractiveContext::SYNAPSE_EDIT_OFF) {
      ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(getFirstOnActiveObject());
      //    if (isStrokeOn()) {
      if (stroke != NULL) {
        ZPoint pt = currentStackPos;
        pt.shiftSliceAxis(getSliceAxis());
        if (m_interactiveContext.swcEditMode() ==
            ZInteractiveContext::SWC_EDIT_EXTEND ||
            m_interactiveContext.swcEditMode() ==
            ZInteractiveContext::SWC_EDIT_SMART_EXTEND) {
          const Swc_Tree_Node *tn = getSelectedSwcNode();
          if (tn != NULL) {
            ZPoint prevPos = SwcTreeNode::center(tn);
            prevPos.shiftSliceAxis(getSliceAxis());
            stroke->set(prevPos.x(), prevPos.y());
            stroke->append(pt.x(), pt.y());
          }
        } else if (m_interactiveContext.strokeEditMode() ==
                   ZInteractiveContext::STROKE_DRAW) {
          stroke->toggleLabel(op.togglingStrokeLabel());
        }
        stroke->setLast(pt.x(), pt.y());
        interactionEvent.setEvent(
              ZInteractionEvent::EVENT_ACTIVE_DECORATION_UPDATED);
        //turnOnStroke();
      }
      op.setOperation(ZStackOperator::OP_NULL);
    }
    break;

  case ZStackOperator::OP_STACK_LOCATE_SLICE:
    if (buddyDocument()->hasStackData()) {
      interactiveContext().setViewMode(ZInteractiveContext::VIEW_NORMAL);
      int sliceIndex =
          buddyDocument()->maxIntesityDepth(currentRawStackPos.x(),
                                            currentRawStackPos.y());
      buddyView()->setSliceIndex(sliceIndex);
      interactionEvent.setEvent(ZInteractionEvent::EVENT_VIEW_SLICE);
    }
    break;
  case ZStackOperator::OP_STACK_VIEW_SLICE:
    interactiveContext().setViewMode(ZInteractiveContext::VIEW_NORMAL);
    buddyView()->setSliceIndex(getSliceIndex());
    interactionEvent.setEvent(ZInteractionEvent::EVENT_VIEW_SLICE);
    break;
  case ZStackOperator::OP_STACK_VIEW_PROJECTION:
    if (buddyDocument()->getTag() != NeuTube::Document::BIOCYTIN_PROJECTION) {
      interactiveContext().setViewMode(ZInteractiveContext::VIEW_PROJECT);
      interactionEvent.setEvent(ZInteractionEvent::EVENT_VIEW_PROJECTION);
    }
    break;
  case ZStackOperator::OP_SWC_LOCATE_FOCUS:
    if (op.getHitObject<Swc_Tree_Node>() != NULL) {
      int sliceIndex = iround(SwcTreeNode::z(op.getHitObject<Swc_Tree_Node>()));
      sliceIndex -= buddyDocument()->getStackOffset().getZ();
      if (sliceIndex >= 0 &&
          sliceIndex < buddyDocument()->getStackSize().getZ()) {
        buddyView()->setSliceIndex(sliceIndex);
        interactiveContext().setViewMode(ZInteractiveContext::VIEW_NORMAL);
        interactionEvent.setEvent(ZInteractionEvent::EVENT_VIEW_SLICE);
      }
    }
    break;
  case ZStackOperator::OP_STROKE_LOCATE_FOCUS:
    if (op.getHitObject<ZStroke2d>() != NULL) {
      int sliceIndex = op.getHitObject<ZStroke2d>()->getZ();
      sliceIndex -= buddyDocument()->getStackOffset().getZ();
      if (sliceIndex >= 0 &&
          sliceIndex < buddyDocument()->getStackSize().getZ()) {
        buddyView()->setSliceIndex(sliceIndex);
        interactiveContext().setViewMode(ZInteractiveContext::VIEW_NORMAL);
        interactionEvent.setEvent(ZInteractionEvent::EVENT_VIEW_SLICE);
      }
    }
    break;
  case ZStackOperator::OP_OBJECT3D_LOCATE_FOCUS:
    if (op.getHitObject<ZObject3d>() != NULL) {
      ZIntPoint pt = op.getHitObject<ZObject3d>()->getHitVoxel();
      int sliceIndex = pt.getZ();
      sliceIndex -= buddyDocument()->getStackOffset().getZ();
      if (sliceIndex >= 0 &&
          sliceIndex < buddyDocument()->getStackSize().getZ()) {
        buddyView()->setSliceIndex(sliceIndex);
        interactiveContext().setViewMode(ZInteractiveContext::VIEW_NORMAL);
        interactionEvent.setEvent(ZInteractionEvent::EVENT_VIEW_SLICE);
      }
    }
    break;
  case ZStackOperator::OP_OBJECT3D_SCAN_LOCATE_FOCUS:
  case ZStackOperator::OP_DVID_SPARSE_STACK_LOCATE_FOCUS:
    if (op.getHitObject() != NULL) {
      op.getHitObject()->setSelected(false);
      int sliceIndex = op.getHitObject()->getHitPoint().getZ();
      sliceIndex -= buddyDocument()->getStackOffset().getZ();
      if (sliceIndex >= 0 &&
          sliceIndex < buddyDocument()->getStackSize().getZ()) {
        buddyView()->setSliceIndex(sliceIndex);
        interactiveContext().setViewMode(ZInteractiveContext::VIEW_NORMAL);
        interactionEvent.setEvent(ZInteractionEvent::EVENT_VIEW_SLICE);
      }
    }
    break;
  case ZStackOperator::OP_ACTIVE_STROKE_DECREASE_SIZE:
  {
    ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(getFirstOnActiveObject());
    if (stroke->isVisible()) {
      stroke->addWidth(-1.0);
      buddyView()->paintActiveDecoration();
    }
  }
    break;
  case ZStackOperator::OP_ACTIVE_STROKE_INCREASE_SIZE:
  {
    ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(getFirstOnActiveObject());
    if (stroke->isVisible()) {
      stroke->addWidth(1.0);
      buddyView()->paintActiveDecoration();
    }
  }
    break;
  case ZStackOperator::OP_ACTIVE_STROKE_ESTIMATE_SIZE:
  {
    ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(getFirstOnActiveObject());
    if (stroke->isVisible()) {
      if (estimateActiveStrokeWidth()) {
        buddyView()->paintActiveDecoration();
      }
    }
  }
    break;
  case ZStackOperator::OP_OBJECT_DELETE_SELECTED:
    if (!buddyDocument()->getSelected(ZStackObject::TYPE_FLYEM_BOOKMARK).isEmpty()) {
      op.setOperation(ZStackOperator::OP_BOOKMARK_DELETE);
    } else {
      buddyDocument()->executeRemoveSelectedObjectCommand();
    }
    break;
#if 0
  case ZStackOperator::OP_TRACK_MOUSE_MOVE_WITH_STROKE_TOGGLE:
    buddyView()->setInfo(
          buddyDocument()->dataInfo(
            currentRawStackPos.x(), currentRawStackPos.y(),
            currentRawStackPos.z()));
    if (isStrokeOn()) {
      m_stroke.set(currentStackPos.x(), currentStackPos.y());
      if (m_interactiveContext.strokeEditMode() !=
          ZInteractiveContext::STROKE_DRAW) {
        m_stroke.setFilled(false);
      } else {
        m_stroke.toggleLabel(/*toggling=*/true);
      }
      interactionEvent.setEvent(
            ZInteractionEvent::EVENT_ACTIVE_DECORATION_UPDATED);
    }
    break;
#endif
  default:

    break;
  }

  processCustomOperator(op, &interactionEvent);

  processEvent(interactionEvent);
}

void ZStackPresenter::acceptActiveStroke()
{
  ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_STROKE);
  ZStroke2d *newStroke = stroke->clone();
  if (!newStroke->isEraser()) {
    if (newStroke->getPointNumber() == 1 &&
        m_mouseEventProcessor.getLatestMouseEvent().getModifiers() ==
        Qt::ShiftModifier &&
        buddyDocument()->getTag() != NeuTube::Document::FLYEM_SPLIT &&
        buddyDocument()->getTag() != NeuTube::Document::FLYEM_PROOFREAD) {
      if (!buddyDocument()->getStrokeList().empty()) {
        LINFO() << "Compute stroke path";
        ZPoint start;
        ZPoint end;
        buddyDocument()->getLastStrokePoint(start.xRef(), start.yRef());
        newStroke->getLastPoint(end.xRef(), end.yRef());
        buddyDocument()->mapToStackCoord(&start);
        buddyDocument()->mapToStackCoord(&end);

        int z0 = buddyView()->getZ(NeuTube::COORD_STACK);
//        int z0 = buddyView()->sliceIndex();
//        int z1 = z0;
        start.setZ(0);
        end.setZ(0);

        int source[3] = {0, 0, 0};
        int target[3] = {0, 0, 0};
        for (int i = 0; i < 3; ++i) {
          source[i] = iround(start[i]);
          target[i] = iround(end[i]);
        }

        ZStack *signal = ZStackFactory::makeSlice(
              *buddyDocument()->getStack(), z0);

        Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
        if (buddyDocument()->getStackBackground() ==
            NeuTube::IMAGE_BACKGROUND_BRIGHT) {
          sgw->wf = Stack_Voxel_Weight;
        } else {
          sgw->wf = Stack_Voxel_Weight_I;
        }

        double pointDistance = Geo3d_Dist(start.x(), start.y(), 0,
            end.x(), end.y(), 0) / 2;
        double cx = (start.x() + end.x()) / 2;
        double cy = (start.y() + end.y()) / 2;
        //double cz = ((double) (start[2] + end[2])) / 2;
        int x0 = (int) (cx - pointDistance) - 2; //expand by 2 to deal with rouding error
        int x1 = (int) (cx + pointDistance) + 2;
        int y0 = (int) (cy - pointDistance) - 2;
        int y1 = (int) (cy + pointDistance) + 2;
        //int z0 = (int) (cz - pointDistance);
        //int z1 = (int) (cz + pointDistance);


        Stack_Graph_Workspace_Set_Range(sgw, x0, x1, y0, y1, 0, 0);
        Stack_Graph_Workspace_Validate_Range(
              sgw, signal->width(), signal->height(), 1);

        //sgw->wf = Stack_Voxel_Weight;

        int channel = 0;
        if (buddyDocument()->getTag() == NeuTube::Document::BIOCYTIN_PROJECTION &&
            signal->channelNumber() > 1) {
          channel = 1;
        }

//        Stack *stack = buddyDocument()->getStack()->c_stack(channel);
        sgw->greyFactor = m_greyScale[channel];
        sgw->greyOffset = m_greyOffset[channel];

        Stack *signalData = signal->c_stack(channel);
        Int_Arraylist *path = Stack_Route(signalData, source, target, sgw);

        newStroke->clear();
#ifdef _DEBUG_2
        std::cout << "Creating new stroke ..." << std::endl;
#endif
        for (int i = 0; i < path->length; ++i) {
          int x, y, z;
          C_Stack::indexToCoord(path->array[i], buddyDocument()->getStack()->width(),
                                buddyDocument()->getStack()->height(),
                                &x, &y, &z);
#ifdef _DEBUG_2
          std::cout << x << " " << y << std::endl;
#endif
          newStroke->append(x + buddyDocument()->getStackOffset().getX(),
                            y + buddyDocument()->getStackOffset().getY());
        }
#ifdef _DEBUG_2
        std::cout << "New stroke created" << std::endl;
        newStroke->print();
#endif

        delete signal;
      }
    }
  } else {
    newStroke->setColor(QColor(0, 0, 0, 0));
  }

  newStroke->setZ(buddyView()->sliceIndex() +
                  buddyDocument()->getStackOffset().getZ());
  newStroke->setPenetrating(false);

  ZStackObjectRole::TRole role = ZStackObjectRole::ROLE_NONE;
//  if (buddyDocument()->getTag() == NeuTube::Document::FLYEM_SPLIT) {
  if (GET_APPLICATION_NAME == "FlyEM") {
    role = ZStackObjectRole::ROLE_SEED |
        ZStackObjectRole::ROLE_3DGRAPH_DECORATOR;
  }

  newStroke->setZOrder(m_zOrder++);
  newStroke->setRole(role);
  if (buddyDocument()->getTag() == NeuTube::Document::BIOCYTIN_PROJECTION) {
    newStroke->setPenetrating(true);
  }
  buddyDocument()->executeAddObjectCommand(newStroke);
  //buddyDocument()->executeAddStrokeCommand(newStroke);

  stroke->clear();
  buddyView()->paintActiveDecoration();
}

ZStackFrame* ZStackPresenter::getParentFrame() const
{
  return qobject_cast<ZStackFrame*>(parent());
}

ZStackMvc* ZStackPresenter::getParentMvc() const
{
  return qobject_cast<ZStackMvc*>(parent());
}

QWidget* ZStackPresenter::getParentWidget() const
{
  return qobject_cast<QWidget*>(parent());
}

bool ZStackPresenter::isSwcFullSkeletonVisible() const
{
  return m_actionMap[ZActionFactory::ACTION_TOGGLE_SWC_SKELETON]->isChecked();
}

void ZStackPresenter::testBiocytinProjectionMask()
{
  interactiveContext().setStrokeEditMode(
        ZInteractiveContext::STROKE_DRAW);
  ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_STROKE);
  stroke->set(23, 21);
  acceptActiveStroke();
  stroke->set(126, 139);
  ZMouseEvent event;
  event.addModifier(Qt::ShiftModifier);
  m_mouseEventProcessor.getRecorder().record(event);
//  m_mouseEventProcessor.getLatestMouseEvent().getModifiers()
  acceptActiveStroke();

  event.removeModifier(Qt::ShiftModifier);
  m_mouseEventProcessor.getRecorder().record(event);
  stroke->set(65, 55);
  stroke->setEraser(true);
  acceptActiveStroke();

  stroke->setEraser(false);
  stroke->setColor(0, 255, 0);
  stroke->set(104, 47);
  acceptActiveStroke();
  stroke->set(49, 89);
  event.addModifier(Qt::ShiftModifier);
  m_mouseEventProcessor.getRecorder().record(event);
  acceptActiveStroke();

  ZStack *stack = buddyView()->getStrokeMask(NeuTube::COLOR_RED);
  stack->save(GET_TEST_DATA_DIR + "/test.tif");
  delete stack;

  stack = buddyView()->getStrokeMask(NeuTube::COLOR_GREEN);
  stack->save(GET_TEST_DATA_DIR + "/test2.tif");


  delete stack;
}

bool ZStackPresenter::isActiveObjectOn(EObjectRole role) const
{
  if (getActiveObject(role) == NULL) {
    return false;
  }

  return getActiveObject(role)->isVisible();
}

bool ZStackPresenter::isActiveObjectOn() const
{
  for (QMap<EObjectRole, ZStackObject*>::const_iterator iter =
       m_activeObjectMap.begin(); iter != m_activeObjectMap.end(); ++iter) {
    if (iter.value()->isVisible()) {
      return true;
    }
  }

  return false;
}

ZStackObject* ZStackPresenter::getActiveObject(EObjectRole role) const
{
  if (!m_activeObjectMap.contains(role)) {
    return NULL;
  }

  return m_activeObjectMap[role];
}

void ZStackPresenter::setSliceAxis(NeuTube::EAxis axis)
{
  m_interactiveContext.setSliceAxis(axis);
  for (QList<ZStackObject*>::iterator iter = m_activeDecorationList.begin();
       iter != m_activeDecorationList.end(); ++iter) {
    ZStackObject *obj = *iter;
    obj->setSliceAxis(axis);
  }
}
