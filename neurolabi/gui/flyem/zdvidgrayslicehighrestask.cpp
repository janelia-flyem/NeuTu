#include "zdvidgrayslicehighrestask.h"
#include "common/debug.h"
#include "zflyemproofdoc.h"
#include "dvid/zdvidgrayslice.h"

ZDvidGraySliceHighresTask::ZDvidGraySliceHighresTask(QObject *parent) :
  ZDvidDataSliceTask(parent)
{
}

void ZDvidGraySliceHighresTask::execute()
{
  ZFlyEmProofDoc *doc = qobject_cast<ZFlyEmProofDoc*>(m_doc);
  if (doc) {
    ZDvidGraySlice *slice = doc->getDvidGraySlice();
    if (slice) {
#ifdef _DEBUG_0
      std::cout << OUTPUT_HIGHTLIGHT_1 << "Checking containment for highres grayscale"
                << std::endl;
#endif
      using namespace std::placeholders;
      if (slice->highResUpdateNeeded(getViewId())) {
        slice->processHighResParam(
              getViewId(),
              std::bind(&ZFlyEmProofDoc::prepareDvidGraySlice, doc,
                        _1, _2, _3, _4, _5, slice->getSource()));
      }
      /*
      if (slice->containedIn(
            m_viewParam, m_zoom,
            m_centerCutWidth, m_centerCutHeight, m_usingCenterCut)) {
        doc->prepareDvidGraySlice(
              m_viewParam, m_zoom, m_centerCutWidth, m_centerCutHeight,
              m_usingCenterCut, slice->getSource());
      }
      */
    }
  }
}
