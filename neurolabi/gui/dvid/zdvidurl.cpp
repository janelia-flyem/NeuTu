#include "zdvidurl.h"
#include <sstream>
#include <iostream>

#include "dvid/zdviddata.h"
#if defined(_QT_GUI_USED_)
#include "neutube.h"
#else
#include "core/neutube_def.h"
#endif
#include "neutubeconfig.h"
#include "zstring.h"
#include "zintpoint.h"
#include "zintcuboid.h"
#if defined(_QT_GUI_USED_)
#include "zdvidutil.h"
#endif

const std::string ZDvidUrl::m_keyCommand = "key";
const std::string ZDvidUrl::m_keysCommand = "keys";
const std::string ZDvidUrl::m_keyRangeCommand = "keyrange";
const std::string ZDvidUrl::m_keyValuesCommand = "keyvalues";
const std::string ZDvidUrl::m_sparsevolCommand = "sparsevol";
const std::string ZDvidUrl::m_coarseSparsevolCommand = "sparsevol-coarse";
//const std::string ZDvidUrl::m_supervoxelCommand = "sparsevol-supervoxel";
const std::string ZDvidUrl::m_supervoxelCommand = "sparsevol";
const std::string ZDvidUrl::m_coarseSupervoxelCommand = "sparsevol-coarse"; //Temporary mockup
const std::string ZDvidUrl::m_infoCommand = "info";
const std::string ZDvidUrl::m_splitCommand = "split";
const std::string ZDvidUrl::m_coarseSplitCommand = "split-coarse";
//const std::string ZDvidUrl::m_splitSupervoxelCommand= "split"; //Temporary mockup
const std::string ZDvidUrl::m_splitSupervoxelCommand = "split-supervoxel";
const std::string ZDvidUrl::m_labelCommand = "label";
const std::string ZDvidUrl::m_labelArrayCommand = "labels";
const std::string ZDvidUrl::m_roiCommand = "roi";
const std::string ZDvidUrl::m_annotationElementCommand = "element";
const std::string ZDvidUrl::m_annotationElementsCommand = "elements";
const std::string ZDvidUrl::m_annotationLabelCommand = "label";
const std::string ZDvidUrl::m_annotationMoveCommand = "move";
const std::string ZDvidUrl::m_annotationTagCommand = "tag";
const std::string ZDvidUrl::m_labelMappingCommand = "mapping";
const std::string ZDvidUrl::m_tarfileCommand = "tarfile";
const std::string ZDvidUrl::SUPERVOXEL_FLAG = "supervoxels";

ZDvidUrl::ZDvidUrl()
{
}

ZDvidUrl::ZDvidUrl(
    const std::string &serverAddress, const std::string &uuid, int port)
{
  m_dvidTarget.set(serverAddress, uuid, port);
}

ZDvidUrl::ZDvidUrl(const ZDvidTarget &target)
{
  m_dvidTarget = target;
}

ZDvidUrl::ZDvidUrl(const ZDvidTarget &target, const std::string &uuid)
{
  setDvidTarget(target, uuid);
}

void ZDvidUrl::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
}

void ZDvidUrl::setDvidTarget(const ZDvidTarget &target, const std::string &uuid)
{
  m_dvidTarget = target;
  m_dvidTarget.setUuid(uuid);
}

std::string ZDvidUrl::GetFullUrl(
    const std::string &prefix, const std::string &path)
{
  std::string url;

  if (!prefix.empty() && !path.empty()) {
    if (path[0] == '/' || path[0] == '?') {
      url = prefix + path;
    } else {
      url = prefix + "/" + path;
    }
  }

  return url;
}

std::string ZDvidUrl::getApiLoadUrl() const
{
  return GetFullUrl(getApiUrl(), "load");
}

std::string ZDvidUrl::getNodeUrl() const
{
  return m_dvidTarget.getUrl();
}

std::string ZDvidUrl::getDataUrl(const std::string &dataName) const
{
  return GetFullUrl(m_dvidTarget.getUrl(), dataName);
}

std::string ZDvidUrl::getDataUrl(ZDvidData::ERole role) const
{
  return getDataUrl(ZDvidData::GetName(role));
}

std::string ZDvidUrl::getDataUrl(
    ZDvidData::ERole role, ZDvidData::ERole prefixRole,
    const std::string &prefixName)
{
  return getDataUrl(ZDvidData::GetName(role, prefixRole, prefixName));
}

std::string ZDvidUrl::getInfoUrl(const std::string &dataName) const
{
  return GetFullUrl(getDataUrl(dataName), m_infoCommand);
}

std::string ZDvidUrl::getApiUrl() const
{
  if (!m_dvidTarget.getAddress().empty()) {
    return  "http://" + m_dvidTarget.getAddressWithPort() + "/api";
  }

  return "";
}

std::string ZDvidUrl::getHelpUrl() const
{
  return GetFullUrl(getApiUrl(), "help");
}

std::string ZDvidUrl::getServerInfoUrl() const
{
  return GetFullUrl(getApiUrl(), "server/info");
}

namespace {

std::string AppendQuery(const std::string &url, const std::string query)
{
  std::string newUrl = url;

  if (!url.empty() && !query.empty()) {
    bool hasQuery = false;
    for (std::string::const_reverse_iterator rit=url.rbegin(); rit!=url.rend(); ++rit) {
      if (*rit == '/') {
        break;
      } else if (*rit == '?') {
        hasQuery = true;
        break;
      }
    }

    if (hasQuery) {
      newUrl += "&";
    } else {
      newUrl += "?";
    }
    newUrl += query;
  }

  return newUrl;
}


template<typename T>
std::string AppendQuery(
      const std::string &url, const std::pair<std::string,T> &query)
{
  if (!query.first.empty()) {
    std::string qstr = query.first + "=" + std::to_string(query.second);
    return AppendQuery(url, qstr);
  }

  return url;
}

template<typename T>
std::string AppendQuery(
      const std::string &url, const std::string &name, const T &value)
{
  return AppendQuery(url, std::pair<std::string,T>(name, value));
}

std::string AppendQuery(
      const std::string &url, const std::pair<std::string,std::string> &query)
{
  if (!query.first.empty()) {
    std::string qstr = query.first + "=" + query.second;
    return AppendQuery(url, qstr);
  }

  return url;
}

std::string AppendQuery(
    const std::string &url, const std::pair<std::string, bool> &query)
{
  if (!query.first.empty()) {
    std::string qstr = query.first + "=";
    if (query.second) {
      qstr += "true";
    } else {
      qstr += "false";
    }

    return AppendQuery(url, qstr);
  }

  return url;
}

}


std::string ZDvidUrl::getMeshUrl()
{
  return getDataUrl(m_dvidTarget.getMeshName());
}

std::string ZDvidUrl::getMeshUrl(uint64_t bodyId, int zoom)
{
  return GetFullUrl(
        GetKeyCommandUrl(getDataUrl(m_dvidTarget.getMeshName(zoom))),
        GetMeshKey(bodyId));
#if 0
  std::string url;

  ZString dataUrl = getMeshUrl();
  if (!dataUrl.empty()) {
    if (zoom > 0) {
      dataUrl += "_";
      dataUrl.appendNumber(zoom);
    }
    url = GetFullUrl(GetKeyCommandUrl(dataUrl), GetBodyKey(bodyId));
  }

  return url;
#endif
}

