#ifndef ZDVIDUTIL_H
#define ZDVIDUTIL_H

#include "dvid/libdvidheader.h"
#include "common/zsharedpointer.h"
#include "dvid/zdviddef.h"

class ZJsonValue;
class ZDvidTarget;
class ZJsonObject;
class ZDvidVersionDag;
class ZDvidInfo;
class ZIntCuboid;
class ZDvidReader;

#define DVID_UUID_COMMON_LENGTH 4

namespace dvid {
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
    const std::string &path, const std::string &method,
    libdvid::BinaryDataPtr payload, libdvid::ConnectionType type,
    int &statusCode);


libdvid::BinaryDataPtr MakeGetRequest(const std::string &url, int &statusCode);
libdvid::BinaryDataPtr MakeGetRequest(
    libdvid::DVIDConnection &connection, const std::string &path,
    int &statusCode);

libdvid::BinaryDataPtr MakePostRequest(
    libdvid::DVIDConnection &connection, const std::string &path,
    const ZJsonObject &obj, int &statusCode);

void MakeHeadRequest(const std::string &url, int &statusCode);

ZSharedPointer<libdvid::DVIDNodeService> MakeDvidNodeService(
    const std::string &web_addr, const std::string &uuid);
ZSharedPointer<libdvid::DVIDNodeService> MakeDvidNodeService(
    const ZDvidTarget &target);
ZSharedPointer<libdvid::DVIDNodeService> MakeDvidNodeService(
    const libdvid::DVIDNodeService *service);

ZSharedPointer<libdvid::DVIDConnection> MakeDvidConnection(
    const std::string &address, const std::string &user, const std::string &app);
ZSharedPointer<libdvid::DVIDConnection> MakeDvidConnection(
    const std::string &address);
ZSharedPointer<libdvid::DVIDConnection> MakeDvidConnection(
    const libdvid::DVIDConnection *conn);

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

dvid::EDataType GetDataTypeFromInfo(const ZJsonObject &obj);
dvid::EDataType GetDataType(const std::string &typeName);

ZDvidTarget MakeTargetFromUrl(const std::string path);

bool IsValidDvidUrl(const std::string &url);

/*!
 * \brief Test if two UUIDs they point to the same DVID node
 *
 * The two UUIDs \a uuid1 and \a uuid2 are mached or considered as the same node
 * iff:
 *   1. Neither \a uuid1 and \a uuid2 are empty
 *   2. Either of the two UUIDs equals to the starting characters of the other
 *
 * \return true if the two UUIDs are matched
 */
bool IsUuidMatched(const std::string &uuid1, const std::string &uuid2);

bool IsDataValid(const std::string &data, const ZDvidTarget &target,
                 const ZJsonObject &infoJson, const ZDvidVersionDag &dag);

std::string GetBodyIdTag(uint64_t bodyId);

ZIntCuboid GetZoomBox(const ZIntCuboid &box, int zoom);
#if defined(_ENABLE_LIBDVIDCPP_)
ZIntCuboid GetAlignedBox(const ZIntCuboid &box, const ZDvidInfo &dvidInfo);
#endif

std::pair<uint64_t, std::vector<uint64_t>> GetMergeConfig(const ZDvidReader &reader, const std::vector<uint64_t> &bodyIdArray,
    bool mergingToLargest);

std::pair<uint64_t, std::vector<uint64_t>> GetMergeConfig(
    const ZDvidReader &reader, uint64_t defaultTargetId,
    const std::vector<uint64_t> &bodyId, bool mergingToLargest);
}

#endif // ZDVIDUTIL_H
