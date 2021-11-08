#include "zflyemproofpresenter.h"

#include <QKeyEvent>
#include <QAction>
#include <QColorDialog>

#ifdef _DEBUG_
#  include "common/debug.h"
#endif

#include "logging/zlog.h"
#include "qt/gui/loghelper.h"
#include "zglobal.h"

#include "zjsonobjectparser.h"
#include "zkeyoperationconfig.h"
#include "zinteractivecontext.h"
#include "mvc/zstackdoc.h"
#include "mvc/zstackview.h"

#include "zflyembookmark.h"
#include "zflyemproofdoc.h"
#include "zkeyoperationconfig.h"
#include "zflyemkeyoperationconfig.h"
#include "zflyemproofdocmenufactory.h"
#include "dvid/zdvidsynapseensenmble.h"
#include "zinteractionevent.h"
#include "zstackdocselector.h"
#include "mvc/zstackdochelper.h"
#include "neutubeconfig.h"
//#include "dvid/zdvidlabelslice.h"
#include "zflyemtododelegate.h"
#include "zflyemproofdocutil.h"
#include "flyemdatareader.h"

#include "neuroglancer/zneuroglancerpathfactory.h"
#include "neuroglancer/zneuroglancerlayerspecfactory.h"
#include "neuroglancer/zneuroglancerannotationlayerspec.h"
#include "mvc/zmousecursorglyph.h"
#include "zrect2d.h"


#ifdef _WIN32
#undef GetUserName
#endif

/*
ZFlyEmProofPresenter::ZFlyEmProofPresenter(ZStackFrame *parent) :
  ZStackPresenter(parent)
{
  init();
}
*/

ZFlyEmProofPresenter::ZFlyEmProofPresenter(QWidget *parent) :
  ZStackPresenter(parent)
{
  init();
}

ZFlyEmProofPresenter::~ZFlyEmProofPresenter()
{

}

void ZFlyEmProofPresenter::init()
{
  m_isHightlightMode = false;
//  m_splitWindowMode = false;
  m_splitMode = neutu::EBodySplitMode::NONE;
  m_highTileContrast = false;
  m_smoothTransform = false;
  m_showingData = false;

  m_synapseContextMenu = NULL;

  interactiveContext().setSwcEditMode(ZInteractiveContext::SWC_EDIT_OFF);

  m_mouseCursorGlyph->setPrepareFunc(
        ZMouseCursorGlyph::ROLE_SYNAPSE, [this](ZStackObject *obj) {
    ZStackBall *ball = dynamic_cast<ZStackBall*>(obj);
    if (ball) {
      ZDvidSynapse::EKind kind  = ZDvidSynapse::EKind::KIND_UNKNOWN;
      switch (interactiveContext().synapseEditMode()) {
      case ZInteractiveContext::SYNAPSE_ADD_PRE:
        kind = ZDvidSynapse::EKind::KIND_PRE_SYN;
        break;
      case ZInteractiveContext::SYNAPSE_ADD_POST:
        kind = ZDvidSynapse::EKind::KIND_POST_SYN;
        break;
      default:
        break;
      }
      QColor color = ZDvidSynapse::GetDefaultColor(kind);
      color.setAlpha(200);
      ball->setColor(color);
      ball->setRadius(ZDvidSynapse::GetDefaultRadius(kind));
    }
  });

//  m_labelAlpha = 128;
//  connectAction();

//  ZKeyOperationConfig::ConfigureFlyEmStackMap(m_stackKeyOperationMap);
}

bool ZFlyEmProofPresenter::connectAction(
    QAction *action, ZActionFactory::EAction item)
{
  bool connected = false;

  if (action != NULL) {
    connected = true;
    switch (item) {
    case ZActionFactory::ACTION_SYNAPSE_DELETE:
      connect(action, SIGNAL(triggered()), this, SLOT(deleteSelectedSynapse()));
      break;
    case ZActionFactory::ACTION_SYNAPSE_ADD_PRE:
      connect(action, SIGNAL(triggered()), this, SLOT(tryAddPreSynapseMode()));
      break;
    case ZActionFactory::ACTION_SYNAPSE_ADD_POST:
      connect(action, SIGNAL(triggered()),
              this, SLOT(tryAddPostSynapseMode()));
      break;
    case ZActionFactory::ACTION_SYNAPSE_MOVE:
      connect(action, SIGNAL(triggered()),
              this, SLOT(tryMoveSynapseMode()));
      break;
    case ZActionFactory::ACTION_SYNAPSE_LINK:
      connect(action, SIGNAL(triggered()), this, SLOT(linkSelectedSynapse()));
      break;
    case ZActionFactory::ACTION_SYNAPSE_REPAIR:
      connect(action, SIGNAL(triggered()), this, SLOT(repairSelectedSynapse()));
      break;
    case ZActionFactory::ACTION_SYNAPSE_UNLINK:
      connect(action, SIGNAL(triggered()), this, SLOT(unlinkSelectedSynapse()));
      break;
    case ZActionFactory::ACTION_SYNAPSE_HLPSD:
      connect(action, SIGNAL(triggered(bool)), this, SLOT(highlightPsd(bool)));
      break;
    case ZActionFactory::ACTION_TRACE:
      connect(action, &QAction::triggered, this, &ZFlyEmProofPresenter::trace);
      break;
    case ZActionFactory::ACTION_ADD_TODO_ITEM:
      connect(action, SIGNAL(triggered()), this, SLOT(tryAddTodoItem()));
      break;
    case ZActionFactory::ACTION_ADD_TODO_ITEM_CHECKED:
      connect(action, SIGNAL(triggered()), this, SLOT(tryAddDoneItem()));
      break;
    case ZActionFactory::ACTION_ADD_TODO_MERGE:
      connect(action, SIGNAL(triggered()), this, SLOT(tryAddToMergeItem()));
      break;
    case ZActionFactory::ACTION_ADD_TODO_SPLIT:
      connect(action, SIGNAL(triggered()), this, SLOT(tryAddToSplitItem()));
      break;
    case ZActionFactory::ACTION_ADD_TODO_SVSPLIT:
      connect(action, SIGNAL(triggered()), this, SLOT(tryAddToSupervoxelSplitItem()));
      break;
    case ZActionFactory::ACTION_ADD_TODO_TRACE_TO_SOMA:
      connect(action, SIGNAL(triggered()), this, SLOT(tryAddTraceToSomaItem()));
      break;
    case ZActionFactory::ACTION_ADD_TODO_NO_SOMA:
      connect(action, SIGNAL(triggered()), this, SLOT(tryAddNoSomaItem()));
      break;
    case ZActionFactory::ACTION_ADD_TODO_DIAGNOSTIC:
      connect(action, SIGNAL(triggered()), this, SLOT(tryAddDiagnosticItem()));
      break;
    case ZActionFactory::ACTION_ADD_TODO_SEGMENTATION_DIAGNOSTIC:
      connect(action, SIGNAL(triggered()),
              this, SLOT(tryAddSegmentationDiagnosticItem()));
      break;
    case ZActionFactory::ACTION_CHECK_TODO_ITEM:
      connect(action, SIGNAL(triggered()), this, SLOT(checkTodoItem()));
      break;
    case ZActionFactory::ACTION_UNCHECK_TODO_ITEM:
      connect(action, SIGNAL(triggered()), this, SLOT(uncheckTodoItem()));
      break;
    case ZActionFactory::ACTION_TODO_ITEM_ANNOT_NORMAL:
      connect(action, SIGNAL(triggered()), this, SLOT(setTodoItemToNormal()));
      break;
    case ZActionFactory::ACTION_TODO_ITEM_ANNOT_IRRELEVANT:
      connect(action, SIGNAL(triggered()), this, SLOT(setTodoItemIrrelevant()));
      break;
    case ZActionFactory::ACTION_TODO_ITEM_ANNOT_MERGE:
      connect(action, SIGNAL(triggered()), this, SLOT(setTodoItemToMerge()));
      break;
    case ZActionFactory::ACTION_TODO_ITEM_ANNOT_SPLIT:
      connect(action, SIGNAL(triggered()), this, SLOT(setTodoItemToSplit()));
      break;
    case ZActionFactory::ACTION_REMOVE_TODO_ITEM:
      connect(action, SIGNAL(triggered()), this, SLOT(removeTodoItem()));
      break;
    case ZActionFactory::ACTION_SELECT_BODY_IN_RECT:
      connect(action, SIGNAL(triggered()), this, SLOT(selectBodyInRoi()));
      break;
    case ZActionFactory::ACTION_ZOOM_TO_RECT:
      connect(action, SIGNAL(triggered()), this, SLOT(zoomInRectRoi()));
      break;
    case ZActionFactory::ACTION_REWRITE_SEGMENTATION:
      connect(action, SIGNAL(triggered()),
              getCompleteDocument(), SLOT(rewriteSegmentation()));
      break;
    case ZActionFactory::ACTION_REFRESH_SEGMENTATION:
      connect(action, &QAction::triggered, this,
              &ZFlyEmProofPresenter::refreshSegmentation);
      break;
    case ZActionFactory::ACTION_REFRESH_DATA:
      connect(action, &QAction::triggered,
              this, &ZFlyEmProofPresenter::refreshData);
      break;
    case ZActionFactory::ACTION_SYNAPSE_VERIFY:
      connect(getAction(ZActionFactory::ACTION_SYNAPSE_VERIFY), SIGNAL(triggered()),
              this, SLOT(verifySelectedSynapse()));
      break;
    case ZActionFactory::ACTION_SYNAPSE_UNVERIFY:
      connect(getAction(ZActionFactory::ACTION_SYNAPSE_UNVERIFY), SIGNAL(triggered()),
              this, SLOT(unverifySelectedSynapse()));
      break;
    case ZActionFactory::ACTION_SHOW_SUPERVOXEL_LIST:
      connect(action, SIGNAL(triggered()), this, SLOT(showSupervoxelList()));
      break;
    case ZActionFactory::ACTION_TOGGLE_SUPERVOXEL_VIEW:
      connect(action, &QAction::triggered, this,
              &ZFlyEmProofPresenter::toggleSupervoxelView);
      break;
    case ZActionFactory::ACTION_VIEW_SCREENSHOT:
      connect(action, &QAction::triggered, this,
              &ZFlyEmProofPresenter::takeScreenshot);
      break;
    case ZActionFactory::ACTION_RUN_TIP_DETECTION:
      connect(action, SIGNAL(triggered()), this, SLOT(runTipDetection()));
      break;
    case ZActionFactory::ACTION_VIEW_AXIS_X:
      connect(action, SIGNAL(triggered()), this, SLOT(setCutPlaneAlongX()));
      break;
    case ZActionFactory::ACTION_VIEW_AXIS_Y:
      connect(action, SIGNAL(triggered()), this, SLOT(setCutPlaneAlongY()));
      break;
    case ZActionFactory::ACTION_VIEW_AXIS_Z:
      connect(action, SIGNAL(triggered()), this, SLOT(setCutPlaneAlongZ()));
      break;
    case ZActionFactory::ACTION_VIEW_LAYOUT_1:
      connect(action, SIGNAL(triggered()), this, SLOT(setViewLayout1()));
      break;
    case ZActionFactory::ACTION_VIEW_LAYOUT_2:
      connect(action, SIGNAL(triggered()), this, SLOT(setViewLayout2()));
      break;
    case ZActionFactory::ACTION_VIEW_LAYOUT_3:
      connect(action, SIGNAL(triggered()), this, SLOT(setViewLayout3()));
      break;
    case ZActionFactory::ACTION_VIEW_AXIS_ARB:
      connect(action, SIGNAL(triggered()), this, SLOT(setCutPlaneArb()));
      break;
    case ZActionFactory::ACTION_BODY_CHANGE_COLOR:
      connect(action, SIGNAL(triggered()), this, SLOT(setBodyColor()));
      break;
    case ZActionFactory::ACTION_BODY_RESET_COLOR:
      connect(action, SIGNAL(triggered()), this, SLOT(resetBodyColor()));
      break;
    default:
      connected = false;
      break;
    }
    if (connected == false) {
      connected = ZStackPresenter::connectAction(action, item);
    }
  }

  return connected;
}

