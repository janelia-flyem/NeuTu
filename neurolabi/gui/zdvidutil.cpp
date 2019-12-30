#include "zdvidutil.h"

#include <QUrl>
#include <cmath>

#include "neutubeconfig.h"
#include "zjsonvalue.h"
#include "zstring.h"
#include "dvid/zdvidtarget.h"
#include "zjsonparser.h"
#include "dvid/zdvidversiondag.h"
#include "geometry/zintcuboid.h"
#include "dvid/zdvidurl.h"
#include "dvid/zdvidreader.h"

#if defined(_ENABLE_LIBDVIDCPP_)

#include "neutube.h"

/*!
 * Note that libdvid::DVIDConnection automatically add '/api' at the beginning
 * of the path.
 */
libdvid::BinaryDataPtr dvid::MakeRequest(
    libdvid::DVIDConnection &connection,
    const std::string &path, const std::string &method,
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
          "/.." + path, connMethod, payload, results, error_msg, type);
  } catch (libdvid::DVIDException &e) {
    std::cout << e.what() << std::endl;
    statusCode = e.getStatus();
  }

  return results;
}

libdvid::BinaryDataPtr dvid::MakeRequest(
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

    std::string path = qurl.path().toStdString();
    std::string query = qurl.query().toStdString();
    path += (!query.empty()) ? "?" + query : "";
    statusCode =
        connection.make_request("/.." + path, connMethod, payload, results, error_msg, type);
  } catch (libdvid::DVIDException &e)  {
    std::cout << e.what() << std::endl;
    statusCode = e.getStatus();
  }

  return results;
}

libdvid::BinaryDataPtr dvid::MakeGetRequest(
    const std::string &url, int &statusCode)
{
  return MakeRequest(url, "GET", libdvid::BinaryDataPtr(), libdvid::DEFAULT,
                     statusCode);
}

libdvid::BinaryDataPtr dvid::MakeGetRequest(
    libdvid::DVIDConnection &connection, const std::string &path,
    int &statusCode)
{
  return MakeRequest(
        connection, path, "GET", libdvid::BinaryDataPtr(), libdvid::DEFAULT,
        statusCode);
}

libdvid::BinaryDataPtr dvid::MakePostRequest(
    libdvid::DVIDConnection &connection, const std::string &path,
    const ZJsonObject &obj, int &statusCode)
{
  std::string payload = obj.dumpString(0);
  libdvid::BinaryDataPtr libdvidPayload;
  if (!payload.empty()) {
    libdvidPayload =
        libdvid::BinaryData::create_binary_data(payload.c_str(), payload.size());
  }

  return MakeRequest(
        connection, path, "POST", libdvidPayload, libdvid::JSON,
        statusCode);
}


void dvid::MakeHeadRequest(const std::string &url, int &statusCode)
{
  MakeRequest(url, "HEAD", libdvid::BinaryDataPtr(), libdvid::DEFAULT,
              statusCode);
}

bool dvid::HasHead(const std::string &url)
{
  int statusCode = 0;
  MakeHeadRequest(url, statusCode);

  return (statusCode == 200);
}

ZSharedPointer<libdvid::DVIDNodeService> dvid::MakeDvidNodeService(
    const std::string &web_addr, const std::string &uuid)
{
  return ZSharedPointer<libdvid::DVIDNodeService>(
        new libdvid::DVIDNodeService(
          web_addr, uuid, GET_FLYEM_CONFIG.getUserName(),
          NeutubeConfig::GetSoftwareName()));
}

ZSharedPointer<libdvid::DVIDNodeService> dvid::MakeDvidNodeService(
    const ZDvidTarget &target)
{
  return MakeDvidNodeService(target.getAddressWithPort(),
                             target.getUuid());
}

ZSharedPointer<libdvid::DVIDNodeService> dvid::MakeDvidNodeService(
    const libdvid::DVIDNodeService *service)
{
  if (service != NULL) {
    return ZSharedPointer<libdvid::DVIDNodeService>(
          new libdvid::DVIDNodeService(*service));
  }

  return ZSharedPointer<libdvid::DVIDNodeService>();
}

ZSharedPointer<libdvid::DVIDConnection> dvid::MakeDvidConnection(
    const std::string &address, const std::string &user, const std::string &app)
{
  try {
    return ZSharedPointer<libdvid::DVIDConnection>(
          new libdvid::DVIDConnection(address, user, app));
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
    return ZSharedPointer<libdvid::DVIDConnection>();
  }
}

