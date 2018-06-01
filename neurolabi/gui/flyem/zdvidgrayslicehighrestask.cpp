#include "zdvidgrayslicehighrestask.h"
#include "zflyemproofdoc.h"
#include "dvid/zdvidgrayslice.h"

ZDvidGraySliceHighresTask::ZDvidGraySliceHighresTask(QObject *parent) : ZTask(parent)
{

}

void ZDvidGraySliceHighresTask::setViewParam(const ZStackViewParam &param)
{
  m_viewParam = param;
}

void ZDvidGraySliceHighresTask::setZoom(int zoom)
{
  m_zoom = zoom;
}

void ZDvidGraySliceHighresTask::setCenterCut(int width, int height)
{
  m_centerCutWidth = width;
  m_centerCutHeight = height;
}

void ZDvidGraySliceHighresTask::setDoc(ZStackDoc *doc)
{
  m_doc = doc;
}

void ZDvidGraySliceHighresTask::useCenterCut(bool on)
{
  m_usingCenterCut = on;
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
              m_usingCenterCut);
      }
    }
  }
}