void ZFlyEmProofPresenter::refreshSegmentation()
{
  getCompleteDocument()->refreshDvidLabelBuffer(0);
  getCompleteDocument()->updateDvidLabelSlice();
}

void ZFlyEmProofPresenter::refreshData()
{
  /*
  getCompleteDocument()->refreshBookmark();
  getCompleteDocument()->refreshSynapse();
  getCompleteDocument()->refreshBookmark();
  getCompleteDocument()->refreshTodo();
  */

  emit refreshingData();
}

void ZFlyEmProofPresenter::updateActions()
{
  bool splitting = isSplitOn();
  getAction(ZActionFactory::ACTION_VIEW_AXIS_X)->setEnabled(!splitting);
  getAction(ZActionFactory::ACTION_VIEW_AXIS_Y)->setEnabled(!splitting);
  getAction(ZActionFactory::ACTION_VIEW_AXIS_ARB)->setEnabled(!splitting);
}

void ZFlyEmProofPresenter::setBodyColor()
{
//  QColor color = QColorDialog::getColor(Qt::white, buddyView());

  QColorDialog dlg;
  dlg.setOption(QColorDialog::ShowAlphaChannel, true);
  if (dlg.exec()) {
    getCompleteDocument()->setSelectedBodyColor(dlg.currentColor());
  }
}

void ZFlyEmProofPresenter::resetBodyColor()
{
  getCompleteDocument()->resetSelectedBodyColor();
}

void ZFlyEmProofPresenter::selectBodyInRoi()
{
  ZStackView *view = getContextView();
  if (view) {
    getCompleteDocument()->selectBodyInRoi(view->getCurrentDepth(), true, true);
  }
}

void ZFlyEmProofPresenter::zoomInRectRoi()
{ 
  ZStackView *view = getContextView();
  if (view) {
    ZAffineRect rect = buddyDocument()->getRectRoi();

    if (!rect.isEmpty()) {
#ifdef _DEBUG_
      std::cout << "Zoom to: " << rect << std::endl;
#endif
      //    ZIntCuboid box = rect.getIntBoundBox();
      view->zoomTo(
            rect.getCenter(), std::max(rect.getWidth(), rect.getHeight()), false);
      //    buddyView()->setViewPort(QRect(rect.getMinX(), rect.getMinY(),
      //                               rect.getWidth(), rect.getHeight()));
      buddyDocument()->executeRemoveRectRoiCommand();
    }
  }
}

ZFlyEmProofPresenter* ZFlyEmProofPresenter::Make(QWidget *parent)
{
  ZFlyEmProofPresenter *presenter = new ZFlyEmProofPresenter(parent);
  presenter->configKeyMap();

  /*
  ZKeyOperationConfig::Configure(
        presenter->m_bookmarkKeyOperationMap, ZKeyOperation::OG_FLYEM_BOOKMARK);
        */

  return presenter;
}

ZStackDocMenuFactory* ZFlyEmProofPresenter::getMenuFactory()
{
  if (!m_menuFactory) {
    m_menuFactory = std::unique_ptr<ZStackDocMenuFactory>(
          new ZFlyEmProofDocMenuFactory);
    m_menuFactory->setAdminState(getCompleteDocument()->isAdmin());
  }

  return m_menuFactory.get();
}

ZKeyOperationConfig* ZFlyEmProofPresenter::getKeyConfig()
{
  if (m_keyConfig == NULL) {
    m_keyConfig = new ZFlyEmKeyOperationConfig();
  }

  return m_keyConfig;
}

void ZFlyEmProofPresenter::configKeyMap()
{
  ZKeyOperationConfig *config = getKeyConfig();

  config->configure(
        m_activeStrokeOperationMap, ZKeyOperation::OG_ACTIVE_STROKE);
  config->configure(
        m_swcKeyOperationMap, ZKeyOperation::OG_SWC_TREE_NODE);
  config->configure(
        m_stackKeyOperationMap, ZKeyOperation::OG_STACK);
  config->configure(m_objectKeyOperationMap, ZKeyOperation::OG_STACK_OBJECT);
  config->configure(m_bookmarkKeyOperationMap, ZKeyOperation::OG_FLYEM_BOOKMARK);

#ifdef _DEBUG_
  std::cout << "Key V mapped to "
            << m_swcKeyOperationMap.getOperation(Qt::Key_V, Qt::NoModifier)
            << std::endl;
#endif
}

