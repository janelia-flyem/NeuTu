#include "zdvidlabelslicehighrestask.h"
#include "zflyemproofmvc.h"
#include "zflyemproofdoc.h"
#include "dvid/zdvidlabelslice.h"
#include "dvid/zdviddataslicehelper.h"

ZDvidLabelSliceHighresTask::ZDvidLabelSliceHighresTask(QObject *parent) : QObject(parent)
{
}

void ZDvidLabelSliceHighresTask::execute()
{
  if (m_mvc != NULL) {
    ZDvidLabelSlice *slice =
        m_mvc->getCompleteDocument()->getDvidLabelSlice(m_viewParam.getSliceAxis());
    if (slice != NULL) {
      if (slice->getHelper()->containedIn(
            m_viewParam, m_zoom, m_centerCutWidth, m_centerCutHeight)) {
        m_mvc->getCompleteDocument()->prepareDvidLabelSlice(
              m_viewParam, m_zoom, m_centerCutWidth, m_centerCutHeight);
      }
    }
  }
}
