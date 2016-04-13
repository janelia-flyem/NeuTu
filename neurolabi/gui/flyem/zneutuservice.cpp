#include "zneutuservice.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidtarget.h"

ZNeutuService::ZNeutuService(const std::string &server)
{
  m_server = server;
  m_status = STATUS_NORMAL;
}

std::string ZNeutuService::getBodyUpdateUrl() const
{
  return m_server + "/update_body";
}

void ZNeutuService::requestBodyUpdate(
    const ZDvidTarget &target, uint64_t bodyId, EUpdateOption option)
{
  std::vector<uint64_t> bodyIdArray;
  bodyIdArray.push_back(bodyId);

  requestBodyUpdate(target, bodyIdArray, option);
}

void ZNeutuService::requestBodyUpdate(
    const ZDvidTarget &target, const std::vector<uint64_t> &bodyIdArray,
    EUpdateOption option)
{
  if (!m_server.empty()) {
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


}
