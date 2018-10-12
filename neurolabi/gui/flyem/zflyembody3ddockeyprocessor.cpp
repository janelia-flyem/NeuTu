#include "zflyembody3ddockeyprocessor.h"
#include <QKeyEvent>
#include "zflyembody3ddoc.h"
#include "zinteractivecontext.h"

ZFlyEmBody3dDocKeyProcessor::ZFlyEmBody3dDocKeyProcessor(QObject *parent) :
  ZStackDocKeyProcessor(parent)
{

}

bool ZFlyEmBody3dDocKeyProcessor::processKeyEvent(QKeyEvent *event)
{
  bool processed = false;

  ZFlyEmBody3dDoc *doc = getDocument<ZFlyEmBody3dDoc>();
  if (doc != NULL) {
    switch (event->key()) {
//    case Qt::Key_1:
//      doc->setSeedType(1);
//      processed = true;
//      break;
//    case Qt::Key_2:
//      doc->setSeedType(2);
//      processed = true;
//      break;
    case Qt::Key_Delete:
      if (doc->hasTodoItemSelected() && (doc->isDvidMutable())) {
        doc->executeRemoveTodoCommand();
        processed = true;
      }
      break;
    }
  }

  return processed;
}

bool ZFlyEmBody3dDocKeyProcessor::processKeyEvent(
    QKeyEvent *event, const ZInteractiveContext &context)
{
  bool processed = false;
  m_operator.clear();
  m_operator.setShift(event->modifiers() == Qt::ShiftModifier);

  ZFlyEmBody3dDoc *doc = getDocument<ZFlyEmBody3dDoc>();
  bool allowingMutableAction = true;
  if (doc) {
    allowingMutableAction = doc->isDvidMutable();
  }

  switch (event->key()) {
  case Qt::Key_R:
#ifdef _FLYEM_
    if (event->modifiers() == Qt::NoModifier) {
      if (allowingMutableAction) {
        m_operator.setOperation(ZStackOperator::OP_START_PAINT_STROKE);
        processed = true;
      }
    } else
#endif
    if (event->modifiers() == Qt::ShiftModifier) {
      m_operator.setOperation(ZStackOperator::OP_RECT_ROI_INIT);
      processed = true;
    }
    break;
  case Qt::Key_Space:
    if (allowingMutableAction) {
      if (event->modifiers() == Qt::ShiftModifier) {
        m_operator.setOperation(ZStackOperator::OP_FLYEM_SPLIT_BODY);
      } else if (event->modifiers() & Qt::AltModifier) {
        m_operator.setOperation(ZStackOperator::OP_FLYEM_SPLIT_BODY_FULL);
      } else {
        m_operator.setOperation(ZStackOperator::OP_FLYEM_SPLIT_BODY_LOCAL);
      }
      processed = true;
    }
    break;
  case Qt::Key_Escape:
    m_operator.setOperation(ZStackOperator::OP_EXIT_EDIT_MODE);
    processed = true;
    break;
  case Qt::Key_0:
  case Qt::Key_1:
  case Qt::Key_2:
  case Qt::Key_3:
  case Qt::Key_4:
  case Qt::Key_5:
  case Qt::Key_6:
  case Qt::Key_7:
    if (context.strokeEditMode() == ZInteractiveContext::STROKE_DRAW) {
      m_operator.setOperation(ZStackOperator::OP_ACTIVE_STROKE_CHANGE_LABEL);
      m_operator.setLabel(event->key() - Qt::Key_0);
      processed = true;
    }
    break;
  case Qt::Key_QuoteLeft:
    if (context.strokeEditMode() == ZInteractiveContext::STROKE_DRAW) {
      m_operator.setOperation(ZStackOperator::OP_ACTIVE_STROKE_CHANGE_LABEL);
      m_operator.setLabel(255);
      processed = true;
    }
    break;
  case Qt::Key_S:
    m_operator.setOperation(ZStackOperator::OP_SWC_SELECT_NODE_IN_ROI);
    processed = true;
    break;
  case Qt::Key_T:
#ifdef _FLYEM_
    if (event->modifiers() == Qt::NoModifier) {
      if (allowingMutableAction) {
        m_operator.setOperation(ZStackOperator::OP_FLYEM_TODO_ADD);
        processed = true;
      }
    }
#else
    if (event->modifiers() & Qt::ControlModifier) {
      m_operator.setOperation(
            ZStackOperator::OP_SWC_SELECT_TERMINAL_BRANCH_IN_ROI);
      processed = true;
    } else {
      m_operator.setOperation(ZStackOperator::OP_SWC_SELECT_TREE_IN_ROI);
      processed = true;
    }
#endif
    break;
  case Qt::Key_X:
    if (allowingMutableAction) {
      m_operator.setOperation(ZStackOperator::OP_FLYEM_CROP_BODY);
      processed = true;
    }
    break;
  case Qt::Key_Delete:
  case Qt::Key_Backspace:
//    if (allowingMutableAction) { //Delegate it to later processing
      m_operator.setOperation(ZStackOperator::OP_OBJECT_DELETE_SELECTED);
      processed = true;
//    }
    break;
  default:
    break;
  }

  return processed;
}
