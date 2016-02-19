#include "zflyemproofpresenter.h"

#include <QKeyEvent>
#include <QAction>

#include "zkeyoperationconfig.h"
#include "zinteractivecontext.h"
#include "zstackdoc.h"
#include "zflyembookmark.h"
#include "zstackview.h"
#include "zflyemproofdoc.h"
#include "zkeyoperationconfig.h"
#include "flyem/zflyemkeyoperationconfig.h"
#include "flyem/zflyemproofdocmenufactory.h"
#include "dvid/zdvidsynapseensenmble.h"
#include "zinteractionevent.h"

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

void ZFlyEmProofPresenter::init()
{
  m_isHightlightMode = false;
  m_splitWindowMode = false;
  m_highTileContrast = false;

  m_synapseContextMenu = NULL;

  interactiveContext().setSwcEditMode(ZInteractiveContext::SWC_EDIT_OFF);

  connect(getAction(ZActionFactory::ACTION_SYNAPSE_DELETE), SIGNAL(triggered()),
          this, SLOT(deleteSelectedSynapse()));
  connect(getAction(ZActionFactory::ACTION_SYNAPSE_ADD_PRE), SIGNAL(triggered()),
          this, SLOT(tryAddPreSynapseMode()));
  connect(getAction(ZActionFactory::ACTION_SYNAPSE_ADD_POST), SIGNAL(triggered()),
          this, SLOT(tryAddPostSynapseMode()));
  connect(getAction(ZActionFactory::ACTION_SYNAPSE_MOVE), SIGNAL(triggered()),
          this, SLOT(tryMoveSynapseMode()));
  connect(getAction(ZActionFactory::ACTION_SYNAPSE_LINK), SIGNAL(triggered()),
          this, SLOT(linkSelectedSynapse()));
  connect(getAction(ZActionFactory::ACTION_SYNAPSE_UNLINK), SIGNAL(triggered()),
          this, SLOT(unlinkSelectedSynapse()));

//  ZKeyOperationConfig::ConfigureFlyEmStackMap(m_stackKeyOperationMap);
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
  if (m_menuFactory == NULL) {
    m_menuFactory = new ZFlyEmProofDocMenuFactory;
    m_menuFactory->setAdminState(NeuTube::IsAdminUser());
  }

  return m_menuFactory;
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
}

bool ZFlyEmProofPresenter::customKeyProcess(QKeyEvent *event)
{
  bool processed = false;

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
      emit deselectingAllBody();
    }
    break;
  case Qt::Key_M:
    emit mergingBody();
    break;
  case Qt::Key_B:
    emit goingToBodyBottom();
    break;
  case Qt::Key_T:
    emit goingToBodyTop();
    break;
  default:
    break;
  }

  ZStackOperator op;
  op.setOperation(m_bookmarkKeyOperationMap.getOperation(
                    event->key(), event->modifiers()));
  if (!op.isNull()) {
    process(op);
    processed = true;
  }

  return processed;
}

