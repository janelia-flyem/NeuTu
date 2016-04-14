#include "zneutuservice.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidtarget.h"
#include "flyem/zflyemmisc.h"

ZNeutuService::ZNeutuService(const std::string &server)
{
  setServer(server);
}

void ZNeutuService::setServer(const std::string &server)
{
   m_server = server;
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

void ZNeutuService::requestBodyUpdate(
    const ZDvidTarget &target, uint64_t bodyId, EUpdateOption option)
{
  if (isNormal()) {
    std::vector<uint64_t> bodyIdArray;
    bodyIdArray.push_back(bodyId);

    requestBodyUpdate(target, bodyIdArray, option);
  }
}

void ZNeutuService::requestBodyUpdate(
    const ZDvidTarget &target, const std::vector<uint64_t> &bodyIdArray,
    EUpdateOption option)
{
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
    }
  }
}

void ZNeutuService::updateStatus()
{
  m_status = STATUS_DOWN;

  if (!m_server.empty()) {
    int statusCode;
    if (ZFlyEmMisc::MakeGetRequest(getHomeUrl(), statusCode)) {
      if (statusCode == 200) {
        m_status = STATUS_NORMAL;
      }
    }
  }
}

bool ZNeutuService::isNormal() const
{
  return getStatus() == STATUS_NORMAL;
}
