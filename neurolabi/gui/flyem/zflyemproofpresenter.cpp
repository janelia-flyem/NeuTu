#include "zflyemproofpresenter.h"

#include <QKeyEvent>
#include <QAction>

ZFlyEmProofPresenter::ZFlyEmProofPresenter(ZStackFrame *parent) :
  ZStackPresenter(parent), m_isHightlightMode(false)
{
  interactiveContext().setSwcEditMode(ZInteractiveContext::SWC_EDIT_OFF);
}

ZFlyEmProofPresenter::ZFlyEmProofPresenter(QWidget *parent) :
  ZStackPresenter(parent), m_isHightlightMode(false)
{
  interactiveContext().setSwcEditMode(ZInteractiveContext::SWC_EDIT_OFF);
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
  default:
    break;
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
  setSplitEnabled(true);
}

void ZFlyEmProofPresenter::disableSplit()
{
  setSplitEnabled(false);
}

void ZFlyEmProofPresenter::setSplitEnabled(bool s)
{
  m_paintStrokeAction->setEnabled(s);
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
  default:
    break;
  }
}
