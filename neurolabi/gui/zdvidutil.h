#ifndef ZDVIDUTIL_H
#define ZDVIDUTIL_H

#include "dvid/libdvidheader.h"
#include "zsharedpointer.h"

class ZJsonValue;
class ZDvidTarget;
class ZJsonObject;

namespace ZDvid {
#if defined(_ENABLE_LIBDVIDCPP_)
libdvid::BinaryDataPtr MakeRequest(
    const std::string &address, const std::string &uuid, const std::string &method,
    libdvid::BinaryDataPtr payload, libdvid::ConnectionType type,
    int &statusCode);

libdvid::BinaryDataPtr MakeRequest(
    const std::string &url, const std::string &method,
    libdvid::BinaryDataPtr payload, libdvid::ConnectionType type,
    int &statusCode);

libdvid::BinaryDataPtr MakeRequest(
    libdvid::DVIDConnection &connection,
    const std::string &endpoint, const std::string &method,
    libdvid::BinaryDataPtr payload, libdvid::ConnectionType type,
    int &statusCode);


libdvid::BinaryDataPtr MakeGetRequest(const std::string &url, int &statusCode);
ZSharedPointer<libdvid::DVIDNodeService> MakeDvidNodeService(
    const std::string &web_addr, const std::string &uuid);
ZSharedPointer<libdvid::DVIDNodeService> MakeDvidNodeService(
    const ZDvidTarget &target);
ZSharedPointer<libdvid::DVIDNodeService> MakeDvidNodeService(
    const libdvid::DVIDNodeService *service);

ZSharedPointer<libdvid::DVIDConnection> MakeDvidConnection(
    const std::string &address);

#if defined(_ENABLE_LOWTIS_)
ZSharedPointer<lowtis::ImageService> MakeLowtisService(const ZDvidTarget &target);
lowtis::ImageService* MakeLowtisServicePtr(const ZDvidTarget &target);
#endif

libdvid::BinaryDataPtr MakePayload(const char *payload, int length);
libdvid::BinaryDataPtr MakePayload(const std::string &payload);
libdvid::BinaryDataPtr MakePayload(const ZJsonValue &payload);



/*
libdvid::BinaryDataPtr Post(
    const std::string &url, const char *payload, int length, bool isJson,
    int &statusCode);
libdvid::BinaryDataPtr Post(
    const std::string &url, const std::string &payload, bool isJson);
*/

//Functions for extracing DVID info
ZJsonObject GetDag(const ZJsonObject &obj);
ZJsonObject GetDataInstances(const ZJsonObject &obj);
ZJsonObject GetDataInstances(const std::string &uuid);
ZJsonObject GetDataInstances(const std::string &type);

#endif
}

#endif // ZDVIDUTIL_H