std::string ZDvidUrl::getMeshInfoUrl(uint64_t bodyId, int zoom)
{
  return GetMeshInfoUrl(getMeshUrl(bodyId, zoom));
}

std::string ZDvidUrl::GetMeshInfoUrl(const std::string &meshUrl)
{
  //Not a conflict-free of assigning a url, but we'll live with it for now.
  return meshUrl + "_info";
}

std::string ZDvidUrl::getMeshesTarsUrl()
{
  return getDataUrl(
        ZDvidData::GetName(ZDvidData::ROLE_MESHES_TARS,
                           ZDvidData::ROLE_BODY_LABEL,
                           m_dvidTarget.getBodyLabelName()));
}

std::string ZDvidUrl::getMeshesTarsUrl(uint64_t bodyId)
{
  ZString dataUrl = getMeshesTarsUrl();
  ZString key = GetBodyKey(bodyId) + ".tar";
  return GetFullUrl(GetKeyCommandUrl(dataUrl), key);
}

std::string ZDvidUrl::getMeshesTarsKeyRangeUrl(uint64_t bodyId1, uint64_t bodyId2)
{
  ZString dataUrl = ZDvidData::GetName(ZDvidData::ROLE_MESHES_TARS,
                                       ZDvidData::ROLE_BODY_LABEL,
                                       m_dvidTarget.getBodyLabelName());
  ZString key1 = GetBodyKey(bodyId1) + ".tar";
  ZString key2 = GetBodyKey(bodyId2) + ".tar";
  return getKeyRangeUrl(dataUrl, key1, key2);
}

std::string ZDvidUrl::getTarSupervoxelsUrl()
{
  return getDataUrl(
        ZDvidData::GetName(ZDvidData::ROLE_TAR_SUPERVOXELS,
                           ZDvidData::ROLE_BODY_LABEL,
                           m_dvidTarget.getBodyLabelName()));
}

std::string ZDvidUrl::getTarSupervoxelsUrl(uint64_t bodyId)
{
  ZString dataUrl = getTarSupervoxelsUrl();
  ZString key = GetBodyKey(bodyId);
  return GetFullUrl(GetTarfileCommandUrl(dataUrl), key);
}

std::string ZDvidUrl::getSupervoxelMeshUrl(uint64_t bodyId)
{
  return GetFullUrl(GetFullUrl(getTarSupervoxelsUrl(), "supervoxel"),
                    GetBodyKey(bodyId));
}

std::string ZDvidUrl::getSupervoxelMapUrl(uint64_t bodyId)
{
  std::string url;
  if (m_dvidTarget.hasSupervoxel()) {
    url = GetFullUrl(getSegmentationUrl(), "supervoxels");
    if (!url.empty()) {
      url += "/" + std::to_string(bodyId);
    }
  }

  return url;
}

std::string ZDvidUrl::getSkeletonUrl() const
{
  return getSkeletonUrl(m_dvidTarget.getBodyLabelName());
}

std::string ZDvidUrl::getSkeletonUrl(const std::string &bodyLabelName) const
{
  return getDataUrl(
        ZDvidData::GetName(ZDvidData::ROLE_SKELETON,
                           ZDvidData::ROLE_BODY_LABEL,
                           bodyLabelName));
}

std::string
ZDvidUrl::getSkeletonUrl(uint64_t bodyId, const std::string &bodyLabelName) const
{
  return GetFullUrl(GetKeyCommandUrl(getSkeletonUrl(bodyLabelName)),
                    GetSkeletonKey(bodyId));
}

std::string ZDvidUrl::getSkeletonUrl(uint64_t bodyId) const
{
  return getSkeletonUrl(bodyId, m_dvidTarget.getBodyLabelName());
}

std::string ZDvidUrl::getSkeletonConfigUrl(const std::string &bodyLabelName)
{
  return GetFullUrl(getSkeletonUrl(bodyLabelName), "key/config.json");
}

std::string ZDvidUrl::GetKeyCommandUrl(const std::string &dataUrl)
{
  return GetFullUrl(dataUrl, m_keyCommand);
}

std::string ZDvidUrl::GetKeyValuesCommandUrl(const std::string &dataUrl)
{
    return GetFullUrl(dataUrl, m_keyValuesCommand);
}

std::string ZDvidUrl::GetTarfileCommandUrl(const std::string &dataUrl)
{
  return GetFullUrl(dataUrl, m_tarfileCommand);
}

std::string ZDvidUrl::getSupervoxelUrl(const std::string &dataName) const
{
  return GetFullUrl(getDataUrl(dataName), m_supervoxelCommand);
}

std::string ZDvidUrl::getSupervoxelUrl(uint64_t bodyId) const
{
  return getSupervoxelUrl(bodyId, m_dvidTarget.getBodyLabelName());
}

std::string ZDvidUrl::getSupervoxelUrl(
    uint64_t bodyId, const std::string &dataName) const
{
  ZString str;
  str.appendNumber(bodyId);
  str += "?supervoxels=true";

  return GetFullUrl(getSupervoxelUrl(dataName), str);
}

std::string ZDvidUrl::getSparsevolUrl(const std::string &dataName) const
{
  return GetFullUrl(getDataUrl(dataName), m_sparsevolCommand);
}

std::string ZDvidUrl::getSparsevolUrl(uint64_t bodyId) const
{
  return getSparsevolUrl(bodyId, m_dvidTarget.getBodyLabelName());
}

std::string ZDvidUrl::getSparsevolUrl(uint64_t bodyId, const std::string &dataName) const
{
  ZString str;
  str.appendNumber(bodyId);

  return GetFullUrl(getSparsevolUrl(dataName), str);
}

std::string ZDvidUrl::getMultiscaleSupervoxelUrl(uint64_t bodyId, int zoom) const
{
  std::string url;

  if (m_dvidTarget.hasMultiscaleSegmentation()) {
    url = getSupervoxelUrl(bodyId);
    if (zoom > m_dvidTarget.getMaxLabelZoom()) {
      zoom = m_dvidTarget.getMaxLabelZoom();
    }
    ZString option = "scale=";
    option.appendNumber(zoom);
    url = AppendQuery(url, option);

//    url += option;
  } else {
    if (zoom == 0) {
      url = getSupervoxelUrl(bodyId);
    } else {
      url = getSupervoxelUrl(bodyId, m_dvidTarget.getBodyLabelName(zoom));
    }
  }

  return url;
}

std::string ZDvidUrl::getSparsevolUrl(
    uint64_t bodyId, int zoom, const ZIntCuboid &box) const
{
  std::string url = getMultiscaleSparsevolUrl(bodyId, zoom);

  return AppendRangeQuery(url, box);
}

std::string ZDvidUrl::getMultiscaleSparsevolUrl(uint64_t bodyId, int zoom) const
{
  std::string url;

  if (m_dvidTarget.hasMultiscaleSegmentation()) {
    url = getSparsevolUrl(bodyId);
    if (zoom > m_dvidTarget.getMaxLabelZoom()) {
      zoom = m_dvidTarget.getMaxLabelZoom();
    }
    ZString option = "?scale=";
    option.appendNumber(zoom);

    url += option;
  } else {
    if (zoom == 0) {
      url = getSparsevolUrl(bodyId);
    } else {
      url = getSparsevolUrl(bodyId, m_dvidTarget.getBodyLabelName(zoom));
    }
  }

  return url;
}

