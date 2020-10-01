#include "zstackdockeyprocessor.h"
#include <QKeyEvent>

#include "zinteractivecontext.h"

ZStackDocKeyProcessor::ZStackDocKeyProcessor(QObject *parent) : QObject(parent)
{

}

bool ZStackDocKeyProcessor::processKeyEvent(QKeyEvent */*event*/)
{
  return false;
}

bool ZStackDocKeyProcessor::processKeyEvent(
    QKeyEvent *event, const ZInteractiveContext &context)
{
  bool processed = false;
  m_operator.clear();
  m_operator.setShift(event->modifiers() == Qt::ShiftModifier);

  switch (event->key()) {
  case Qt::Key_R:
    if (event->modifiers() == Qt::NoModifier) {
      m_operator.setOperation(ZStackOperator::OP_START_PAINT_STROKE);
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
  default:
    break;
  }

  m_operator.setViewId(context.getViewId());

  return processed;
}