bool ZFlyEmProofPresenter::customKeyProcess(QKeyEvent *event)
{
  bool processed = false;

  ZFlyEmProofDoc *doc = getCompleteDocument();

  switch (event->key()) {
  case Qt::Key_H:
    if (!isSplitOn()) {
      toggleHighlightMode();
      emit highlightingSelected(isHighlight());
      processed = true;
    }
    break;
  case Qt::Key_C:
    if (!isSplitOn()) {
      bool asking = (event->modifiers() != Qt::ShiftModifier);
      emit deselectingAllBody(asking);
      processed = true;
    }
    break;
  case Qt::Key_M:
    if (!doc->getDvidTarget().readOnly()) {
      emit mergingBody();
      processed = true;
    }
    break;
  case Qt::Key_U:
    if (!doc->getDvidTarget().readOnly()) {
      if (!isSplitWindow()) {
        emit uploadingMerge();
        processed = true;
      }
    }
    break;
  case Qt::Key_B:
    if (event->modifiers() == Qt::NoModifier &&
        interactiveContext().isFreeMode() &&
        buddyDocument()->getTag() ==  neutu::Document::ETag::FLYEM_PROOFREAD) {
      if (getCompleteDocument()->hasBodySelected()) {
        emit goingToBodyBottom();
        processed = true;
      }
    }
    break;
  case Qt::Key_Tab:
    if (event->modifiers() == Qt::NoModifier) {
      emit goingToTBar();
      processed = true;
    }
    break;
  case Qt::Key_T:
      if (interactiveContext().isFreeMode()) {
        if (buddyDocument()->getTag() ==  neutu::Document::ETag::FLYEM_PROOFREAD) {
          if (event->modifiers() == Qt::NoModifier) {
            emit goingToBodyTop();
            processed = true;
          } else if (event->modifiers() == Qt::ShiftModifier) {
            ZStackOperator op;
            op.setOperation(ZStackOperator::OP_GRAYSCALE_TOGGLE);
            processed = true;
            op.setViewId(m_interactiveContext.getViewId());
            process(op);
          }
        }
      } else {
        if (ZFlyEmProofDocUtil::HasWrittableSynapse(doc)) {
          ZStackOperator op;
          if (interactiveContext().bookmarkEditMode() ==
              ZInteractiveContext::BOOKMARK_ADD) {
            if (event->modifiers() == Qt::NoModifier) {
              op.setOperation(ZStackOperator::OP_DVID_SYNAPSE_START_TBAR);
            } else if (event->modifiers() == Qt::ShiftModifier) {
              op.setOperation(ZStackOperator::OP_DVID_SYNAPSE_START_PSD);
            }
          } else if (interactiveContext().synapseEditMode() ==
                     ZInteractiveContext::SYNAPSE_ADD_PRE) {
            op.setOperation(ZStackOperator::OP_DVID_SYNAPSE_START_PSD);
          } else if (interactiveContext().synapseEditMode() ==
                     ZInteractiveContext::SYNAPSE_ADD_POST) {
            op.setOperation(ZStackOperator::OP_DVID_SYNAPSE_START_TBAR);
          }
          if (!op.isNull()) {
            op.setViewId(m_interactiveContext.getViewId());
            processed = true;
            process(op);
          }
        }
      }

    break;
  case Qt::Key_I:
    if (event->modifiers() == Qt::NoModifier) {
      process(ZStackOperator::OP_DVID_SYNAPSE_START_PSD);
      processed = true;
    }
    break;
  case Qt::Key_O:
    if (event->modifiers() == Qt::NoModifier) {
      process(ZStackOperator::OP_DVID_SYNAPSE_START_TBAR);
      processed = true;
    }
    break;
  case Qt::Key_1:
    if (interactiveContext().todoEditMode() ==
        ZInteractiveContext::TODO_ADD_ITEM) {

      processed = true;
    }
    break;
  case Qt::Key_V:
  if (ZFlyEmProofDocUtil::HasWrittableSynapse(doc)) {
    if (event->modifiers() == Qt::NoModifier) {
      QAction *action = getAction(ZActionFactory::ACTION_SYNAPSE_MOVE);
      if (action != NULL) {
        action->trigger();
        processed = true;
      }
    }
  }
    break;
  case Qt::Key_X:
  if (ZFlyEmProofDocUtil::HasWrittableSynapse(doc)) {
    if (event->modifiers() == Qt::NoModifier) {
      QAction *action = getAction(ZActionFactory::ACTION_SYNAPSE_DELETE);
      if (action != NULL) {
        action->trigger();
        processed = true;
      }
    }
  }
    break;
  case Qt::Key_Y:
  if (ZFlyEmProofDocUtil::HasWrittableSynapse(doc)) {
    if (event->modifiers() == Qt::NoModifier) {
      QAction *action = getAction(ZActionFactory::ACTION_SYNAPSE_VERIFY);
      if (action != NULL) {
        action->trigger();
        processed = true;
      }
    }
  }
    break;
  case Qt::Key_N:
  if (ZFlyEmProofDocUtil::HasWrittableSynapse(doc)) {
    if (event->modifiers() == Qt::NoModifier) {
      QAction *action = getAction(ZActionFactory::ACTION_SYNAPSE_UNVERIFY);
      if (action != NULL) {
        action->trigger();
        processed = true;
      }
    }
  }
    break;
  default:
    break;
  }

  ZStackOperator op;
  if (!processed) {
    op.setOperation(m_bookmarkKeyOperationMap.getOperation(
                      event->key(), event->modifiers()));
    op.setViewId(m_interactiveContext.getViewId());
    processed = process(op);
  }

  return processed;
}

bool ZFlyEmProofPresenter::processKeyPressEvent(QKeyEvent *event, int viewId)
{
  neutu::LogKeyPressEvent(event, "ZFlyEmProofMvc");
//  KINFO << QString("Key %1 pressed in ZFlyEmProofMvc").
//           arg(QKeySequence(event->key()).toString());

  bool processed = false;

  switch (event->key()) {
  case Qt::Key_Space:
    if (isSplitOn()) {
      if (event->modifiers() == Qt::ShiftModifier) {
        emit runningSplit();
        processed = true;
      } else if (event->modifiers() == Qt::NoModifier) {
        emit runningLocalSplit();
        processed = true;
      } else if (event->modifiers() & Qt::AltModifier) {
        emit runningFullSplit();
        processed = true;
      }
    }
    break;
  case Qt::Key_F1:
    emit goingToBody();
    processed = true;
    break;
  case Qt::Key_F2:
    emit selectingBody();
    processed = true;
    break;
//  case Qt::Key_F3:
//    emit goingToPosition();
//    processed = true;
//    break;
  case Qt::Key_D:
    if (event->modifiers() == Qt::ControlModifier) {
      LINFO() << "Ctrl+D pressed: Toggling data";
      emit togglingData();
      processed = true;
    }
    break;
  case Qt::Key_3:
    if (!isSplitOn()) {
      emit togglingBodyColorMap();
    }
    break;
  default:
    break;
  }


  if (processed == false) {
    processed = ZStackPresenter::processKeyPressEvent(event, viewId);
  }

  return processed;
}

void ZFlyEmProofPresenter::createSynapseContextMenu()
{
  if (m_synapseContextMenu == NULL) {
//    ZStackDocMenuFactory menuFactory;
    m_synapseContextMenu =
        getMenuFactory()->makeSynapseContextMenu(this, getParentWidget(), NULL);
  }
}

void ZFlyEmProofPresenter::verifySelectedSynapse()
{
  getCompleteDocument()->verifySelectedSynapse();
}

void ZFlyEmProofPresenter::unverifySelectedSynapse()
{
  getCompleteDocument()->unverifySelectedSynapse();
}

void ZFlyEmProofPresenter::deleteSelectedSynapse()
{
  getCompleteDocument()->executeRemoveSynapseCommand();
//  getCompleteDocument()->deleteSelectedSynapse();
}

void ZFlyEmProofPresenter::linkSelectedSynapse()
{
  getCompleteDocument()->executeLinkSynapseCommand();
}

void ZFlyEmProofPresenter::repairSelectedSynapse()
{
  getCompleteDocument()->repairSelectedSynapses();
}

void ZFlyEmProofPresenter::unlinkSelectedSynapse()
{
  getCompleteDocument()->executeUnlinkSynapseCommand();
}

void ZFlyEmProofPresenter::highlightPsd(bool on)
{
  getCompleteDocument()->highlightPsd(on);
}

void ZFlyEmProofPresenter::tryAddPreSynapseMode()
{
  tryAddSynapseMode(ZDvidSynapse::EKind::KIND_PRE_SYN);
}

void ZFlyEmProofPresenter::tryAddPostSynapseMode()
{
  tryAddSynapseMode(ZDvidSynapse::EKind::KIND_POST_SYN);
}

void ZFlyEmProofPresenter::tryAddSynapseMode(ZDvidSynapse::EKind kind)
{
  exitEdit();

  if (ZFlyEmProofDocUtil::HasWrittableSynapse(getCompleteDocument())) {
//    turnOnActiveObject(ROLE_SYNAPSE, false);
    switch (kind) {
    case ZDvidSynapse::EKind::KIND_PRE_SYN:
      m_interactiveContext.setSynapseEditMode(
            ZInteractiveContext::SYNAPSE_ADD_PRE);
      break;
    case ZDvidSynapse::EKind::KIND_POST_SYN:
      m_interactiveContext.setSynapseEditMode(
            ZInteractiveContext::SYNAPSE_ADD_POST);
      break;
    default:
      m_interactiveContext.setSynapseEditMode(
            ZInteractiveContext::SYNAPSE_ADD_PRE);
      break;
    }

    m_mouseCursorGlyph->activate(ZMouseCursorGlyph::ERole::ROLE_SYNAPSE);
    updateMouseCursorGlyphPos();

    /*
    turnOnActiveObject(ROLE_SYNAPSE, [this](ZStackObject *obj) {
      ZStackBall *ball = dynamic_cast<ZStackBall*>(obj);
      if (ball) {
        ZDvidSynapse::EKind kind  = ZDvidSynapse::EKind::KIND_UNKNOWN;
        switch (interactiveContext().synapseEditMode()) {
        case ZInteractiveContext::SYNAPSE_ADD_PRE:
          kind = ZDvidSynapse::EKind::KIND_PRE_SYN;
          break;
        case ZInteractiveContext::SYNAPSE_ADD_POST:
          kind = ZDvidSynapse::EKind::KIND_POST_SYN;
          break;
        default:
          break;
        }
        QColor color = ZDvidSynapse::GetDefaultColor(kind);
        color.setAlpha(200);
        ball->setColor(color);
        ball->setRadius(ZDvidSynapse::GetDefaultRadius(kind));
      }
    });
    */

//    updateActiveObjectForSynapseAdd();

//    buddyView()->paintActiveDecoration();

    updateCursor();
  } else {
    getCompleteDocument()->emitWarning(
          "Cannot add a synapse "
          "because either no synapse data is specified or it is readonly.");
  }
}