std::string ZDvidUrl::getSparsevolUrl(const SparsevolConfig &config)
{
  std::string url = getMultiscaleSparsevolUrl(config.bodyId, config.zoom);

  if (!config.format.empty()) {
    url = AppendQuery(url, std::make_pair(std::string("format"), config.format));
  }
  if (config.labelType == flyem::EBodyLabelType::SUPERVOXEL) {
    url = AppendQuery(url, std::make_pair(std::string("supervoxels"), true));
  }

  url = AppendRangeQuery(url, config.range);

  return url;
}

std::string ZDvidUrl::getSparsevolUrl(
    uint64_t bodyId, int z, neutube::EAxis axis) const
{
  if (axis == neutube::EAxis::ARB) {
    return "";
  }

  ZString url = getSparsevolUrl(bodyId);

  if (url.empty()) {
    return "";
  }

  return AppendRangeQuery(url, z, z, axis, true);

#if 0
  switch (axis) {
  case neutube::Z_AXIS:
    url = AppendQueryM(
          url, {std::make_pair("minz", z),
                std::make_pair("maxz", z)});
//    url = AppendQuery(url, std::make_pair("maxz", z));

//    url += "?minz=";
//    url.appendNumber(z);
//    url += "&maxz=";
    break;
  case neutube::X_AXIS:
    url = AppendQueryM(
          url, {std::make_pair("minx", z),
                std::make_pair("maxx", z)});
//    url += "?minx=";
//    url.appendNumber(z);
//    url += "&maxx=";
    break;
  case neutube::Y_AXIS:
    url = AppendQueryM(
          url, {std::make_pair("miny", z),
                std::make_pair("maxy", z)});
//    url += "?miny=";
//    url.appendNumber(z);
//    url += "&maxy=";
    break;
  case neutube::A_AXIS:
    break;
  }

  url = AppendQuery(url, "exact=false");
//  url.appendNumber(z);

//  url += "&exact=false";

  return url;
#endif
}

std::string ZDvidUrl::getSparsevolLastModUrl(uint64_t bodyId)
{
  if (m_dvidTarget.getSegmentationType() == ZDvidData::TYPE_LABELMAP) {
    return GetFullUrl(
          GetFullUrl(getSegmentationUrl(), "lastmod"), std::to_string(bodyId));
  }

  return "";
}

std::string ZDvidUrl::getSupervoxelUrl(
    uint64_t bodyId, int z, neutube::EAxis axis) const
{
  if (axis == neutube::EAxis::ARB) {
    return "";
  }

  ZString url = getSupervoxelUrl(bodyId);

  if (url.empty()) {
    return "";
  }

  return AppendRangeQuery(url, z, z, axis, true);

  /*
  switch (axis) {
  case neutube::Z_AXIS:
    url += "?minz=";
    url.appendNumber(z);
    url += "&maxz=";
    break;
  case neutube::X_AXIS:
    url += "?minx=";
    url.appendNumber(z);
    url += "&maxx=";
    break;
  case neutube::Y_AXIS:
    url += "?miny=";
    url.appendNumber(z);
    url += "&maxy=";
    break;
  case neutube::A_AXIS:
    break;
  }

  url.appendNumber(z);

  url += "&exact=false";

  return url;
  */
}

std::string ZDvidUrl::getSupervoxelUrl(
    uint64_t bodyId, int minZ, int maxZ, neutube::EAxis axis) const
{
  if (axis == neutube::EAxis::ARB) {
    return "";
  }

  ZString url = getSupervoxelUrl(bodyId);

  if (url.empty()) {
    return "";
  }

  return AppendRangeQuery(url, minZ, maxZ, axis, true);

#if 0
  switch (axis) {
  case neutube::Z_AXIS:
    url += "?minz=";
    url.appendNumber(minZ);
    url += "&maxz=";
    url.appendNumber(maxZ);
    break;
  case neutube::X_AXIS:
    url += "?minx=";
    url.appendNumber(minZ);
    url += "&maxx=";
    url.appendNumber(maxZ);
    break;
  case neutube::Y_AXIS:
    url += "?miny=";
    url.appendNumber(minZ);
    url += "&maxy=";
    url.appendNumber(maxZ);
    break;
  case neutube::A_AXIS:
    break;
  }

  return url;
#endif
}

std::string ZDvidUrl::getSparsevolUrl(
    uint64_t bodyId, int minZ, int maxZ, neutube::EAxis axis) const
{
  if (axis == neutube::EAxis::ARB) {
    return "";
  }

  ZString url = getSparsevolUrl(bodyId);

  if (url.empty()) {
    return "";
  }

  return AppendRangeQuery(url, minZ, maxZ, axis, true);

  /*
  switch (axis) {
  case neutube::Z_AXIS:
    url += "?minz=";
    url.appendNumber(minZ);
    url += "&maxz=";
    url.appendNumber(maxZ);
    break;
  case neutube::X_AXIS:
    url += "?minx=";
    url.appendNumber(minZ);
    url += "&maxx=";
    url.appendNumber(maxZ);
    break;
  case neutube::Y_AXIS:
    url += "?miny=";
    url.appendNumber(minZ);
    url += "&maxy=";
    url.appendNumber(maxZ);
    break;
  case neutube::A_AXIS:
    break;
  }

  return url;
  */
}

std::string ZDvidUrl::getSparsevolSizeUrl(uint64_t bodyId) const
{
  ZString url;

  if (m_dvidTarget.hasSparsevolSizeApi()) {
    url = getDataUrl(m_dvidTarget.getBodyLabelName());
    if (!url.empty()) {
      url += "/" + ZDvidData::GetName(ZDvidData::ROLE_SPARSEVOL_SIZE);
      url += "/";
      url.appendNumber(bodyId);
    }
  }

  return url;
}


std::string ZDvidUrl::AppendQueryM(
    const std::string &url, const std::vector<std::pair<std::string, int>> &query)
{
  std::string qstr;
  for (auto &q : query) {
    if (!q.first.empty()) {
      std::string subqstr = q.first + "=" + std::to_string(q.second);
      if (qstr.empty()) {
        qstr = subqstr;
      } else {
        qstr += "&" + subqstr;
      }
    }
  }

  return AppendQuery(url, qstr);
}

std::string ZDvidUrl::AppendRangeQuery(
    const std::string &url, int minZ, int maxZ, neutube::EAxis axis, bool exact)
{
  if (url.empty()) {
    return "";
  }

  std::string newUrl = url;

  switch (axis) {
  case neutube::EAxis::Z:
    newUrl = AppendQueryM(
          url, {std::make_pair("minz", minZ),
                std::make_pair("maxz", maxZ)});
    break;
  case neutube::EAxis::X:
    newUrl = AppendQueryM(
          url, {std::make_pair("minx", minZ),
                std::make_pair("maxx", maxZ)});
    break;
  case neutube::EAxis::Y:
    newUrl = AppendQueryM(
          url, {std::make_pair("miny", minZ),
                std::make_pair("maxy", maxZ)});
    break;
  case neutube::EAxis::ARB:
    break;
  }

  newUrl = AppendQuery(newUrl, std::make_pair("exact", exact));


  return newUrl;

}

