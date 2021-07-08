#include "zflyemproofpresenter.h"

#include <QKeyEvent>
#include <QAction>
#include <QColorDialog>

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
#include "neutubeconfig.h"
#include "dvid/zdvidlabelslice.h"
#include "zflyemtododelegate.h"
#include "zflyemproofdocutil.h"
#include "flyemdatareader.h"

#include "neuroglancer/zneuroglancerpathfactory.h"
#include "neuroglancer/zneuroglancerlayerspecfactory.h"
#include "neuroglancer/zneuroglancerannotationlayerspec.h"


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
  getCompleteDocument()->updateDvidLabelSlice(buddyView()->getSliceAxis());
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
  getCompleteDocument()->selectBodyInRoi(buddyView()->getCurrentZ(), true, true);
}

void ZFlyEmProofPresenter::zoomInRectRoi()
{ 
  ZRect2d rect = buddyDocument()->getRect2dRoi();

  if (rect.isValid()) {
    buddyView()->setViewPort(QRect(rect.getMinX(), rect.getMinY(),
                               rect.getWidth(), rect.getHeight()));
    buddyDocument()->executeRemoveRectRoiCommand();
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
    processed = process(op);
  }

  return processed;
}

bool ZFlyEmProofPresenter::processKeyPressEvent(QKeyEvent *event)
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
    processed = ZStackPresenter::processKeyPressEvent(event);
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
    turnOnActiveObject(ROLE_SYNAPSE, false);
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
    updateActiveObjectForSynapseAdd();
    buddyView()->paintActiveDecoration();

    updateCursor();
  } else {
    getCompleteDocument()->emitWarning(
          "Cannot add a synapse "
          "because either no synapse data is specified or it is readonly.");
  }
}

void ZFlyEmProofPresenter::tryMoveSynapseMode()
{
  if (updateActiveObjectForSynapseMove()) {
    turnOnActiveObject(ROLE_SYNAPSE);
    m_interactiveContext.setSynapseEditMode(ZInteractiveContext::SYNAPSE_MOVE);
    updateCursor();
  }
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
  exitStrokeEdit();
  QPointF pos = mapFromGlobalToStack(QCursor::pos());
  tryAddBookmarkMode(pos.x(), pos.y());
}

void ZFlyEmProofPresenter::tryTodoItemMode()
{
  exitStrokeEdit();
  QPointF pos = mapFromGlobalToStack(QCursor::pos());
  tryAddTodoItemMode(pos.x(), pos.y());
}

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
  tryAddToMergeItem(pt.toIntPoint());
}

void ZFlyEmProofPresenter::tryAddToSplitItem()
{
  const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
        Qt::RightButton, ZMouseEvent::EAction::RELEASE);
  ZPoint pt = event.getDataPosition();
  tryAddToSplitItem(pt.toIntPoint());
}

void ZFlyEmProofPresenter::tryAddToSupervoxelSplitItem()
{
  const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
        Qt::RightButton, ZMouseEvent::EAction::RELEASE);
  ZPoint pt = event.getDataPosition();
  tryAddToSupervoxelSplitItem(pt.toIntPoint());
}

void ZFlyEmProofPresenter::tryAddTraceToSomaItem()
{
  const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
        Qt::RightButton, ZMouseEvent::EAction::RELEASE);
  ZPoint pt = event.getDataPosition();
  tryAddTraceToSomaItem(pt.toIntPoint());
}

void ZFlyEmProofPresenter::tryAddNoSomaItem()
{
  const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
        Qt::RightButton, ZMouseEvent::EAction::RELEASE);
  ZPoint pt = event.getDataPosition();
  tryAddNoSomaItem(pt.toIntPoint());
}

void ZFlyEmProofPresenter::tryAddDiagnosticItem()
{
  tryAddDiagnosticItem(
        getLastMouseReleasePosition(Qt::RightButton).toIntPoint());
}

void ZFlyEmProofPresenter::tryAddSegmentationDiagnosticItem()
{
  tryAddSegmentationDiagnosticItem(
        getLastMouseReleasePosition(Qt::RightButton).toIntPoint());
}

