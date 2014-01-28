#ifndef ZDVIDREQUEST_H
#define ZDVIDREQUEST_H

#include <QVariant>

class ZDvidRequest
{
public:
  ZDvidRequest();

  enum EDvidRequest {
    DVID_GET_OBJECT, DVID_SAVE_OBJECT, DVID_UPLOAD_SWC, DVID_GET_SWC,
    DVID_NULL_REQUEST
  };

  void setGetSwcRequest(int bodyId);
  void setGetObjectRequest(int bodyId);

  inline EDvidRequest getType() const { return m_requestType; }
  inline const QVariant& getParameter() const { return m_parameter; }

private:
  EDvidRequest m_requestType;
  QVariant m_parameter;
};

#endif // ZDVIDREQUEST_H
