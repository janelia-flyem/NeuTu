#include "zdvidgrayslicehighrestask.h"
#include "zflyemproofdoc.h"
#include "dvid/zdvidgrayslice.h"

ZDvidGraySliceHighresTask::ZDvidGraySliceHighresTask(QObject *parent) :
  ZDvidDataSliceTask(parent)
{
}

void ZDvidGraySliceHighresTask::execute()
{
  ZFlyEmProofDoc *doc = qobject_cast<ZFlyEmProofDoc*>(m_doc);
  if (doc != NULL) {

    ZDvidGraySlice *slice = doc->getDvidGraySlice(m_viewParam.getSliceAxis());
    if (slice != NULL) {
      if (slice->containedIn(
            m_viewParam, m_zoom, m_centerCutWidth, m_centerCutHeight, m_usingCenterCut)) {
        doc->prepareDvidGraySlice(
              m_viewParam, m_zoom, m_centerCutWidth, m_centerCutHeight,
              m_usingCenterCut, slice->getSource());
      }
    }
  }
}
