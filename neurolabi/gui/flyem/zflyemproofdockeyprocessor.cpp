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
  }

  if (!processed) {
    processed = ZStackDocKeyProcessor::processKeyEvent(event, context);
  }

  return processed;
}