std::string ZDvidUrl::AppendRangeQuery(
    const std::string &url, const ZIntCuboid &box)
{
  ZString query;
  if (!url.empty() && !box.isEmpty()) {
    query += "minx=";
    query.appendNumber(box.getFirstCorner().getX());
    query += "&maxx=";
    query.appendNumber(box.getLastCorner().getX());

    query += "&miny=";
    query.appendNumber(box.getFirstCorner().getY());
    query += "&maxy=";
    query.appendNumber(box.getLastCorner().getY());

    query += "&minz=";
    query.appendNumber(box.getFirstCorner().getZ());
    query += "&maxz=";
    query.appendNumber(box.getLastCorner().getZ());
  }

  return AppendQuery(url, query);
}

std::string ZDvidUrl::getSupervoxelUrl
(uint64_t bodyId, int zoom, const ZIntCuboid &box) const
{
  std::string url = getMultiscaleSupervoxelUrl(bodyId, zoom);

  return AppendRangeQuery(url, box);
}

std::string ZDvidUrl::getSupervoxelUrl(
    uint64_t bodyId, const ZIntCuboid &box) const
{
  std::string url = getSupervoxelUrl(bodyId);

  return AppendRangeQuery(url, box);
#if 0
  if (url.empty()) {
    return "";
  }

  if (!box.isEmpty()) {
    url += "?minx=";
    url.appendNumber(box.getFirstCorner().getX());
    url += "&maxx=";
    url.appendNumber(box.getLastCorner().getX());

    url += "&miny=";
    url.appendNumber(box.getFirstCorner().getY());
    url += "&maxy=";
    url.appendNumber(box.getLastCorner().getY());

    url += "&minz=";
    url.appendNumber(box.getFirstCorner().getZ());
    url += "&maxz=";
    url.appendNumber(box.getLastCorner().getZ());
  }

  return url;
#endif
}

std::string ZDvidUrl::getSparsevolUrl(
    uint64_t bodyId, const ZIntCuboid &box) const
{
  ZString url = getSparsevolUrl(bodyId);

  if (url.empty()) {
    return "";
  }

  return AppendRangeQuery(url, box);

  /*
  if (!box.isEmpty()) {
    url += "?minx=";
    url.appendNumber(box.getFirstCorner().getX());
    url += "&maxx=";
    url.appendNumber(box.getLastCorner().getX());

    url += "&miny=";
    url.appendNumber(box.getFirstCorner().getY());
    url += "&maxy=";
    url.appendNumber(box.getLastCorner().getY());

    url += "&minz=";
    url.appendNumber(box.getFirstCorner().getZ());
    url += "&maxz=";
    url.appendNumber(box.getLastCorner().getZ());
  }

  return url;
  */
}

std::string ZDvidUrl::getCoarseSupervoxelUrl(const std::string &dataName) const
{
  return GetFullUrl(getDataUrl(dataName), m_coarseSupervoxelCommand);
//      ZDvidData::GetName(ZDvidData::ROLE_SPARSEVOL_COARSE);
}

std::string ZDvidUrl::getCoarseSupervoxelUrl(
    uint64_t bodyId, const std::string &dataName) const
{
  if (bodyId == 0) {
    return "";
  }

  ZString str;
  str.appendNumber(bodyId);
  str += "?supervoxels=true";

  return GetFullUrl(getCoarseSupervoxelUrl(dataName), str);
}

std::string ZDvidUrl::getCoarseSupervoxelUrl(uint64_t bodyId) const
{
  return getCoarseSparsevolUrl(bodyId, m_dvidTarget.getBodyLabelName());
}

std::string ZDvidUrl::getCoarseSparsevolUrl(const std::string &dataName) const
{
  return GetFullUrl(getDataUrl(dataName), m_coarseSparsevolCommand);
//      ZDvidData::GetName(ZDvidData::ROLE_SPARSEVOL_COARSE);
}

std::string ZDvidUrl::getCoarseSparsevolUrl(
    uint64_t bodyId, const std::string &dataName) const
{
  if (bodyId == 0) {
    return "";
  }

  ZString str;
  str.appendNumber(bodyId);

  return GetFullUrl(getCoarseSparsevolUrl(dataName), str);
}

std::string ZDvidUrl::getCoarseSparsevolUrl(uint64_t bodyId) const
{
  return getCoarseSparsevolUrl(bodyId, m_dvidTarget.getBodyLabelName());
}

std::string ZDvidUrl::getThumbnailUrl(uint64_t bodyId) const
{
  return getThumbnailUrl(bodyId, m_dvidTarget.getBodyLabelName());
}

std::string ZDvidUrl::getThumbnailUrl(const std::string &bodyLabelName) const
{
  return getDataUrl(
        ZDvidData::GetName(ZDvidData::ROLE_THUMBNAIL,
                           ZDvidData::ROLE_BODY_LABEL, bodyLabelName));
}

std::string ZDvidUrl::getThumbnailUrl(uint64_t bodyId, const std::string &bodyLabelName) const
{
  /*
  if (bodyId < 0) {
    return "";
  }
  */

  ZString str;
  str.appendNumber(bodyId);

  return getKeyUrl(ZDvidData::GetName(ZDvidData::ROLE_THUMBNAIL,
                                      ZDvidData::ROLE_BODY_LABEL, bodyLabelName),
                   str + "_mraw");
//  return getThumbnailUrl(bodyLabelName) + "/" + str + ".mraw";
}

std::string ZDvidUrl::getRepoUrl() const
{
  if (m_dvidTarget.isValid()) {
    return GetFullUrl(getApiUrl(), "repo/" + m_dvidTarget.getUuid());
  }

  return "";
}

std::string ZDvidUrl::getInfoUrl() const
{
  return GetFullUrl(getRepoUrl(), "info");
}

std::string ZDvidUrl::getInstanceUrl() const
{
  return GetFullUrl(getRepoUrl(), "instance");
}

std::string ZDvidUrl::getMasterUrl() const
{
  return getKeyUrl("branches", "master");
}

std::string ZDvidUrl::getMirrorInfoUrl() const
{
  return getKeyUrl("branches", "mirror");
}

std::string ZDvidUrl::getDefaultDataInstancesUrl() const
{
  return getKeyUrl("default_instances", "data");
}

std::string ZDvidUrl::getDataMapUrl() const
{
  return getKeyUrl("default_instances", "data_map");
}

std::string ZDvidUrl::getCommitInfoUrl() const
{
  return GetFullUrl(getNodeUrl(), "commit");
}

std::string ZDvidUrl::getSp2bodyUrl() const
{
  return getDataUrl(ZDvidData::GetName(ZDvidData::ROLE_SP2BODY));
}

std::string ZDvidUrl::getSp2bodyUrl(const std::string &suffix) const
{
  return GetFullUrl(getSp2bodyUrl(), suffix);
}

std::string ZDvidUrl::getGrayscaleUrl() const
{
  return getDataUrl(m_dvidTarget.getGrayScaleName());
//  return getDataUrl(ZDvidData::getName(ZDvidData::ROLE_GRAY_SCALE));
}

std::string ZDvidUrl::getGrayscaleUrl(int sx, int sy, int x0, int y0, int z0,
                                      const std::string &format)
const
{
  std::ostringstream stream;
  stream << "raw/0_1/" << sx << "_" << sy << "/" << x0 << "_" << y0 << "_" << z0;
  if (!format.empty()) {
    stream << "/" << format;
  }
  std::string url = GetFullUrl(getGrayscaleUrl(), stream.str());

  return url;
}

std::string ZDvidUrl::getGrayscaleUrl(int sx, int sy, int sz,
                                      int x0, int y0, int z0) const
{
  std::ostringstream stream;
  stream << "raw/0_1_2/" << sx << "_" << sy << "_" << sz << "/"
         << x0 << "_" << y0 << "_" << z0;
  return GetFullUrl(getGrayscaleUrl(), stream.str());
}

