#include "zdvidrequest.h"

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
