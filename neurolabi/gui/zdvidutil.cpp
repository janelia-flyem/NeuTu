#include "zdvidutil.h"

#include <QUrl>

#include "neutubeconfig.h"
#include "zjsonvalue.h"
#include "zstring.h"
#include "dvid/zdvidtarget.h"

#if defined(_ENABLE_LIBDVIDCPP_)

#include "neutube.h"

libdvid::BinaryDataPtr ZDvid::MakeRequest(
    libdvid::DVIDConnection &connection,
    const std::string &endpoint, const std::string &method,
    libdvid::BinaryDataPtr payload, libdvid::ConnectionType type,
    int &statusCode)
{
  libdvid::ConnectionMethod connMethod = libdvid::GET;
  if (method == "HEAD") {
    connMethod = libdvid::HEAD;
  } else if (method == "POST") {
    connMethod = libdvid::POST;
  } else if (method == "PUT") {
    connMethod = libdvid::PUT;
  } else if (method == "DELETE") {
    connMethod = libdvid::DELETE;
  } else if (method == "GET") {
    connMethod = libdvid::GET;
  }

  libdvid::BinaryDataPtr results = libdvid::BinaryData::create_binary_data();
  try {
    std::string error_msg;

    //  qDebug() << "address: " << address;
    //  qDebug() << "path: " << qurl.path();

    statusCode = connection.make_request(
          endpoint, connMethod, payload, results, error_msg, type);
  } catch (libdvid::DVIDException &e) {
    std::cout << e.what() << std::endl;
    statusCode = e.getStatus();
  }

  return results;
}

libdvid::BinaryDataPtr ZDvid::MakeRequest(
    const std::string &url, const std::string &method,
    libdvid::BinaryDataPtr payload, libdvid::ConnectionType type,
    int &statusCode)
{
  libdvid::ConnectionMethod connMethod = libdvid::GET;
  if (method == "HEAD") {
    connMethod = libdvid::HEAD;
  } else if (method == "POST") {
    connMethod = libdvid::POST;
  } else if (method == "PUT") {
    connMethod = libdvid::PUT;
  } else if (method == "DELETE") {
    connMethod = libdvid::DELETE;
  } else if (method == "GET") {
    connMethod = libdvid::GET;
  }

  QUrl qurl(url.c_str());
//  qurl.setScheme("http");
  ZString address = qurl.host();
  if (qurl.port() >= 0) {
    address += ":";
    address.appendNumber(qurl.port());
  }

  libdvid::BinaryDataPtr results = libdvid::BinaryData::create_binary_data();
  try {
    libdvid::DVIDConnection connection(address, GET_FLYEM_CONFIG.getUserName(),
                                       NeutubeConfig::GetSoftwareName());


    std::string error_msg;

    //  qDebug() << "address: " << address;
    //  qDebug() << "path: " << qurl.path();

    statusCode = connection.make_request(
          "/.." + qurl.path().toStdString(), connMethod, payload, results,
          error_msg, type);
  } catch (libdvid::DVIDException &e) {
    std::cout << e.what() << std::endl;
    statusCode = e.getStatus();
  }

  return results;
}

libdvid::BinaryDataPtr ZDvid::MakeGetRequest(
    const std::string &url, int &statusCode)
{
  return MakeRequest(url, "GET", libdvid::BinaryDataPtr(), libdvid::DEFAULT,
                     statusCode);
}

ZSharedPointer<libdvid::DVIDNodeService> ZDvid::MakeDvidNodeService(
    const std::string &web_addr, const std::string &uuid)
{
  return ZSharedPointer<libdvid::DVIDNodeService>(
        new libdvid::DVIDNodeService(
          web_addr, uuid, GET_FLYEM_CONFIG.getUserName(),
          NeutubeConfig::GetSoftwareName()));
}

ZSharedPointer<libdvid::DVIDNodeService> ZDvid::MakeDvidNodeService(
    const ZDvidTarget &target)
{
  return MakeDvidNodeService(target.getAddressWithPort(),
                             target.getUuid());
}

