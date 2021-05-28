#ifdef _QT5_
#include <QtConcurrent>
#else
#include <QtConcurrentRun>
#endif

#include "tz_geo3d_utils.h"
//#include "tz_rastergeom.h"
#include "neulib/math/utilities.h"

#include "zstackpresenter.h"
#include "zstackframe.h"
#include "zstackview.h"
#include "zlocalneuroseg.h"
#include "zlocsegchain.h"
#include "zstack.hxx"
#include "zstackdoc.h"
#include "swctreenode.h"
#include "data3d/displayconfig.h"
#include "widgets/zimagewidget.h"
#include "dialogs/channeldialog.h"
#include "neutubeconfig.h"
#include "zcursorstore.h"
#include "zstroke2d.h"
#include "zstackdocmenufactory.h"
#include "zinteractionevent.h"
#include "zellipse.h"
#include "zstackoperator.h"
#include "zrect2d.h"
#include "zstackobjectsource.h"
#include "zstackobjectsourcefactory.h"
#include "zstackmvc.h"
#include "dvid/zdvidlabelslice.h"
//#include "dvid/zdvidsparsestack.h"
#include "zkeyoperationconfig.h"
#include "zstackfactory.h"
#include "zstackdocselector.h"
#include "zglobal.h"
#include "zstackdockeyprocessor.h"
#include "zstackobjectinfo.h"
#include "zobject3d.h"
#include "zmenuconfig.h"
#include "zmenufactory.h"
#include "zpunctum.h"
#include "zmousecursorglyph.h"

ZStackPresenter::ZStackPresenter(QWidget *parent) : QObject(parent)
{
  init();
}

ZStackPresenter::~ZStackPresenter()
{
  clearData();

  /*
  for (QMap<EObjectRole, ZStackObject*>::iterator iter =
       m_activeObjectMap.begin(); iter != m_activeObjectMap.end(); ++iter) {
    delete iter.value();
  }
  m_activeObjectMap.clear();
  m_activeDecorationList.clear();
  */

  delete m_swcNodeContextMenu;
  delete m_strokePaintContextMenu;
  delete m_stackContextMenu;
  delete m_keyConfig;
  delete m_actionFactory;
//  delete m_menuFactory;
}

ZStackPresenter* ZStackPresenter::Make(QWidget *parent)
{
  ZStackPresenter *presenter = new ZStackPresenter(parent);
  presenter->configKeyMap();

  return presenter;
}

#if 0
void ZStackPresenter::initActiveObject()
{
  m_defaultDecorationSize[ROLE_STROKE] = 3.0;
  m_defaultDecorationSize[ROLE_SWC] = 5.0;
  m_defaultDecorationSize[ROLE_SYNAPSE] = 5.0;
  m_defaultDecorationSize[ROLE_BOOKMARK] = 5.0;
  m_defaultDecorationSize[ROLE_TODO_ITEM] = 5.0;

  {
    ZStroke2d *stroke = new ZStroke2d;
    stroke->setVisible(false);
    stroke->setFilled(true);
    stroke->setPenetrating(true);
    stroke->useCosmeticPen(false);
    stroke->hideStart(false);
    stroke->setTarget(neutu::data3d::ETarget::ROAMING_OBJECT_CANVAS);
    addActiveObject(ROLE_STROKE, stroke);
  }

  {
    ZSwcTree *obj = new ZSwcTree;
    obj->setVisible(false);
    obj->useCosmeticPen(true);
    Swc_Tree_Node *tn = SwcTreeNode::MakePointer(
          0, 0, 0, m_defaultDecorationSize[ROLE_SWC]);
    obj->forceVirtualRoot();
    obj->addRegularRoot(tn);
    obj->setColor(0, 0, 255);
    obj->setTarget(neutu::data3d::ETarget::ROAMING_OBJECT_CANVAS);
    addActiveObject(ROLE_SWC, obj);
  }


  {
    ZStackBall *obj = new ZStackBall;
    obj->setVisible(false);
    obj->useCosmeticPen(true);
    obj->setRadius(m_defaultDecorationSize[ROLE_SYNAPSE]);
    obj->setColor(0, 255, 0);
    obj->setTarget(neutu::data3d::ETarget::ROAMING_OBJECT_CANVAS);
    addActiveObject(ROLE_SYNAPSE, obj);
  }

  {
    ZStackBall *obj = new ZStackBall;
    obj->setVisible(false);
    obj->useCosmeticPen(true);
    obj->setRadius(m_defaultDecorationSize[ROLE_BOOKMARK]);
    obj->setColor(255, 0, 0);
    obj->setTarget(neutu::data3d::ETarget::ROAMING_OBJECT_CANVAS);
    addActiveObject(ROLE_BOOKMARK, obj);
  }

  {
    ZStackBall *obj = new ZStackBall;
    obj->setVisible(false);
    obj->useCosmeticPen(true);
    obj->setRadius(m_defaultDecorationSize[ROLE_TODO_ITEM]);
    obj->setColor(255, 0, 0);
    obj->setTarget(neutu::data3d::ETarget::ROAMING_OBJECT_CANVAS);
    addActiveObject(ROLE_TODO_ITEM, obj);
    /*
    ZStroke2d *stroke = new ZStroke2d;
    stroke->setVisible(false);
    stroke->setFilled(false);
    stroke->setPenetrating(true);
    stroke->useCosmeticPen(true);
    stroke->hideStart(true);
    stroke->setWidth(10.0);
    stroke->setColor(QColor(200, 128, 200));
    stroke->setTarget(neutu::data3d::ETarget::ROAMING_OBJECT_CANVAS);
    addActiveObject(ROLE_TODO_ITEM, stroke);
    */
  }
}
#endif

namespace {

void set_single_node_mode(ZSwcTree *tree)
{
  if (tree) {
    Swc_Tree_Node *vtn = tree->root();
    Swc_Tree_Node *tn = tree->firstRegularRoot();
    if (SwcTreeNode::hasChild(tn)) {
      SwcTreeNode::setRadius(tn, vtn->node.d);
      Swc_Tree_Node *child = tn->first_child;
      while (child) {
        Swc_Tree_Node *nextChild = child->next_sibling;
        SwcTreeNode::killSubtree(child);
        child = nextChild;
      }
      tree->deprecate(ZSwcTree::ALL_COMPONENT);
    }
  }
}

void set_double_node_mode(
    ZSwcTree *tree, const ZPoint &startCenter, double endRadius)
{
  if (tree) {
    Swc_Tree_Node *tn = tree->firstRegularRoot();
    SwcTreeNode::setCenter(tn, startCenter);
    if (!SwcTreeNode::hasChild(tn)) {
      SwcTreeNode::setRadius(tree->root(), SwcTreeNode::radius(tn));
      SwcTreeNode::setRadius(tn, 0);
      tn = SwcTreeNode::MakeChild(tn);
    } else {
      tn = SwcTreeNode::firstChild(tn);
    }
    SwcTreeNode::setCenter(tn, startCenter);
    SwcTreeNode::setRadius(tn, endRadius);
    tree->deprecate(ZSwcTree::ALL_COMPONENT);
  }
}

void set_double_node_mode(
    ZSwcTree *tree, const Swc_Tree_Node *refNode)
{
  if (tree && refNode) {
    set_double_node_mode(
          tree, SwcTreeNode::center(refNode), SwcTreeNode::radius(refNode));
  }
}

}

void ZStackPresenter::prepareSwcGlyph(ZStackObject *obj)
{
  ZSwcTree *tree = dynamic_cast<ZSwcTree*>(obj);
  if (tree) {
    switch (interactiveContext().swcEditMode()) {
    case ZInteractiveContext::SWC_EDIT_ADD_NODE:
      set_single_node_mode(tree);
      break;
    case ZInteractiveContext::SWC_EDIT_EXTEND:
    case ZInteractiveContext::SWC_EDIT_SMART_EXTEND:
      set_double_node_mode(tree, getSelectedSwcNode());
      break;
    default:
      break;
    }
  }
}

#if 0
void ZStackPresenter::prepareActiveDecoration(ZStackObject *obj)
{
  if (obj->getType() == ZStackObject::EType::SWC) {
    ZSwcTree *tree = dynamic_cast<ZSwcTree*>(obj);
    switch (interactiveContext().swcEditMode()) {
    case ZInteractiveContext::SWC_EDIT_ADD_NODE:
      set_single_node_mode(tree);
      break;
    case ZInteractiveContext::SWC_EDIT_EXTEND:
    case ZInteractiveContext::SWC_EDIT_SMART_EXTEND:
      set_double_node_mode(tree, getSelectedSwcNode());
      break;
    default:
      break;
    }
  }
#if 0
  switch(obj->getType()) {
  case ZStackObject::EType::SWC:
  if (role == ROLE_SWC) {
    ZSwcTree *tree = dynamic_cast<ZSwcTree*>(obj);
    switch (interactiveContext().swcEditMode()) {
    case ZInteractiveContext::SWC_EDIT_ADD_NODE:
      set_single_node_mode(tree);
      break;
    case ZInteractiveContext::SWC_EDIT_EXTEND:
    case ZInteractiveContext::SWC_EDIT_SMART_EXTEND:
      set_double_node_mode(tree, getSelectedSwcNode());
      break;
    default:
      break;
    }

/*
    if (tree) {
      Swc_Tree_Node *tn = tree->firstRegularRoot();
      if (tn) {
        if (SwcTreeNode::hasChild(tn)) {
          tn = SwcTreeNode::firstChild(tn);
        }
        tree->deprecate(ZSwcTree::BOUND_BOX);
      }
    }
    */
  }
    break;
  default:
    obj = nullptr;
    break;
  }
#endif
}
#endif

void ZStackPresenter::init()
{
  m_showObject = true;
  m_oldShowObject = true;
//  m_isStrokeOn = false;
  m_skipMouseReleaseEvent = 0;
  m_zOrder = 2;

  initInteractiveContext();

  m_grayScale.resize(5, 1.0);
  m_grayOffset.resize(5, 0.0);

  m_objStyle = ZStackObject::EDisplayStyle::BOUNDARY;
  m_threshold = -1;
  m_mouseLeftButtonPressed = false;
  m_mouseRightButtonPressed = false;

  m_usingHighContrast = false;

  m_paintingRoi = false;

  for (int i = 0; i < 3; i++) {
//    m_mouseLeftReleasePosition[i] = -1;
    m_mouseRightReleasePosition[i] = -1;
    m_mouseLeftPressPosition[i] = -1;
    m_mouseRightPressPosition[i] = -1;
    m_mouseLeftDoubleClickPosition[i] = -1;
    m_mouseMovePosition[i] = -1;
  }

  m_cursorRadius = 10;
//  initActiveObject();
  m_mouseCursorGlyph =
      std::shared_ptr<ZMouseCursorGlyph>(new ZMouseCursorGlyph);
  m_mouseCursorGlyph->setPrepareFunc(
        ZMouseCursorGlyph::ROLE_SWC,
        std::bind(
          &ZStackPresenter::prepareSwcGlyph, this, std::placeholders::_1));

  m_highlightDecoration.setRadius(5.0);
  m_highlightDecoration.setColor(QColor(255, 255, 255, 160));
  m_highlightDecoration.setVisualEffect(neutu::display::Sphere::VE_FORCE_FILL);
  m_highlightDecorationList.append(&m_highlightDecoration);
  m_highlight = false;

  m_swcNodeContextMenu = NULL;
  m_strokePaintContextMenu = NULL;
  m_stackContextMenu = NULL;
  m_bodyContextMenu = NULL;

  //m_leftButtonReleaseMapper.setContext(&m_interactiveContext);
  //m_moveMapper.setContext(&m_interactiveContext);
  m_mouseEventProcessor.setInteractiveContext(&m_interactiveContext);

  m_keyConfig = NULL;

//  m_menuFactory = NULL;
  m_actionFactory = new ZActionFactory;

  m_docSelector.setSelectOption(ZStackObject::EType::FLYEM_SYNAPSE_ENSEMBLE,
                                ZStackDocSelector::SELECT_RECURSIVE);
  m_docSelector.setSelectOption(ZStackObject::EType::FLYEM_TODO_ENSEMBLE,
                                ZStackDocSelector::SELECT_RECURSIVE);
}

/*
void ZStackPresenter::addActiveObject(EObjectRole role, ZStackObject *obj)
{
  if (m_activeObjectMap.contains(role)) {
    m_activeDecorationList.removeOne(obj);
    delete m_activeObjectMap[role];
  }

  m_activeObjectMap[role] = obj;
  m_activeDecorationList.append(obj);
}
*/

ZKeyOperationConfig* ZStackPresenter::getKeyConfig()
{
  if (m_keyConfig == NULL) {
    m_keyConfig = new ZKeyOperationConfig;
  }

  return m_keyConfig;
}

ZMenuConfig ZStackPresenter::getMenuConfig() const
{
  ZMenuConfig config;
  switch (buddyDocument()->getTag()) {
  case neutu::Document::ETag::FLYEM_SPLIT:
  case neutu::Document::ETag::SEGMENTATION_TARGET:
    config << ZActionFactory::ACTION_SPLIT_DATA
           << ZActionFactory::ACTION_ADD_SPLIT_SEED;
    break;
  case neutu::Document::ETag::NORMAL:
  case neutu::Document::ETag::BIOCYTIN_STACK:
    config << ZActionFactory::ACTION_ADD_SWC_NODE
           << ZActionFactory::ACTION_TOGGLE_SWC_SKELETON;
    break;
  default:
    break;
  }

  return config;
}

ZStackDocMenuFactory* ZStackPresenter::getMenuFactory()
{
  if (!m_menuFactory) {
    m_menuFactory = std::unique_ptr<ZStackDocMenuFactory>(
          new ZStackDocMenuFactory);
    m_menuFactory->setAdminState(neutu::IsAdminUser());
  }

  return m_menuFactory.get();
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
    action = const_cast<ZStackPresenter&>(*this).makeAction(item);
//    if (m_actionMap.contains(item)) {
//      action = m_actionMap[item];
//    }
    if (action == NULL) {
      action = buddyDocument()->getAction(item);
    }
  }

  return action;
}

