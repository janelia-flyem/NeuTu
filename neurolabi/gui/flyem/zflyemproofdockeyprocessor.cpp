#include "zflyemproofdockeyprocessor.h"

#include <QKeyEvent>

#include "zflyemproofdoc.h"
#include "zinteractivecontext.h"

ZFlyEmProofDocKeyProcessor::ZFlyEmProofDocKeyProcessor(QObject *parent) :
  ZStackDocKeyProcessor(parent)
{

}

bool ZFlyEmProofDocKeyProcessor::processKeyEvent(
    QKeyEvent *event, const ZInteractiveContext &context)
{
  bool processed = false;
  m_operator.clear();
  m_operator.setShift(event->modifiers() == Qt::ShiftModifier);

  ZFlyEmProofDoc *doc = getDocument<ZFlyEmProofDoc>();
  if (doc) {
    switch (event->key()) {
    case Qt::Key_T:
      if (event->modifiers() == Qt::NoModifier) {
        if (context.strokeEditMode() == ZInteractiveContext::STROKE_DRAW) {
          m_operator.setOperation(ZStackOperator::OP_DVID_SYNAPSE_START_TBAR);
          processed = true;
        } else if (context.synapseEditMode() == ZInteractiveContext::SYNAPSE_ADD_PRE) {
          m_operator.setOperation(ZStackOperator::OP_DVID_SYNAPSE_START_PSD);
          processed = true;
        } else if (context.synapseEditMode() == ZInteractiveContext::SYNAPSE_ADD_POST) {
          m_operator.setOperation(ZStackOperator::OP_DVID_SYNAPSE_START_TBAR);
          processed = true;
        }
      }
      break;
    default:
      break;
    }
  }

  if (!processed) {
    processed = ZStackDocKeyProcessor::processKeyEvent(event, context);
  }

  return processed;
}