void ZFlyEmProofPresenter::tryMoveSynapseMode()
{
  ZDvidSynapse synapse = getCompleteDocument()->getSingleSelectedSynapse();
  if (synapse.isValid()) {
    m_mouseCursorGlyph->activate(
          ZMouseCursorGlyph::ERole::ROLE_SYNAPSE, [&](ZStackObject *obj) {
      ZStackBall *ball = dynamic_cast<ZStackBall*>(obj);
      ball->setColor(synapse.getColor());
      ball->setRadius(synapse.getRadius());
      const ZMouseEvent& event = m_mouseEventProcessor.getLatestMouseEvent();
      ball->setCenter(event.getDataPosition());
    });
    m_interactiveContext.setSynapseEditMode(ZInteractiveContext::SYNAPSE_MOVE);
    updateMouseCursorGlyphPos();
    updateCursor();
  }

  /*
  if (updateActiveObjectForSynapseMove()) {
    turnOnActiveObject(ROLE_SYNAPSE);
    m_interactiveContext.setSynapseEditMode(ZInteractiveContext::SYNAPSE_MOVE);
    updateCursor();
  }
  */
}

QMenu* ZFlyEmProofPresenter::getSynapseContextMenu()
{
  if (m_synapseContextMenu == NULL) {
    createSynapseContextMenu();
  }

  return m_synapseContextMenu;
}

QMenu* ZFlyEmProofPresenter::getContextMenu()
{
//  if (m_contextMenu == NULL) {
  m_contextMenu = getMenuFactory()->makeContextMenu(this, NULL, m_contextMenu);
//  }

  return m_contextMenu;
  /*
  if (getCompleteDocument()->hasDvidSynapseSelected()) {
    return getSynapseContextMenu();
  }

  if (getCompleteDocument()->hasDvidSynapse()) {
    return getStackContextMenu();
  }

  return NULL;
  */
}


bool ZFlyEmProofPresenter::isHighlight() const
{
  return m_isHightlightMode && !isSplitOn();
}

void ZFlyEmProofPresenter::setHighlightMode(bool hl)
{
  if (m_isHightlightMode != hl) {
    m_isHightlightMode = hl;
    emit highlightModeChanged();
  }
}

void ZFlyEmProofPresenter::toggleHighlightMode()
{
  setHighlightMode(!isHighlight());
}

bool ZFlyEmProofPresenter::isSplitOn() const
{
  return getAction(ZActionFactory::ACTION_PAINT_STROKE)->isEnabled();
}

void ZFlyEmProofPresenter::enableSplit(neutu::EBodySplitMode mode)
{
  if (mode == neutu::EBodySplitMode::NONE) {
    disableSplit();
  } else {
    setSplitMode(mode);
    setSplitEnabled(true);
  }
}

void ZFlyEmProofPresenter::disableSplit()
{
  setSplitMode(neutu::EBodySplitMode::NONE);
  setSplitEnabled(false);
}

void ZFlyEmProofPresenter::setSplitEnabled(bool s)
{
  getAction(ZActionFactory::ACTION_PAINT_STROKE)->setEnabled(s);
}

void ZFlyEmProofPresenter::tryAddBookmarkMode()
{
  exitEdit();

  interactiveContext().setBookmarkEditMode(ZInteractiveContext::BOOKMARK_ADD);
  m_mouseCursorGlyph->activate(ZMouseCursorGlyph::ERole::ROLE_BOOKMARK);
  updateMouseCursorGlyphPos();
  //buddyView()->paintActiveDecoration();
  updateCursor();
}

/*
void ZFlyEmProofPresenter::tryTodoItemMode()
{
  exitStrokeEdit();
  ZPoint pos = getModelPositionFromGlobalCursor(QCursor::pos());
//  QPointF pos = mapFromGlobalToStack(QCursor::pos());
  tryAddTodoItemMode(pos.x(), pos.y());
}
*/

void ZFlyEmProofPresenter::tryAddSynapse(const ZIntPoint &pt, bool tryingLink)
{
  switch (interactiveContext().synapseEditMode()) {
  case ZInteractiveContext::SYNAPSE_ADD_PRE:
    tryAddSynapse(pt, ZDvidSynapse::EKind::KIND_PRE_SYN, tryingLink);
    break;
  case ZInteractiveContext::SYNAPSE_ADD_POST:
    tryAddSynapse(pt, ZDvidSynapse::EKind::KIND_POST_SYN, tryingLink);
    break;
  default:
    break;
  }
}

void ZFlyEmProofPresenter::tryAddSynapse(
    const ZIntPoint &pt, ZDvidSynapse::EKind kind, bool tryingLink)
{
  ZDvidSynapse synapse;
  synapse.setPosition(pt);
  synapse.setKind(kind);
  synapse.setDefaultRadius();
  synapse.setDefaultColor();
  synapse.setUserName(neutu::GetCurrentUserName());
  getCompleteDocument()->executeAddSynapseCommand(synapse, tryingLink);
//  getCompleteDocument()->addSynapse(pt, kind);
}

void ZFlyEmProofPresenter::tryAddTodoItem(
    const ZIntPoint &pt, bool checked, neutu::EToDoAction action,
    uint64_t bodyId)
{
  tryAddTodoItem(pt.getX(), pt.getY(), pt.getZ(), checked, action, bodyId);
}

void ZFlyEmProofPresenter::tryAddTodoItem(
    int x, int y, int z, bool checked, neutu::EToDoAction action,
    uint64_t bodyId)
{
  if (m_todoDelegate) {
    m_todoDelegate->add(x, y, z, checked, action, bodyId);
  } else {
    getCompleteDocument()->executeAddTodoCommand(
          x, y, z, checked, action, bodyId);
  }
}

void ZFlyEmProofPresenter::tryAddTodoItem(
    const ZIntPoint &pt, bool checked, neutu::EToDoAction action)
{
  tryAddTodoItem(pt, checked, action, 0);
}

void ZFlyEmProofPresenter::tryAddTodoItem(const ZIntPoint &pt)
{
  tryAddTodoItem(pt, false, neutu::EToDoAction::TO_DO);
//  getCompleteDocument()->executeAddTodoItemCommand(pt, false);
}

void ZFlyEmProofPresenter::tryAddToMergeItem(const ZIntPoint &pt)
{
  tryAddTodoItem(pt, false, neutu::EToDoAction::TO_MERGE);
//  getCompleteDocument()->executeAddToMergeItemCommand(pt);
}

void ZFlyEmProofPresenter::tryAddToSplitItem(const ZIntPoint &pt)
{
  tryAddTodoItem(pt, false, neutu::EToDoAction::TO_SPLIT);
//  getCompleteDocument()->executeAddToSplitItemCommand(pt);
}

void ZFlyEmProofPresenter::tryAddTraceToSomaItem(const ZIntPoint &pt)
{
  tryAddTodoItem(pt, false, neutu::EToDoAction::TO_TRACE_TO_SOMA);
}

void ZFlyEmProofPresenter::tryAddNoSomaItem(const ZIntPoint &pt)
{
  tryAddTodoItem(pt, true, neutu::EToDoAction::NO_SOMA);
}

void ZFlyEmProofPresenter::tryAddDiagnosticItem(const ZIntPoint &pt)
{
  tryAddTodoItem(pt, false, neutu::EToDoAction::DIAGNOSTIC);
}

void ZFlyEmProofPresenter::tryAddSegmentationDiagnosticItem(const ZIntPoint &pt)
{
  tryAddTodoItem(pt, false, neutu::EToDoAction::SEGMENTATION_DIAGNOSIC);
}

void ZFlyEmProofPresenter::tryAddToSupervoxelSplitItem(const ZIntPoint &pt)
{
  tryAddTodoItem(pt, false, neutu::EToDoAction::TO_SUPERVOXEL_SPLIT);
}

void ZFlyEmProofPresenter::tryAddDoneItem(const ZIntPoint &pt)
{
  tryAddTodoItem(pt, true, neutu::EToDoAction::TO_DO);
//  getCompleteDocument()->executeAddTodoItemCommand(pt, true);
}

void ZFlyEmProofPresenter::removeTodoItem()
{
  getCompleteDocument()->executeRemoveTodoItemCommand();
}