bool ZStackPresenter::connectAction(
    QAction *action, ZActionFactory::EAction item)
{
  bool connected = false;

  //Additional behaviors
  if (action != NULL) {
    connected = true;
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
      if (getParentFrame() != NULL) {
        connect(action, SIGNAL(triggered()),
                getParentFrame(), SLOT(locateSwcNodeIn3DView()));
      }
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
    case ZActionFactory::ACTION_BODY_EXPERT_STATUS:
      connect(action, &QAction::triggered,
              this, &ZStackPresenter::notifyExpertBodyStatus);
      break;
    case ZActionFactory::ACTION_BODY_CONNECTION:
      connect(action, SIGNAL(triggered()),
              this, SLOT(notifyBodyConnectionTriggered()));
      break;
    case ZActionFactory::ACTION_BODY_PROFILE:
      connect(action, &QAction::triggered,
              this, &ZStackPresenter::notifyBodyProfileTriggered);
      break;
    case ZActionFactory::ACTION_BODY_SPLIT_START:
      connect(action, SIGNAL(triggered()),
              this, SLOT(notifyBodySplitTriggered()));
      break;
    case ZActionFactory::ACTION_SPLIT_DATA:
      connect(action, SIGNAL(triggered()), this, SLOT(runSeededWatershed()));
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
    case ZActionFactory::ACTION_BODY_CROP:
      connect(action, SIGNAL(triggered()),
              this, SLOT(notifyBodyCropTriggered()));
      break;
    case ZActionFactory::ACTION_BODY_CHOP:
      connect(action, SIGNAL(triggered()),
              this, SLOT(notifyBodyChopTriggered()));
      break;
    case ZActionFactory::ACTION_BODY_MERGE:
      connect(action, SIGNAL(triggered()),
              this, SLOT(notifyBodyMergeTriggered()));
      break;
    case ZActionFactory::ACTION_BODY_UNMERGE:
      connect(action, SIGNAL(triggered()),
              this, SLOT(notifyBodyUnmergeTriggered()));
      break;
    case ZActionFactory::ACTION_BOOKMARK_CHECK:
      connect(action, SIGNAL(triggered()), this, SIGNAL(checkingBookmark()));
      break;
    case ZActionFactory::ACTION_BOOKMARK_UNCHECK:
      connect(action, SIGNAL(triggered()), this, SIGNAL(uncheckingBookmark()));
      break;
    case ZActionFactory::ACTION_MEASURE_SWC_NODE_LENGTH:
      connect(action, SIGNAL(triggered()),
              buddyDocument(), SLOT(showSeletedSwcNodeLength()));
      break;
    case ZActionFactory::ACTION_MEASURE_SCALED_SWC_NODE_LENGTH:
      connect(action, SIGNAL(triggered()),
              buddyDocument(), SLOT(showSeletedSwcNodeScaledLength()));
      break;
    case ZActionFactory::ACTION_ENTER_RECT_ROI_MODE:
      connect(action, SIGNAL(triggered()), this, SLOT(tryDrawRectMode()));
      break;
    case ZActionFactory::ACTION_CANCEL_RECT_ROI:
      connect(action, SIGNAL(triggered()), this, SLOT(cancelRectRoi()));
      break;
    case ZActionFactory::ACTION_SAVE_STACK:
      connect(action, SIGNAL(triggered()), this, SIGNAL(savingStack()));
      break;
    case ZActionFactory::ACTION_SHOW_ORTHO:
      connect(action, SIGNAL(triggered()),
              this, SLOT(notifyOrthoViewTriggered()));
      break;
    case ZActionFactory::ACTION_SHOW_ORTHO_BIG:
      connect(action, SIGNAL(triggered()),
              this, SLOT(notifyOrthoViewBigTriggered()));
      break;
    case ZActionFactory::ACTION_COPY_POSITION:
      connect(action, SIGNAL(triggered()), this, SLOT(copyCurrentPosition()));
      break;
    case ZActionFactory::ACTION_COPY_BODY_ID:
      connect(action, SIGNAL(triggered()), this, SLOT(copyLabelId()));
      break;
    case ZActionFactory::ACTION_COPY_SUPERVOXEL_ID:
      connect(action, SIGNAL(triggered()), this, SLOT(copySupervoxelId()));
      break;
    case ZActionFactory::ACTION_COPY_NEUROGLANCER_LINK:
      connect(action, SIGNAL(triggered()), this, SLOT(copyNeuroglancerLink()));
      break;
    case ZActionFactory::ACTION_COPY_NEUROGLANCER_LINK_AT_RECT_ROI:
      connect(action, SIGNAL(triggered()),
              this, SLOT(copyNeuroglancerLinkAtRectRoi()));
      break;
    default:
      connected = false;
      break;
    }
  }

  return connected;
}

QAction* ZStackPresenter::makeAction(ZActionFactory::EAction item)
{
  QAction *action = NULL;

  if (ZActionFactory::IsRegularAction(item)) {
    if (!m_actionMap.contains(item)) {
      action = m_actionFactory->makeAction(item, this);
      if (action != NULL) {
        m_actionMap[item] = action;

        if (!connectAction(action, item)) {
          LWARN() << "Failed to connect action: " << action->text();
        }
      }
    } else {
      action = m_actionMap[item];
    }
  }

  return action;
}

bool ZStackPresenter::paintingStroke() const
{
  ZStackOperator op = m_mouseEventProcessor.getOperator();

  return op.getOperation() == ZStackOperator::OP_PAINT_STROKE;
}

void ZStackPresenter::forEachView(std::function<void (ZStackView *)> f) const
{
  auto viewList = getViewList();
  for (ZStackView *view : viewList) {
    f(view);
  }
}

ZStackView* ZStackPresenter::getMouseHoveredView() const
{
  auto viewList = getViewList();
  for (ZStackView *view : viewList) {
    if (view->imageWidget()->containsCurrentMousePostion()) {
      return view;
    }
  }

  return nullptr;
}

ZPoint ZStackPresenter::getCurrentMousePosition(neutu::data3d::ESpace space)
{
  ZPoint dataPos = ZPoint::INVALID_POINT;
  ZStackView *view = getMouseHoveredView();
  if (view) {
    dataPos =
        view->getCurrentMousePosition(space);
  }

  return dataPos;
}

void ZStackPresenter::updateMouseCursorGlyphPos()
{
  ZPoint dataPos = getCurrentMousePosition(neutu::data3d::ESpace::MODEL);
  if (dataPos.isValid()) {
    auto postProc = [=](ZStackObject *obj) {
      buddyDocument()->processObjectModified(obj);
    };
    if (paintingStroke()) {
      m_mouseCursorGlyph->appendActiveGlyphPosition(dataPos, postProc);
    } else {
      m_mouseCursorGlyph->setActiveGlyphPosition(dataPos, postProc);
    }
  }

#if 0
  foreach(ZStackObject *obj, m_activeDecorationList) {
    if (obj->isVisible() && !paintingStroke()) {
      switch (obj->getType()) {
      case ZStackObject::EType::STROKE:
      {
        ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(obj);
        if (stroke) {
          stroke->updateWithLast(dataPos);
  #ifdef _DEBUG_
          std::cout << "Current stroke: ";
          stroke->print();
  #endif
//          buddyDocument()->bufferObjectModified(stroke);
        }
      }
        break;
      case ZStackObject::EType::STACK_BALL:
      {
        ZStackBall *ball = dynamic_cast<ZStackBall*>(obj);
        if (ball) {
           ball->setCenter(dataPos.toIntPoint());
        }
      }
        break;
      case ZStackObject::EType::SWC:
      {
        ZSwcTree *tree = dynamic_cast<ZSwcTree*>(obj);
        if (tree) {
          Swc_Tree_Node *tn = tree->firstRegularRoot();
          if (tn) {
            if (SwcTreeNode::hasChild(tn)) {
              tn = SwcTreeNode::firstChild(tn);
            }
            SwcTreeNode::setCenter(tn, dataPos);
            tree->deprecate(ZSwcTree::BOUND_BOX);
          }
#ifdef _DEBUG_2
        tree->print();
#endif
        }
      }
        break;
      default:
        obj = nullptr;
        break;
      }
      buddyDocument()->bufferObjectModified(obj);
    }
  }
  buddyDocument()->processObjectModified();
#endif
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
/*
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
    if (action != NULL) {
      connect(action, SIGNAL(triggered()), this, SLOT(fitSegment()));
    }
  }

  {
    QAction *action =
        ZActionFactory::MakeAction(ZActionFactory::ACTION_DROPSEG, this);
    if (action != NULL) {
      connect(action, SIGNAL(triggered()), this, SLOT(dropSegment()));
    }
  }
}

void ZStackPresenter::createPunctaActions()
{
  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_PUNCTA_MARK, this);
    if (action != NULL) {
      connect(action, SIGNAL(triggered()), this, SLOT(markPuncta()));
    }
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_PUNCTA_ENLARGE, this);
    if (action != NULL) {
      connect(action, SIGNAL(triggered()), this, SLOT(enlargePuncta()));
    }
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_PUNCTA_NARROW, this);
    if (action != NULL) {
      connect(action, SIGNAL(triggered()), this, SLOT(narrowPuncta()));
    }
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_PUNCTA_MEANSHIFT, this);
    if (action != NULL) {
      connect(action, SIGNAL(triggered()), this, SLOT(meanshiftPuncta()));
    }
  }

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_PUNCTA_MEANSHIFT_ALL, this);
    if (action != NULL) {
      connect(action, SIGNAL(triggered()), this, SLOT(meanshiftAllPuncta()));
    }
  }
}
*/

#if 0
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
#endif

#if 0
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
    if (action != NULL) {
      connect(action, SIGNAL(triggered()),
              getParentFrame(), SLOT(locateSwcNodeIn3DView()));
      m_actionMap[ZActionFactory::ACTION_LOCATE_SELECTED_SWC_NODES_IN_3D] = action;
    }
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
#endif

#if 0
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
  makeAction(ZActionFactory::ACTION_SHOW_ORTHO);
  makeAction(ZActionFactory::ACTION_SAVE_STACK);
}
#endif

#if 0
void ZStackPresenter::createBodyActions()
{
  makeAction(ZActionFactory::ACTION_BODY_SPLIT_START);
  makeAction(ZActionFactory::ACTION_BODY_ANNOTATION);
  makeAction(ZActionFactory::ACTION_BODY_CHECKIN);
  makeAction(ZActionFactory::ACTION_BODY_FORCE_CHECKIN);
  makeAction(ZActionFactory::ACTION_BODY_CHECKOUT);
  makeAction(ZActionFactory::ACTION_BODY_MERGE);
  makeAction(ZActionFactory::ACTION_BODY_UNMERGE);


#if 0
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

  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_BODY_UNMERGE, this);
    connect(action, SIGNAL(triggered()),
            this, SLOT(notifyBodyUnmergeTriggered()));
    m_actionMap[ZActionFactory::ACTION_BODY_UNMERGE] = action;
  }
#endif

//  action = new QAction(tr("Add split seed"), this);
//  connect(action, SIGNAL(triggered()), this, SLOT());
//  m_actionMap[ACTION_ADD_SPLIT_SEED] = action;
}
#endif

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