std::string ZDvidUrl::getGrayScaleBlockUrl(
    int ix, int iy, int iz, int blockNumber) const
{
  std::string url = getGrayscaleUrl();

  std::ostringstream stream;
  if (!url.empty()) {
    stream << "blocks/" << ix << "_" << iy << "_" << iz << "/" << blockNumber;
  }

  return GetFullUrl(url, stream.str());
}

std::string ZDvidUrl::getSegmentationUrl() const
{
  return getDataUrl(m_dvidTarget.getSegmentationName());
}

std::string ZDvidUrl::getLabels64Url() const
{
  return getSegmentationUrl();
//  return getDataUrl(m_dvidTarget.getSegmentationName());
}

std::string ZDvidUrl::getLabels64Url(int zoom) const
{
  std::string dataName = m_dvidTarget.getValidSegmentationName(zoom);

  return getDataUrl(dataName);
}

std::string ZDvidUrl::getLabels64Url(
    const std::string &name, int sx, int sy, int sz,
    int x0, int y0, int z0) const
{
  if (name.empty()) {
    return "";
  }

  std::ostringstream stream;
  stream << "/raw/0_1_2/" << sx << "_" << sy << "_" << sz << "/"
         << x0 << "_" << y0 << "_" << z0;
//  return getDataUrl(name) + stream.str();
  return GetFullUrl(getDataUrl(name), stream.str());
}

std::string ZDvidUrl::getLabels64Url(int sx, int sy, int sz,
    int x0, int y0, int z0, int zoom) const
{
  return getLabels64Url(m_dvidTarget.getValidSegmentationName(zoom),
                        sx, sy, sz, x0, y0, z0);
}

std::string ZDvidUrl::getKeyUrl(const std::string &name, const std::string &key) const
{
  //new dvid api
//  return GetKeyCommandUrl(getDataUrl(name)) + "/" + key;

  return GetFullUrl(GetKeyCommandUrl(getDataUrl(name)), key);
}

std::string ZDvidUrl::getAllKeyUrl(const std::string &name) const
{
//  return getDataUrl(name) + "/" + m_keysCommand;
  return GetFullUrl(getDataUrl(name), m_keysCommand);
}

std::string ZDvidUrl::getKeyRangeUrl(
    const std::string &name,
    const std::string &key1, const std::string &key2) const
{
//  return getDataUrl(name) + "/" + m_keyRangeCommand + "/" + key1 + "/" + key2;

  /*
  std::string url = getDataUrl(name) + "/" + key1;
  if (!key2.empty()) {
    url += "/" + key2;
  }

  return url;
  */

  return GetFullUrl(getDataUrl(name), m_keyRangeCommand + "/" + key1 + "/" + key2);
}

std::string ZDvidUrl::getKeyValuesUrl(const std::string &name) const
{
    std::string url = GetKeyValuesCommandUrl(getDataUrl(name));
    url += "?jsontar=true";
    return url;
}

std::string ZDvidUrl::getBodyAnnotationName() const
{
  return ZDvidData::GetName(ZDvidData::ROLE_BODY_ANNOTATION,
                            ZDvidData::ROLE_BODY_LABEL,
                            m_dvidTarget.getBodyLabelName());
}

std::string ZDvidUrl::getBodyAnnotationUrl(const std::string &bodyLabelName) const
{
  return getDataUrl(ZDvidData::GetName(ZDvidData::ROLE_BODY_ANNOTATION,
                                       ZDvidData::ROLE_BODY_LABEL,
                                       bodyLabelName));
}

std::string ZDvidUrl::getBodyAnnotationUrl(
    uint64_t bodyId, const std::string &bodyLabelName) const
{
//  return GetKeyCommandUrl(getBodyAnnotationUrl(bodyLabelName)) + "/" +
//      ZString::num2str(bodyId);

  return GetFullUrl(GetKeyCommandUrl(getBodyAnnotationUrl(bodyLabelName)),
                    ZString::num2str(bodyId));
}

std::string ZDvidUrl::getBodyAnnotationUrl(uint64_t bodyId) const
{
  return getBodyAnnotationUrl(bodyId, m_dvidTarget.getBodyLabelName());
}

std::string ZDvidUrl::getBodyInfoUrl(const std::string &bodyLabelName) const
{
  return getDataUrl(ZDvidData::GetName(ZDvidData::ROLE_BODY_INFO,
                                       ZDvidData::ROLE_BODY_LABEL,
                                       bodyLabelName));
}

std::string ZDvidUrl::getBodyInfoUrl(uint64_t bodyId, const std::string &bodyName) const
{
  return getKeyUrl(ZDvidData::GetName(ZDvidData::ROLE_BODY_INFO,
                                      ZDvidData::ROLE_BODY_LABEL,
                                      bodyName), ZString::num2str(bodyId));
}

std::string ZDvidUrl::getBodyInfoUrl(uint64_t bodyId) const
{
  return getBodyInfoUrl(bodyId, m_dvidTarget.getBodyLabelName());
}

std::string ZDvidUrl::getBodySizeUrl(uint64_t bodyId) const
{
  if (m_dvidTarget.getSegmentationType() == ZDvidData::TYPE_LABELMAP) {
    return GetFullUrl(getDataUrl(m_dvidTarget.getSegmentationName()),
                      "size/" + std::to_string(bodyId));
  }

  return "";
}

std::string ZDvidUrl::getSupervoxelSizeUrl(uint64_t bodyId) const
{
  return AppendQuery(getBodySizeUrl(bodyId), std::make_pair(SUPERVOXEL_FLAG, true));
}

std::string ZDvidUrl::getBoundBoxUrl() const
{
  return getDataUrl(ZDvidData::ROLE_BOUND_BOX);
}

std::string ZDvidUrl::getBoundBoxUrl(int z) const
{
//  return GetKeyCommandUrl(getBoundBoxUrl()) + "/" + ZString::num2str(z);

  return GetFullUrl(GetKeyCommandUrl(getBoundBoxUrl()),
                    ZString::num2str(z));
}

std::string ZDvidUrl::getBodyLabelUrl(const std::string &dataName) const
{
  return getDataUrl(dataName);
}

std::string ZDvidUrl::getBodyLabelUrl() const
{
  return getDataUrl(m_dvidTarget.getBodyLabelName());
  //return getDataUrl(ZDvidData::ROLE_BODY_LABEL);
}

#if 0
std::string ZDvidUrl::getBodyLabelUrl(const std::string &dataName,
    int x0, int y0, int z0, int width, int height, int depth) const
{
  std::ostringstream stream;
  stream << "/raw/0_1_2/" << width << "_" << height << "_" << depth << "/"
         << x0 << "_" << y0 << "_" << z0;
  return getBodyLabelUrl(dataName) + stream.str();
}

std::string ZDvidUrl::getBodyLabelUrl(
    int x0, int y0, int z0, int width, int height, int depth) const
{
  std::ostringstream stream;
  stream << "/raw/0_1_2/" << width << "_" << height << "_" << depth << "/"
         << x0 << "_" << y0 << "_" << z0;
  return getBodyLabelUrl() + stream.str();
}
#endif

