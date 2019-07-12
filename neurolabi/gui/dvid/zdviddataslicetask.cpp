#include "zdviddataslicetask.h"

ZDvidDataSliceTask::ZDvidDataSliceTask(QObject *parent) : ZTask(parent)
{

}

void ZDvidDataSliceTask::setViewParam(const ZStackViewParam &param)
{
  m_viewParam = param;
}

void ZDvidDataSliceTask::setZoom(int zoom)
{
  m_zoom = zoom;
}

void ZDvidDataSliceTask::setCenterCut(int width, int height)
{
  m_centerCutWidth = width;
  m_centerCutHeight = height;
}

void ZDvidDataSliceTask::setDoc(ZStackDoc *doc)
{
  m_doc = doc;
}

void ZDvidDataSliceTask::useCenterCut(bool on)
{
  m_usingCenterCut = on;
}

void ZDvidDataSliceTask::setSupervoxel(bool on)
{
  m_supervoxel = on;
}

void ZDvidDataSliceTask::setHandle(const std::string &handle)
{
  m_handle = handle;
}