void ZFlyEmProofPresenter::checkTodoItem()
{
  getCompleteDocument()->checkTodoItem(true);
}

void ZFlyEmProofPresenter::uncheckTodoItem()
{
  getCompleteDocument()->checkTodoItem(false);
}

void ZFlyEmProofPresenter::setTodoItemToNormal()
{
  getCompleteDocument()->setTodoItemToNormal();
}

void ZFlyEmProofPresenter::setTodoItemIrrelevant()
{
  getCompleteDocument()->setTodoItemIrrelevant();
}

void ZFlyEmProofPresenter::setTodoItemToMerge()
{
  getCompleteDocument()->setTodoItemAction(neutu::EToDoAction::TO_MERGE);
}

void ZFlyEmProofPresenter::setTodoItemToSplit()
{
  getCompleteDocument()->setTodoItemAction(neutu::EToDoAction::TO_SPLIT);
}

void ZFlyEmProofPresenter::setTodoItemToTraceToSoma()
{
  getCompleteDocument()->setTodoItemAction(neutu::EToDoAction::TO_TRACE_TO_SOMA);
}

void ZFlyEmProofPresenter::setTodoItemToNoSoma()
{
  getCompleteDocument()->setTodoItemAction(neutu::EToDoAction::NO_SOMA);
  getCompleteDocument()->checkTodoItem(true);
}

void ZFlyEmProofPresenter::setTodoDelegate(
    std::unique_ptr<ZFlyEmToDoDelegate> &&delegate)
{
  m_todoDelegate = std::move(delegate);
}

ZPoint ZFlyEmProofPresenter::getLastMouseReleasePosition(
    Qt::MouseButtons buttons) const
{
  const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
        buttons, ZMouseEvent::EAction::RELEASE);
  ZPoint pt = event.getDataPosition();

  return pt;
}

void ZFlyEmProofPresenter::trace()
{
  const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
        Qt::RightButton, ZMouseEvent::EAction::RELEASE);
  ZPoint pt = event.getDataPosition();

  getCompleteDocument()->trace(pt);
}

void ZFlyEmProofPresenter::tryAddTodoItem()
{
  const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
        Qt::RightButton, ZMouseEvent::EAction::RELEASE);
  ZPoint pt = event.getDataPosition();
  tryAddTodoItem(pt.toIntPoint());
}

void ZFlyEmProofPresenter::tryAddToMergeItem()
{
  const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
        Qt::RightButton, ZMouseEvent::EAction::RELEASE);
  ZPoint pt = event.getDataPosition();
  tryAddToMergeItem(pt.roundToIntPoint());
}

void ZFlyEmProofPresenter::tryAddToSplitItem()
{
  const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
        Qt::RightButton, ZMouseEvent::EAction::RELEASE);
  ZPoint pt = event.getDataPosition();
  tryAddToSplitItem(pt.roundToIntPoint());
}

void ZFlyEmProofPresenter::tryAddToSupervoxelSplitItem()
{
  const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
        Qt::RightButton, ZMouseEvent::EAction::RELEASE);
  ZPoint pt = event.getDataPosition();
  tryAddToSupervoxelSplitItem(pt.roundToIntPoint());
}

void ZFlyEmProofPresenter::tryAddTraceToSomaItem()
{
  const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
        Qt::RightButton, ZMouseEvent::EAction::RELEASE);
  ZPoint pt = event.getDataPosition();
  tryAddTraceToSomaItem(pt.roundToIntPoint());
}

void ZFlyEmProofPresenter::tryAddNoSomaItem()
{
  const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
        Qt::RightButton, ZMouseEvent::EAction::RELEASE);
  ZPoint pt = event.getDataPosition();
  tryAddNoSomaItem(pt.roundToIntPoint());
}

void ZFlyEmProofPresenter::tryAddDiagnosticItem()
{
  tryAddDiagnosticItem(
        getLastMouseReleasePosition(Qt::RightButton).roundToIntPoint());
}

void ZFlyEmProofPresenter::tryAddSegmentationDiagnosticItem()
{
  tryAddSegmentationDiagnosticItem(
        getLastMouseReleasePosition(Qt::RightButton).roundToIntPoint());
}

void ZFlyEmProofPresenter::tryAddDoneItem()
{
  const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
        Qt::RightButton, ZMouseEvent::EAction::RELEASE);
  ZPoint pt = event.getDataPosition();
  tryAddDoneItem(pt.roundToIntPoint());
}

void ZFlyEmProofPresenter::tryMoveSynapse(const ZIntPoint &pt)
{
  getCompleteDocument()->executeMoveSynapseCommand(pt);
//  getCompleteDocument()->tryMoveSelectedSynapse(pt);
  exitSynapseEdit();
//  m_interactiveContext.setSynapseEditMode(ZInteractiveContext::SYNAPSE_EDIT_OFF);
  updateCursor();
}

/*
void ZFlyEmProofPresenter::tryAddTodoItemMode(double x, double y)
{
  interactiveContext().setTodoEditMode(ZInteractiveContext::TODO_ADD_ITEM);

  m_mouseCursorGlyph->activate(ZMouseCursorGlyph::ERole::ROLE_TODO_ITEM);
  m_mouseCursorGlyph->update
  ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_TODO_ITEM);

  buddyDocument()->mapToDataCoord(&x, &y, NULL);
  stroke->set(x, y);

  turnOnActiveObject(ROLE_TODO_ITEM);
  updateCursor();
}
*/

#if 0
void ZFlyEmProofPresenter::tryAddBookmarkMode(double x, double y)
{
  exitEdit();

  interactiveContext().setBookmarkEditMode(ZInteractiveContext::BOOKMARK_ADD);

//  ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_BOOKMARK);

//  stroke->setWidth(10.0);

//  buddyDocument()->mapToDataCoord(&x, &y, NULL);
//  stroke->set(x, y);
//  m_stroke.setEraser(false);
//  m_stroke.setFilled(false);
//  m_stroke.setTarget(neutu::data3d::ETarget::TARGET_WIDGET);
//  turnOnStroke();
  turnOnActiveObject(ROLE_BOOKMARK);
  //buddyView()->paintActiveDecoration();
  updateCursor();
}
#endif

void ZFlyEmProofPresenter::runTipDetection() {
    const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
          Qt::RightButton, ZMouseEvent::EAction::RELEASE);
    ZPoint pt = event.getDataPosition();
    uint64_t bodyId = getCompleteDocument()->getLabelId(
          pt.getX(), pt.getY(), pt.getZ(), neutu::ELabelSource::ORIGINAL);

    emit tipDetectRequested(pt.roundToIntPoint(), bodyId);
}

void ZFlyEmProofPresenter::setCutPlaneAlongX()
{
  updateCutPlane(neutu::EAxis::X, neutu::EAxis::Z, neutu::EAxis::Y);

  /*
  getMainView()->setCutPlane(neutu::EAxis::X);
  auto viewList = getViewList();
  if (viewList.size() > 1) {
    viewList[1]->setCutPlane(neutu::EAxis::Z);
    if (viewList.size() > 2) {
      viewList[2]->setCutPlane(neutu::EAxis::Y);
    }
  }
  updateViewLayout();
//  emit updatingViewLayout(std::vector<int>{0, 1, 3});
    */
}

void ZFlyEmProofPresenter::setCutPlaneAlongY()
{
  updateCutPlane(neutu::EAxis::Y, neutu::EAxis::Z, neutu::EAxis::X);
  /*
  getMainView()->setCutPlane(neutu::EAxis::Y);
  auto viewList = getViewList();
  if (viewList.size() > 1) {
    viewList[1]->setCutPlane(neutu::EAxis::Z);
    if (viewList.size() > 2) {
      viewList[2]->setCutPlane(neutu::EAxis::X);
    }
  }

  emit updatingViewLayout(std::vector<int>{0, 2, 3});
  */
}

void ZFlyEmProofPresenter::setCutPlaneAlongZ()
{
  updateCutPlane(neutu::EAxis::Z, neutu::EAxis::X, neutu::EAxis::Y);
  /*
  getMainView()->setCutPlane(neutu::EAxis::Z);
  auto viewList = getViewList();
  if (viewList.size() > 1) {
    viewList[1]->setCutPlane(neutu::EAxis::X);
    if (viewList.size() > 2) {
      viewList[2]->setCutPlane(neutu::EAxis::Y);
    }
  }

  emit updatingViewLayout(std::vector<int>());
  */
}

void ZFlyEmProofPresenter::setCutPlaneArb()
{
  setMainSliceAxis(neutu::EAxis::ARB);
  getMainView()->setCutPlane(neutu::EAxis::ARB);
  getMainView()->setRightHanded(true);

  ZPlane ort = getMainView()->getCutOrientation();

  auto viewList = getViewList();
  if (viewList.size() > 1) {
    viewList[1]->enableViewChangeSignal(false);
    viewList[1]->setCutPlane(ort.getNormal(), ort.getV2());
    viewList[1]->setRightHanded(false);
    viewList[1]->enableViewChangeSignal(true);
    if (viewList.size() > 2) {
      viewList[2]->enableViewChangeSignal(false);
      viewList[2]->setCutPlane(ort.getV1(), ort.getNormal());
      viewList[2]->setRightHanded(false);
      viewList[2]->enableViewChangeSignal(true);
    }
  }

  updateViewLayout();

//  emit updatingViewLayout(std::vector<int>());
}

