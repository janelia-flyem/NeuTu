#include "zdvidrequest.h"
#include <QVector>

ZDvidRequest::ZDvidRequest()
{
}

void ZDvidRequest::setGetObjectRequest(int bodyId)
{
  m_requestType = ZDvidRequest::DVID_GET_OBJECT;
  m_parameter = bodyId;
}

void ZDvidRequest::setGetSwcRequest(int bodyId)
{
  m_requestType = ZDvidRequest::DVID_GET_SWC;
  m_parameter = bodyId;
}

void ZDvidRequest::setGetImageRequest(int x0, int y0, int z0, int width, int height)
{
  m_requestType = ZDvidRequest::DVID_GET_GRAY_SCALE;
  QList<QVariant> parameter;
  parameter << x0 << y0 << z0 << width << height;
  m_parameter = parameter;
}