std::string ZDvidUrl::getBodyListUrl(int minSize) const
{
  std::ostringstream stream;
  stream << "sizerange/" << minSize;

//  return getBodyLabelUrl() + stream.str();

  return GetFullUrl(getBodyLabelUrl(), stream.str());
}

std::string ZDvidUrl::getBodyListUrl(int minSize, int maxSize) const
{
  std::ostringstream stream;
  stream << "sizerange/" << minSize << "/" << maxSize;

//  return getBodyLabelUrl() + stream.str();
  return GetFullUrl(getBodyLabelUrl(), stream.str());
}

std::string ZDvidUrl::getSynapseListUrl() const
{
//  return getBodyLabelUrl(m_dvidTarget.getBodyLabelName()) + "/allsynapse";
  return GetFullUrl(getBodyLabelUrl(m_dvidTarget.getBodyLabelName()),
                    "allsynapse");
}

std::string ZDvidUrl::getSynapseAnnotationUrl(const std::string &name) const
{
  return  GetFullUrl(
        GetKeyCommandUrl(getBodyAnnotationUrl(m_dvidTarget.getBodyLabelName())),
        name);
}

std::string ZDvidUrl::getSynapseAnnotationUrl() const
{
  return getSynapseAnnotationUrl("annotations-synapse");
}

std::string ZDvidUrl::getMergeUrl(const std::string &dataName) const
{
  return GetFullUrl(getDataUrl(dataName), "merge");
}

/*
std::string ZDvidUrl::getMergeOperationUrl(const std::string &dataName) const
{
  return getDataUrl(dataName);
}
*/

std::string ZDvidUrl::getMergeOperationUrl(const std::string &userName) const
{
  std::string key = m_dvidTarget.getSegmentationName();
  if (key.empty()) {
    return "";
  }

  if (!userName.empty()) {
    key += "_" + userName;
  }

  return GetFullUrl(GetKeyCommandUrl(
        getDataUrl(ZDvidData::GetName(ZDvidData::ROLE_MERGE_OPERATION))), key);
}

std::string ZDvidUrl::getSplitUrl(
    const std::string &dataName, uint64_t originalLabel,
    const std::string &command) const
{
  std::ostringstream stream;
  if (command.empty()) {
    stream << originalLabel;
  } else {
    stream << command << "/" << originalLabel;
  }

//  return stream.str();
  return GetFullUrl(getDataUrl(dataName), stream.str());
}

std::string ZDvidUrl::getSplitUrl(
    const std::string &dataName, uint64_t originalLabel) const
{
  return getSplitUrl(dataName, originalLabel, m_splitCommand);
}

std::string ZDvidUrl::getSplitUrl(
    const std::string &dataName, uint64_t originalLabel, uint64_t newLabel) const
{
  std::string url = getSplitUrl(dataName, originalLabel);

  if (url.empty()) {
    return "";
  }

  std::ostringstream stream;
  stream << newLabel;

  return url + "?splitlabel=" + stream.str();
}

std::string ZDvidUrl::getSplitSupervoxelUrl(
    const std::string &dataName, uint64_t originalLabel) const
{
  return getSplitUrl(dataName, originalLabel, m_splitSupervoxelCommand);
}

std::string ZDvidUrl::getCoarseSplitUrl(
    const std::string &dataName, uint64_t originalLabel) const
{
  if (!m_coarseSplitCommand.empty()) {
    return getSplitUrl(dataName, originalLabel, m_coarseSplitCommand);
  }

  return "";

#if 0
  std::ostringstream stream;
  if (m_coarseSplitCommand.empty()) {
    stream << getDataUrl(dataName) << "/" << m_coarseSplitCommand << "/"
           << originalLabel;
  }

  return stream.str();
#endif
}

/*
std::string ZDvidUrl::getMaxBodyIdUrl() const
{
  return getKeyUrl(ZDvidData::GetName(ZDvidData::ROLE_MAX_BODY_ID),
                   m_dvidTarget.getBodyLabelName());
}
*/

std::string ZDvidUrl::getTileUrl(const std::string &dataName) const
{
  return getDataUrl(dataName);
}

std::string ZDvidUrl::getBookmarkKeyUrl() const
{
  return getDataUrl(ZDvidData::GetName(ZDvidData::ROLE_BOOKMARK_KEY));
}

std::string ZDvidUrl::getBookmarkKeyUrl(int x, int y, int z) const
{
  std::ostringstream stream;

  stream << m_keyCommand << "/" << x << "_" << y << "_" << z;

//  return stream.str();
  return GetFullUrl(getBookmarkKeyUrl(), stream.str());
}

std::string ZDvidUrl::getBookmarkKeyUrl(const ZIntPoint &pt) const
{
  return getBookmarkKeyUrl(pt.getX(), pt.getY(), pt.getZ());
}

std::string ZDvidUrl::getBookmarkUrl() const
{
  return getDataUrl(ZDvidData::GetName(ZDvidData::ROLE_BOOKMARK));
}

std::string ZDvidUrl::getBookmarkUrl(
    int x, int y, int z, int width, int height, int depth) const
{
  std::ostringstream stream;

  stream << m_annotationElementsCommand << "/"
         << width << "_" << height << "_" << depth << "/"
         << x << "_" << y << "_" << z;

//  return stream.str();

  return GetFullUrl(getBookmarkUrl(), stream.str());
}

std::string ZDvidUrl::getBookmarkUrl(
    const ZIntPoint &pt, int width, int height, int depth) const
{
  return getBookmarkUrl(pt.getX(), pt.getY(), pt.getZ(), width, height, depth);
}

std::string ZDvidUrl::getBookmarkUrl(const ZIntCuboid &box) const
{
  return getBookmarkUrl(box.getFirstCorner(), box.getWidth(), box.getHeight(),
                        box.getDepth());
}

std::string ZDvidUrl::getCustomBookmarkUrl(const std::string &userName) const
{
  return getKeyUrl(ZDvidData::GetName(ZDvidData::ROLE_BOOKMARK_KEY), userName);
}

std::string ZDvidUrl::getTileUrl(
    const std::string &dataName, int resLevel) const
{
  ZString url = getTileUrl(dataName);

  if (!url.empty()) {
    url += "/tile/xy/";
    url.appendNumber(resLevel);
  }

  return url;
}

std::string ZDvidUrl::getTileUrl(
    const std::string &dataName, int resLevel, int xi0, int yi0, int z0) const
{
  ZString url = getTileUrl(dataName, resLevel);

  if (!url.empty()) {
    url += "/";
    url.appendNumber(xi0);
    url += "_";
    url.appendNumber(yi0);
    url += "_";
    url.appendNumber(z0);
  }

  return url;
}

std::string ZDvidUrl::getRepoInfoUrl() const
{
  return GetFullUrl(getApiUrl(), "/repos/info");
}

std::string ZDvidUrl::getLockUrl() const
{
  return GetFullUrl(getNodeUrl(), "/lock");
}

std::string ZDvidUrl::getBranchUrl() const
{
  return GetFullUrl(getNodeUrl(), "/branch");
}

std::string ZDvidUrl::GetPath(const std::string &url)
{
  std::string marker = "api/node/";

  std::string::size_type markerPos = url.find(marker);

  if (markerPos != std::string::npos) {
    markerPos += marker.size();
    std::string::size_type uuidPos = url.find('/', markerPos);
    return url.substr(uuidPos);
  }

  return url;
}

