#include "zdvidlabelslicehighrestask.h"

#include <iostream>

//#include "zflyemproofmvc.h"
#include "common/debug.h"

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
  if (doc) {
    ZDvidLabelSlice *slice = doc->getDvidLabelSlice(m_supervoxel);
    if (slice != NULL) {
#ifdef _DEBUG_0
      std::cout << OUTPUT_HIGHTLIGHT_1 << "Checking containment for highres label"
                << std::endl;
#endif
      using namespace std::placeholders;
      if (slice->highResUpdateNeeded(getViewId())) {
        slice->processHighResParam(
              m_viewParam.getViewId(),
              std::bind(&ZFlyEmProofDoc::prepareDvidLabelSlice, doc,
                        _1, _2, _3, _4, _5, _6, slice->getSource()));
      }
#if 0
      if (slice->needsUpdate(
            m_viewParam, m_zoom, m_centerCutWidth, m_centerCutHeight,
            m_usingCenterCut)) {
#ifdef _DEBUG_
        std::cout << OUTPUT_HIGHTLIGHT << "Preparing highres label"
                << std::endl;
#endif
        doc->prepareDvidLabelSlice(
              m_viewParam, m_zoom, m_centerCutWidth, m_centerCutHeight,
              m_usingCenterCut, m_supervoxel);
//        LDEBUG() << "Task executed in thread: " << QThread::currentThreadId();
      } else {
//        LDEBUG() << "Task ignored in thread: " << QThread::currentThreadId();
      }
#endif
    }
  }
}