void ZFlyEmProofPresenter::tryAddDoneItem()
{
  const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
        Qt::RightButton, ZMouseEvent::EAction::RELEASE);
  ZPoint pt = event.getDataPosition();
  tryAddDoneItem(pt.toIntPoint());
}

void ZFlyEmProofPresenter::tryMoveSynapse(const ZIntPoint &pt)
{
  getCompleteDocument()->executeMoveSynapseCommand(pt);
//  getCompleteDocument()->tryMoveSelectedSynapse(pt);
  exitSynapseEdit();
//  m_interactiveContext.setSynapseEditMode(ZInteractiveContext::SYNAPSE_EDIT_OFF);
  updateCursor();
}

void ZFlyEmProofPresenter::tryAddTodoItemMode(double x, double y)
{
  interactiveContext().setTodoEditMode(ZInteractiveContext::TODO_ADD_ITEM);
  ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_TODO_ITEM);

  buddyDocument()->mapToDataCoord(&x, &y, NULL);
  stroke->set(x, y);

  turnOnActiveObject(ROLE_TODO_ITEM);
  updateCursor();
}

void ZFlyEmProofPresenter::tryAddBookmarkMode(double x, double y)
{
  exitEdit();

  interactiveContext().setBookmarkEditMode(ZInteractiveContext::BOOKMARK_ADD);

  ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_BOOKMARK);

//  stroke->setWidth(10.0);

  buddyDocument()->mapToDataCoord(&x, &y, NULL);
  stroke->set(x, y);
//  m_stroke.setEraser(false);
//  m_stroke.setFilled(false);
//  m_stroke.setTarget(ZStackObject::ETarget::TARGET_WIDGET);
//  turnOnStroke();
  turnOnActiveObject(ROLE_BOOKMARK);
  //buddyView()->paintActiveDecoration();
  updateCursor();
}

void ZFlyEmProofPresenter::runTipDetection() {
    const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
          Qt::RightButton, ZMouseEvent::EAction::RELEASE);
    ZPoint pt = event.getDataPosition();
    uint64_t bodyId = getCompleteDocument()->getLabelId(pt.getX(), pt.getY(), pt.getZ());

    emit tipDetectRequested(pt.toIntPoint(), bodyId);
}

ZFlyEmProofDoc* ZFlyEmProofPresenter::getCompleteDocument() const
{
  return qobject_cast<ZFlyEmProofDoc*>(buddyDocument());
}