std::string ZDvidUrl::getLocalBodyIdUrl(int x, int y, int z) const
{
  ZString url = getLabels64Url();

  if (!url.empty()) {
    url += "/" + m_labelCommand + "/";
    url.appendNumber(x);
    url += "_";
    url.appendNumber(y);
    url += "_";
    url.appendNumber(z);
  }

  return url;
}

std::string ZDvidUrl::getLocalSupervoxelIdUrl(int x, int y, int z) const
{
  std::string url = getLocalBodyIdUrl(x, y, z);
  if (!url.empty()) {
    url += "?supervoxels=true";
  }

  return url;
}

std::string ZDvidUrl::getLocalBodyIdArrayUrl() const
{
  return GetFullUrl(getLabels64Url(), m_labelArrayCommand);
}

std::string ZDvidUrl::getRoiUrl(const std::string &dataName) const
{
  return GetFullUrl(getDataUrl(dataName), m_roiCommand);
}

std::string ZDvidUrl::getManagedRoiUrl(const std::string &key) const
{
  return getKeyUrl(ZDvidData::GetName(ZDvidData::ROLE_ROI_KEY), key);
}

std::string ZDvidUrl::getRoiMeshUrl(const std::string &key) const
{
  return getKeyUrl(ZDvidData::GetName(ZDvidData::ROLE_ROI_DATA_KEY), key);
}

std::string ZDvidUrl::getAnnotationUrl(const std::string &dataName) const
{
  return getDataUrl(dataName);
}

std::string ZDvidUrl::getAnnotationSyncUrl(const std::string &dataName) const
{
  return GetFullUrl(getAnnotationUrl(dataName), "sync");
}

std::string ZDvidUrl::getAnnotationSyncUrl(
    const std::string &dataName, const std::string &queryString) const
{
  std::string url = getAnnotationSyncUrl(dataName);

  if (!queryString.empty()) {
   url = GetFullUrl(url, "?" + queryString);
  }

  return url;
}

std::string ZDvidUrl::getDataSyncUrl(
    const std::string &dataName, const std::string &queryString) const
{
  std::string url = GetFullUrl(getDataUrl(dataName), "sync");

  if (!queryString.empty()) {
   url = GetFullUrl(url, "?" + queryString);
  }

  return url;
}

std::string ZDvidUrl::getLabelszSyncUrl(const std::string &dataName) const
{
  return GetFullUrl(getDataUrl(dataName), "sync");
}

std::string ZDvidUrl::getLabelMappingUrl() const
{
  return GetFullUrl(
        getDataUrl(m_dvidTarget.getSegmentationName()), m_labelMappingCommand);
}

std::string ZDvidUrl::getAnnotationUrl(
    const std::string &dataName, const std::string tag) const
{  
  return GetFullUrl(getAnnotationUrl(dataName),
                    m_annotationTagCommand + "/" + tag);
}

std::string ZDvidUrl::getAnnotationUrl(
    const std::string &dataName, uint64_t label) const
{
  std::string url = getAnnotationUrl(dataName);

  if (!url.empty()) {
    if (m_dvidTarget.isSegmentationSyncable()) {
      std::ostringstream stream;
      stream << label;
      url += "/" + m_annotationLabelCommand + "/" + stream.str();
    } else {
      //A workaround for syncing GetBodyIdTag(temporary solution)
#if defined(_QT_GUI_USED_)
      url = getAnnotationUrl(dataName, ZDvid::GetBodyIdTag(label));
#else
      std::ostringstream stream;
      stream << "body:" << label;
      url = getAnnotationUrl(dataName, stream.str());
#endif

    }
  }

  return url;
}


std::string ZDvidUrl::getAnnotationUrl(
    const std::string &dataName, int x, int y, int z) const
{
  std::string url = getAnnotationUrl(dataName);

  if (!url.empty()) {
    std::ostringstream stream;
    stream << "/" << m_annotationElementCommand << "/" << x << "_" << y << "_" << z;

    url += stream.str();
  }

  return url;
}

std::string ZDvidUrl::getAnnotationUrl(
    const std::string &dataName,
    int x, int y, int z, int width, int height, int depth) const
{
  std::string url = getAnnotationUrl(dataName);

  if (!url.empty()) {
    std::ostringstream stream;

    stream << "/" << m_annotationElementsCommand << "/"
           << width << "_" << height << "_" << depth << "/"
           << x << "_" << y << "_" << z;
    url += stream.str();
  }

  return url;
}

std::string ZDvidUrl::getAnnotationUrl(
    const std::string &dataName, const ZIntCuboid &box) const
{
  return getAnnotationUrl(
        dataName, box.getFirstCorner().getX(), box.getFirstCorner().getY(),
        box.getFirstCorner().getZ(), box.getWidth(), box.getHeight(),
        box.getDepth());
}

std::string ZDvidUrl::getAnnotationElementsUrl(const std::string &dataName) const
{
  return GetFullUrl(getAnnotationUrl(dataName), m_annotationElementsCommand);
}

std::string ZDvidUrl::getAnnotationDeleteUrl(const std::string &dataName) const
{
  if (dataName.empty()) {
    return "";
  }

  return getAnnotationUrl(dataName) + "/element";
}

std::string ZDvidUrl::getAnnotationDeleteUrl(
    const std::string &dataName, int x, int y, int z) const
{
  if (dataName.empty()) {
    return "";
  }

  std::ostringstream stream;
  stream << x << "_" << y << "_" << z;

  return GetFullUrl(getAnnotationDeleteUrl(dataName), stream.str());
}

std::string ZDvidUrl::getSynapseUrl() const
{
  return getDataUrl(m_dvidTarget.getSynapseName());
}

std::string ZDvidUrl::getSynapseUrl(int x, int y, int z) const
{
  std::ostringstream stream;

  stream << m_annotationElementCommand << "/" << x
         << "_" << y << "_" << z;

  return GetFullUrl(getSynapseUrl(), stream.str());
}

std::string ZDvidUrl::getSynapseUrl(const ZIntPoint &pos) const
{
  return getSynapseUrl(pos.getX(), pos.getY(), pos.getZ());
}

std::string ZDvidUrl::getSynapseUrl(
    int x, int y, int z, int width, int height, int depth) const
{
  std::ostringstream stream;

  stream << m_annotationElementsCommand << "/"
         << width << "_" << height << "_" << depth << "/"
         << x << "_" << y << "_" << z;

  return GetFullUrl(getSynapseUrl(), stream.str());
}

std::string ZDvidUrl::getSynapseElementsUrl() const
{
//  std::ostringstream stream;
  return GetFullUrl(getSynapseUrl(), m_annotationElementsCommand);
}

std::string ZDvidUrl::getSynapseUrl(
    const ZIntPoint &pos, int width, int height, int depth) const
{
  return getSynapseUrl(pos.getX(), pos.getY(), pos.getZ(),
                       width, height, depth);
}

std::string ZDvidUrl::getSynapseMoveUrl(
    const ZIntPoint &from, const ZIntPoint &to) const
{
  std::ostringstream stream;

  stream << m_annotationMoveCommand << "/"
         << from.getX() << "_" << from.getY() << "_" << from.getZ() << "/"
         << to.getX() << "_" << to.getY() << "_" << to.getZ();



  return GetFullUrl(getSynapseUrl(), stream.str());
}

std::string ZDvidUrl::getSynapseUrl(const ZIntCuboid &box) const
{
  return getSynapseUrl(box.getFirstCorner(), box.getWidth(), box.getHeight(),
                       box.getDepth());
}