void ZFlyEmProofPresenter::setViewLayout1()
{
  m_viewCount = 1;
  updateViewLayout();
}

void ZFlyEmProofPresenter::setViewLayout2()
{
  m_viewCount = 2;
  updateViewLayout();
}

void ZFlyEmProofPresenter::setViewLayout3()
{
  m_viewCount = 3;
  updateViewLayout();
}


ZFlyEmProofDoc* ZFlyEmProofPresenter::getCompleteDocument() const
{
  return qobject_cast<ZFlyEmProofDoc*>(buddyDocument());
}

void ZFlyEmProofPresenter::addActiveDecorationAsBookmark()
{
  ZWeightedPoint pt = m_mouseCursorGlyph->getActiveGlyphGeometry();
//  ZStackBall *ball = getActiveObject<ZStackBall>(ROLE_BOOKMARK);
  ZIntPoint pos = pt.toIntPoint();
  if (pt.isValid() && getCompleteDocument()->canAddBookmarkAt(pos, true)) {
//    stroke->getLastPoint(&x, &y);
//    double radius = ball->getRadius();

    ZFlyEmBookmark *bookmark = new ZFlyEmBookmark;
    bookmark->setLocation(pos);
    bookmark->setRadius(pt.weight());
    bookmark->setCustom(true);
    bookmark->setUser(neutu::GetCurrentUserName());
    bookmark->addUserTag();
    ZFlyEmProofDoc *doc = qobject_cast<ZFlyEmProofDoc*>(buddyDocument());
    if (doc != NULL) {
      bookmark->setBodyId(doc->getBodyId(bookmark->getLocation()));
    }

    getCompleteDocument()->executeAddBookmarkCommand(bookmark);
  }
}

void ZFlyEmProofPresenter::showSupervoxelList()
{
  emit showingSupervoxelList();
}

bool ZFlyEmProofPresenter::allowingBlinkingSegmentation() const
{
  return m_blinkingSegmenationAllowed;
}

void ZFlyEmProofPresenter::allowBlinkingSegmentation(bool on)
{
  m_blinkingSegmenationAllowed = on;
  getCompleteDocument()->allowDvidLabelSliceBlinking(on);
}

bool ZFlyEmProofPresenter::allowingBodySplit() const
{
  return (getSliceAxis() == neutu::EAxis::Z) &&
      ZStackDocHelper::AllowingBodySplit(buddyDocument());
}

bool ZFlyEmProofPresenter::allowingBodySelection() const
{
  return !isSplitOn() && getCompleteDocument()->hasSegmentation();
}

void ZFlyEmProofPresenter::toggleSupervoxelView(bool on)
{
  getCompleteDocument()->setSupervoxelMode(on, getMainView()->getViewParameter());
}

void ZFlyEmProofPresenter::takeScreenshot()
{
  getMainView()->takeScreenshot();
}

void ZFlyEmProofPresenter::setBodyHittable(bool on)
{
  getCompleteDocument()->setLabelSliceHittable(on);
  m_isBodyHittable = on;
}