bool ZFlyEmProofPresenter::processKeyPressEvent(QKeyEvent *event)
{
  bool processed = false;

  switch (event->key()) {
  case Qt::Key_Space:
    if (event->modifiers() == Qt::ShiftModifier) {
      emit runningSplit();
      processed = true;
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

void ZFlyEmProofPresenter::deleteSelectedSynapse()
{
  getCompleteDocument()->executeRemoveSynapseCommand();
//  getCompleteDocument()->deleteSelectedSynapse();
}

void ZFlyEmProofPresenter::linkSelectedSynapse()
{
  getCompleteDocument()->executeLinkSynapseCommand();
}

void ZFlyEmProofPresenter::unlinkSelectedSynapse()
{
  getCompleteDocument()->executeUnlinkSynapseCommand();
}

void ZFlyEmProofPresenter::tryAddPreSynapseMode()
{
  tryAddSynapseMode(ZDvidSynapse::KIND_PRE_SYN);
}

void ZFlyEmProofPresenter::tryAddPostSynapseMode()
{
  tryAddSynapseMode(ZDvidSynapse::KIND_POST_SYN);
}

void ZFlyEmProofPresenter::tryAddSynapseMode(ZDvidSynapse::EKind kind)
{
  turnOnActiveObject(ROLE_SYNAPSE, false);
  switch (kind) {
  case ZDvidSynapse::KIND_PRE_SYN:
    m_interactiveContext.setSynapseEditMode(
          ZInteractiveContext::SYNAPSE_ADD_PRE);
    break;
  case ZDvidSynapse::KIND_POST_SYN:
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
  m_contextMenu = getMenuFactory()->makeContextMenu(this, NULL, m_contextMenu);

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
  m_isHightlightMode = hl;
}

void ZFlyEmProofPresenter::toggleHighlightMode()
{
  setHighlightMode(!isHighlight());
}

bool ZFlyEmProofPresenter::isSplitOn() const
{
  return getAction(ZActionFactory::ACTION_PAINT_STROKE)->isEnabled();
}

void ZFlyEmProofPresenter::enableSplit()
{
  setSplitWindow(true);
  setSplitEnabled(true);
}

void ZFlyEmProofPresenter::disableSplit()
{
  setSplitWindow(false);
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

void ZFlyEmProofPresenter::tryAddSynapse(const ZIntPoint &pt)
{
  switch (interactiveContext().synapseEditMode()) {
  case ZInteractiveContext::SYNAPSE_ADD_PRE:
    tryAddSynapse(pt, ZDvidSynapse::KIND_PRE_SYN);
    break;
  case ZInteractiveContext::SYNAPSE_ADD_POST:
    tryAddSynapse(pt, ZDvidSynapse::KIND_POST_SYN);
    break;
  default:
    break;
  }
}

void ZFlyEmProofPresenter::tryAddSynapse(
    const ZIntPoint &pt, ZDvidSynapse::EKind kind)
{
  ZDvidSynapse synapse;
  synapse.setPosition(pt);
  synapse.setKind(kind);
  synapse.setDefaultRadius();
  synapse.setDefaultColor();
  synapse.setUserName(NeuTube::GetCurrentUserName());
  getCompleteDocument()->executeAddSynapseCommand(synapse);
//  getCompleteDocument()->addSynapse(pt, kind);
}

void ZFlyEmProofPresenter::tryMoveSynapse(const ZIntPoint &pt)
{
  getCompleteDocument()->executeMoveSynapseCommand(pt);
//  getCompleteDocument()->tryMoveSelectedSynapse(pt);
  exitSynapseEdit();
//  m_interactiveContext.setSynapseEditMode(ZInteractiveContext::SYNAPSE_EDIT_OFF);
  updateCursor();
}

void ZFlyEmProofPresenter::tryAddBookmarkMode(double x, double y)
{
  interactiveContext().setBookmarkEditMode(ZInteractiveContext::BOOKMARK_ADD);

  ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_BOOKMARK);

//  stroke->setWidth(10.0);

  buddyDocument()->mapToDataCoord(&x, &y, NULL);
  stroke->set(x, y);
//  m_stroke.setEraser(false);
//  m_stroke.setFilled(false);
//  m_stroke.setTarget(ZStackObject::TARGET_WIDGET);
//  turnOnStroke();
  turnOnActiveObject(ROLE_BOOKMARK);
  //buddyView()->paintActiveDecoration();
  updateCursor();
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

    ZFlyEmBookmark *bookmark = new ZFlyEmBookmark;
    ZIntPoint pos(x, y, buddyView()->getZ(NeuTube::COORD_STACK));
    pos.shiftSliceAxisInverse(getSliceAxis());
    bookmark->setLocation(pos);
    bookmark->setRadius(radius);
    bookmark->setCustom(true);
    bookmark->setUser(NeuTube::GetCurrentUserName().c_str());
    bookmark->addUserTag();
    ZFlyEmProofDoc *doc = qobject_cast<ZFlyEmProofDoc*>(buddyDocument());
    if (doc != NULL) {
      bookmark->setBodyId(doc->getBodyId(bookmark->getLocation()));
    }

    getCompleteDocument()->executeAddBookmarkCommand(bookmark);
//    buddyDocument()->executeAddObjectCommand(bookmark);

//    emit bookmarkAdded(bookmark);
  }
}

void ZFlyEmProofPresenter::processCustomOperator(
    const ZStackOperator &op, ZInteractionEvent *e)
{
  const ZMouseEvent& event = m_mouseEventProcessor.getLatestMouseEvent();
  ZPoint currentStackPos = event.getPosition(NeuTube::COORD_STACK);

  switch (op.getOperation()) {
  case ZStackOperator::OP_CUSTOM_MOUSE_RELEASE:
    if (isHighlight()) {
      const ZMouseEvent& event = m_mouseEventProcessor.getLatestMouseEvent();
      ZPoint currentStackPos = event.getPosition(NeuTube::COORD_STACK);
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
  case ZStackOperator::OP_BOOKMARK_ADD_NEW:
    addActiveStrokeAsBookmark();
    break;
  case ZStackOperator::OP_BOOKMARK_ANNOTATE:
    emit annotatingBookmark(op.getHitObject<ZFlyEmBookmark>());
    break;
  case ZStackOperator::OP_OBJECT_SELECT_IN_ROI:
    emit selectingBodyInRoi(true);
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_SELECT_SINGLE:
  {
    QList<ZDvidSynapseEnsemble*> seList =
        getCompleteDocument()->getDvidSynapseEnsembleList();
    ZIntPoint hitPoint = op.getHitObject()->getHitPoint();

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
    tryAddSynapse(currentStackPos.toIntPoint());
    break;
  case ZStackOperator::OP_DVID_SYNAPSE_MOVE:
    tryMoveSynapse(currentStackPos.toIntPoint());
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
      stroke->setLast(currentStackPos.x(), currentStackPos.y());
      if (e != NULL) {
        e->setEvent(
              ZInteractionEvent::EVENT_ACTIVE_DECORATION_UPDATED);
      }
    }
    break;
  default:
    break;
  }

  getAction(ZActionFactory::ACTION_BODY_SPLIT_START)->setVisible(
        !isSplitWindow());
  getAction(ZActionFactory::ACTION_BODY_DECOMPOSE)->setVisible(
        isSplitWindow());
}

bool ZFlyEmProofPresenter::highTileContrast() const
{
  return m_highTileContrast;
}

void ZFlyEmProofPresenter::setHighTileContrast(bool high)
{
  m_highTileContrast = high;
}

void ZFlyEmProofPresenter::processRectRoiUpdate(ZRect2d *rect, bool appending)
{
  if (isSplitOn()) {
    ZFlyEmProofDoc *doc = qobject_cast<ZFlyEmProofDoc*>(buddyDocument());
    if (doc != NULL) {
      doc->updateSplitRoi(rect, appending);
    }
  } else {
    buddyDocument()->processRectRoiUpdate(rect, appending);
  }
}

bool ZFlyEmProofPresenter::updateActiveObjectForSynapseMove()
{
  const ZMouseEvent& event = m_mouseEventProcessor.getLatestMouseEvent();
  ZPoint currentStackPos = event.getPosition(NeuTube::COORD_STACK);
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
      const ZDvidSynapse &synapse = se->getSynapse(
            pt, ZDvidSynapseEnsemble::DATA_LOCAL);
      if (synapse.isValid()) {
        ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_SYNAPSE);
        stroke->setColor(synapse.getColor());
        stroke->setWidth(synapse.getRadius() * 2.0);
        stroke->set(pt.getX(), pt.getY());
        stroke->append(currentStackPos.x(), currentStackPos.y());

        return true;
      }
    }
  }

  return false;
}

void ZFlyEmProofPresenter::updateActiveObjectForSynapseAdd()
{
  const ZMouseEvent& event = m_mouseEventProcessor.getLatestMouseEvent();
  ZPoint currentStackPos = event.getPosition(NeuTube::COORD_STACK);
  updateActiveObjectForSynapseAdd(currentStackPos);
}

void ZFlyEmProofPresenter::updateActiveObjectForSynapseAdd(
    const ZPoint &currentStackPos)
{
  ZStroke2d *stroke = getActiveObject<ZStroke2d>(ROLE_SYNAPSE);
  stroke->set(currentStackPos.x(), currentStackPos.y());

  ZDvidSynapse::EKind kind  = ZDvidSynapse::KIND_UNKNOWN;
  switch (interactiveContext().synapseEditMode()) {
  case ZInteractiveContext::SYNAPSE_ADD_PRE:
    kind = ZDvidSynapse::KIND_PRE_SYN;
    break;
  case ZInteractiveContext::SYNAPSE_ADD_POST:
    kind = ZDvidSynapse::KIND_POST_SYN;
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