#if 0
void ZStackPresenter::createActions()
{
  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_DELETE_SELECTED, this);
    if (action != NULL) {
      connect(action, SIGNAL(triggered()), this, SLOT(deleteSelected()));
      m_actionMap[ZActionFactory::ACTION_DELETE_SELECTED] = action;
    }
  }


  {
    QAction *action = ZActionFactory::MakeAction(
          ZActionFactory::ACTION_FIT_ELLIPSE, this);
    if (action != NULL) {
      connect(action, SIGNAL(triggered()), this, SLOT(fitEllipse()));
    }
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
#endif

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

bool ZStackPresenter::hasHighContrastProtocal() const
{
  return !m_highContrastProtocal.isEmpty();
}

ZJsonObject ZStackPresenter::getHighContrastProtocal() const
{
  return m_highContrastProtocal;
}

void ZStackPresenter::setHighContrastProtocal(const ZJsonObject &obj)
{
  m_highContrastProtocal = obj;
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

#include "flyem/zflyemproofdocmenufactory.h"
void ZStackPresenter::createBodyContextMenu()
{
  if (dynamic_cast<ZFlyEmProofDocMenuFactory*>(getMenuFactory()) != NULL) {
    ZOUT(std::cout, 2) << "ZFlyEmProofDocMenuFactory ready";
//    std::cout << "ZFlyEmProofDocMenuFactory" << std::endl;
  }

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

/*
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
*/

QList<ZStackObject*> ZStackPresenter::getActiveDecorationList() const
{
  return m_mouseCursorGlyph->getGlyphList();
}

/*
QList<ZStackObject*> ZStackPresenter::getActiveDecorationList(
    std::function<bool(const ZStackObject*)> pred) const
{
  QList<ZStackObject*> objList;
  for (ZStackObject *obj : m_activeDecorationList) {
    if (pred(obj)) {
      objList.append(obj);
    }
  }

  return objList;
}
*/

#if 0
void ZStackPresenter::setActiveObjectSize(EObjectRole role, double radius)
{
  ZStackObject *obj = getActiveObject(role);
  ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(obj);
  if (stroke != NULL) {
    stroke->setWidth(radius * 2.0);
  }
}

void ZStackPresenter::setDefaultActiveObjectSize(EObjectRole role)
{
  if (role == ROLE_SWC) {
    setActiveObjectSize(role, 6.0);
  }
}

void ZStackPresenter::turnOnActiveObject(
    EObjectRole role, std::function<void(ZStackObject*)> prepare)
{
  turnOffActiveObject();
  ZStackObject *obj = getActiveObject(role);
  if (obj) {
    obj->setVisible(true);
    prepare(obj);
    updateActiveDecorationPos();
    buddyDocument()->processObjectModified(obj);
  }
}


void ZStackPresenter::turnOnActiveObject(
    EObjectRole role, std::function<void(ZStackObject*, EObjectRole)> prepare)
{
  turnOnActiveObject(role, [&](ZStackObject *obj) {
    prepare(obj, role);
  });
}

void ZStackPresenter::turnOnActiveObject(EObjectRole role)
{
  turnOnActiveObject(
        role, std::bind(
          &ZStackPresenter::prepareActiveDecoration, this,
          std::placeholders::_1, std::placeholders::_2));
}
#endif

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
  m_mouseCursorGlyph->deactivate();
  m_mouseCursorGlyph->processActiveChange([this](ZStackObject *obj) {
    buddyDocument()->processObjectModified(obj);
  });
#if 0
  for (QMap<EObjectRole, ZStackObject*>::iterator iter =
       m_activeObjectMap.begin(); iter != m_activeObjectMap.end(); ++iter) {
    iter.value()->setVisible(false);
    /*
    if (iter.value()->getType() == ZStackObject::EType::SWC) {
      ZSwcTree *tree = dynamic_cast<ZSwcTree*>(iter.value());
      if (tree) {
        Swc_Tree_Node *tn = tree->firstRegularRoot();
        SwcTreeNode::setRadius(tn, m_defaultDecorationSize[ROLE_SWC]);
        Swc_Tree_Node *child = tn->first_child;
        while (child) {
          Swc_Tree_Node *nextChild = child->next_sibling;
          SwcTreeNode::killSubtree(child);
          child = nextChild;
        }
        tree->deprecate(ZSwcTree::ALL_COMPONENT);
      }
    }
    */
  }
#endif
}

/*
void ZStackPresenter::turnOffActiveObject(EObjectRole role)
{
  ZStackObject *obj = getActiveObject(role);
  if (obj != NULL) {
    obj->setVisible(false);
    buddyView()->paintActiveDecoration();
  }
}
*/

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

ZStackView* ZStackPresenter::getDefaultView() const
{
  if (getParentFrame() != NULL) {
    return getParentFrame()->view();
  } else if (getParentMvc() != NULL) {
    return getParentMvc()->getDefaultView();
  }

  return nullptr;
}

ZStackView* ZStackPresenter::getMainView() const
{
  if (getParentFrame() != NULL) {
    return getParentFrame()->view();
  } else if (getParentMvc() != NULL) {
    return getParentMvc()->getMainView();
  }

  return nullptr;
}

ZStackView* ZStackPresenter::getContextView() const
{
  return getView(m_contextViewId);
}

std::vector<ZStackView*> ZStackPresenter::getViewList() const
{
  if (getParentFrame() != NULL) {
    return std::vector<ZStackView*>{getParentFrame()->view()};
  } else if (getParentMvc() != NULL) {
    return getParentMvc()->getViewList();
  }

  return std::vector<ZStackView*>();
}

ZStackView* ZStackPresenter::getView(int viewId) const
{
  if (getParentFrame() != NULL) {
    return getParentFrame()->view();
  } else if (getParentMvc() != NULL) {
    return getParentMvc()->getView(viewId);
  }

  return nullptr;
}

void ZStackPresenter::addPunctaEditFunctionToRightMenu(ZStackView *view)
{
  if (view) {
    updateRightMenu(
          view, getAction(ZActionFactory::ACTION_PUNCTA_ENLARGE), false);
//    updateRightMenu(getAction(ZActionFactory::ACTION_PUNCTA_ENLARGE), false);
    updateRightMenu(
          view, getAction(ZActionFactory::ACTION_PUNCTA_MEANSHIFT), false);
    updateRightMenu(
          view, getAction(ZActionFactory::ACTION_PUNCTA_MEANSHIFT_ALL), false);
    updateRightMenu(
          view, getAction(ZActionFactory::ACTION_DELETE_SELECTED), false);
  }
  //updateRightMenu(m_deleteAllPunctaAction, false);
}

void ZStackPresenter::prepareView()
{
//  createActions();
//  createDocDependentActions();
  m_mouseEventProcessor.setDocument(getSharedBuddyDocument());

  auto viewList = getViewList();

  for (ZStackView *view : viewList) {
    if (NeutubeConfig::getInstance().getMainWindowConfig().isTracingOn()) {
      updateLeftMenu(view, getAction(ZActionFactory::ACTION_TRACE));
    } else {
      updateLeftMenu(view, NULL);
    }

    view->rightMenu()->clear();
    //m_interactiveContext.setView(buddyView()->imageWidget()->projectRegion(),
    //                             buddyView()->imageWidget()->viewPort());
    //  m_mouseEventProcessor.setImageWidget(buddyView()->imageWidget());
    //  m_mouseEventProcessor.setSliceAxis(buddyView()->getSliceAxis());
    //  m_mouseEventProcessor.setArbSlice(buddyView()->getAffinePlane());
    setSliceViewTransform(view->getViewId(), view->getSliceViewTransform());
  }

//  setSliceAxis(buddyView()->getSliceAxis());

//  m_swcKeyMapper.setTag(buddyDocument()->getTag());
}

void ZStackPresenter::updateLeftMenu(ZStackView *view, QAction *action, bool clear)
{
  if (view) {
    if (clear == true) {
      view->leftMenu()->clear();
    }
    if (action != NULL) {
      view->leftMenu()->addAction(action);
    }
  }
}

void ZStackPresenter::updateLeftMenu(ZStackView *view)
{
  bool traceOnFlag = false;
  if (interactiveContext().tracingTube()) {
    updateLeftMenu(view, getAction(ZActionFactory::ACTION_TRACE), true);
    traceOnFlag = true;
  } else if (interactiveContext().fittingSegment()) {
    updateLeftMenu(view, getAction(ZActionFactory::ACTION_FITSEG), true);
    updateLeftMenu(view, getAction(ZActionFactory::ACTION_DROPSEG), false);
    updateLeftMenu(view, getAction(ZActionFactory::ACTION_FIT_ELLIPSE), false);
    traceOnFlag = true;
  }

  if (interactiveContext().markingPuncta()) {
    if (traceOnFlag) {
      updateLeftMenu(view, getAction(ZActionFactory::ACTION_PUNCTA_MARK), false);
    } else {
      updateLeftMenu(view, getAction(ZActionFactory::ACTION_PUNCTA_MARK), true);
    }
  }
}

void ZStackPresenter::updateRightMenu(ZStackView *view, QAction *action, bool clear)
{
  if (view) {
    if (clear == true) {
      view->rightMenu()->clear();
    }

    view->rightMenu()->addAction(action);
  }
}

void ZStackPresenter::updateRightMenu(ZStackView *view, QMenu *submenu, bool clear)
{
  if (view) {
    if (clear == true) {
      view->rightMenu()->clear();
    }

    view->rightMenu()->addMenu(submenu);
  }
}

void ZStackPresenter::updateView() const
{
  auto viewList = getViewList();
  for (ZStackView *view : viewList) {
    view->redraw();
  }
//  buddyView()->redraw();
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

/*
ZPoint ZStackPresenter::getModelPositionFromGlobalCursor(
    const QPoint &pos) const
{
  QPoint widgetPos = buddyView()->imageWidget()->mapFromGlobal(pos);
  return buddyView()->imageWidget()->transform(
        ZPoint(widgetPos.x(), widgetPos.y(), 0), neutu::data3d::ESpace::CANVAS,
        neutu::data3d::ESpace::MODEL);
}
*/

ZPoint ZStackPresenter::getModelPositionFromMouse(MouseButtonAction mba) const
{
//  int x, y;
  ZPoint pos;
  switch (mba) {
  case LEFT_RELEASE:
    pos = m_mouseEventProcessor.getLatestPosition(
          Qt::LeftButton, ZMouseEvent::EAction::RELEASE,
          neutu::data3d::ESpace::MODEL);
//    x = m_mouseLeftReleasePosition[0];
//    y = m_mouseLeftReleasePosition[1];
    break;
  case RIGHT_RELEASE:
    pos = m_mouseEventProcessor.getLatestPosition(
          Qt::RightButton, ZMouseEvent::EAction::RELEASE,
          neutu::data3d::ESpace::MODEL);
//    x = m_mouseRightReleasePosition[0];
//    y = m_mouseRightReleasePosition[1];
    break;
  case LEFT_PRESS:
    pos = m_mouseEventProcessor.getLatestPosition(
          Qt::LeftButton, ZMouseEvent::EAction::PRESS,
          neutu::data3d::ESpace::MODEL);
//    x = m_mouseLeftPressPosition[0];
//    y = m_mouseLeftPressPosition[1];
    break;
  case RIGHT_PRESS:
    pos = m_mouseEventProcessor.getLatestPosition(
          Qt::RightButton, ZMouseEvent::EAction::PRESS,
          neutu::data3d::ESpace::MODEL);
//    x = m_mouseRightPressPosition[0];
//    y = m_mouseRightPressPosition[1];
    break;
  case LEFT_DOUBLE_CLICK:
    pos = m_mouseEventProcessor.getLatestPosition(
          Qt::LeftButton, ZMouseEvent::EAction::DOUBLE_CLICK,
          neutu::data3d::ESpace::MODEL);
//    x = m_mouseLeftDoubleClickPosition[0];
//    y = m_mouseLeftDoubleClickPosition[1];
    break;
  case MOVE:
    pos = m_mouseEventProcessor.getLatestMouseEvent().getPosition(
          neutu::data3d::ESpace::MODEL);
//    x = m_mouseMovePosition[0];
//    y = m_mouseMovePosition[1];
    break;
  default:
    break;
  }

  return pos;

  /*
  return buddyView()->imageWidget()->transform(
        ZPoint(x, y, 0), neutu::data3d::ESpace::CANVAS,
        neutu::data3d::ESpace::MODEL);
        */
//  return buddyView()->imageWidget()->canvasCoordinate(QPoint(x, y));
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

#if 0
int ZStackPresenter::getSliceIndex() const {
  int sliceIndex = -1;
  if (!m_interactiveContext.isProjectView()) {
    sliceIndex = buddyView()->sliceIndex();
  }

#ifdef _DEBUG_2
  std::cout << "Slice index: " << sliceIndex << std::endl;
#endif

  return sliceIndex;
}
#endif

void ZStackPresenter::processMouseReleaseEvent(QMouseEvent *event, int viewId)
{
//  interactiveContext().setUniqueMode(ZInteractiveContext::INTERACT_FREE);

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
      m_mouseEventProcessor.process(event, ZMouseEvent::EAction::RELEASE, viewId);

  if (mouseEvent.isNull()) {
    return;
  }

  ZStackOperator op = m_mouseEventProcessor.getOperator();
  op.setViewId(viewId);
  process(op);

  if (isContextMenuOn()) {
    m_interactiveContext.blockContextMenu();
  }
}

void ZStackPresenter::moveCrossHairToMouse(int mouseX, int mouseY)
{
  emit movingCrossHairTo(mouseX, mouseY);
}

#if 0
void ZStackPresenter::moveImageToMouse(
    double srcX, double srcY, int mouseX, int mouseY)
{
#if 0
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
#endif

  buddyView()->moveViewPort(
        QPoint(neutu::iround(srcX), neutu::iround(srcY)), QPointF(mouseX, mouseY));

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
#endif

void ZStackPresenter::moveViewPort(ZStackView *view, int dx, int dy)
{
  if (view) {
    view->moveViewPort(dx, dy);
  }
//  buddyView()->updateImageScreen(ZStackView::EUpdateOption::QUEUED);
}

void ZStackPresenter::moveViewPort(
    ZStackView *view, const ZPoint &src, int a, int b)
{
  if (view) {
    view->moveViewPort(src, a, b);
  }
}

/*
void ZStackPresenter::moveViewPortTo(int x, int y)
{
  buddyView()->setViewPortOffset(x, y);
//  buddyView()->updateImageScreen(ZStackView::EUpdateOption::UPDATE_QUEUED);
}
*/

void ZStackPresenter::increaseZoomRatio(ZStackView *view)
{
  if (view) {
    view->increaseZoomRatio();
  }
}

void ZStackPresenter::decreaseZoomRatio(ZStackView *view)
{
  if (view) {
    view->decreaseZoomRatio();
  }
  //buddyView()->updateImageScreen();
  /*
  m_interactiveContext.setView(buddyView()->imageWidget()->projectRegion(),
                               buddyView()->imageWidget()->viewPort());
                               */
}

void ZStackPresenter::increaseZoomRatio(ZStackView *view, int x, int y)
{
  if (view) {
    view->increaseZoomRatio(x, y);
  }
}

void ZStackPresenter::decreaseZoomRatio(ZStackView *view, int x, int y)
{
  if (view) {
    view->decreaseZoomRatio(x, y);
  }
}

void ZStackPresenter::processMouseMoveEvent(QMouseEvent *event, int viewId)
{
#ifdef _DEBUG_2
  std::cout << "Recorder address: " << &(m_mouseEventProcessor.getRecorder())
            << std::endl;
#endif

  updateMouseCursorGlyphPos();

  const ZMouseEvent &mouseEvent = m_mouseEventProcessor.process(
        event, ZMouseEvent::EAction::MOVE, viewId);

  if (mouseEvent.isNull()) {
    return;
  }

#ifdef _DEBUG_2
  std::cout << "Recorder address: " << &(m_mouseEventProcessor.getRecorder())
            << std::endl;
#endif

  //mouseEvent.set(event, ZMouseEvent::EAction::ACTION_MOVE, m_mouseMovePosition[2]);
  ZStackOperator op = m_mouseEventProcessor.getOperator();

  process(op);
}

#if 0
QPointF ZStackPresenter::mapFromWidgetToStack(const QPoint &pos)
{
  /*
  return buddyView()->imageWidget()->canvasCoordinate(pos) +
      QPoint(buddyDocument()->getStackOffset().getX(),
             buddyDocument()->getStackOffset().getY());
             */
}

QPointF ZStackPresenter::mapFromGlobalToStack(const QPoint &pos)
{
  return mapFromWidgetToStack(buddyView()->imageWidget()->mapFromGlobal(pos));
}
#endif

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

void ZStackPresenter::setContextMenuFactory(
    std::unique_ptr<ZStackDocMenuFactory> factory)
{
  m_menuFactory = std::move(factory);
}

void ZStackPresenter::processMousePressEvent(QMouseEvent *event, int viewId)
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
        event, ZMouseEvent::EAction::PRESS, viewId);
  if (mouseEvent.isNull()) {
    return;
  }

  ZStackOperator op = m_mouseEventProcessor.getOperator();

  if (op.getHitObject() != NULL) {
    if (interactiveContext().getUniqueMode() ==
        ZInteractiveContext::EUniqueMode::INTERACT_FREE) {
      if (op.getHitObject()->getType() == ZStackObject::EType::CROSS_HAIR) {
        op.setOperation(ZStackOperator::OP_CROSSHAIR_GRAB);
      }
    }
  }

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
      op.setViewId(m_interactiveContext.getViewId());
      taken = process(op);
    }
  }

  return taken;
}

bool ZStackPresenter::processKeyPressEventForActiveStroke(QKeyEvent *event)
{
  bool taken = false;

  if (m_mouseCursorGlyph->isActivated(ZMouseCursorGlyph::ERole::ROLE_STROKE)) {
    ZStackOperator::EOperation opId =
        m_activeStrokeOperationMap.getOperation(
          event->key(), event->modifiers());

    ZStackOperator op;
    op.setOperation(opId);
    if (!op.isNull()) {
      op.setViewId(m_interactiveContext.getViewId());
      taken = process(op);
    }
  }

  return taken;
}

bool ZStackPresenter::processKeyPressEventForSwc(QKeyEvent *event)
{
  bool taken = false;

#ifdef _DEBUG_2
  std::cout << "Key V mapped to "
            << m_swcKeyOperationMap.getOperation(Qt::Key_V, Qt::NoModifier)
            << std::endl;
#endif

  ZStackOperator::EOperation opId =
      m_swcKeyOperationMap.getOperation(event->key(), event->modifiers());
  if (isOperatable(opId)) {
    ZStackOperator op;
    op.setOperation(opId);
    if (!op.isNull()) {
      op.setViewId(m_interactiveContext.getViewId());
      taken = process(op);
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
      op.setViewId(m_interactiveContext.getViewId());
      taken = process(op);
    }
  }

  return taken;
}

bool ZStackPresenter::processKeyPressEventForStroke(QKeyEvent *event)
{
  bool taken = false;
  switch (event->key()) {
  case Qt::Key_R:
    if (event->modifiers() == Qt::ShiftModifier ||
        event->modifiers() == Qt::ControlModifier) {
      tryDrawRectMode();
      if (event->modifiers() == Qt::ControlModifier) {
        interactiveContext().setRectSpan(true);
      }
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
      m_mouseCursorGlyph->updateActiveGlyph([&](ZStackObject *obj) {
        obj->setLabel(event->key() - Qt::Key_0);
        buddyDocument()->processObjectModified(obj);
      });
      /*
      ZStroke2d *stroke = m_mouseCursorGlyph->getGlyph<ZStroke2d>(
            ZMouseCursorGlyph::ERole::ROLE_STROKE);
      stroke->setLabel(event->key() - Qt::Key_0);
      buddyDocument()->processObjectModified(stroke);
      */
      taken = true;
    }
    break;
  case Qt::Key_QuoteLeft:
    if (m_interactiveContext.strokeEditMode() ==
        ZInteractiveContext::STROKE_DRAW) {
      m_mouseCursorGlyph->updateActiveGlyph([&](ZStackObject *obj) {
        obj->setLabel(255);
        buddyDocument()->processObjectModified(obj);
      });

//      ZStroke2d *stroke = m_mouseCursorGlyph->getGlyph<ZStroke2d>(
//            ZMouseCursorGlyph::ERole::ROLE_STROKE);
//      stroke->setLabel(255);
//      buddyView()->paintActiveDecoration();
//      buddyDocument()->processObjectModified(stroke);
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

/*
void ZStackPresenter::setZoomRatio(double ratio)
{
  setZoomRatio(getView(), ratio);
}
*/

void ZStackPresenter::setZoomRatio(ZStackView *view, double ratio)
{
  //m_zoomRatio = ratio;
  //CLIP_VALUE(m_zoomRatio, 1, 16);
  //buddyView()->imageWidget()->setZoomRatio(m_zoomRatio);
  if (view) {
    view->imageWidget()->setZoomRatio(ratio);
  }
}

#if 0
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
#endif

bool ZStackPresenter::processKeyPressEventOther(QKeyEvent *event, int viewId)
{
  bool processed = false;
  ZStackView *view = getView(viewId);

  switch (event->key()) {
  case Qt::Key_P:
    if (event->modifiers() == Qt::ControlModifier) {
      if (interactiveContext().isProjectView()) {
        interactiveContext().setViewMode(ZInteractiveContext::VIEW_NORMAL);
      } else {
        interactiveContext().setViewMode(ZInteractiveContext::VIEW_PROJECT);
      }
      updateView();
      emit(viewModeChanged());
      processed = true;
    } else if (event->modifiers() == Qt::ShiftModifier) {
      buddyDocument()->pushSelectedLocsegChain();
      updateView();
      processed = true;
    }
    break;

  case Qt::Key_Equal:
  case Qt::Key_Up:
    if (event->modifiers() == Qt::NoModifier) {
      increaseZoomRatio(view);
      processed = true;
    }
    break;
  case Qt::Key_2:
    if (m_interactiveContext.strokeEditMode() !=
        ZInteractiveContext::STROKE_DRAW) {
      increaseZoomRatio(view);
      processed = true;
    }
    break;
  case Qt::Key_Minus:
  case Qt::Key_Down:
    if (event->modifiers() == Qt::NoModifier) {
      decreaseZoomRatio(view);
      processed = true;
    }
    break;
  case Qt::Key_1:
    if (m_interactiveContext.strokeEditMode() !=
        ZInteractiveContext::STROKE_DRAW) {
      decreaseZoomRatio(view);
      processed = true;
    }
    break;
  case Qt::Key_W:
    if (event->modifiers() == Qt::ShiftModifier) {
      moveViewPort(view, 0, 10);
    } else {
      moveViewPort(view, 0, 1);
    }
    processed = true;
    break;
  case Qt::Key_A:
    if (event->modifiers() == Qt::ShiftModifier) {
      moveViewPort(view, 10, 0);
    } else {
      moveViewPort(view, 1, 0);
    }
    processed = true;
    break;

  case Qt::Key_S:
    if (event->modifiers() == Qt::ShiftModifier) {
      moveViewPort(view, 0, -10);
      processed = true;
    } else if (event->modifiers() == Qt::NoModifier) {
      moveViewPort(view, 0, -1);
      processed = true;
    } else if (event->modifiers() == Qt::ControlModifier) {
      if (getParentFrame() != NULL) {
        buddyDocument()->saveSwc(getParentFrame());
        processed = true;
      }
    }
    break;

  case Qt::Key_D:
    if (event->modifiers() == Qt::ShiftModifier) {
      moveViewPort(view, -10, 0);
    } else {
      moveViewPort(view, -1, 0);
    }
    processed = true;
    break;

  case Qt::Key_Left:
    if (!interactiveContext().isProjectView()) {
      int step = -1;

      if (event->modifiers() & Qt::ShiftModifier) {
        step = -5;
      }
      if (view) {
        view->stepSlice(step);
      }
      processed = true;
    }
    break;

  case Qt::Key_Right:
    if (!interactiveContext().isProjectView()) {
      int step = 1;

      if (event->modifiers() & Qt::ShiftModifier) {
        step = 5;
      }
      if (view) {
        view->stepSlice(step);
      }
      processed = true;
    }
    break;


  case Qt::Key_M:
    if (m_interactiveContext.isStackSliceView()) {
      if (interactiveContext().markingPuncta() && buddyDocument()->hasStackData() &&
          (!buddyDocument()->getStack()->isVirtual())) {
        ZPoint dataPos = getModelPositionFromMouse(MOVE);
        buddyDocument()->markPunctum(dataPos.x(), dataPos.y(), dataPos.z());
        processed = true;
      }
    }
    break;

  case Qt::Key_Escape:
    enterSwcSelectMode();
//    m_interactiveContext.setSwcEditMode(ZInteractiveContext::SWC_EDIT_SELECT);
    m_interactiveContext.setTubeEditMode(ZInteractiveContext::TUBE_EDIT_OFF);
    //turnOffStroke();
    exitStrokeEdit();
    updateCursor();
    processed = true;
    break;
  case Qt::Key_Comma:
    m_mouseCursorGlyph->addActiveGlyphSize(
          -1.0, [&](ZStackObject *obj) {
      if (obj) {
        buddyDocument()->processObjectModified(obj);
        processed = true;
      }
    });

    /*
    if (isActiveObjectOn()) {
      ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(getFirstOnActiveObject());
      if (stroke != NULL) {
        stroke->addWidth(-1.0);
      }
      buddyView()->paintActiveDecoration();
      processed = true;
    }
    */
    break;
  case Qt::Key_Period:
    if (m_mouseCursorGlyph->isActivated()) {
      if (event->modifiers() == Qt::NoModifier) {
        m_mouseCursorGlyph->addActiveGlyphSize(
              1.0, [&](ZStackObject *obj) {
          if (obj) {
            buddyDocument()->processObjectModified(obj);
            processed = true;
          }
        });
        /*
        ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(getFirstOnActiveObject());
        if (stroke != NULL) {
          stroke->addWidth(1.0);
        }
        buddyView()->paintActiveDecoration();
        processed = true;
        */
      } else if (event->modifiers() == Qt::ShiftModifier) {
        /*
        if (estimateActiveStrokeWidth()) {
          buddyView()->paintActiveDecoration();
          processed = true;
        }
        */
      }
    }
    break;
  case Qt::Key_Space:
    if (GET_APPLICATION_NAME == "FlyEM") {
      if (buddyDocument()->getTag() != neutu::Document::ETag::FLYEM_PROOFREAD) {
        if (event->modifiers() == Qt::ShiftModifier) {
          ZOUT(LTRACE(), 5) << "Starting watershed ...";
          buddyDocument()->runSeededWatershed();
        } else {
          buddyDocument()->runLocalSeededWatershed();
        }
        processed = true;
      }
    }
    break;
  case Qt::Key_Z:
    if (getParentMvc() != NULL) {
      if (event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier)) {
        buddyDocument()->getAction(ZActionFactory::ACTION_REDO)->trigger();
//        buddyDocument()->undoStack()->redo();
        processed = true;
      } else if (event->modifiers() == Qt::ControlModifier) {
        buddyDocument()->getAction(ZActionFactory::ACTION_UNDO)->trigger();
//        buddyDocument()->undoStack()->undo();
        processed = true;
      } else if (event->modifiers() == Qt::ShiftModifier) {
        if (view) {
          view->maximizeViewPort();
        }
        processed = true;
      }
    }
    break;
    /*
  case Qt::Key_F:
    toggleObjectVisible();
    break;
    */
  default:
    break;
  }

  return processed;
}

bool ZStackPresenter::processKeyPressEvent(QKeyEvent *event, int viewId)
{
  m_interactiveContext.setViewId(viewId);

  bool processed = buddyDocument()->getKeyProcessor()->processKeyEvent(event);

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


  processed = processKeyPressEventOther(event, viewId);

  if (!processed) {
    processed = customKeyProcess(event);
  }

  return processed;
}

bool ZStackPresenter::customKeyProcess(QKeyEvent * /*event*/)
{
  return false;
}

void ZStackPresenter::processMouseDoubleClickEvent(
    QMouseEvent *event, int viewId)
{
  const ZMouseEvent &mouseEvent = m_mouseEventProcessor.process(
        event, ZMouseEvent::EAction::DOUBLE_CLICK, viewId);

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
    auto viewList = getViewList();
    for (ZStackView *view : viewList) {
      view->updateObjectCanvasVisbility(v);
    }
  }
}

void ZStackPresenter::toggleObjectVisible()
{
  setObjectVisible(!isObjectVisible());
}

void ZStackPresenter::suppressObjectVisible(bool v)
{
  if (v) {
    m_oldShowObject = m_showObject;
    m_showObject = false;
  } else {
    m_showObject = m_oldShowObject;
  }
}

void ZStackPresenter::setObjectStyle(ZStackObject::EDisplayStyle style)
{
  if (m_objStyle != style) {
    m_objStyle = style;
    auto viewList = getViewList();
    for (ZStackView *view : viewList) {
      view->redrawObject();
    }
//    buddyView()->redrawObject();
//    updateView();
  }
}

void ZStackPresenter::setSliceMode(
    int viewId, neutu::data3d::EDisplaySliceMode mode)
{
  m_interactiveContext.setSliceMode(viewId, mode);
}

neutu::data3d::EDisplaySliceMode ZStackPresenter::getSliceMode(int viewId) const
{
  return m_interactiveContext.getSliceMode(viewId);
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
  if (c >= 0 && c < (int) m_grayScale.size()) {
    m_grayScale[c] = scale;
    m_grayOffset[c] = offset;
  }
}

double ZStackPresenter::getGrayScale(int c) const
{
  double scale = 1.0;
  if (c >= 0 && c < (int) m_grayScale.size()) {
    scale = m_grayScale[c];
  }

  return scale;
}

double ZStackPresenter::getGrayOffset(int c) const
{
  double offset = 0.0;
  if (c >= 0 && c < (int) m_grayOffset.size()) {
    offset = m_grayOffset[c];
  }
  return offset;
}

void ZStackPresenter::optimizeStackBc()
{
  if (buddyDocument() != NULL) {
    ZStack *stack = buddyDocument()->getStack();
    if (stack != NULL) {
      if (!stack->isVirtual()) {
        double scale, offset;
        m_grayScale.resize(stack->channelNumber());
        m_grayOffset.resize(stack->channelNumber());
        for (int i=0; i<stack->channelNumber(); i++) {
          stack->bcAdjustHint(&scale, &offset, i);
          setStackBc(scale, offset, i);
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
  buddyDocument()->binarize(getDefaultView()->getIntensityThreshold());
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

ZPoint ZStackPresenter::getMousePositionInStack(
    Qt::MouseButtons buttons, ZMouseEvent::EAction action) const
{
  const ZMouseEvent &event =
      m_mouseEventProcessor.getMouseEvent(buttons, action);
  ZPoint pt = event.getStackPosition();

  return pt;
}

void ZStackPresenter::traceTube()
{
  //QPointF dataPos = stackPositionFromMouse(LEFT_RELEASE);
//  buddyView()->setScreenCursor(Qt::BusyCursor);

  ZPoint pt = getMousePositionInStack(
        Qt::LeftButton, ZMouseEvent::EAction::RELEASE);
  ZStackDoc *doc = buddyDocument();
  if (doc->getStack()->channelNumber() == 1) {
    if (doc->isZProjection(pt.getZ())) {
      doc->executeTraceSwcBranchCommand(pt.x(), pt.y());
    } else {
      doc->executeTraceSwcBranchCommand(pt.x(), pt.y(), pt.z());
    }
  } else if (buddyDocument()->getStack()->channelNumber() > 1) {
    ChannelDialog dlg(NULL, buddyDocument()->getStack()->channelNumber());
    if (dlg.exec() == QDialog::Accepted) {
      int channel = dlg.channel();
      buddyDocument()->executeTraceSwcBranchCommand(
            pt.x(), pt.y(), pt.z(), channel);
    }
  }
  updateCursor();
}

void ZStackPresenter::fitSegment()
{
  ZPoint dataPos = getModelPositionFromMouse(LEFT_RELEASE);
//  QPointF dataPos = stackPositionFromMouse(LEFT_RELEASE);
  /*
  buddyDocument()->fitseg(dataPos.x(), dataPos.y(),
                          m_mouseLeftReleasePosition[2]);
                          */
  if (buddyDocument()->getStack()->depth() == 1) {
    buddyDocument()->fitRect(dataPos.x(), dataPos.y(), dataPos.z());
  } else {
    buddyDocument()->fitseg(dataPos.x(), dataPos.y(), dataPos.z());
  }
}

void ZStackPresenter::fitEllipse()
{
  ZPoint dataPos = getModelPositionFromMouse(LEFT_RELEASE);
//  ZPoint dataPos = stackPositionFromMouse(LEFT_RELEASE);
  buddyDocument()->fitEllipse(dataPos.x(), dataPos.y(), dataPos.z());
}

void ZStackPresenter::markPuncta()
{
  const ZMouseEvent& event = m_mouseEventProcessor.getMouseEvent(
        Qt::LeftButton, ZMouseEvent::EAction::RELEASE);
  ZPoint currentStackPos = event.getPosition(neutu::ECoordinateSystem::STACK);
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
  ZPoint dataPos = getModelPositionFromMouse(LEFT_RELEASE);
//  QPointF dataPos = stackPositionFromMouse(LEFT_RELEASE);
  buddyDocument()->dropseg(dataPos.x(), dataPos.y(), dataPos.z());
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
  buddyDocument()->deleteObject(ZStackObject::EType::TYPE_PUNCTUM);
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
  if (buddyDocument()->getStack()->kind() != GREY) {
    optimizeStackBc();
  }
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

#if 0
void ZStackPresenter::updateSwcExtensionHint()
{
  if (isActiveObjectOn(ROLE_SWC)) {
    const Swc_Tree_Node *selected = getSelectedSwcNode();
    if (selected) {
      ZSwcTree *tree = getActiveObject<ZSwcTree>(ROLE_SWC);
      set_double_node_mode(tree, selected);
#if 0
      if (tree) {
        Swc_Tree_Node *tn = tree->firstRegularRoot();
        SwcTreeNode::setCenter(tn, SwcTreeNode::center(selected));
        SwcTreeNode::setRadius(tn, 0);
        if (!SwcTreeNode::hasChild(tn)) {
          tn = SwcTreeNode::MakeChild(tn);
        } else {
          tn = SwcTreeNode::firstChild(tn);
        }
        const ZMouseEvent& event = m_mouseEventProcessor.getLatestMouseEvent();
        SwcTreeNode::setCenter(tn, event.getDataPosition());
        SwcTreeNode::setRadius(tn, SwcTreeNode::radius(selected));
        tree->deprecate(ZSwcTree::ALL_COMPONENT);
#ifdef _DEBUG_2
        tree->print();
#endif
      }
#endif
      /*
      ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_SWC);
      stroke->set(SwcTreeNode::x(tn), SwcTreeNode::y(tn));
      stroke->setWidth(SwcTreeNode::radius(tn) * 2.0);
//      QPointF pos = mapFromGlobalToStack(QCursor::pos());
      const ZMouseEvent& event = m_mouseEventProcessor.getLatestMouseEvent();
      ZPoint currentStackPos = event.getPosition(neutu::ECoordinateSystem::STACK);
      stroke->append(currentStackPos.getX(), currentStackPos.getY());
      */
    }
  }
}
#endif

void ZStackPresenter::processObjectModified(const ZStackObjectInfoSet &objSet)
{
  auto viewList = getViewList();
  for (ZStackView *view : viewList) {
    if (objSet.hasObjectAddedOrRemoved()) {
      view->invalidateObjectSorter();
    }
    view->paintObject(objSet.getTarget());
  }
}

/*
ZSliceViewTransform ZStackPresenter::getSliceViewTransform() const
{
  return buddyView()->getSliceViewTransform();
}
*/

void ZStackPresenter::setSliceViewTransform(
    int viewId, const ZSliceViewTransform &transform)
{
  /*
  if (transform.getSliceAxis() !=
      m_interactiveContext.getSliceViewTransform(viewId).getSliceAxis()) {
    setSliceAxis(viewId, transform.getSliceAxis());
  }
  */
  m_interactiveContext.setSliceViewTransform(viewId, transform);
  updateMouseCursorGlyphPos();
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
      notifyUser("Left click to extend. Path calculation is off when '"
                 CTRL_KEY_NAME "' is pressed."
                 "Right click to exit extending mode.");
      startSwcEdit(ZInteractiveContext::SWC_EDIT_EXTEND);
#if 0
      interactiveContext().setSwcEditMode(ZInteractiveContext::SWC_EDIT_EXTEND);
      m_mouseCursorGlyph->activate(ZMouseCursorGlyph::ERole::ROLE_SWC);
      updateMouseCursorGlyphPos();

//      m_stroke.setFilled(false);
//      QPointF pos = mapFromGlobalToStack(QCursor::pos());

//      turnOnActiveObject(ROLE_SWC);
//      updateSwcExtensionHint();
      /*
      ZSwcTree *obj = getActiveObject<ZStackBall>(ROLE_SWC);
      ball->setCenter(SwcTreeNode::center(tn));

      const ZMouseEvent& event = m_mouseEventProcessor.getLatestMouseEvent();
      ZPoint currentStackPos = event.getPosition(neutu::ECoordinateSystem::STACK);
      ball->append(currentStackPos.getX(), currentStackPos.getY());

//      stroke->append(pos.x(), pos.y());
      //m_stroke.set(SwcTreeNode::x(tn), SwcTreeNode::y(tn));
      ball->setWidth(SwcTreeNode::radius(tn) * 2.0);
      */
//      turnOnStroke();
//      m_stroke.setTarget(neutu::data3d::ETarget::TARGET_WIDGET);

      updateCursor();
#endif
      succ = true;
    }
  }

  return succ;
}

void ZStackPresenter::exitSwcExtendMode()
{
  if (interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_EXTEND ||
      interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_SMART_EXTEND) {
    interactiveContext().setSwcEditMode(ZInteractiveContext::SWC_EDIT_OFF);
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

#if 0
void ZStackPresenter::enterSwcAddNodeMode(double x, double y)
{
  exitEdit();

  interactiveContext().setSwcEditMode(ZInteractiveContext::SWC_EDIT_ADD_NODE);

  turnOnActiveObject(ROLE_SWC);
  updateCursor();

#if 0
  ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_SWC);

  stroke->set(x, y);

  turnOnActiveObject(ROLE_SWC);
//  m_stroke.setEraser(false);
//  m_stroke.setFilled(false);
//  m_stroke.setTarget(neutu::data3d::ETarget::TARGET_WIDGET);
//  turnOnStroke();
  //buddyView()->paintActiveDecoration();
  updateCursor();
#endif
}
#endif

void ZStackPresenter::toggleSwcSkeleton(bool state)
{
  buddyDocument()->showSwcFullSkeleton(state);
}

template<typename T>
void ZStackPresenter::startSwcEdit(T option)
{
  exitEdit();

  interactiveContext().setSwcEditMode(option);

  m_mouseCursorGlyph->activate(ZMouseCursorGlyph::ERole::ROLE_SWC);
  updateMouseCursorGlyphPos();
  updateCursor();
}

void ZStackPresenter::trySwcAddNodeMode()
{
  if (interactiveContext().strokeEditMode() !=
      ZInteractiveContext::STROKE_DRAW) {
    startSwcEdit(ZInteractiveContext::SWC_EDIT_ADD_NODE);
  }
  /*
  ZPoint pos = getModelPositionFromGlobalCursor(QCursor::pos());
//  QPointF pos = mapFromGlobalToStack(QCursor::pos());
  trySwcAddNodeMode(pos.x(), pos.y());
  */
}

ZStackBall ZStackPresenter::getActiveDecorationForSwc() const
{
  ZStackBall ball;

  m_mouseCursorGlyph->useActiveGlyph([&](const ZPoint &center, double r) {
    ball.set(center, r);
  });

  return ball;
}

bool ZStackPresenter::addActiveDecorationAsSwc()
{
  bool succ = false;
  ZStackObjectRole::TRole role = ZStackObjectRole::ROLE_NONE;
  if (buddyDocument()->getTag() == neutu::Document::ETag::FLYEM_ROI ||
      paintingRoi()) {
    role = ZStackObjectRole::ROLE_ROI;
  }
  m_mouseCursorGlyph->useActiveGlyph([&](const ZPoint &center, double r) {
    if (buddyDocument()->executeAddSwcNodeCommand(
          center, r, role)) {
      if (buddyDocument()->getTag() == neutu::Document::ETag::FLYEM_ROI) {
        buddyDocument()->selectSwcTreeNode(center, false);
        enterSwcExtendMode();
      }
      succ = true;
    }
  });

  return succ;
  /*
  ZSwcTree *tree = m_mouseCursorGlyph->getActiveGlyph<ZSwcTree>(
        ZMouseCursorGlyph::ERole::ROLE_SWC);
//  ZSwcTree *tree = getActiveObject<ZSwcTree>(ROLE_SWC);
  if (tree) {
    ZStackObjectRole::TRole role = ZStackObjectRole::ROLE_NONE;
    if (buddyDocument()->getTag() == neutu::Document::ETag::FLYEM_ROI ||
        paintingRoi()) {
      role = ZStackObjectRole::ROLE_ROI;
    }
    Swc_Tree_Node *tn = tree->firstRegularRoot();
    if (buddyDocument()->executeAddSwcNodeCommand(
          SwcTreeNode::center(tn), SwcTreeNode::radius(tn), role)) {
      if (buddyDocument()->getTag() == neutu::Document::ETag::FLYEM_ROI) {
        buddyDocument()->selectSwcTreeNode(SwcTreeNode::center(tn), false);
        enterSwcExtendMode();
      }
      return true;
    }
  }

  return false;
  */
}

/*
void ZStackPresenter::trySwcAddNodeMode(double x, double y)
{
  if (interactiveContext().strokeEditMode() !=
      ZInteractiveContext::STROKE_DRAW) {
    enterSwcAddNodeMode(x, y);
  }
}
*/

void ZStackPresenter::tryPaintStrokeMode()
{
  ZPoint pos = getCurrentMousePosition(neutu::data3d::ESpace::MODEL);
  if (pos.isValid()) {
//  ZPoint pos = getModelPositionFromGlobalCursor(QCursor::pos());
//  QPointF pos = mapFromGlobalToStack(QCursor::pos());
    tryDrawStrokeMode(pos.x(), pos.y(), false);
  }
}

#if 0
void ZStackPresenter::tryDrawRectMode()
{
  /*
//  ZPoint pos = getModelPositionFromGlobalCursor(QCursor::pos());
//  QPointF pos = mapFromGlobalToStack(QCursor::pos());
  ZPoint pos = getCurrentMousePosition(neutu::data3d::ESpace::MODEL);
  if (pos.isValid()) {
#ifdef _DEBUG_
    std::cout << "ZStackPresenter::tryDrawRectMode " << "@" << pos << std::endl;
#endif
    tryDrawRectMode(pos.x(), pos.y());
  }
  */
}
#endif

void ZStackPresenter::cancelRectRoi()
{
  buddyDocument()->executeRemoveRectRoiCommand();
}

void ZStackPresenter::tryEraseStrokeMode()
{
//  ZPoint pos = getModelPositionFromGlobalCursor(QCursor::pos());
//  QPointF pos = mapFromGlobalToStack(QCursor::pos());
  ZPoint pos = getCurrentMousePosition(neutu::data3d::ESpace::MODEL);
  if (pos.isValid()) {
    tryDrawStrokeMode(pos.x(), pos.y(), true);
  }
}

void ZStackPresenter::tryDrawStrokeMode(double x, double y, bool isEraser)
{
  if (GET_APPLICATION_NAME == "Biocytin" ||
      GET_APPLICATION_NAME == "FlyEM") {
    if ((interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_OFF /*||
        interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_SELECT*/) &&
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

void ZStackPresenter::tryDrawRectMode()
{
  if (GET_APPLICATION_NAME == "FlyEM") {
    if ((interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_OFF /*||
         interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_SELECT*/) &&
        interactiveContext().tubeEditMode() == ZInteractiveContext::TUBE_EDIT_OFF &&
        interactiveContext().isStrokeEditModeOff()) {
      enterDrawRectMode();
    }
  } else if (buddyDocument()->getTag() == neutu::Document::ETag::BIOCYTIN_PROJECTION) {
    if ((interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_OFF /*||
         interactiveContext().swcEditMode() == ZInteractiveContext::SWC_EDIT_SELECT*/) &&
        interactiveContext().tubeEditMode() == ZInteractiveContext::TUBE_EDIT_OFF &&
        interactiveContext().isStrokeEditModeOff()) {
      enterDrawRectMode();
    }
  }
  interactiveContext().setRectSpan(false);
}

void ZStackPresenter::enterDrawStrokeMode(double /*x*/, double /*y*/)
{
//  buddyDocument()->mapToDataCoord(&x, &y, NULL);

  interactiveContext().setStrokeEditMode(ZInteractiveContext::STROKE_DRAW);
  m_mouseCursorGlyph->activate(ZMouseCursorGlyph::ERole::ROLE_STROKE);
  updateMouseCursorGlyphPos();

//  ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_STROKE);
//  stroke->set(x, y);
//  stroke->setEraser(false);
//  turnOnActiveObject(ROLE_STROKE);
//  m_stroke.setFilled(true);
//  m_stroke.setTarget(neutu::data3d::ETarget::TARGET_OBJECT_CANVAS);
//  turnOnStroke();
  //buddyView()->paintActiveDecoration();

  updateCursor();
}

void ZStackPresenter::enterDrawRectMode()
{
#ifdef _DEBUG_
    std::cout << "Enter rect drawing" << std::endl;
#endif

//  buddyDocument()->mapToDataCoord(&x, &y, NULL);
  interactiveContext().setRectEditMode(ZInteractiveContext::RECT_DRAW);
  updateCursor();
}

void ZStackPresenter::enterEraseStrokeMode(double x, double y)
{
  m_mouseCursorGlyph->activate(
        ZMouseCursorGlyph::ERole::ROLE_STROKE, [=](ZStackObject *obj) {
    ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(obj);
    if (stroke) {
      stroke->set(x, y);
      stroke->setEraser(true);
      buddyDocument()->processObjectModified(stroke);
    }
  });

//  buddyDocument()->mapToDataCoord(&x, &y, NULL);
//  ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_STROKE);
//  stroke->set(x, y);
//  m_stroke.setFilled(true);
//  stroke->setEraser(true);
//  m_stroke.setTarget(neutu::data3d::ETarget::TARGET_OBJECT_CANVAS);
//  turnOnStroke();
//  turnOnActiveObject(ROLE_STROKE);
  //buddyView()->paintActiveDecoration();
  interactiveContext().setStrokeEditMode(ZInteractiveContext::STROKE_DRAW);
  updateCursor();
}

void ZStackPresenter::exitEdit()
{
  if (interactiveContext().turnOffEditMode()) {
    turnOffActiveObject();
    //  interactiveContext().setTubeEditMode(ZInteractiveContext::TUBE_EDIT_OFF);
    //  interactiveContext().setMarkPunctaMode(ZInteractiveContext::MARK_PUNCTA_OFF);
    //  interactiveContext().setStrokeEditMode(ZInteractiveContext::STROKE_EDIT_OFF);
    //  interactiveContext().setRectEditMode(ZInteractiveContext::RECT_EDIT_OFF);
    //  interactiveContext().setBookmarkEditMode(ZInteractiveContext::BOOKMARK_EDIT_OFF);
    //  interactiveContext().setTodoEditMode(ZInteractiveContext::TODO_EDIT_OFF);
    //  interactiveContext().setSynapseEditMode(ZInteractiveContext::SYNAPSE_EDIT_OFF);
    //  interactiveContext().setSwcEditMode(ZInteractiveContext::SWC_EDIT_OFF);

    updateCursor();
    m_interactiveContext.setExitingEdit(true);
  }
}

void ZStackPresenter::exitStrokeEdit()
{
  turnOffActiveObject();
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

//    emit exitingRectEdit();
  }
}

void ZStackPresenter::exitBookmarkEdit()
{
  turnOffActiveObject();
  interactiveContext().setBookmarkEditMode(ZInteractiveContext::BOOKMARK_EDIT_OFF);
  updateCursor();
  m_interactiveContext.setExitingEdit(true);
}

void ZStackPresenter::exitTodoEdit()
{
  turnOffActiveObject();
  interactiveContext().setTodoEditMode(ZInteractiveContext::TODO_EDIT_OFF);
  updateCursor();
  m_interactiveContext.setExitingEdit(true);
}

void ZStackPresenter::exitSynapseEdit()
{
  turnOffActiveObject();
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
  buddyDocument()->executeSwcNodeChangeZCommand(getDefaultView()->sliceIndex());
  updateCursor();
}

void ZStackPresenter::changeSelectedSwcNodeFocus()
{
  buddyDocument()->executeSwcNodeChangeZCommand(getDefaultView()->sliceIndex());
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
  if (m_interactiveContext.swcEditMode() != ZInteractiveContext::SWC_EDIT_OFF /*||
      m_interactiveContext.swcEditMode() != ZInteractiveContext::SWC_EDIT_SELECT*/) {
    m_interactiveContext.setExitingEdit(true);
  }

  if (buddyDocument()->hasSwc()) {
    notifyUser("Use mouse to select nodes");
  }

  turnOffActiveObject();
  m_interactiveContext.setSwcEditMode(ZInteractiveContext::SWC_EDIT_OFF);
  updateCursor();
}

void ZStackPresenter::updateCutPlane(
    neutu::EAxis a1, neutu::EAxis a2, neutu::EAxis a3)
{
  auto viewList = getViewList();
  if (!viewList.empty()) {
    viewList[0]->setCutPlane(a1);
    if (viewList.size() > 1) {
      viewList[1]->setCutPlane(a2);
      if (viewList.size() > 2) {
        viewList[2]->setCutPlane(a3);
      }
    }
  }
  m_mainViewAxis = a1;
  updateViewLayout();
}

void ZStackPresenter::updateViewLayout()
{
  std::vector<int> viewLayout;
  switch (m_viewCount) {
  case 1:
    viewLayout = {0};
    break;
  case 2:
    if (m_mainViewAxis == neutu::EAxis::Y) {
      viewLayout = {0, 2};
    } else {
      viewLayout = {0, 1};
    }
    break;
  case 3:
    switch (m_mainViewAxis) {
    case neutu::EAxis::X:
      viewLayout = {0, 1, 3};
      break;
    case neutu::EAxis::Y:
      viewLayout = {0, 2, 3};
      break;
    default:
      viewLayout = {0, 1, 2};
    }
    break;
  default:
    break;
  }

#ifdef _DEBUG_
  std::cout << "Updating view layout: ";
  for (int index : viewLayout) {
    std::cout << index << ", ";
  }
  std::cout << std::endl;
#endif

  emit updatingViewLayout(viewLayout);
}

void ZStackPresenter::setViewCursor(const QCursor &cursor)
{
  auto viewList = getViewList();
  for (ZStackView *view : viewList) {
    view->setScreenCursor(cursor);
  }
}

void ZStackPresenter::updateCursor()
{
  if (this->interactiveContext().swcEditMode() ==
      ZInteractiveContext::SWC_EDIT_EXTEND ||
      this->interactiveContext().swcEditMode() ==
      ZInteractiveContext::SWC_EDIT_SMART_EXTEND ||
      this->interactiveContext().rectEditMode() ==
      ZInteractiveContext::RECT_DRAW) {
    setViewCursor(Qt::PointingHandCursor);
  } else if (this->interactiveContext().swcEditMode() ==
      ZInteractiveContext::SWC_EDIT_MOVE_NODE) {
    setViewCursor(Qt::ClosedHandCursor);
  } else if (this->interactiveContext().swcEditMode() ==
             ZInteractiveContext::SWC_EDIT_LOCK_FOCUS) {
    setViewCursor(Qt::ClosedHandCursor);
  } else if (interactiveContext().swcEditMode() ==
             ZInteractiveContext::SWC_EDIT_ADD_NODE ||
             interactiveContext().synapseEditMode() ==
             ZInteractiveContext::SYNAPSE_ADD_PRE ||
             interactiveContext().synapseEditMode() ==
             ZInteractiveContext::SYNAPSE_ADD_POST ||
             interactiveContext().synapseEditMode() ==
             ZInteractiveContext::SYNAPSE_MOVE){
    if (buddyDocument()->getTag() == neutu::Document::ETag::FLYEM_ROI) {
      setViewCursor(Qt::PointingHandCursor);
    } else {
      setViewCursor(Qt::PointingHandCursor);
    }
    /*
    buddyView()->setScreenCursor(
          ZCursorStore::getInstance().getSmallCrossCursor());
          */
  } else if (this->interactiveContext().tubeEditMode() ==
             ZInteractiveContext::TUBE_EDIT_HOOK ||
             interactiveContext().swcEditMode() ==
             ZInteractiveContext::SWC_EDIT_CONNECT) {
    setViewCursor(Qt::SizeBDiagCursor);
  } else if (this->interactiveContext().strokeEditMode() ==
             ZInteractiveContext::STROKE_DRAW) {
    setViewCursor(Qt::PointingHandCursor);
    /*
    buddyView()->setScreenCursor(
          ZCursorStore::getInstance().getSmallCrossCursor());
          */
    //buddyView()->setScreenCursor(Qt::PointingHandCursor);
  } else if (interactiveContext().bookmarkEditMode() ==
             ZInteractiveContext::BOOKMARK_ADD ||
             interactiveContext().todoEditMode() ==
             ZInteractiveContext::TODO_ADD_ITEM){
    setViewCursor(Qt::PointingHandCursor);
  } else if (interactiveContext().getUniqueMode()
             == ZInteractiveContext::INTERACT_MOVE_CROSSHAIR) {
    setViewCursor(Qt::ClosedHandCursor);
  } else {
    setViewCursor(Qt::CrossCursor);
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
        Qt::RightButton, ZMouseEvent::EAction::RELEASE);
  ZPoint pt = event.getStackPosition();

//  ZPoint pt = getLastMousePosInStack();

  emit orthoViewTriggered(pt.x(), pt.y(), pt.z());
}

void ZStackPresenter::notifyOrthoViewBigTriggered()
{
  const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
        Qt::RightButton, ZMouseEvent::EAction::RELEASE);
  ZPoint pt = event.getStackPosition();

//  ZPoint pt = getLastMousePosInStack();

  emit orthoViewBigTriggered(pt.x(), pt.y(), pt.z());
}

void ZStackPresenter::copyCurrentPosition()
{
  const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
        Qt::RightButton, ZMouseEvent::EAction::RELEASE);
  ZPoint pt = event.getDataPosition();

  ZGlobal::GetInstance().setDataPosition(pt.x(), pt.y(), pt.z());

  std::string posStr =
      pt.roundToIntPoint().toString(NeutubeConfig::GetPointPosFormat());
  ZGlobal::CopyToClipboard(posStr);

  buddyDocument()->notify(QString("%1 copied").arg(posStr.c_str()));
}

void ZStackPresenter::copyLabelId()
{
  const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
        Qt::RightButton, ZMouseEvent::EAction::RELEASE);
  ZPoint pt = event.getDataPosition();

  uint64_t id = buddyDocument()->getLabelId(
        neutu::iround(pt.x()), neutu::iround(pt.y()), neutu::iround(pt.z()));

  ZGlobal::CopyToClipboard(std::to_string(id));

  buddyDocument()->notify(QString("%1 copied").arg(id));
}

void ZStackPresenter::copySupervoxelId()
{
  const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
        Qt::RightButton, ZMouseEvent::EAction::RELEASE);
  ZPoint pt = event.getDataPosition();

  uint64_t id = buddyDocument()->getSupervoxelId(
        neutu::iround(pt.x()), neutu::iround(pt.y()), neutu::iround(pt.z()));

  ZGlobal::CopyToClipboard(std::to_string(id));

  buddyDocument()->notify(QString("%1 copied").arg(id));
}

void ZStackPresenter::copyLink(const QString &/*option*/) const
{

}

void ZStackPresenter::copyNeuroglancerLink()
{
  ZJsonObject obj;
  obj.setEntry("type", "neuroglancer");
  copyLink(obj.dumpString(0).c_str());
}

void ZStackPresenter::copyNeuroglancerLinkAtRectRoi()
{
  ZJsonObject obj;
  obj.setEntry("type", "neuroglancer");
  obj.setEntry("location", "rectroi");
  copyLink(obj.dumpString(0).c_str());
}

void ZStackPresenter::notifyBodyDecomposeTriggered()
{
  emit bodyDecomposeTriggered();
}

void ZStackPresenter::notifyBodyCropTriggered()
{
  emit bodyCropTriggered();
}

void ZStackPresenter::notifyBodyChopTriggered()
{
  emit bodyChopTriggered();
}

void ZStackPresenter::notifyBodyMergeTriggered()
{
  emit bodyMergeTriggered();
}

void ZStackPresenter::notifyBodyUnmergeTriggered()
{
  emit bodyUnmergeTriggered();
}

void ZStackPresenter::notifyBodyAnnotationTriggered()
{
  emit bodyAnnotationTriggered();
}

void ZStackPresenter::notifyExpertBodyStatus()
{
  emit bodyExpertStatusTriggered();
}

void ZStackPresenter::notifyBodyConnectionTriggered()
{
  emit bodyConnectionTriggered();
}

void ZStackPresenter::notifyBodyProfileTriggered()
{
  emit bodyProfileTriggered();
}

void ZStackPresenter::notifyBodyCheckinTriggered()
{
  emit bodyCheckinTriggered(neutu::EBodySplitMode::NONE);
}

void ZStackPresenter::notifyBodyForceCheckinTriggered()
{
  emit bodyForceCheckinTriggered();
}

void ZStackPresenter::notifyBodyCheckoutTriggered()
{
  emit bodyCheckoutTriggered(neutu::EBodySplitMode::NONE);
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
        ZStackObject::EType::RECT2D,
        ZStackObjectSourceFactory::MakeRectRoiSource());
  ZRect2d *rect = dynamic_cast<ZRect2d*>(obj);
  if (rect) {
    rect->setColor(QColor(255, 255, 255));
    if (interactiveContext().rectSpan()) {
      rect->updateZSpanWithMinSide();
    }
    processRectRoiUpdate(rect, appending);
  }

//  exitRectEdit();
}

void ZStackPresenter::addActiveMouseGlyphSize(double dr)
{
  m_mouseCursorGlyph->addActiveGlyphSize(
        dr, [this](ZStackObject *obj) {
    if (obj) {
      buddyDocument()->processObjectModified(obj);
    }
  });
}

void ZStackPresenter::processEvent(ZInteractionEvent &event)
{
  switch (event.getEvent()) {
  case ZInteractionEvent::EVENT_SWC_NODE_SELECTED:
    if (buddyDocument()->getSelectedSwcNodeNumber() != 1) {
      exitSwcExtendMode();
    }
        /*
    buddyView()->redrawObject();
    */
    break;
  case ZInteractionEvent::EVENT_VIEW_PROJECTION:
  case ZInteractionEvent::EVENT_VIEW_SLICE:
    updateView();
    emit(viewModeChanged());
    break;
  case ZInteractionEvent::EVENT_ACTIVE_DECORATION_UPDATED:
    forEachView([](ZStackView *view) {
      view->paintActiveDecoration();
    });
    break;
  case ZInteractionEvent::EVENT_STROKE_SELECTED:
  case ZInteractionEvent::EVENT_ALL_OBJECT_DESELCTED:
  case ZInteractionEvent::EVENT_OBJ3D_SELECTED:
  case ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED:
  case ZInteractionEvent::EVENT_OBJECT_SELECTED:
  case ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED_IN_LABEL_SLICE:
//    buddyView()->redrawObject();
    buddyDocument()->notifySelectorChanged();
    /*
    if (event.getEvent() ==
        ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED_IN_LABEL_SLICE) {
      emit labelSliceSelectionChanged();
    }
    */
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

bool ZStackPresenter::processCustomOperator(
    const ZStackOperator &/*op*/, ZInteractionEvent * /*e*/)
{
  return false;
}

bool ZStackPresenter::hasDrawable(neutu::data3d::ETarget target) const
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

neutu::EAxis ZStackPresenter::getSliceAxis() const
{
  return m_mainViewAxis;
//  return getMainView()->getSliceAxis();
}

static void SyncDvidLabelSliceSelection(
    ZStackDoc *doc, ZDvidLabelSlice *labelSlice)
{
  ZOUT(LTRACE(), 5) << "Sync dvid label selection";
  QList<ZDvidLabelSlice*> sliceList = doc->getObjectList<ZDvidLabelSlice>();
  for (QList<ZDvidLabelSlice*>::iterator iter = sliceList.begin();
       iter != sliceList.end(); ++iter) {
    ZDvidLabelSlice *buddySlice = *iter;
    if ((buddySlice != labelSlice) &&
        (buddySlice->isSupervoxel() == labelSlice->isSupervoxel())) {
      const std::set<uint64_t> &selectedSet =
          labelSlice->getSelectedOriginal();
      buddySlice->setSelection(selectedSet, neutu::ELabelSource::ORIGINAL);
    }
  }
}

void ZStackPresenter::moveSwcNode(ZStackView *view, double du, double dv)
{
  ZAffinePlane plane = view->getSliceViewTransform().getCutPlane();
  ZPoint dp = plane.getV1() * du + plane.getV2() * dv;
  buddyDocument()->executeMoveSwcNodeCommand(dp.getX(), dp.getY(), dp.getZ());
}

bool ZStackPresenter::process(ZStackOperator::EOperation op)
{
  ZStackOperator opr(op);
  return process(opr);
}

bool ZStackPresenter::process(ZStackOperator &op)
{
  if (op.isNull()) {
    return false;
  }

  bool processed = true;

#ifdef _DEBUG_
  if (op.getOperation() != ZStackOperator::OP_TRACK_MOUSE_MOVE) {
    std::cout << "Operator: " << op << std::endl;
    if (op.isNull()) {
      std::cout << "debug here" << std::endl;
    }
  }
#endif

  ZInteractionEvent interactionEvent;
  const ZMouseEvent& event = m_mouseEventProcessor.getLatestMouseEvent();
  ZIntPoint widgetPos = event.getWidgetPosition();
  QPoint currentWidgetPos(widgetPos.getX(), widgetPos.getY());
//  ZPoint currentStackPos = event.getPosition(neutu::ECoordinateSystem::STACK);
  ZPoint currentModelPos = event.getPosition(neutu::data3d::ESpace::MODEL);

#ifdef _DEBUG_0
  std::cout << "Current model pos: " << currentModelPos << std::endl;
#endif
//  ZPoint currentRawStackPos = event.getPosition(neutu::ECoordinateSystem::RAW_STACK);

  m_docSelector.setDocument(getSharedBuddyDocument());

  buddyDocument()->getObjectGroup().resetSelector();
  ZStackView *view = getView(op.getViewId());

  switch (op.getOperation()) {
  case ZStackOperator::OP_IMAGE_MOVE_DOWN:
    moveViewPort(view, 0, -1);
    break;
  case ZStackOperator::OP_IMAGE_MOVE_DOWN_FAST:
    moveViewPort(view, 0, -10);
    break;
  case ZStackOperator::OP_IMAGE_MOVE_UP:
    moveViewPort(view, 0, 1);
    break;
  case ZStackOperator::OP_IMAGE_MOVE_UP_FAST:
    moveViewPort(view, 0, 10);
    break;
  case ZStackOperator::OP_IMAGE_MOVE_LEFT:
    moveViewPort(view, 1, 0);
    break;
  case ZStackOperator::OP_IMAGE_MOVE_LEFT_FAST:
    moveViewPort(view, 10, 0);
    break;
  case ZStackOperator::OP_IMAGE_MOVE_RIGHT:
    moveViewPort(view, -1, 0);
    break;
  case ZStackOperator::OP_IMAGE_MOVE_RIGHT_FAST:
    moveViewPort(view, -10, 0);
    break;
  case ZStackOperator::OP_STACK_DEC_SLICE:
    if (view) {
      view->setSliceIndex(view->sliceIndex() - 1);
    }
    break;
  case ZStackOperator::OP_STACK_INC_SLICE:
    if (view) {
      view->setSliceIndex(view->sliceIndex() + 1);
    }
    break;
  case ZStackOperator::OP_STACK_DEC_SLICE_FAST:
    if (view) {
      view->setSliceIndex(view->sliceIndex() - 10);
    }
    break;
  case ZStackOperator::OP_STACK_INC_SLICE_FAST:
    if (view) {
      view->setSliceIndex(view->sliceIndex() + 10);
    }
    break;
  case ZStackOperator::OP_ZOOM_TO:
    if (view) {
      view->zoomTo(currentModelPos.roundToIntPoint());
    }
    break;
  case ZStackOperator::OP_START_ROTATE_VIEW:
    this->interactiveContext().backupExploreMode();
    this->interactiveContext().
        setExploreMode(ZInteractiveContext::EXPLORE_ROTATE_IMAGE);
    break;
  case ZStackOperator::OP_ROTATE_VIEW:
  {
    ZPoint mouseOffset = op.getMouseOffset(neutu::ECoordinateSystem::WIDGET);
    if (view) {
      view->rotateView(mouseOffset.getX(), mouseOffset.getY());
    }
  }
    break;
  case ZStackOperator::OP_SWC_ENTER_ADD_NODE:
  {
//    QPointF pos = mapFromGlobalToStack(QCursor::pos());
    trySwcAddNodeMode();
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
    buddyDocument()->processObjectModified(op.getHitObject<ZSwcTree>());

    if (buddyDocument()->getSelectedSwcNodeNumber() == 1) {
      if (buddyDocument()->getTag() != neutu::Document::ETag::BIOCYTIN_PROJECTION) {
        if (NeutubeConfig::getInstance().getApplication() == "Biocytin" ||
            buddyDocument()->getTag() == neutu::Document::ETag::FLYEM_PROOFREAD) {
          enterSwcExtendMode();
        }
      }

      if (interactiveContext().swcEditMode() ==
          ZInteractiveContext::SWC_EDIT_OFF) {
        interactionEvent.setEvent(ZInteractionEvent::EVENT_SWC_NODE_SELECTED);
      }
    }
    break;
  case ZStackOperator::OP_SWC_DESELECT_ALL_NODE:
    buddyDocument()->deselectAllSwcTreeNodes();
    interactionEvent.setEvent(
          ZInteractionEvent::EVENT_SWC_NODE_SELECTION_CLEARED);
    break;
  case ZStackOperator::OP_SWC_DESELECT_SINGLE_NODE:
    buddyDocument()->deselectHitSwcTreeNode(op.getHitObject<ZSwcTree>());
    if (buddyDocument()->getSelectedSwcNodeNumber() == 0) {
      interactionEvent.setEvent(
            ZInteractionEvent::EVENT_SWC_NODE_SELECTION_CLEARED);
    }
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
    ZStackBall ball = getActiveDecorationForSwc();
    if (buddyDocument()->executeSwcNodeExtendCommand(
          ball.getCenter(), ball.getRadius())) {
      m_mouseCursorGlyph->updateActiveGlyph([&](ZStackObject *obj) {
        ZSwcTree *tree = dynamic_cast<ZSwcTree*>(obj);
        set_double_node_mode(tree, getSelectedSwcNode());
      });
      interactionEvent.setEvent(ZInteractionEvent::EVENT_SWC_NODE_EXTENDED);
    }
  }
    break;
  case ZStackOperator::OP_SWC_SMART_EXTEND:
  {
    ZStackBall ball = getActiveDecorationForSwc();

    if (buddyDocument()->hasStackData()) {

      if (buddyDocument()->executeSwcNodeSmartExtendCommand(
            ball.getCenter(), ball.getRadius())) {
        m_mouseCursorGlyph->updateActiveGlyph([&](ZStackObject *obj) {
          ZSwcTree *tree = dynamic_cast<ZSwcTree*>(obj);
          set_double_node_mode(tree, getSelectedSwcNode());
        });
        updateMouseCursorGlyphPos();
//        updateSwcExtensionHint();
        //status = MOUSE_COMMAND_EXECUTED;
//        interactionEvent.setEvent(ZInteractionEvent::EVENT_SWC_NODE_EXTENDED);
      }
    } else {
//      ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_SWC);
      if (buddyDocument()->executeSwcNodeExtendCommand(
            ball.getCenter(), ball.getRadius())) {
        m_mouseCursorGlyph->updateActiveGlyph([&](ZStackObject *obj) {
          ZSwcTree *tree = dynamic_cast<ZSwcTree*>(obj);
          set_double_node_mode(tree, getSelectedSwcNode());
        });
        updateMouseCursorGlyphPos();
//        updateSwcExtensionHint();
//        interactionEvent.setEvent(ZInteractionEvent::EVENT_SWC_NODE_EXTENDED);
      }
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
    if (addActiveDecorationAsSwc()) {
      interactionEvent.setEvent(ZInteractionEvent::EVENT_SWC_NODE_ADDED);
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
    moveSwcNode(view, 0, -1.0);
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_UP_FAST:
    moveSwcNode(view, 0, -10.0);
//    buddyDocument()->executeMoveSwcNodeCommand(0, -10.0, 0);
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_LEFT:
    moveSwcNode(view, -1.0, 0);
//    buddyDocument()->executeMoveSwcNodeCommand(-1.0, 0, 0);
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_LEFT_FAST:
    moveSwcNode(view, -10.0, 0);
//    buddyDocument()->executeMoveSwcNodeCommand(-10.0, 0, 0);
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_DOWN:
    moveSwcNode(view, 0, 1.0);
//    buddyDocument()->executeMoveSwcNodeCommand(0, 1.0, 0);
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_DOWN_FAST:
    moveSwcNode(view, 0, 10.0);
//    buddyDocument()->executeMoveSwcNodeCommand(0, 10.0, 0);
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_RIGHT:
    moveSwcNode(view, 1.0, 0);
//    buddyDocument()->executeMoveSwcNodeCommand(1.0, 0, 0);
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE_RIGHT_FAST:
    moveSwcNode(view, 10.0, 0);
//    buddyDocument()->executeMoveSwcNodeCommand(10.0, 0, 0);
    break;
  case ZStackOperator::OP_SWC_MOVE_NODE:
    enterSwcMoveMode();
    break;
  case ZStackOperator::OP_SWC_INSERT_NODE:
    buddyDocument()->executeInsertSwcNode();
    break;
  case ZStackOperator::OP_SWC_INCREASE_NODE_SIZE:
      buddyDocument()->executeSwcNodeChangeSizeCommand(0.5);
    break;
  case ZStackOperator::OP_SWC_DECREASE_NODE_SIZE:
      buddyDocument()->executeSwcNodeChangeSizeCommand(-0.5);
    break;
  case ZStackOperator::OP_SWC_INCREASE_NODE_SIZE_FAST:
      buddyDocument()->executeSwcNodeChangeSizeCommand(5);
    break;
  case ZStackOperator::OP_SWC_DECREASE_NODE_SIZE_FAST:
      buddyDocument()->executeSwcNodeChangeSizeCommand(-5);
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
    view->processViewChange(false, false);
    view->redraw();
    break;
  case ZStackOperator::OP_SHOW_CONTEXT_MENU:
    view->showContextMenu(getContextMenu(), currentWidgetPos);
    m_contextViewId = view->getViewId();
    m_skipMouseReleaseEvent = 1;
    //status = CONTEXT_MENU_POPPED;
    break;
  case ZStackOperator::OP_SHOW_SWC_CONTEXT_MENU:
    if (getSwcNodeContextMenu()->isHidden()) {
      view->showContextMenu(getSwcNodeContextMenu(), currentWidgetPos);
      m_contextViewId = view->getViewId();
    }
    //status = CONTEXT_MENU_POPPED;
    break;
  case ZStackOperator::OP_SWC_CHANGE_NODE_FOCUS:
    changeSelectedSwcNodeFocus();
    break;
  case ZStackOperator::OP_SHOW_STROKE_CONTEXT_MENU:
    view->showContextMenu(getStrokeContextMenu(), currentWidgetPos);
    m_contextViewId = view->getViewId();
    //status = CONTEXT_MENU_POPPED;
    break;
  case ZStackOperator::OP_SHOW_TRACE_CONTEXT_MENU:
    if (buddyDocument()->hasTracable()) {
      if (m_interactiveContext.isContextMenuActivated()) {
        if (view->popLeftMenu(currentWidgetPos)) {
          m_interactiveContext.blockContextMenu();
          m_contextViewId = view->getViewId();
          //status = CONTEXT_MENU_POPPED;
        }
      } else {
        m_interactiveContext.blockContextMenu(false);
      }
    }
    break;
  case ZStackOperator::OP_SHOW_PUNCTA_CONTEXT_MENU:    
    if (view) {
      view->rightMenu()->clear();
      addPunctaEditFunctionToRightMenu(view);
      view->popRightMenu(currentWidgetPos);
      m_contextViewId = view->getViewId();
    }
    break;
  case ZStackOperator::OP_SHOW_BODY_CONTEXT_MENU:
    view->showContextMenu(getBodyContextMenu(), currentWidgetPos);
    m_contextViewId = view->getViewId();
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
    exitTodoEdit();
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
      if (view->popLeftMenu(currentWidgetPos)) {
        m_interactiveContext.blockContextMenu();
        m_contextViewId = view->getViewId();
      }
    } else {
      m_interactiveContext.blockContextMenu(false);
    }
    break;
  case ZStackOperator::OP_DESELECT_ALL:
    m_docSelector.deselectAll();
//    buddyDocument()->processObjectModified();
//    buddyDocument()->deselectAllObject();
    interactionEvent.setEvent(ZInteractionEvent::EVENT_ALL_OBJECT_DESELCTED);
    break;
  case ZStackOperator::OP_OBJECT_SELECT_SINGLE:
  {
//    buddyDocument()->deselectAllObject(false);
//    ZStackDocSelector docSelector(getSharedBuddyDocument());
    m_docSelector.deselectAll();
    ZStackObject *obj = op.getHitObject<ZStackObject>();
    if (obj) {
      obj->processHit(ZStackObject::ESelection::SELECT_SINGLE);
      buddyDocument()->setSelected(obj, obj->isSelected());
//      buddyDocument()->processObjectModified(
//            obj, ZStackObjectInfo::STATE_SELECTION_CHANGED);
      interactionEvent.setEvent(
            ZInteractionEvent::EVENT_OBJECT_SELECTED);
    }
  }
    break;
  case ZStackOperator::OP_OBJECT_SELECT_MULTIPLE:
  {
    ZStackObject *obj = op.getHitObject<ZStackObject>();
    if (obj) {
//      buddyDocument()->setSelected(obj, true);
      obj->processHit(ZStackObject::ESelection::SELECT_MULTIPLE);
      buddyDocument()->setSelected(obj, obj->isSelected());
      buddyDocument()->processObjectModified(
            obj, ZStackObjectInfo::STATE_SELECTION_CHANGED);
//      buddyDocument()->toggleSelected(obj);

//      buddyDocument()->setSelected(op.getHitObject<ZStackObject>(), true);
      interactionEvent.setEvent(
            ZInteractionEvent::EVENT_OBJECT_SELECTED);
    }
  }
    break;
  case ZStackOperator::OP_OBJECT_SELECT_TOGGLE:
  {
    {
      ZStackObject *obj = op.getHitObject<ZStackObject>();
      if (obj) {
//        buddyDocument()->toggleSelected(obj);
        obj->processHit(ZStackObject::ESelection::SELECT_TOGGLE);
        buddyDocument()->setSelected(obj, obj->isSelected());
        buddyDocument()->processObjectModified(
              obj, ZStackObjectInfo::STATE_SELECTION_CHANGED);
        interactionEvent.setEvent(
              ZInteractionEvent::EVENT_OBJECT_SELECTED);
      }
    }
  }
    break;
  case ZStackOperator::OP_BOOKMARK_SELECT_SIGNLE:
//    buddyDocument()->deselectAllObject(false);
//    buddyDocument()->deselectAllObject(ZStackObject::EType::TYPE_FLYEM_BOOKMARK);
    if (op.getHitObject<ZStackObject>() != NULL) {
      m_docSelector.deselectAll();

      buddyDocument()->setSelected(op.getHitObject<ZStackObject>(), true);
      interactionEvent.setEvent(
            ZInteractionEvent::EVENT_OBJECT_SELECTED);
    }
    break;
  case ZStackOperator::OP_STROKE_SELECT_SINGLE:
    m_docSelector.deselectAll();
//    buddyDocument()->deselectAllObject();
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
    } else if (op.getHitObject<ZDvidLabelSlice>() != NULL) {
      ZDvidLabelSlice *labelSlice =  op.getHitObject<ZDvidLabelSlice>();
      labelSlice->startSelection();
      labelSlice->selectHit();
      labelSlice->endSelection();
      SyncDvidLabelSliceSelection(buddyDocument(), labelSlice);
      interactionEvent.setEvent(
            ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED_IN_LABEL_SLICE);
    }
    break;
  case ZStackOperator::OP_OBJECT3D_SCAN_SELECT_MULTIPLE:
    if (op.getHitObject<ZObject3dScan>() != NULL) {
      buddyDocument()->setSelected(op.getHitObject<ZObject3dScan>());
      interactionEvent.setEvent(ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED);
    } else if (op.getHitObject<ZDvidLabelSlice>() != NULL) {
      ZDvidLabelSlice *labelSlice =  op.getHitObject<ZDvidLabelSlice>();
      labelSlice->startSelection();
      labelSlice->selectHit(true);
      labelSlice->endSelection();
      SyncDvidLabelSliceSelection(buddyDocument(), labelSlice);
      interactionEvent.setEvent(
            ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED_IN_LABEL_SLICE);
    }
    break;
  case ZStackOperator::OP_OBJECT3D_SCAN_TOGGLE_SELECT:
    if (op.getHitObject<ZObject3dScan>() != NULL) {
      buddyDocument()->toggleSelected(op.getHitObject<ZObject3dScan>());
      interactionEvent.setEvent(ZInteractionEvent::EVENT_OBJ3D_SELECTED);
    } else {
      ZDvidLabelSlice *labelSlice =  op.getHitObject<ZDvidLabelSlice>();
      if (labelSlice != NULL) {
        labelSlice->startSelection();
        labelSlice->toggleHitSelection(true);
        labelSlice->endSelection();
        SyncDvidLabelSliceSelection(buddyDocument(), labelSlice);
        interactionEvent.setEvent(
              ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED_IN_LABEL_SLICE);
      }
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
      } else if (op.getHitObject<ZDvidLabelSlice>() != NULL) {
        buddyDocument()->deselectAllObject(false);
        buddyDocument()->deselectAllSwcTreeNodes();
//        op.getHitObject<ZDvidLabelSlice>()->clearSelection();
        ZDvidLabelSlice *labelSlice =  op.getHitObject<ZDvidLabelSlice>();
        labelSlice->startSelection();
        labelSlice->toggleHitSelection(
              labelSlice->hasVisualEffect(
                neutu::display::LabelField::VE_HIGHLIGHT_SELECTED));
        labelSlice->endSelection();
        SyncDvidLabelSliceSelection(buddyDocument(), labelSlice);
        interactionEvent.setEvent(
              ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED_IN_LABEL_SLICE);
      }
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
    updateMouseCursorGlyphPos();
//    ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_STROKE);
//    stroke->set(currentModelPos);
//    stroke->set(currentStackPos.x(), currentStackPos.y());
          //m_mouseEventProcessor.getLatestStackPosition().x(),
            //    m_mouseEventProcessor.getLatestStackPosition().y());
  }
    break;
  case ZStackOperator::OP_MOVE_OBJECT:
  {
    ZPoint dp = op.getMouseEventRecorder()->
        getPositionOffset(neutu::ECoordinateSystem::WIDGET);
    dp /= view->getSliceViewTransform().getScale();
    ZPoint offset = view->getSliceViewTransform().getCutOrientation().mapAligned(
          dp.getX(), dp.getY());

    buddyDocument()->executeMoveObjectCommand(
          offset.x(), offset.y(), offset.z(), glm::mat4(1.f), glm::mat4(1.f));
  }
    break;
  case ZStackOperator::OP_MOVE_IMAGE:
  {
    Qt::MouseButtons grabButton = op.getPressedButtons();
    if (grabButton == Qt::NoButton || (grabButton & Qt::LeftButton)) {
      grabButton = Qt::LeftButton;
    }
    ZPoint grabPosition = op.getMouseEventRecorder()->getPosition(
          grabButton, ZMouseEvent::EAction::PRESS, neutu::data3d::ESpace::MODEL);
#ifdef _DEBUG_
  std::cout << "======> Grab position: " << grabPosition.toString() << std::endl;
#endif
//    grabPosition.shiftSliceAxis(getSliceAxis());
  moveViewPort(view, grabPosition, currentWidgetPos.x(), currentWidgetPos.y());
//    moveImageToMouse(
//          grabPosition.x(), grabPosition.y(),
//          currentWidgetPos.x(), currentWidgetPos.y());
  }
    break;
  case ZStackOperator::OP_CROSSHAIR_MOVE:
    moveCrossHairToMouse(currentWidgetPos.x(), currentWidgetPos.y());
    break;
  case ZStackOperator::OP_CROSSHAIR_GRAB:
    m_interactiveContext.setUniqueMode(ZInteractiveContext::INTERACT_MOVE_CROSSHAIR);
    updateCursor();
    break;
  case ZStackOperator::OP_CROSSHAIR_RELEASE:
    m_interactiveContext.setUniqueMode(ZInteractiveContext::INTERACT_FREE);
    updateCursor();
    break;
  case ZStackOperator::OP_ZOOM_IN:
    increaseZoomRatio(view);
    break;
  case ZStackOperator::OP_ZOOM_OUT:
    decreaseZoomRatio(view);
    break;
  case ZStackOperator::OP_ZOOM_IN_GRAB_POS:
  {
    m_interactiveContext.blockContextMenu();
    ZPoint grabPosition = op.getMouseEventRecorder()->getPosition(
          Qt::RightButton, ZMouseEvent::EAction::PRESS,
          neutu::ECoordinateSystem::WIDGET);
    m_interactiveContext.setExploreMode(
          ZInteractiveContext::EXPLORE_ZOOM_IN_IMAGE);
//    buddyView()->blockViewChangeEvent(true);
    increaseZoomRatio(view, grabPosition.x(), grabPosition.y());
//    buddyView()->blockViewChangeEvent(false);
  }
    break;
  case ZStackOperator::OP_ZOOM_OUT_GRAB_POS:
  {
    m_interactiveContext.blockContextMenu();
    ZPoint grabPosition = op.getMouseEventRecorder()->getPosition(
          Qt::RightButton, ZMouseEvent::EAction::PRESS,
          neutu::ECoordinateSystem::WIDGET);
    m_interactiveContext.setExploreMode(
          ZInteractiveContext::EXPLORE_ZOOM_OUT_IMAGE);
    decreaseZoomRatio(view, grabPosition.x(), grabPosition.y());
  }
    break;
  case ZStackOperator::OP_EXIT_ZOOM_MODE:
    m_interactiveContext.setExploreMode(ZInteractiveContext::EXPLORE_OFF);
    if (!view->restoreFromBadView()) {
      emit updatingViewData();
//      view->updateViewData();
//      view->redraw();
    }
//    view->processViewChange(true, false);
//    view->updateImageScreen(ZStackView::EUpdateOption::QUEUED);
    break;
  case ZStackOperator::OP_PAINT_STROKE:
    updateMouseCursorGlyphPos();
    /*
    ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_STROKE);
#ifdef _DEBUG_
    std::cout << "Appending stroke: " << currentModelPos << std::endl;
#endif
    stroke->append(currentModelPos);
    buddyDocument()->processObjectModified(stroke);
    */
//    buddyView()->paintActiveDecoration();
    break;
  case ZStackOperator::OP_RECT_ROI_INIT:
  {
    ZRect2d *rect = new ZRect2d;
    rect->setViewId(op.getViewId());
    rect->setStartCorner(currentWidgetPos.x(), currentWidgetPos.y());
    rect->setSource(ZStackObjectSourceFactory::MakeRectRoiSource());
    rect->setPenetrating(true);
    rect->setZ(view->getCurrentDepth());
    rect->setColor(255, 128, 128);

#ifdef _DEBUG_
    std::cout << "Adding roi: " << rect << std::endl;
#endif
//    buddyDocument()->removeObject(rect->getSource(), false);

    ZStackObject *obj = buddyDocument()->getObjectGroup().findFirstSameSource(
          ZStackObject::EType::RECT2D,
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
          ZStackObject::EType::RECT2D,
          ZStackObjectSourceFactory::MakeRectRoiSource());
    ZRect2d *rect = dynamic_cast<ZRect2d*>(obj);
    if (rect != NULL) {
      /*
      ZPoint grabPosition = op.getMouseEventRecorder()->getPosition(
            Qt::LeftButton, ZMouseEvent::EAction::PRESS, neutu::ECoordinateSystem::STACK);
//      grabPosition.shiftSliceAxis(getSliceAxis());
      ZPoint shiftedStackPos = currentStackPos;
//      shiftedStackPos.shiftSliceAxis(getSliceAxis());

      int x0 = std::min(grabPosition.x(), shiftedStackPos.x());
      int y0 = std::min(grabPosition.y(), shiftedStackPos.y());

      int x1 = std::max(grabPosition.x(), shiftedStackPos.x());
      int y1 = std::max(grabPosition.y(), shiftedStackPos.y());

      rect->setMinCorner(x0, y0);
      rect->setMaxCorner(x1, y1);
      */
      rect->setEndCorner(currentWidgetPos.x(), currentWidgetPos.y());
      buddyDocument()->bufferObjectModified(rect);
      buddyDocument()->processObjectModified();
//      buddyDocument()->processObjectModified(rect);
//      buddyDocument()->processObjectModified();
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
    if (view->isImageMovable()) {
      this->interactiveContext().backupExploreMode();
      this->interactiveContext().
          setExploreMode(ZInteractiveContext::EXPLORE_MOVE_IMAGE);
      //m_grabPosition = buddyView()->screen()->canvasCoordinate(event->pos());
    }
    break;
  case ZStackOperator::OP_OBJECT_TOGGLE_TMP_RESULT_VISIBILITY:
    if (buddyDocument()->hasObject(ZStackObjectRole::ROLE_TMP_RESULT)) {
      buddyDocument()->toggleVisibility(ZStackObjectRole::ROLE_TMP_RESULT);
    } else {
      processed = false;
      op.setOperation(ZStackOperator::OP_NULL);
    }
    break;
  case ZStackOperator::OP_TRACK_MOUSE_MOVE:
    view->updateDataInfo(currentWidgetPos);
    /*
    buddyView()->setInfo(
          buddyDocument()->rawDataInfo(
            currentRawStackPos.getX(),
            currentRawStackPos.getY(),
            currentRawStackPos.getZ(),
            buddyView()->getSliceAxis()));
            */
#if 0
    if (m_interactiveContext.synapseEditMode() ==
        ZInteractiveContext::SYNAPSE_EDIT_OFF) {
      ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(getFirstOnActiveObject());
      //    if (isStrokeOn()) {
      if (stroke != NULL) {
        ZPoint pt = currentStackPos;
//        pt.shiftSliceAxis(getSliceAxis());
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
#endif
    //    processed = false;
    break;

  case ZStackOperator::OP_STACK_LOCATE_SLICE:
    if (buddyDocument()->hasStackData()) {
      interactiveContext().setViewMode(ZInteractiveContext::VIEW_NORMAL);
      /*
      int sliceIndex =
          buddyDocument()->maxIntesityDepth(currentRawStackPos.x(),
                                            currentRawStackPos.y());
      buddyView()->setSliceIndex(sliceIndex);
      */
      interactionEvent.setEvent(ZInteractionEvent::EVENT_VIEW_SLICE);
    }
    break;
  case ZStackOperator::OP_STACK_VIEW_SLICE:
    interactiveContext().setViewMode(ZInteractiveContext::VIEW_NORMAL);
//    buddyView()->setSliceIndex(getSliceIndex());
    interactionEvent.setEvent(ZInteractionEvent::EVENT_VIEW_SLICE);
    break;
  case ZStackOperator::OP_STACK_VIEW_PROJECTION:
    if (buddyDocument()->getTag() != neutu::Document::ETag::BIOCYTIN_PROJECTION) {
      interactiveContext().setViewMode(ZInteractiveContext::VIEW_PROJECT);
      interactionEvent.setEvent(ZInteractionEvent::EVENT_VIEW_PROJECTION);
    }
    break;
  case ZStackOperator::OP_SWC_LOCATE_FOCUS:
    if (op.getHitObject<Swc_Tree_Node>() != NULL) {
      int sliceIndex =
          neutu::iround(SwcTreeNode::z(op.getHitObject<Swc_Tree_Node>()));
      sliceIndex -= buddyDocument()->getStackOffset().getZ();
      if (sliceIndex >= 0 &&
          sliceIndex < buddyDocument()->getStackSize().getZ()) {
        view->setSliceIndex(sliceIndex);
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
        view->setSliceIndex(sliceIndex);
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
        view->setSliceIndex(sliceIndex);
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
        view->setSliceIndex(sliceIndex);
        interactiveContext().setViewMode(ZInteractiveContext::VIEW_NORMAL);
        interactionEvent.setEvent(ZInteractionEvent::EVENT_VIEW_SLICE);
      }
    }
    break;
  case ZStackOperator::OP_ACTIVE_STROKE_DECREASE_SIZE:
    addActiveMouseGlyphSize(-1.0);
    /*
  {
    ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(getFirstOnActiveObject());
    if (stroke->isVisible()) {
      stroke->addWidth(-1.0);
      buddyView()->paintActiveDecoration();
    }
  }
  */
    break;
  case ZStackOperator::OP_ACTIVE_STROKE_INCREASE_SIZE:
    addActiveMouseGlyphSize(1.0);
    /*
  {
    ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(getFirstOnActiveObject());
    if (stroke->isVisible()) {
      stroke->addWidth(1.0);
      buddyView()->paintActiveDecoration();
    }
  }
  */
    break;
  case ZStackOperator::OP_ACTIVE_STROKE_ESTIMATE_SIZE:
    //Todo: recover size estimation for new mouse glyph data structure
    /*
    if (buddyDocument()->hasStackData()) {
      ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(getFirstOnActiveObject());
      if (stroke->isVisible()) {
        if (estimateActiveStrokeWidth()) {
          buddyView()->paintActiveDecoration();
        }
      }
    }
    */
    break;
  case ZStackOperator::OP_OBJECT_DELETE_SELECTED:
    if (!buddyDocument()->getSelected(ZStackObject::EType::FLYEM_BOOKMARK).isEmpty()) {
      op.setOperation(ZStackOperator::OP_BOOKMARK_DELETE);
    } else {
      buddyDocument()->executeRemoveSelectedObjectCommand();
    }
    break;
  case ZStackOperator::OP_VIEW_DISABLE_SCROLL:
    getContextView()->ignoreScroll(true);
    break;
  case ZStackOperator::OP_VIEW_ENABLE_SCROLL:
    getContextView()->ignoreScroll(false);
    break;
  case ZStackOperator::OP_VIEW_PAUSE_SCROLL:
    getContextView()->pauseScroll();
    break;
  default:
    processed = false;
    break;
  }

//  if (!processed) {
  processed = processCustomOperator(op, &interactionEvent) || processed;
//  }

  if (interactionEvent.getEvent() != ZInteractionEvent::EVENT_NULL) {
    processEvent(interactionEvent);
  }

  return processed;
}

void ZStackPresenter::acceptActiveStroke()
{
//  ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_STROKE);
//  ZStroke2d *newStroke = dynamic_cast<ZStroke2d*>(stroke->clone());
  ZStroke2d *newStroke = m_mouseCursorGlyph->makeStrokeFromActiveGlyph();
  if (newStroke) {
    if (!newStroke->isEraser()) {
      if (newStroke->getPointNumber() == 1 &&
          m_mouseEventProcessor.getLatestMouseEvent().getModifiers() ==
          Qt::ShiftModifier &&
          buddyDocument()->getTag() != neutu::Document::ETag::FLYEM_SPLIT &&
          buddyDocument()->getTag() != neutu::Document::ETag::FLYEM_PROOFREAD) {
        if (!buddyDocument()->getStrokeList().empty()) {
          LINFO() << "Compute stroke path";
          ZPoint start;
          ZPoint end;
          buddyDocument()->getLastStrokePoint(start.xRef(), start.yRef());
          newStroke->getLastPoint(end.xRef(), end.yRef());
          buddyDocument()->mapToStackCoord(&start);
          buddyDocument()->mapToStackCoord(&end);

//          int z0 = buddyView()->getZ(neutu::ECoordinateSystem::STACK);
          int z0 = neulib::iround(getMainView()->getCutCenter().getZ());
          //        int z0 = buddyView()->sliceIndex();
          //        int z1 = z0;
          start.setZ(0);
          end.setZ(0);

          int source[3] = {0, 0, 0};
          int target[3] = {0, 0, 0};
          for (int i = 0; i < 3; ++i) {
            source[i] = neutu::iround(start[i]);
            target[i] = neutu::iround(end[i]);
          }

          ZStack *signal = ZStackFactory::makeSlice(
                *buddyDocument()->getStack(), z0);

          Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
          if (buddyDocument()->getStackBackground() ==
              neutu::EImageBackground::BRIGHT) {
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
          if (buddyDocument()->getTag() == neutu::Document::ETag::BIOCYTIN_PROJECTION &&
              signal->channelNumber() > 1) {
            channel = 1;
          }

          //        Stack *stack = buddyDocument()->getStack()->c_stack(channel);
          sgw->greyFactor = m_grayScale[channel];
          sgw->greyOffset = m_grayOffset[channel];

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

    //  newStroke->setZ(buddyView()->sliceIndex() +
    //                  buddyDocument()->getStackOffset().getZ());
    newStroke->setPenetrating(false);

    ZStackObjectRole::TRole role = ZStackObjectRole::ROLE_NONE;
    //  if (buddyDocument()->getTag() == NeuTube::Document::FLYEM_SPLIT) {
    if (GET_APPLICATION_NAME == "FlyEM") {
      role = ZStackObjectRole::ROLE_SEED |
          ZStackObjectRole::ROLE_3DGRAPH_DECORATOR;
    }

    newStroke->setZOrder(m_zOrder++);
    newStroke->setRole(role);
    if (buddyDocument()->getTag() == neutu::Document::ETag::BIOCYTIN_PROJECTION) {
      newStroke->setPenetrating(true);
    }
    buddyDocument()->executeAddObjectCommand(newStroke);

    //buddyDocument()->executeAddStrokeCommand(newStroke);

    m_mouseCursorGlyph->updateActiveGlyph([](ZStackObject *obj) {
      ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(obj);
      stroke->clear();
    });
//    stroke->clear();
  }
//  buddyView()->paintActiveDecoration();
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
  ZStroke2d *stroke =
      dynamic_cast<ZStroke2d*>(m_mouseCursorGlyph->_getActiveGlyph());
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

  ZStack *stack = getMainView()->getStrokeMask(neutu::EColor::RED);
  stack->save(GET_TEST_DATA_DIR + "/test.tif");
  delete stack;

  stack = getMainView()->getStrokeMask(neutu::EColor::GREEN);
  stack->save(GET_TEST_DATA_DIR + "/test2.tif");


  delete stack;
}

#if 0
bool ZStackPresenter::isActiveObjectOn(EObjectRole role) const
{
  if (getActiveObject(role) == NULL) {
    return false;
  }

  return getActiveObject(role)->isVisible();
}

bool ZStackPresenter::isActiveObjectOn() const
{
  return m_mouseCursorGlyph->getActiveGlyph();
  /*
  for (QMap<EObjectRole, ZStackObject*>::const_iterator iter =
       m_activeObjectMap.begin(); iter != m_activeObjectMap.end(); ++iter) {
    if (iter.value()->isVisible()) {
      return true;
    }
  }

  return false;
  */
}
#endif

#if 0
ZStackObject* ZStackPresenter::getActiveObject(EObjectRole role) const
{
  return m_mouseCursorGlyph->getGlyph(role);
  /*
  if (!m_activeObjectMap.contains(role)) {
    return NULL;
  }

  return m_activeObjectMap[role];
  */
}
#endif

void ZStackPresenter::setViewCount(int n)
{
  m_viewCount = n;
}

void ZStackPresenter::setSliceAxis(int viewId, neutu::EAxis axis)
{
  m_interactiveContext.setSliceAxis(viewId, axis);
}

void ZStackPresenter::setMainSliceAxis(neutu::EAxis axis)
{
  m_mainViewAxis = axis;
  m_mouseCursorGlyph->setSliceAxis(axis);
}
