#include "zactionactivator.h"
#include "zstackdoc.h"
#include "zstackframe.h"

ZActionActivator::ZActionActivator()
{
}

void ZActionActivator::update(const ZStackFrame *frame)
{
  bool positive = isPositive(frame);

  foreach (QAction *action, m_postiveActionList) {
    action->setEnabled(positive);
  }

  foreach (QAction *action, m_negativeActionList) {
    action->setEnabled(!positive);
  }
}

void ZActionActivator::registerAction(QAction *action, bool positive)
{
  if (positive) {
    m_postiveActionList.insert(action);
  } else {
    m_negativeActionList.insert(action);
  }
}

/////////////////////////////////////
bool ZStackActionActivator::isPositive(const ZStackFrame *frame) const
{
  if (frame == NULL) {
    return false;
  }

  if (frame->document() == NULL) {
    return false;
  }

  return frame->document()->hasStackData();
}