ZSharedPointer<libdvid::DVIDNodeService> ZDvid::MakeDvidNodeService(
    const libdvid::DVIDNodeService *service)
{
  if (service != NULL) {
    return ZSharedPointer<libdvid::DVIDNodeService>(
          new libdvid::DVIDNodeService(*service));
  }

  return ZSharedPointer<libdvid::DVIDNodeService>();
}

ZSharedPointer<libdvid::DVIDConnection> ZDvid::MakeDvidConnection(
    const std::string &address)
{
  try {
    return ZSharedPointer<libdvid::DVIDConnection>(
          new libdvid::DVIDConnection(
            address, GET_FLYEM_CONFIG.getUserName(),
            NeutubeConfig::GetSoftwareName()));
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
    return ZSharedPointer<libdvid::DVIDConnection>();
  }
}

#if defined(_ENABLE_LOWTIS_)
ZSharedPointer<lowtis::ImageService> ZDvid::MakeLowtisService(const ZDvidTarget &target)
{
  lowtis::DVIDLabelblkConfig config;
  config.username = NeuTube::GetCurrentUserName();
  config.dvid_server = target.getAddressWithPort();
  config.dvid_uuid = target.getUuid();
  config.datatypename = target.getLabelBlockName();


  return ZSharedPointer<lowtis::ImageService>(new lowtis::ImageService(config));
}

lowtis::ImageService* ZDvid::MakeLowtisServicePtr(const ZDvidTarget &target)
{
  lowtis::DVIDLabelblkConfig config;
  config.username = NeuTube::GetCurrentUserName();
  config.dvid_server = target.getAddressWithPort();
  config.dvid_uuid = target.getUuid();
  config.datatypename = target.getLabelBlockName();


  return new lowtis::ImageService(config);
}
#endif

libdvid::BinaryDataPtr ZDvid::MakePayload(const char *payload, int length)
{
  return libdvid::BinaryData::create_binary_data(payload, length);
}

libdvid::BinaryDataPtr ZDvid::MakePayload(const std::string &payload)
{
  return MakePayload(payload.c_str(), payload.length());
}

libdvid::BinaryDataPtr ZDvid::MakePayload(const ZJsonValue &payload)
{
  return MakePayload(payload.dumpString(0));
}

#if 0
libdvid::BinaryDataPtr ZDvid::Post(
    const std::string &url, const char *payload, int length, bool isJson,
    int &statusCode)
{
  LINFO() << "HTTP " + method + ": " + url;

  statusCode = 0;
#if defined(_ENABLE_LIBDVIDCPP_)
  try {
    libdvid::BinaryDataPtr libdvidPayload;
    if (payload != NULL && length > 0) {
      libdvidPayload =
          libdvid::BinaryData::create_binary_data(payload, length);
    }

    libdvid::ConnectionMethod connMethod = libdvid::POST;
    if (method == "POST") {
      connMethod = libdvid::POST;
    } else if (method == "PUT") {
      connMethod = libdvid::PUT;
    } else if (method == "DELETE") {
      connMethod = libdvid::DELETE;
    }

    libdvid::BinaryDataPtr data;
    bool requested = false;
    libdvid::DVIDNodeService *service =
    if (m_service != NULL) {
      std::string endPoint = ZDvidUrl::GetEndPoint(url);

      if (!endPoint.empty()) {
        //    std::cout << libdvidPayload->get_data().size() << std::endl;
        requested = true;
        data = m_service->custom_request(endPoint, libdvidPayload, connMethod);

        m_statusCode = 200;
      }
    }

    if (!requested) {
      libdvid::ConnectionType type = libdvid::BINARY;
      if (isJson) {
        type = libdvid::JSON;
      }
      data = makeRequest(url, method, libdvidPayload, type, m_statusCode);
    }
    response = data->get_data();
  } catch (libdvid::DVIDException &e) {
    std::cout << e.what() << std::endl;
    LWARN() << "HTTP " + method + " exception (" << e.getStatus() << "): " << e.what();
    m_statusCode = e.getStatus();
  }
#endif

#ifdef _DEBUG_
  std::cout << response << std::endl;
#endif

  return response;
}
#endif
#endif