bool ZFlyEmProofPresenter::processCustomOperator(
    const ZStackOperator &op, ZInteractionEvent *e)
{
  const ZMouseEvent& event = m_mouseEventProcessor.getLatestMouseEvent();
//  ZPoint currentStackPos = event.getPosition(neutu::ECoordinateSystem::STACK);
  ZPoint currentDataPos = event.getDataPosition();

  bool processed = true;

  m_docSelector.setDocument(getSharedBuddyDocument());

  ZStackView *view = getView(op.getViewId());

  switch (op.getOperation()) {
  case ZStackOperator::OP_CUSTOM_MOUSE_RELEASE:
    if (isHighlight() && m_isBodyHittable) {
//      const ZMouseEvent& event = m_mouseEventProcessor.getLatestMouseEvent();
//      ZPoint currentStackPos = event.getPosition(neutu::ECoordinateSystem::STACK);
//      ZIntPoint pos = currentStackPos.roundToIntPoint();
      emit selectingBodyAt(
            currentDataPos.getX(), currentDataPos.getY(), currentDataPos.getZ());
    }
    break;
  case ZStackOperator::OP_SHOW_BODY_CONTEXT_MENU:
    break;
  case ZStackOperator::OP_BOOKMARK_DELETE:
    getCompleteDocument()->executeRemoveBookmarkCommand();
    break;
  case ZStackOperator::OP_BOOKMARK_ENTER_ADD_MODE:
    tryAddBookmarkMode();
    break;
  case ZStackOperator::OP_FLYEM_TOD_ENTER_ADD_MODE:
//    tryTodoItemMode();
    break;
  case ZStackOperator::OP_BOOKMARK_ADD_NEW:
    addActiveDecorationAsBookmark();
    break;
  case ZStackOperator::OP_BOOKMARK_ANNOTATE:
    emit annotatingBookmark(op.getHitObject<ZFlyEmBookmark>());
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_ANNOTATE:
    emit annotatingSynapse();
    break;
  case ZStackOperator::OP_FLYEM_TODO_ANNOTATE:
    emit annotatingTodo();
    break;
  case ZStackOperator::OP_OBJECT_SELECT_IN_ROI:
    emit selectingBodyInRoi(op.getViewId(), true);
    break;
  case ZStackOperator::OP_BOOKMARK_SELECT_BODY_SINGLE:
  {
    ZStackObject *obj = op.getHitObject<ZStackObject>();
    if (obj) {
      m_docSelector.deselectAll();
      buddyDocument()->setSelected(obj, true);
      getCompleteDocument()->toggleBodyUnderObject(obj);
      e->setEvent(
            ZInteractionEvent::EVENT_OBJECT_SELECTED);
    }
  }
    break;
  case ZStackOperator::OP_FLYEM_TODO_SELECT_SINGLE:
  {
    ZOUT(LTRACE(), 5) << "Get todo list selection";
    QList<ZFlyEmToDoList*> todoList =
        getCompleteDocument()->getObjectList<ZFlyEmToDoList>();
    ZIntPoint hitPoint = op.getHitObject()->getHitPoint();

    m_docSelector.deselectAll();
//    docSelector.setSelectOption(ZStackObject);

    for (QList<ZFlyEmToDoList*>::iterator iter = todoList.begin();
         iter != todoList.end(); ++iter) {
      ZFlyEmToDoList *item = *iter;
      item->setHitPoint(hitPoint);
      item->selectHit(false);
    }
    if (e != NULL) {
      e->setEvent(ZInteractionEvent::EVENT_OBJECT_SELECTED);
    }
  }
    break;
  case ZStackOperator::OP_FLYEM_TODO_SELECT_MULTIPLE:
  {
    ZOUT(LTRACE(), 5) << "Get todo list selection";
    QList<ZFlyEmToDoList*> todoList =
        getCompleteDocument()->getObjectList<ZFlyEmToDoList>();
    ZIntPoint hitPoint = op.getHitObject()->getHitPoint();

    for (QList<ZFlyEmToDoList*>::iterator iter = todoList.begin();
         iter != todoList.end(); ++iter) {
      ZFlyEmToDoList *item = *iter;
      item->setHitPoint(hitPoint);
      item->selectHit(true);
    }
    if (e != NULL) {
      e->setEvent(ZInteractionEvent::EVENT_OBJECT_SELECTED);
    }
  }
    break;
  case ZStackOperator::OP_FLYEM_TODO_SELECT_TOGGLE:
  {
    ZOUT(LTRACE(), 5) << "Toggle todo selection";
    QList<ZFlyEmToDoList*> todoList =
        getCompleteDocument()->getObjectList<ZFlyEmToDoList>();
    ZIntPoint hitPoint = op.getHitObject()->getHitPoint();

    for (QList<ZFlyEmToDoList*>::iterator iter = todoList.begin();
         iter != todoList.end(); ++iter) {
      ZFlyEmToDoList *item = *iter;
      item->setHitPoint(hitPoint);
      item->toggleHitSelect();
    }
    if (e != NULL) {
      e->setEvent(ZInteractionEvent::EVENT_OBJECT_SELECTED);
    }
  }
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_SELECT_SINGLE:
  {
    QList<ZDvidSynapseEnsemble*> seList =
        getCompleteDocument()->getDvidSynapseEnsembleList();
    ZIntPoint hitPoint = op.getHitObject()->getHitPoint();
    m_docSelector.deselectAll();

    for (QList<ZDvidSynapseEnsemble*>::iterator iter = seList.begin();
         iter != seList.end(); ++iter) {
      ZDvidSynapseEnsemble *se = *iter;
      se->setHitPoint(hitPoint);
      se->selectHitWithPartner(false);
//      getCompleteDocument()->getDvidSynapseEnsemble(
//            buddyView()->getSliceAxis())->selectHitWithPartner(false);
    }
    if (e != NULL) {
      e->setEvent(ZInteractionEvent::EVENT_OBJECT_SELECTED);
    }
  }
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_SELECT_TOGGLE:
    if (view) {
      getCompleteDocument()->getDvidSynapseEnsemble(
            view->getSliceAxis())->toggleHitSelectWithPartner();
      if (e != NULL) {
        e->setEvent(ZInteractionEvent::EVENT_OBJECT_SELECTED);
      }
    }
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_ADD:
    tryAddSynapse(currentDataPos.roundToIntPoint(), true);
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_ADD_ORPHAN:
    tryAddSynapse(currentDataPos.roundToIntPoint(), false);
    break;
  case ZStackOperator::OP_FLYEM_TODO_ADD:
    tryAddTodoItem(currentDataPos.roundToIntPoint());
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_MOVE:
    tryMoveSynapse(currentDataPos.roundToIntPoint());
    break;
  case ZStackOperator::OP_TRACK_MOUSE_MOVE:
    updateMouseCursorGlyphPos();
    /*
    if (m_interactiveContext.synapseEditMode() !=
        ZInteractiveContext::SYNAPSE_EDIT_OFF) {
      ZStackBall *ball = getActiveObject<ZStackBall>(ROLE_SYNAPSE);
//      ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_SYNAPSE);
      if (m_interactiveContext.synapseEditMode() ==
          ZInteractiveContext::SYNAPSE_ADD_PRE ||
          m_interactiveContext.synapseEditMode() ==
                    ZInteractiveContext::SYNAPSE_ADD_POST) {
        updateActiveObjectForSynapseAdd(currentDataPos);
      } else if (m_interactiveContext.synapseEditMode() ==
                 ZInteractiveContext::SYNAPSE_MOVE) {
        updateActiveObjectForSynapseMove(currentDataPos);
      }
      ball->setCenter(currentDataPos);
      */
      /*
      ZPoint pos = currentStackPos;
//      pos.shiftSliceAxis(buddyView()->getSliceAxis());
      stroke->setLast(pos.x(), pos.y());
      */
    /*
      if (e != NULL) {
        e->setEvent(
              ZInteractionEvent::EVENT_ACTIVE_DECORATION_UPDATED);
      }
    }
    */
    break;
  case ZStackOperator::OP_DVID_LABEL_SLICE_SELECT_SINGLE:
    getCompleteDocument()->processLabelSliceHit(
          op.getHitObject(), ZStackObject::ESelection::SELECT_SINGLE);
    /*
  {
    ZStackObject *obj = op.getHitObject();
//    ZDvidLabelSlice *labelSlice =  op.getHitObject<ZDvidLabelSlice>();
    if (obj) {
      obj->processHit(ZStackObject::ESelection::SELECT_SINGLE);
      getCompleteDocument()->notifyBodySelectionChanged();
    }
  }
  */
    /*
  {
//    buddyDocument()->deselectAllObject(false);
    std::set<uint64_t> bodySet;
    if (op.getHitObject<ZDvidLabelSlice>() != NULL) {
      ZDvidLabelSlice *labelSlice =  op.getHitObject<ZDvidLabelSlice>();
      uint64_t label = labelSlice->getHitLabel();
      if (label > 0) {
        bodySet.insert(label);
      }
    }
    getCompleteDocument()->setSelectedBody(
          bodySet, neutu::ELabelSource::ORIGINAL);
  }
  */
    break;
  case ZStackOperator::OP_DVID_LABEL_SLICE_TOGGLE_SELECT:
    getCompleteDocument()->processLabelSliceHit(
          op.getHitObject(), ZStackObject::ESelection::SELECT_TOGGLE);
    /*
  {
    ZStackObject *labelSlice = op.getHitObject();
//    ZDvidLabelSlice *labelSlice =  op.getHitObject<ZDvidLabelSlice>();
    if (labelSlice) {
      labelSlice->processHit(ZStackObject::ESelection::SELECT_TOGGLE);
      getCompleteDocument()->notifyBodySelectionChanged();
    }
  }
  */
    /*
  {
    std::set<uint64_t> bodySet = getCompleteDocument()->getSelectedBodySet(
          neutu::ELabelSource::MAPPED);
    if (op.getHitObject<ZDvidLabelSlice>() != NULL) {
      ZDvidLabelSlice *labelSlice =  op.getHitObject<ZDvidLabelSlice>();
      uint64_t label = labelSlice->getHitLabel();

      if (label > 0) {
        if (bodySet.count(label) > 0) {
          bodySet.erase(label);
        } else {
          bodySet.insert(label);
        }
      }
    }
    getCompleteDocument()->setSelectedBody(
          bodySet, neutu::ELabelSource::MAPPED);
//    e->setEvent(
//          ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED_IN_LABEL_SLICE);
  }
  */
    break;
  case ZStackOperator::OP_DVID_LABEL_SLICE_TOGGLE_SELECT_SINGLE:
    getCompleteDocument()->processLabelSliceHit(
          op.getHitObject(), ZStackObject::ESelection::SELECT_TOGGLE_SINGLE);
    /*
  {
    ZStackObject *labelSlice = op.getHitObject();
//    ZDvidLabelSlice *labelSlice =  op.getHitObject<ZDvidLabelSlice>();
    if (labelSlice) {
      labelSlice->processHit(ZStackObject::ESelection::SELECT_TOGGLE_SINGLE);
      getCompleteDocument()->notifyBodySelectionChanged();
    }
  }
  */
#if 0
  { //Deselect all other bodies. Select the hit body if it is not selected.
    std::set<uint64_t> bodySet = getCompleteDocument()->getSelectedBodySet(
          neutu::ELabelSource::MAPPED);
    std::set<uint64_t> newBodySet;
    if (op.getHitObject<ZDvidLabelSlice>() != NULL) {
      ZDvidLabelSlice *labelSlice =  op.getHitObject<ZDvidLabelSlice>();
      uint64_t label = labelSlice->getHitLabel();

      if (label > 0) {
        if (bodySet.count(label) == 0) {
          newBodySet.insert(label);
        }
      }
    }
    getCompleteDocument()->setSelectedBody(
          newBodySet, neutu::ELabelSource::MAPPED);
//    e->setEvent(
//          ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED_IN_LABEL_SLICE);
  }
#endif
    break;
  case ZStackOperator::OP_DVID_LABEL_SLICE_SELECT_MULTIPLE:
    getCompleteDocument()->processLabelSliceHit(
          op.getHitObject(), ZStackObject::ESelection::SELECT_MULTIPLE);
    /*
  {
    ZStackObject *labelSlice = op.getHitObject();
//    ZDvidLabelSlice *labelSlice =  op.getHitObject<ZDvidLabelSlice>();
    if (labelSlice) {
      labelSlice->processHit(ZStackObject::ESelection::SELECT_MULTIPLE);
      getCompleteDocument()->notifyBodySelectionChanged();
    }
  }
  */
    /*
  {
    std::set<uint64_t> bodySet = getCompleteDocument()->getSelectedBodySet(
          neutu::ELabelSource::MAPPED);
    if (op.getHitObject<ZDvidLabelSlice>() != NULL) {
      ZDvidLabelSlice *labelSlice =  op.getHitObject<ZDvidLabelSlice>();
      uint64_t label = labelSlice->getHitLabel();

      if (label > 0) {
        bodySet.insert(label);
      }
    }
    getCompleteDocument()->setSelectedBody(
          bodySet, neutu::ELabelSource::MAPPED);
//    e->setEvent(
//          ZInteractionEvent::EVENT_OBJECT3D_SCAN_SELECTED_IN_LABEL_SLICE);
  }
  */
    break;
  case ZStackOperator::OP_TOGGLE_SEGMENTATION:
    break;
  case ZStackOperator::OP_REFRESH_SEGMENTATION:
    getCompleteDocument()->rewriteSegmentation();
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_START_TBAR:
    tryAddPreSynapseMode();
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_START_PSD:
    tryAddPostSynapseMode();
    break;
  case ZStackOperator::OP_GRAYSCALE_TOGGLE:
    if (view) {
      getCompleteDocument()->toggleGrayscale(view->getSliceAxis());
    }
    break;
  default:
    processed = false;
    break;
  }

  getAction(ZActionFactory::ACTION_BODY_SPLIT_START)->setVisible(
        !isSplitWindow());
  getAction(ZActionFactory::ACTION_BODY_DECOMPOSE)->setVisible(
        isSplitWindow());
  getAction(ZActionFactory::ACTION_MERGE_LINK_ACTIVATE)->setVisible(
        !isSplitWindow());

  return processed;
}

