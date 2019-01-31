#include "zdvidlabelslicehighrestask.h"
//#include "zflyemproofmvc.h"
#include "logging/zqslog.h"
#include "zflyemproofdoc.h"
#include "dvid/zdvidlabelslice.h"

ZDvidLabelSliceHighresTask::ZDvidLabelSliceHighresTask(QObject *parent) :
  ZDvidDataSliceTask(parent)
{
}

void ZDvidLabelSliceHighresTask::execute()
{
  ZFlyEmProofDoc *doc = qobject_cast<ZFlyEmProofDoc*>(m_doc);
  if (doc != NULL) {

    ZDvidLabelSlice *slice = doc->getDvidLabelSlice(m_viewParam.getSliceAxis());
    if (slice != NULL) {
      if (slice->containedIn(
            m_viewParam, m_zoom, m_centerCutWidth, m_centerCutHeight,
            m_usingCenterCut)) {
        doc->prepareDvidLabelSlice(
              m_viewParam, m_zoom, m_centerCutWidth, m_centerCutHeight,
              m_usingCenterCut);
//        LDEBUG() << "Task executed in thread: " << QThread::currentThreadId();
      } else {
//        LDEBUG() << "Task ignored in thread: " << QThread::currentThreadId();
      }
    }
  }
}
