#include "zflyemroutinechecktask.h"

#include "zflyemproofdoc.h"

ZFlyEmRoutineCheckTask::ZFlyEmRoutineCheckTask(QObject *parent) : ZTask(parent)
{
}

void ZFlyEmRoutineCheckTask::execute()
{
  if (m_doc != NULL) {
    m_doc->runRoutineCheck();
  }
}

void ZFlyEmRoutineCheckTask::setDoc(ZFlyEmProofDoc *doc)
{
  m_doc = doc;
}