ZSharedPointer<libdvid::DVIDConnection> dvid::MakeDvidConnection(
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

ZSharedPointer<libdvid::DVIDConnection> dvid::MakeDvidConnection(
    const libdvid::DVIDConnection *conn)
{
  if (conn != NULL) {
    return ZSharedPointer<libdvid::DVIDConnection>(
          new libdvid::DVIDConnection(*conn));
  }

  return ZSharedPointer<libdvid::DVIDConnection>();
}


#if defined(_ENABLE_LOWTIS_)
ZSharedPointer<lowtis::ImageService> dvid::MakeLowtisService(const ZDvidTarget &target)
{
  lowtis::DVIDLabelblkConfig config;
  config.username = neutu::GetCurrentUserName();
  config.dvid_server = target.getAddressWithPort();
  config.dvid_uuid = target.getUuid();
  config.datatypename = target.getSegmentationName();


  return ZSharedPointer<lowtis::ImageService>(new lowtis::ImageService(config));
}

lowtis::ImageService* dvid::MakeLowtisServicePtr(const ZDvidTarget &target)
{
  lowtis::DVIDLabelblkConfig config;
  config.username = neutu::GetCurrentUserName();
  config.dvid_server = target.getAddressWithPort();
  config.dvid_uuid = target.getUuid();
  config.datatypename = target.getSegmentationName();
  config.supervoxelview = target.isSupervoxelView();

  return new lowtis::ImageService(config);
}
#endif

libdvid::BinaryDataPtr dvid::MakePayload(const char *payload, int length)
{
  return libdvid::BinaryData::create_binary_data(payload, length);
}

libdvid::BinaryDataPtr dvid::MakePayload(const std::string &payload)
{
  return MakePayload(payload.c_str(), payload.length());
}

libdvid::BinaryDataPtr dvid::MakePayload(const ZJsonValue &payload)
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

#if defined(_ENABLE_LIBDVIDCPP_)
ZIntCuboid dvid::GetAlignedBox(const ZIntCuboid &box, const ZDvidInfo &dvidInfo)
{
  ZIntCuboid alignedBox;
  alignedBox.setFirstCorner(
        dvidInfo.getBlockBox(
          dvidInfo.getBlockIndex(box.getFirstCorner())).getFirstCorner());
  alignedBox.setLastCorner(
        dvidInfo.getBlockBox(
          dvidInfo.getBlockIndex(box.getLastCorner())).getLastCorner());

  return alignedBox;
}
#endif

ZIntCuboid dvid::GetZoomBox(const ZIntCuboid &box, int zoom)
{
  ZIntCuboid zoomBox;

  int zoomRatio = pow(2, zoom);
  zoomBox.setFirstCorner(box.getFirstCorner() / zoomRatio);
  zoomBox.setWidth(box.getWidth() / zoomRatio);
  zoomBox.setHeight(box.getHeight() / zoomRatio);

  return zoomBox;
}

bool dvid::IsDataValid(const std::string &data, const ZDvidTarget &target,
                        const ZJsonObject &infoJson, const ZDvidVersionDag &dag)
{
  bool valid = false;

  const char *instanceKey = "DataInstances";
  if (infoJson.hasKey(instanceKey)) {
    ZJsonObject instanceJson(infoJson.value(instanceKey));
    if (instanceJson.hasKey(data.c_str())) {
      ZJsonObject dataJson(instanceJson.value(data.c_str()));
      if (dataJson.hasKey("Base")) {
        ZJsonObject baseJson(dataJson.value("Base"));
        std::string repoUuid = ZJsonParser::stringValue(baseJson["RepoUUID"]);
#if 1
        if (ZString(repoUuid).startsWith(target.getUuid())) {
          valid = true;
        } else if (dag.isAncester(target.getUuid(), repoUuid)) {
          valid = true;
        }
#endif

#if 0
        if (repoUuid.size() > 4) {
          repoUuid = repoUuid.substr(0, 4);
          if (repoUuid == target.getUuid()) {
            valid = true;
          } else if (dag.isAncester(target.getUuid(), repoUuid)) {
            valid = true;
          }
        }
#endif
      }
    }
  }

  return valid;
}

bool dvid::IsUuidMatched(const std::string &uuid1, const std::string &uuid2)
{
  bool matched = false;
  if (!uuid1.empty() && !uuid2.empty()) {
    if (ZString(uuid1).startsWith(uuid2)){
      matched = true;
    } else if (ZString(uuid2).startsWith(uuid1)) {
      matched = true;
    }
  }

  return matched;
}

struct _DataTypeMap {
  static std::map<std::string, dvid::EDataType> CreateMap()
  {
    std::map<std::string, dvid::EDataType> m;

    m["labelblk"] = dvid::EDataType::LABELBLK;
    m["annotation"] = dvid::EDataType::ANNOTATION;
    m["imagetile"] = dvid::EDataType::IMAGETILE;
    m["keyvalue"] = dvid::EDataType::KEYVALUE;
    m["labelgraph"] = dvid::EDataType::LABELGRAPH;
    m["labelsz"] = dvid::EDataType::LABELSZ;
    m["labelvol"] = dvid::EDataType::LABELVOL;
    m["roi"] = dvid::EDataType::ROI;
    m["uint8blk"] = dvid::EDataType::UINT8BLK;

    return m;
  }

  static const std::map<std::string, dvid::EDataType> M;
};

const std::map<std::string, dvid::EDataType> _DataTypeMap::M =
    _DataTypeMap::CreateMap();

dvid::EDataType dvid::GetDataType(const std::string &typeName)
{
  dvid::EDataType type = dvid::EDataType::UNKNOWN;

  std::map<std::string, dvid::EDataType>::const_iterator iter =
      _DataTypeMap::M.find(typeName);
  if (iter != _DataTypeMap::M.end()) {
    type = iter->second;
  }

  return type;
}

dvid::EDataType dvid::GetDataTypeFromInfo(const ZJsonObject &obj)
{
  dvid::EDataType type = dvid::EDataType::UNKNOWN;

  if (obj.hasKey("Base")) {
    ZJsonObject baseObj(obj.value("Base"));
    std::string typeName = ZJsonParser::stringValue(baseObj["TypeName"]);
    type = GetDataType(typeName);
  }

  return type;
}

bool dvid::IsValidDvidUrl(const std::string &url)
{
  ZDvidTarget target;
  target.setFromUrl(url);

  return target.isValid();
}

ZDvidTarget dvid::MakeTargetFromUrl(const std::string path)
{
  ZDvidTarget target;
  target.setFromUrl(path);
  return target;

#if 0
  QUrl url(path.c_str());

  std::string uuid;

  std::string marker = "api/node/";
  std::string::size_type markerPos = path.find(marker);

  if (markerPos != std::string::npos) {
    markerPos += marker.size();
    std::string::size_type uuidPos = path.find('/', markerPos);
    uuid = path.substr(markerPos, uuidPos - markerPos);
  }


  ZDvidTarget target;
  target.set(url.host().toStdString(), uuid, url.port());

  return target;
#endif
}


std::string dvid::GetBodyIdTag(uint64_t bodyId)
{
  std::ostringstream stream;
  stream << "body:" << bodyId;

  return stream.str();
}

std::pair<uint64_t, std::vector<uint64_t>> dvid::GetMergeConfig(
    const ZDvidReader &reader,
    const std::vector<uint64_t> &bodyIdArray, bool mergingToLargest)
{
  std::vector<uint64_t> merged;
  uint64_t target = 0;
  if (bodyIdArray.size() > 1) {
    target = bodyIdArray[0];

    if (mergingToLargest) {
      int maxSize = 0;
      for (uint64_t bodyId : bodyIdArray) {
        const int bodySize = reader.readBodyBlockCount(
              bodyId, neutu::EBodyLabelType::BODY);

        DEBUG_OUT << bodyId << ": " << bodySize << std::endl;

        if (bodySize > maxSize) {
          maxSize = bodySize;
          target = bodyId;
        }
      }

      for (uint64_t bodyId : bodyIdArray) {
        if (bodyId != target) {
          merged.push_back(bodyId);
        }
      }
    } else {
      merged.insert(merged.begin(), bodyIdArray.begin() + 1, bodyIdArray.end());
    }
  }

  return std::pair<uint64_t, std::vector<uint64_t>>(target, merged);
}


std::pair<uint64_t, std::vector<uint64_t>> dvid::GetMergeConfig(
    const ZDvidReader &reader, uint64_t defaultTargetId,
    const std::vector<uint64_t> &bodyId, bool mergingToLargest)
{
  std::vector<uint64_t> bodyArray;

  if (!bodyId.empty()) {
    bodyArray.push_back(defaultTargetId);
    bodyArray.insert(bodyArray.end(), bodyId.begin(), bodyId.end());
  }

  return GetMergeConfig(reader, bodyArray, mergingToLargest);
}