std::string ZDvidUrl::getSynapseUrl(uint64_t label, bool relation) const
{
  std::ostringstream stream;

  stream << m_annotationLabelCommand << "/" << label << "?relationships=";

  if (relation) {
    stream << "true";
  } else {
    stream << "false";
  }

  return GetFullUrl(getSynapseUrl(), stream.str());
}

std::string ZDvidUrl::getTodoListUrl() const
{
  return getAnnotationUrl(m_dvidTarget.getTodoListName());
}

std::string ZDvidUrl::getTodoListElementsUrl() const
{
  return getAnnotationElementsUrl(m_dvidTarget.getTodoListName());
}

std::string ZDvidUrl::getTodoListDeleteUrl(int x, int y, int z) const
{
  return getAnnotationDeleteUrl(m_dvidTarget.getTodoListName(), x, y, z);
}

std::string ZDvidUrl::getTodoListUrl(
    int x, int y, int z, int width, int height, int depth) const
{
  return getAnnotationUrl(m_dvidTarget.getTodoListName(),
                          x, y, z, width, height, depth);
}

std::string ZDvidUrl::getTodoListUrl(int x, int y, int z) const
{
  return getTodoListUrl(x, y, z, 1, 1, 1);
}

std::string ZDvidUrl::getConfigUrl() const
{
  return getDataUrl("neutu_config");
}

std::string ZDvidUrl::getContrastUrl() const
{
  return getKeyUrl("neutu_config", "contrast");
}

std::string ZDvidUrl::getTodoListUrl(const ZIntCuboid &cuboid) const
{
  return getTodoListUrl(cuboid.getFirstCorner().getX(),
                        cuboid.getFirstCorner().getY(),
                        cuboid.getFirstCorner().getZ(),
                        cuboid.getWidth(), cuboid.getHeight(),
                        cuboid.getDepth());
}

std::string ZDvidUrl::getSynapseLabelszBodyUrl(uint64_t bodyId) const
{
  std::string url = getDataUrl(m_dvidTarget.getSynapseLabelszName());

  if (!url.empty()) {
    url += "/count/" + ZString::num2str(bodyId);
  }

  return url;
}

std::string ZDvidUrl::getSynapseLabelszUrl(int n) const
{
  std::string url = getDataUrl(m_dvidTarget.getSynapseLabelszName());

  if (!url.empty()) {
    url += "/top/" + ZString::num2str(n);
  }

  return url;
}

std::string ZDvidUrl::GetLabelszIndexTypeStr(ZDvid::ELabelIndexType type)
{
  std::string name;
  switch (type) {
  case ZDvid::INDEX_PRE_SYN:
    name = "PreSyn";
    break;
  case ZDvid::INDEX_POST_SYN:
    name = "PostSyn";
    break;
  case ZDvid::INDEX_ALL_SYN:
    name = "AllSyn";
    break;
  case ZDvid::INDEX_GAP:
    name = "Gap";
    break;
  case ZDvid::INDEX_NOTE:
    name = "Note";
    break;
  case ZDvid::INDEX_VOXEL:
    name = "Voxels";
    break;
  }

  return name;
}

std::string ZDvidUrl::getSynapseLabelszUrl(
    int n, ZDvid::ELabelIndexType indexType) const
{
  return GetFullUrl(getSynapseLabelszUrl(n), GetLabelszIndexTypeStr(indexType));
}

std::string ZDvidUrl::getSynapseLabelszBodyUrl(
    uint64_t bodyId, ZDvid::ELabelIndexType indexType) const
{
  return GetFullUrl(getSynapseLabelszBodyUrl(bodyId),
                    GetLabelszIndexTypeStr(indexType));
}

std::string ZDvidUrl::getSynapseLabelszThresholdUrl(int threshold) const {
    std::string name;

    std::string dataName = m_dvidTarget.getSynapseLabelszName();
    if (!dataName.empty()) {
      name = getDataUrl(dataName);
      name += "/threshold/";
      name += ZString::num2str(threshold);
    }

    return name;
}

std::string ZDvidUrl::getSynapseLabelszThresholdUrl(
    int threshold, ZDvid::ELabelIndexType indexType)  const {
  return getSynapseLabelszThresholdUrl(threshold) + "/" + GetLabelszIndexTypeStr(indexType);
}

std::string ZDvidUrl::getSynapseLabelszThresholdUrl(
    int threshold, ZDvid::ELabelIndexType indexType, int offset, int number)  const {
    std::string url = getSynapseLabelszThresholdUrl(threshold, indexType);
    url += "/?offset=";
    url += ZString::num2str(offset);
    url += "&n=";
    url += ZString::num2str(number);
    return url;
}

void ZDvidUrl::setUuid(const std::string &uuid)
{
  m_dvidTarget.setUuid(uuid);
}

std::string ZDvidUrl::GetBodyKey(uint64_t bodyId)
{
  std::ostringstream stream;
  stream << bodyId;

  return stream.str();
}

std::string ZDvidUrl::GetSkeletonKey(uint64_t bodyId)
{
  std::ostringstream stream;
  stream << bodyId << "_swc";

  return stream.str();
}

std::string ZDvidUrl::GetMeshKey(uint64_t bodyId)
{
  return GetBodyKey(bodyId);
}

std::string ZDvidUrl::GetMeshInfoKey(uint64_t bodyId)
{
  return GetMeshKey(bodyId) + "_info";
}

std::string ZDvidUrl::GetTaskKey()
{
  return "task__" + NeutubeConfig::GetUserName();
}

std::string ZDvidUrl::GetServiceResultEndPoint()
{
  return "result";
}

std::string ZDvidUrl::GetResultKeyFromTaskKey(const std::string &key)
{
  ZString newKey = key;
  if (newKey.startsWith("task__")) {
//    newKey.replace("head__", "task__");
    newKey = key;
  } else {
    newKey = "";
  }

  return newKey;
}

std::string ZDvidUrl::ExtractSplitTaskKey(const std::string &url)
{
  std::string w("task_split/key/");
  std::string::size_type index = url.rfind(w);

  if (index == std::string::npos) {
    return "";
  }

  return url.substr(index + w.size());
}

std::string ZDvidUrl::getSplitTaskKey(const uint64_t bodyId) const
{
  std::string key;
  std::string bodyUrl = getSparsevolUrl(bodyId);
  if (!bodyUrl.empty()) {
    key = "task__" + ZString(bodyUrl).replace(":", "-").replace("/", "+");
  }

  return key;
}

std::string ZDvidUrl::getSplitResultKey(const uint64_t bodyId) const
{
  std::string taskKey = getSplitTaskKey(bodyId);

  return GetResultKeyFromTaskKey(taskKey);
}

std::string ZDvidUrl::getTestTaskUrl(const std::string &key)
{
  return getKeyUrl(ZDvidData::GetName(ZDvidData::ROLE_TEST_TASK_KEY), key);
}

/*
std::string ZDvidUrl::getTestTaskUrl() const
{
  return getTestTaskUrl(GetTaskKey());
}
*/

/*
bool ZDvidUrl::IsSplitTask(const std::string &url)
{
  return ZString()
}
*/

uint64_t ZDvidUrl::GetBodyId(const std::string &url)
{
  uint64_t id = 0;

  ZString idWord = ZString(url).getLastWord('/');

  if (idWord.isAllDigit()) {
    id = idWord.firstUint64();
  }

  return id;
}
