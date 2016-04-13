#include "zneutuservice.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidtarget.h"

ZNeutuService::ZNeutuService(const std::string &server)
{
  m_server = server;
}

std::string ZNeutuService::getBodyUpdateUrl() const
{
  return m_server + "/update_body";
}

void ZNeutuService::requestBodyUpdate(const ZDvidTarget &target, uint64_t bodyId)
{
  std::vector<uint64_t> bodyIdArray;
  bodyIdArray.push_back(bodyId);

  requestBodyUpdate(target, bodyIdArray);
}

void ZNeutuService::requestBodyUpdate(
    const ZDvidTarget &target, const std::vector<uint64_t> &bodyIdArray)
{
  if (!m_server.empty()) {
    if (target.isValid() && !bodyIdArray.empty()) {
      ZJsonObject obj;
      obj.setEntry("dvid-server", target.getAddressWithPort());
      obj.setEntry("uuid", target.getUuid());
      obj.setEntry("labelvol", target.getBodyLabelName());

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
