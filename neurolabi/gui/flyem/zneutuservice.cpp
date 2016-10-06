#include "zneutuservice.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidtarget.h"
#include "flyem/zflyemmisc.h"
#include "zstring.h"
#include "zdvidutil.h"

ZNeutuService::ZNeutuService(const std::string &server)
{
  setServer(server);
}

void ZNeutuService::setServer(const std::string &server)
{
  if (server == ":") {
    m_server = "";
  } else {
    m_server = server;
  }
  if (!m_server.empty()) {
    if (!ZString(server).startsWith("http://", ZString::CASE_INSENSITIVE)) {
      m_server = "http://" + m_server;
    }
  }
  updateStatus();
}

std::string ZNeutuService::getBodyUpdateUrl() const
{
  return m_server + "/update_body";
}

std::string ZNeutuService::getHomeUrl() const
{
  return m_server + "/home";
}

ZNeutuService::ERequestStatus ZNeutuService::requestBodyUpdate(
    const ZDvidTarget &target, uint64_t bodyId, EUpdateOption option)
{
  if (isNormal()) {
    std::vector<uint64_t> bodyIdArray;
    bodyIdArray.push_back(bodyId);

    return requestBodyUpdate(target, bodyIdArray, option);
  }

  return REQUEST_IGNORED;
}

ZNeutuService::ERequestStatus ZNeutuService::requestBodyUpdate(
    const ZDvidTarget &target, const std::vector<uint64_t> &bodyIdArray,
    EUpdateOption option)
{
  ERequestStatus status = REQUEST_IGNORED;

  if (!m_server.empty() && isNormal()) {
    if (target.isValid() && !bodyIdArray.empty()) {
      ZJsonObject obj;
      obj.setEntry("dvid-server", target.getAddressWithPort());
      obj.setEntry("uuid", target.getUuid());
      obj.setEntry("labelvol", target.getBodyLabelName());
      switch (option) {
      case UPDATE_ALL:
        obj.setEntry("option", "update");
        break;
      case UPDATE_DELETE:
        obj.setEntry("option", "delete");
        break;
      case UPDATE_INVALIDATE:
        obj.setEntry("option", "invalidate");
        break;
      }

      ZJsonArray bodyJson;
      for (std::vector<uint64_t>::const_iterator
           iter = bodyIdArray.begin(); iter != bodyIdArray.end(); ++iter) {
        bodyJson.append(*iter);
      }
      obj.setEntry("bodies", bodyJson);

      ZDvidWriter writer;
      writer.post(getBodyUpdateUrl(), obj);

      if (writer.getStatusCode() != 200) {
        status = REQUEST_FAILED;
//        LWARN() << "Computing service failed: " << m_server;
      } else {
        status = REQUEST_SUCC;
      }
    }
  }

  return status;
}

void ZNeutuService::updateStatus()
{
  m_status = STATUS_DOWN;
#if defined(_FLYEM_)
  if (!m_server.empty()) {
    int statusCode;
    if (ZDvid::MakeGetRequest(getHomeUrl(), statusCode)) {
      if (statusCode == 200) {
        m_status = STATUS_NORMAL;
      }
    }
  }
#endif
}

bool ZNeutuService::isNormal() const
{
  return getStatus() == STATUS_NORMAL;
}