void ZFlyEmProofPresenter::copyLink(const QString &option) const
{
  ZStackView *view = getContextView();
  if (view == nullptr) {
    return;
  }

  ZJsonObject obj;
  obj.decode(option.toStdString(), true);

  ZJsonObjectParser parser;
  if (parser.GetValue(obj, "type", "") == "neuroglancer") {
    const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
          Qt::RightButton, ZMouseEvent::EAction::RELEASE);
    ZPoint pt = event.getDataPosition();

    ZAffineRect rect = buddyDocument()->getRectRoi();

    if (parser.GetValue(obj, "location", "") == "rectroi") {
      if (rect.isNonEmpty()) {
        pt.set(rect.getCenter());
      }
    }

//    ZDvidTarget target = getCompleteDocument()->getDvidTarget();

    ZDvidInfo dvidInfo = getCompleteDocument()->getDvidInfo();
    ZResolution res = dvidInfo.getVoxelResolution();


//    QList<ZFlyEmBookmark*> bookmarkList =
//        ZFlyEmProofDocUtil::GetUserBookmarkList(getCompleteDocument());

    QList<std::shared_ptr<ZNeuroglancerLayerSpec>> additionalLayers;
    ZIntCuboid box = buddyDocument()->getCuboidRoi();
    if (!box.isEmpty()) {
      auto layer = ZNeuroglancerLayerSpecFactory::MakeLocalAnnotationLayer(
            "local_annotation");
      layer->addAnnotation(ZCuboid::FromIntCuboid(box));
      additionalLayers.append(layer);
    }

    QString path = ZNeuroglancerPathFactory::MakePath(
          getCompleteDocument()->getDvidEnv(), res,
          pt, view->getViewParameter().getZoomRatio(),
          additionalLayers);
    ZGlobal::CopyToClipboard(
          GET_FLYEM_CONFIG.getNeuroglancerServer() + path.toStdString());
  }
}

bool ZFlyEmProofPresenter::highTileContrast() const
{
  return m_highTileContrast;
}

bool ZFlyEmProofPresenter::smoothTransform() const
{
  return m_smoothTransform;
}

void ZFlyEmProofPresenter::setHighTileContrast(bool high)
{
  m_highTileContrast = high;
}

void ZFlyEmProofPresenter::setSmoothTransform(bool on)
{
  m_smoothTransform = on;
}

void ZFlyEmProofPresenter::showData(bool on)
{
  m_showingData = on;
}

bool ZFlyEmProofPresenter::showingData() const
{
  return m_showingData;
}

void ZFlyEmProofPresenter::processRectRoiUpdate(ZRect2d *rect, bool appending)
{
  if (!rect->isEmpty()) {
    ZStackDoc::ERoiRole roiRole = ZStackDoc::ERoiRole::GENERAL;
    if (isSplitOn()) {
      roiRole =  (rect->getZSpan() > 0)
          ? ZStackDoc::ERoiRole::SPLIT
          : ZStackDoc::ERoiRole::NONE;
    }
#ifdef _DEBUG_
    std::cout << OUTPUT_HIGHTLIGHT_2
              << "Process rect with role " << neutu::ToString(roiRole) << std::endl;
#endif
    if (roiRole == ZStackDoc::ERoiRole::NONE) {
      rect->setSize(0, 0);
      buddyDocument()->processObjectModified(rect);
    }

    buddyDocument()->processRectRoiUpdate(rect, roiRole, appending);
    if (roiRole == ZStackDoc::ERoiRole::GENERAL) {
      interactiveContext().setAcceptingRect(true);
      QMenu *menu = getContextMenu();
      if (!menu->isEmpty()) {
        const ZMouseEvent& event = m_mouseEventProcessor.getMouseEvent(
              Qt::LeftButton, ZMouseEvent::EAction::RELEASE);
        QPoint currentWidgetPos(event.getWidgetPosition().getX(),
                                event.getWidgetPosition().getY());
        getView(event.getViewId())->showContextMenu(menu, currentWidgetPos);
      }
      interactiveContext().setAcceptingRect(false);
    }
  }
}

#if 0
bool ZFlyEmProofPresenter::updateActiveObjectForSynapseMove()
{
  const ZMouseEvent& event = m_mouseEventProcessor.getLatestMouseEvent();
//  ZPoint currentStackPos = event.getPosition(neutu::ECoordinateSystem::STACK);
  return updateActiveObjectForSynapseMove(event.getDataPosition());
}

bool ZFlyEmProofPresenter::updateActiveObjectForSynapseMove(
    const ZPoint &currentPos)
{
  ZDvidSynapse synapse = getCompleteDocument()->getSingleSelectedSynapse();
  if (synapse.isValid()) {
    ZStackBall *ball = getActiveObject<ZStackBall>(ROLE_SYNAPSE);
    ball->setColor(synapse.getColor());
    ball->setRadius(synapse.getRadius());
    ball->setCenter(currentPos);

    return true;
  }

  return false;

#if 0
  ZDvidSynapseEnsemble *se = getCompleteDocument()->getDvidSynapseEnsemble(
        buddyView()->getSliceAxis());
  if (se != NULL) {
    const std::set<ZIntPoint>& selectedSet =
        se->getSelector().getSelectedSet();
    if (selectedSet.size() == 1) {
      const ZIntPoint &pt = *(selectedSet.begin());
      ZDvidSynapse synapse = se->getSynapse(
            pt, ZDvidSynapseEnsemble::EDataScope::LOCAL);
      if (synapse.isValid()) {
        ZStackBall *ball = getActiveObject<ZStackBall>(ROLE_SYNAPSE);
        ball->setColor(synapse.getColor());
        ball->setRadius(synapse.getRadius());
        ball->setCenter(currentPos);
        /*
        ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_SYNAPSE);
        stroke->setColor(synapse.getColor());
        stroke->setWidth(synapse.getRadius() * 2.0);
        stroke->set(pt.getX(), pt.getY());

        ZPoint pos = currentStackPos;
//        pos.shiftSliceAxis(buddyView()->getSliceAxis());
        stroke->set(pos.x(), pos.y());

        stroke->append(pos.x(), pos.y());
        */

        return true;
      }
    }
  }

  return false;
#endif
}
#endif

#if 0
void ZFlyEmProofPresenter::updateActiveObjectForSynapseAdd()
{
  const ZMouseEvent& event = m_mouseEventProcessor.getLatestMouseEvent();
  updateActiveObjectForSynapseAdd(event.getDataPosition());
//  ZPoint currentStackPos = event.getPosition(neutu::ECoordinateSystem::STACK);
//  updateActiveObjectForSynapseAdd(currentStackPos);
}

void ZFlyEmProofPresenter::updateActiveObjectForSynapseAdd(
    const ZPoint &currentDataPos)
{
  ZStackBall *ball = getActiveObject<ZStackBall>(ROLE_SYNAPSE);
  if (ball) {
    ball->setCenter(currentDataPos);
    ZDvidSynapse::EKind kind  = ZDvidSynapse::EKind::KIND_UNKNOWN;
    switch (interactiveContext().synapseEditMode()) {
    case ZInteractiveContext::SYNAPSE_ADD_PRE:
      kind = ZDvidSynapse::EKind::KIND_PRE_SYN;
      break;
    case ZInteractiveContext::SYNAPSE_ADD_POST:
      kind = ZDvidSynapse::EKind::KIND_POST_SYN;
      break;
    default:
      break;
    }
    QColor color = ZDvidSynapse::GetDefaultColor(kind);
    color.setAlpha(200);
    ball->setColor(color);
    ball->setRadius(ZDvidSynapse::GetDefaultRadius(kind));
  }
#if 0
  ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_SYNAPSE);

  ZPoint pos = currentDataPos;
//  pos.shiftSliceAxis(buddyView()->getSliceAxis());
#ifdef _DEBUG_2
  std::cout << "Update stroke: " << this << " " << pos.x() << ", " << pos.y()
            << std::endl;
#endif

  stroke->set(pos.x(), pos.y());

  ZDvidSynapse::EKind kind  = ZDvidSynapse::EKind::KIND_UNKNOWN;
  switch (interactiveContext().synapseEditMode()) {
  case ZInteractiveContext::SYNAPSE_ADD_PRE:
    kind = ZDvidSynapse::EKind::KIND_PRE_SYN;
    break;
  case ZInteractiveContext::SYNAPSE_ADD_POST:
    kind = ZDvidSynapse::EKind::KIND_POST_SYN;
    break;
  default:
    break;
  }
  QColor color = ZDvidSynapse::GetDefaultColor(kind);
  color.setAlpha(200);
  stroke->setColor(color);
  stroke->setWidth(ZDvidSynapse::GetDefaultRadius(kind) * 2.0);
#endif
}

/*
void ZFlyEmProofPresenter::createBodyContextMenu()
{
  if (m_bodyContextMenu == NULL) {
//    ZStackDocMenuFactory menuFactory;
//    menuFactory.setAdminState(NeuTube::IsAdminUser());
    m_bodyContextMenu =
        getMenuFactory()->makeBodyContextMenu(this, getParentWidget(), NULL);
  }
}
*/
#endif
