#include "zdvidlabelslicehighrestask.h"
//#include "zflyemproofmvc.h"
#include "QsLog.h"
#include "zflyemproofdoc.h"
#include "dvid/zdvidlabelslice.h"

ZDvidLabelSliceHighresTask::ZDvidLabelSliceHighresTask(QObject *parent) : ZTask(parent)
{
}

void ZDvidLabelSliceHighresTask::execute()
{
  ZFlyEmProofDoc *doc = qobject_cast<ZFlyEmProofDoc*>(m_doc);
  if (doc != NULL) {

    ZDvidLabelSlice *slice = doc->getDvidLabelSlice(m_viewParam.getSliceAxis());
    if (slice != NULL) {
      if (slice->containedIn(
            m_viewParam, m_zoom, m_centerCutWidth, m_centerCutHeight)) {
        doc->prepareDvidLabelSlice(
              m_viewParam, m_zoom, m_centerCutWidth, m_centerCutHeight);
//        LDEBUG() << "Task executed in thread: " << QThread::currentThreadId();
      } else {
//        LDEBUG() << "Task ignored in thread: " << QThread::currentThreadId();
      }
    }
  }
}

void ZDvidLabelSliceHighresTask::setViewParam(const ZStackViewParam &param)
{
  m_viewParam = param;
}

void ZDvidLabelSliceHighresTask::setZoom(int zoom)
{
  m_zoom = zoom;
}

void ZDvidLabelSliceHighresTask::setCenterCut(int width, int height)
{
  m_centerCutWidth = width;
  m_centerCutHeight = height;
}

void ZDvidLabelSliceHighresTask::setDoc(ZStackDoc *doc)
{
  m_doc = doc;
}
