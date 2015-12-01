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

  interactiveContext().setSwcEditMode(ZInteractiveContext::SWC_EDIT_OFF);

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
  return m_paintStrokeAction->isEnabled();
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
  m_paintStrokeAction->setEnabled(s);
}

void ZFlyEmProofPresenter::tryAddBookmarkMode()
{
  exitStrokeEdit();
  QPointF pos = mapFromGlobalToStack(QCursor::pos());
  tryAddBookmarkMode(pos.x(), pos.y());
}

void ZFlyEmProofPresenter::tryAddBookmarkMode(double x, double y)
{
  interactiveContext().setBookmarkEditMode(ZInteractiveContext::BOOKMARK_ADD);
  m_stroke.setWidth(10.0);

  buddyDocument()->mapToDataCoord(&x, &y, NULL);
  m_stroke.set(x, y);
  m_stroke.setEraser(false);
  m_stroke.setFilled(false);
  m_stroke.setTarget(ZStackObject::TARGET_WIDGET);
  turnOnStroke();
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
  m_stroke.getLastPoint(&x, &y);
  double radius = m_stroke.getWidth() / 2.0;

  ZFlyEmBookmark *bookmark = new ZFlyEmBookmark;
  bookmark->setLocation(x, y, buddyView()->getZ(NeuTube::COORD_STACK));
  bookmark->setRadius(radius);
  bookmark->setCustom(true);
  bookmark->setUser(NeuTube::GetCurrentUserName().c_str());
  ZFlyEmProofDoc *doc = qobject_cast<ZFlyEmProofDoc*>(buddyDocument());
  if (doc != NULL) {
    bookmark->setBodyId(doc->getBodyId(bookmark->getLocation()));
  }
  buddyDocument()->executeAddObjectCommand(bookmark);

  emit bookmarkAdded(bookmark);
}

void ZFlyEmProofPresenter::processCustomOperator(const ZStackOperator &op)
{
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
    getCompleteDocument()->getDvidSynapseEnsemble()->selectHit(false);
    break;
  default:
    break;
  }

  getAction(ZStackPresenter::ACTION_BODY_SPLIT_START)->setVisible(
        !isSplitWindow());
  getAction(ZStackPresenter::ACTION_BODY_DECOMPOSE)->setVisible(
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
