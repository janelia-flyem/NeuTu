#include "zneutuservice.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidtarget.h"
#include "flyem/zflyemmisc.h"
#include "neutubeconfig.h"
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

#if defined(_ENABLE_LIBDVIDCPP_)
    m_connection = ZSharedPointer<libdvid::DVIDConnection>(
          new libdvid::DVIDConnection(m_server, GET_FLYEM_CONFIG.getUserName(),
                                      NeutubeConfig::GetSoftwareName()));
#endif
  }

  updateStatus();
}

std::string ZNeutuService::getBodyUpdateUrl() const
{
  return m_server + GetBodyUpdatePath();
}

std::string ZNeutuService::GetBodyUpdatePath()
{
  return "/update_body";
}

std::string ZNeutuService::getHomeUrl() const
{
  return m_server + GetHomePath();
}

std::string ZNeutuService::GetHomePath()
{
  return "/home";
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

template <class InputIterator>
ZNeutuService::ERequestStatus ZNeutuService::requestBodyUpdate(
    const ZDvidTarget &target, const InputIterator &first,
    const InputIterator &last, EUpdateOption option)
{
  ERequestStatus status = REQUEST_IGNORED;

  if (!m_server.empty() && isNormal()) {
    if (target.isValid() && (first != last)) {
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
      case UPDATE_MISSING:
        obj.setEntry("option" ,"add");
        break;
      }

      ZJsonArray bodyJson;
      for (InputIterator iter = first; iter != last; ++iter) {
        bodyJson.append(*iter);
      }
      obj.setEntry("bodies", bodyJson);

      int statusCode;
      QMutexLocker locker(&m_connectionMutex);
      dvid::MakePostRequest(*m_connection, GetBodyUpdatePath(),  obj, statusCode);

      if (statusCode != 200) {
        status = REQUEST_FAILED;
//        LWARN() << "Computing service failed: " << m_server;
      } else {
        status = REQUEST_SUCC;
      }
    }
  }

  return status;
}

ZNeutuService::ERequestStatus ZNeutuService::requestBodyUpdate(
    const ZDvidTarget &target, const std::vector<uint64_t> &bodyIdArray,
    EUpdateOption option)
{
  return
      requestBodyUpdate(target, bodyIdArray.begin(), bodyIdArray.end(), option);
}

ZNeutuService::ERequestStatus ZNeutuService::requestBodyUpdate(
    const ZDvidTarget &target, const std::set<uint64_t> &bodyIdArray,
    EUpdateOption option)
{
  return
      requestBodyUpdate(target, bodyIdArray.begin(), bodyIdArray.end(), option);
}

void ZNeutuService::reset()
{
  m_server.clear();
  m_status = EStatus::DOWN;
  m_connection.reset();
}

void ZNeutuService::updateStatus()
{
  m_status = EStatus::DOWN;
#if defined(_FLYEM_)
  if (!m_server.empty()) {
    int statusCode;
#if defined(_ENABLE_LIBDVIDCPP_)
    if (m_connection) {
      QMutexLocker locker(&m_connectionMutex);
      if (dvid::MakeGetRequest(*m_connection, GetHomePath(), statusCode)) {
        if (statusCode == 200) {
          m_status = EStatus::NORMAL;
        }
      }
    }
#endif
  }
#endif
}

bool ZNeutuService::isNormal() const
{
  return getStatus() == EStatus::NORMAL;
}