void ZFlyEmProofPresenter::addActiveStrokeAsBookmark()
{
  int x = 0;
  int y = 0;

  ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_BOOKMARK);
  if (stroke != NULL) {
    stroke->getLastPoint(&x, &y);
    double radius = stroke->getWidth() / 2.0;
    ZIntPoint pos(x, y, buddyView()->getZ(neutu::ECoordinateSystem::STACK));

    if (getCompleteDocument()->canAddBookmarkAt(pos, true)) {
      ZFlyEmBookmark *bookmark = new ZFlyEmBookmark;
      pos.shiftSliceAxisInverse(getSliceAxis());
      bookmark->setLocation(pos);
      bookmark->setRadius(radius);
      bookmark->setCustom(true);
      bookmark->setUser(neutu::GetCurrentUserName());
      bookmark->addUserTag();
      ZFlyEmProofDoc *doc = qobject_cast<ZFlyEmProofDoc*>(buddyDocument());
      if (doc != NULL) {
        bookmark->setBodyId(doc->getBodyId(bookmark->getLocation()));
      }

      bookmark->updateTimestamp();

      getCompleteDocument()->executeAddBookmarkCommand(bookmark);
    }
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

void ZFlyEmProofPresenter::toggleSupervoxelView(bool on)
{
  getCompleteDocument()->setSupervoxelMode(on, buddyView()->getViewParameter());
}

void ZFlyEmProofPresenter::takeScreenshot()
{
  buddyView()->takeScreenshot();
}

bool ZFlyEmProofPresenter::processCustomOperator(
    const ZStackOperator &op, ZInteractionEvent *e)
{
  const ZMouseEvent& event = m_mouseEventProcessor.getLatestMouseEvent();
  ZPoint currentStackPos = event.getPosition(neutu::ECoordinateSystem::STACK);
  ZPoint currentDataPos = event.getDataPosition();

  bool processed = true;

  switch (op.getOperation()) {
  case ZStackOperator::OP_CUSTOM_MOUSE_RELEASE:
    if (isHighlight()) {
      const ZMouseEvent& event = m_mouseEventProcessor.getLatestMouseEvent();
      ZPoint currentStackPos = event.getPosition(neutu::ECoordinateSystem::STACK);
      ZIntPoint pos = currentStackPos.toIntPoint();
      emit selectingBodyAt(pos.getX(), pos.getY(), pos.getZ());
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
    tryTodoItemMode();
    break;
  case ZStackOperator::OP_BOOKMARK_ADD_NEW:
    addActiveStrokeAsBookmark();
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
    emit selectingBodyInRoi(true);
    break;
  case ZStackOperator::OP_FLYEM_TODO_SELECT_SINGLE:
  {
    ZOUT(LTRACE(), 5) << "Get todo list selection";
    QList<ZFlyEmToDoList*> todoList =
        getCompleteDocument()->getObjectList<ZFlyEmToDoList>();
    ZIntPoint hitPoint = op.getHitObject()->getHitPoint();

    ZStackDocSelector docSelector(getSharedBuddyDocument());
    docSelector.setSelectOption(ZStackObject::EType::DVID_SYNAPE_ENSEMBLE,
                                ZStackDocSelector::SELECT_RECURSIVE);
    docSelector.deselectAll();
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

    ZStackDocSelector docSelector(getSharedBuddyDocument());
    docSelector.setSelectOption(ZStackObject::EType::FLYEM_TODO_LIST,
                                ZStackDocSelector::SELECT_RECURSIVE);
    docSelector.deselectAll();

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
    getCompleteDocument()->getDvidSynapseEnsemble(
          buddyView()->getSliceAxis())->toggleHitSelectWithPartner();
    if (e != NULL) {
      e->setEvent(ZInteractionEvent::EVENT_OBJECT_SELECTED);
    }
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_ADD:
    tryAddSynapse(currentDataPos.toIntPoint(), true);
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_ADD_ORPHAN:
    tryAddSynapse(currentDataPos.toIntPoint(), false);
    break;
  case ZStackOperator::OP_FLYEM_TODO_ADD:
    tryAddTodoItem(currentDataPos.toIntPoint());
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_MOVE:
    tryMoveSynapse(currentDataPos.toIntPoint());
    break;
  case ZStackOperator::OP_TRACK_MOUSE_MOVE:
    if (m_interactiveContext.synapseEditMode() !=
        ZInteractiveContext::SYNAPSE_EDIT_OFF) {
      ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_SYNAPSE);
      if (m_interactiveContext.synapseEditMode() ==
          ZInteractiveContext::SYNAPSE_ADD_PRE ||
          m_interactiveContext.synapseEditMode() ==
                    ZInteractiveContext::SYNAPSE_ADD_POST) {
        updateActiveObjectForSynapseAdd(currentStackPos);
      } else if (m_interactiveContext.synapseEditMode() ==
                 ZInteractiveContext::SYNAPSE_MOVE) {
        updateActiveObjectForSynapseMove(currentStackPos);
      }
      ZPoint pos = currentStackPos;
//      pos.shiftSliceAxis(buddyView()->getSliceAxis());
      stroke->setLast(pos.x(), pos.y());
      if (e != NULL) {
        e->setEvent(
              ZInteractionEvent::EVENT_ACTIVE_DECORATION_UPDATED);
      }
    }
    break;
  case ZStackOperator::OP_DVID_LABEL_SLICE_SELECT_SINGLE:
  {
    buddyDocument()->deselectAllObject(false);
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
    break;
  case ZStackOperator::OP_DVID_LABEL_SLICE_TOGGLE_SELECT:
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
    break;
  case ZStackOperator::OP_DVID_LABEL_SLICE_TOGGLE_SELECT_SINGLE:
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
    break;
  case ZStackOperator::OP_DVID_LABEL_SLICE_SELECT_MULTIPLE:
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
    getCompleteDocument()->toggleGrayscale(
          buddyView()->getSliceAxis());
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
  ZJsonObject obj;
  obj.decode(option.toStdString(), true);

  ZJsonObjectParser parser;
  if (parser.GetValue(obj, "type", "") == "neuroglancer") {
    const ZMouseEvent &event = m_mouseEventProcessor.getMouseEvent(
          Qt::RightButton, ZMouseEvent::EAction::RELEASE);
    ZPoint pt = event.getDataPosition();

    if (parser.GetValue(obj, "location", "") == "rectroi") {
      ZRect2d rect = buddyDocument()->getRect2dRoi();
      pt.set(rect.getCenter().toPoint());
    }

//    ZDvidTarget target = getCompleteDocument()->getDvidTarget();

    ZDvidInfo dvidInfo = getCompleteDocument()->getDvidInfo();
    ZResolution res = dvidInfo.getVoxelResolution();


//    QList<ZFlyEmBookmark*> bookmarkList =
//        ZFlyEmProofDocUtil::GetUserBookmarkList(getCompleteDocument());

    QList<std::shared_ptr<ZNeuroglancerLayerSpec>> additionalLayers;
    ZRect2d rect = buddyDocument()->getRect2dRoi();
    if (rect.isValid()) {
      auto layer = ZNeuroglancerLayerSpecFactory::MakeLocalAnnotationLayer(
            "local_annotation");
      layer->addAnnotation(rect.getBoundBox());
      additionalLayers.append(layer);
    }

    QString path = ZNeuroglancerPathFactory::MakePath(
          getCompleteDocument()->getDvidEnv(), res,
          pt, buddyView()->getViewParameter().getZoomRatio(),
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
  if (isSplitOn()) {
    ZFlyEmProofDoc *doc = qobject_cast<ZFlyEmProofDoc*>(buddyDocument());
    if (doc != NULL) {
      doc->updateSplitRoi(rect, appending);
    }
  } else {
    if (interactiveContext().rectSpan()) {
      rect->setZSpan((rect->getWidth() + rect->getHeight()) / 4);
      rect->setPenetrating(false);
    }
    buddyDocument()->processRectRoiUpdate(rect, appending);
    interactiveContext().setAcceptingRect(true);
    QMenu *menu = getContextMenu();
    if (!menu->isEmpty()) {
      const ZMouseEvent& event = m_mouseEventProcessor.getMouseEvent(
            Qt::LeftButton, ZMouseEvent::EAction::RELEASE);
      QPoint currentWidgetPos(event.getWidgetPosition().getX(),
                              event.getWidgetPosition().getY());
      buddyView()->showContextMenu(menu, currentWidgetPos);
    }
    interactiveContext().setAcceptingRect(false);
  }
}

bool ZFlyEmProofPresenter::updateActiveObjectForSynapseMove()
{
  const ZMouseEvent& event = m_mouseEventProcessor.getLatestMouseEvent();
  ZPoint currentStackPos = event.getPosition(neutu::ECoordinateSystem::STACK);
  return updateActiveObjectForSynapseMove(currentStackPos);
}

bool ZFlyEmProofPresenter::updateActiveObjectForSynapseMove(
    const ZPoint &currentStackPos)
{
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
        ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_SYNAPSE);
        stroke->setColor(synapse.getColor());
        stroke->setWidth(synapse.getRadius() * 2.0);
        stroke->set(pt.getX(), pt.getY());

        ZPoint pos = currentStackPos;
//        pos.shiftSliceAxis(buddyView()->getSliceAxis());
        stroke->set(pos.x(), pos.y());

        stroke->append(pos.x(), pos.y());

        return true;
      }
    }
  }

  return false;
}

void ZFlyEmProofPresenter::updateActiveObjectForSynapseAdd()
{
  const ZMouseEvent& event = m_mouseEventProcessor.getLatestMouseEvent();
  ZPoint currentStackPos = event.getPosition(neutu::ECoordinateSystem::STACK);
  updateActiveObjectForSynapseAdd(currentStackPos);
}

void ZFlyEmProofPresenter::updateActiveObjectForSynapseAdd(
    const ZPoint &currentStackPos)
{
  ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_SYNAPSE);

  ZPoint pos = currentStackPos;
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
