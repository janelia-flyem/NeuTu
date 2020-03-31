#include "zdvidtarget.h"
#include "zstring.h"
#include "zerror.h"
#include "zjsonparser.h"
#include "zjsonobject.h"
#include "zdviddata.h"
#if _QT_APPLICATION_
#include <QtDebug>
#include "logging/zqslog.h"
#include "dvid/zdvidbufferreader.h"
#endif
#include "neutubeconfig.h"

const char* ZDvidTarget::m_commentKey = "comment";
const char* ZDvidTarget::m_nameKey = "name";
const char* ZDvidTarget::m_localKey = "local";
const char* ZDvidTarget::m_debugKey = "debug";
const char* ZDvidTarget::m_bgValueKey = "background";
const char* ZDvidTarget::m_bodyLabelNameKey = "body_label";
const char* ZDvidTarget::m_segmentationNameKey = "label_block";
const char* ZDvidTarget::m_newSegmentationNameKey = "segmentation";
const char* ZDvidTarget::m_grayScaleNameKey = "gray_scale";
const char* ZDvidTarget::m_multiscale2dNameKey = "multires_tile";
const char* ZDvidTarget::m_tileConfigKey = "multires_tile_config";
const char* ZDvidTarget::m_synapseNameKey = "synapse";
const char* ZDvidTarget::m_userNameKey = "user_name";
const char* ZDvidTarget::m_supervisorKey = "supervised";
const char* ZDvidTarget::m_supervisorServerKey = "librarian";
const char* ZDvidTarget::m_roiListKey = "roi_list";
const char* ZDvidTarget::m_roiNameKey = "roi";
const char* ZDvidTarget::m_maxLabelZoomKey = "label_max_zoom";
const char* ZDvidTarget::m_maxGrayscaleZoomKey = "grayscale_max_zoom";
const char* ZDvidTarget::m_synapseLabelszKey = "labelsz";
const char* ZDvidTarget::m_todoListNameKey = "todo";
const char* ZDvidTarget::m_defaultSettingKey = "default";
const char* ZDvidTarget::m_sourceConfigKey = "@source";
const char* ZDvidTarget::m_proofreadingKey = "proofreading";
const char* ZDvidTarget::m_adminTokenKey = "admin_token";

ZDvidTarget::ZDvidTarget()
{
//  init();
}

ZDvidTarget::ZDvidTarget(
    const std::string &address, const std::string &uuid, int port)
{
//  init();

  set(address, uuid, port);
}

ZDvidTarget::ZDvidTarget(const ZDvidNode &node)
{
//  init();
  m_node = node;
}
/*
void ZDvidTarget::init()
{
  m_isSupervised = true;
  m_bgValue = 255;
  m_isEditable = true;
  m_readOnly = false;
  m_nodeStatus = dvid::ENodeStatus::OFFLINE;
  m_maxLabelZoom = 0;
  m_maxGrayscaleZoom = 0;
  m_usingMultresBodyLabel = true;
  m_usingDefaultSetting = false;
  m_isInferred = false;

  setDefaultMultiscale2dName();
  setSegmentationName("*");
//  m_multiscale2dName = ZDvidData::GetName(ZDvidData::ROLE_MULTISCALE_2D);
}
*/

std::string ZDvidTarget::getSourceString(bool withHttpPrefix, int uuidBrief) const
{
  std::string source = m_node.getSourceString(withHttpPrefix, uuidBrief);
/*
  if (!getAddress().empty()) {
    source = getAddress() + ":" + ZString::num2str(getPort()) + ":" + getUuid();
    if (withHttpPrefix) {
      source = "http:" + source;
    }
  }
*/
  if (!getBodyLabelName().empty()) {
    source += ":" + getBodyLabelName();
  }

  return source;
}

std::string ZDvidTarget::getGrayscaleSourceString() const
{
  std::string source = getGrayScaleSource().getSourceString(true, 4);

  source += "::" + getGrayScaleName();

  return source;
}

void ZDvidTarget::set(
    const std::string &address, const std::string &uuid, int port)
{
  m_node.set(address, uuid, port);
}

void ZDvidTarget::clear()
{
  *this = ZDvidTarget();
}

#if 0
void ZDvidTarget::clear()
{
  m_node.clear();
//  init();
  m_name = "";
  m_comment = "";
  m_localFolder = "";
  m_bodyLabelName = "*";
  m_segmentationName.clear();
  m_multiscale2dName.clear();
  m_grayScaleName = "";
  m_synapseLabelszName = "";
  m_synapseName = "";
  m_roiName = "";
  m_todoListName = "";
  m_roiList.clear();
  m_userList.clear();
  m_supervisorServer.clear();
  m_tileConfig.clear();
  m_sourceConfig.clear();
  m_isInferred = false;
}
#endif

void ZDvidTarget::setServer(const std::string &address)
{
  m_node.setHost(address);
}

void ZDvidTarget::setUuid(const std::string &uuid)
{
  m_node.setUuid(uuid);
}

void ZDvidTarget::setPort(int port)
{
  m_node.setPort(port);
}

void ZDvidTarget::setFromUrl_deprecated(const std::string &url)
{
  clear();
  if (url.empty()) {
    return;
  }

  ZString zurl(url);
  if (zurl.startsWith("http://")) {
    zurl.replace("http://", "");
  } else if (zurl.startsWith("mock://")) {
    zurl.replace("mock://", "");
    setMock(true);
  } else if (zurl.startsWith("https://")) {
    zurl.replace("https://", "");
    setScheme("https");
  }

  std::vector<std::string> tokens = zurl.tokenize('/');
  ZString addressToken = tokens[0];
  std::vector<std::string> tokens2 = addressToken.tokenize(':');
  int port = -1;
  if (tokens2.size() > 1) {
    if (!tokens2[1].empty()) {
      port = ZString::FirstInteger(tokens2[1]);
      if (tokens2[1][0] == '-') {
        port = -port;
      }
    }
  }
  std::string uuid;
  if (tokens.size() > 3) {
    if (tokens[1] == "api" && tokens[2] == "node") {
      uuid = tokens[3];
    }
    if (tokens.size() > 6) {
      if (tokens[5] == "sparsevol") {
        setBodyLabelName(tokens[4]);
      }
    }
  }
  set(tokens2[0], uuid, port);
}

bool ZDvidTarget::hasDvidUuid() const
{
  return getNode().hasDvidUuid();
}

void ZDvidTarget::setFromSourceString(const std::string &sourceString)
{
  clear();

  std::vector<std::string> tokens = ZString(sourceString).tokenize(':');
  m_node.setFromSourceToken(tokens);
  if (tokens.size() >= 5) {
    setBodyLabelName(tokens[4]);
  }

  if (tokens.size() >= 6) {
    setGrayScaleName(tokens[5]);
  }
#if 0
  if (tokens.size() < 4 || tokens[0] != "http" || tokens[0] != "mock") {
#if defined(_QT_APPLICATION_)
    LWARN() << "Invalid source string for dvid target:" << sourceString.c_str();
#else
    RECORD_WARNING_UNCOND("Invalid source string");
#endif
  } else {
    int port = -1;
    if (!tokens[2].empty()) {
      port = ZString::FirstInteger(tokens[2]);
      if (tokens[2][0] == '-') {
        port = -port;
      }
    }
    set(tokens[1], tokens[3], port);
    if (tokens.size() >= 5) {
      setBodyLabelName(tokens[4]);
    }

    if (tokens.size() >= 6) {
      setGrayScaleName(tokens[5]);
    }
    if (tokens[0] == "mock") {
      setMock(true);
    }
  }
#endif
}

void ZDvidTarget::setFromSourceString(
    const std::string &sourceString, dvid::EDataType dataType)
{
  clear();

  std::vector<std::string> tokens = ZString(sourceString).tokenize(':');

  if (tokens.size() < 4 || (tokens[0] != "http" && tokens[0] != "mock")) {
#if defined(_QT_APPLICATION_)
    LWARN() << "Invalid source string for dvid target:" << sourceString.c_str();
#else
    RECORD_WARNING_UNCOND("Invalid source string");
#endif
  } else {
    int port = -1;
    if (!tokens[2].empty()) {
      port = ZString::FirstInteger(tokens[2]);
      if (tokens[2][0] == '-') {
        port = -port;
      }
    }
    set(tokens[1], tokens[3], port);
    if (tokens.size() >= 5) {
      switch (dataType) {
      case dvid::EDataType::LABELVOL:
        setBodyLabelName(tokens[4]);
        break;
      case dvid::EDataType::UINT8BLK:
        setGrayScaleName(tokens[4]);
        break;
      default:
        break;
      }
    }
    if (tokens[0] == "mock") {
      setMock(true);
    }
  }
}

void ZDvidTarget::setNodeStatus(dvid::ENodeStatus status)
{
  m_nodeStatus = status;
}

bool ZDvidTarget::hasPort() const
{
  return getPort() >= 0;
}

bool ZDvidTarget::isValid() const
{
  return !getAddress().empty() && !getUuid().empty() &&
      (m_nodeStatus != dvid::ENodeStatus::INVALID);
}

std::string ZDvidTarget::getAddressWithPort() const
{
  std::string address;

  if (!getAddress().empty()) {
    address = getAddress();
    if (hasPort()) {
      address += ":" + ZString::num2str(getPort());
    }
  }

  return address;
}

std::string ZDvidTarget::getHostWithScheme() const
{
  return m_node.getHostWithScheme();
}

std::string ZDvidTarget::getRootUrl() const
{
  return m_node.getRootUrl();
}

void ZDvidTarget::setScheme(const std::string &scheme)
{
  m_node.setScheme(scheme);
}

std::string ZDvidTarget::getScheme() const
{
  return m_node.getScheme();
}

void ZDvidTarget::setMock(bool on)
{
  m_node.setMock(on);
}

bool ZDvidTarget::isMock() const
{
  return m_node.isMock();
}

void ZDvidTarget::print() const
{
  std::cout << getSourceString() << std::endl;
}

bool ZDvidTarget::readOnly() const
{
  return m_readOnly || getNodeStatus() == dvid::ENodeStatus::LOCKED;
}

void ZDvidTarget::setSynapseReadonly(bool on)
{
  m_isSynpaseEditable = !on;
}

bool ZDvidTarget::isSupervoxelView() const
{
  return m_supervoxelView;
}

void ZDvidTarget::setSupervoxelView(bool on)
{
  m_supervoxelView = on;
}

dvid::ENodeStatus ZDvidTarget::getNodeStatus() const
{
  return m_nodeStatus;
}

/*
std::string ZDvidTarget::getBodyPath(uint64_t bodyId) const
{
  return getSourceString() + ":" + ZString::num2str(bodyId);
}
*/

namespace {
template<typename T>
ZJsonObject MakeJsonObject(const std::map<std::string, T> &config)
{
  ZJsonObject configJson;
  if (!config.empty()) {
    for (const auto& tg : config) {
      ZJsonObject subjson = tg.second.toJsonObject();
      configJson.setEntry(tg.first.c_str(), subjson);
    }
  }
  return configJson;
}
} //namespace


ZJsonObject ZDvidTarget::toJsonObject() const
{
  ZJsonObject obj = m_node.toJsonObject();
  obj.setNonEmptyEntry(m_commentKey, m_comment);
  obj.setNonEmptyEntry(m_nameKey, m_name);
  obj.setNonEmptyEntry(m_localKey, m_localFolder);
  obj.setEntry(m_bgValueKey, m_bgValue);
  obj.setNonEmptyEntry(m_bodyLabelNameKey, m_bodyLabelName);
  obj.setNonEmptyEntry(m_segmentationNameKey, m_segmentationName);
  obj.setEntry(m_maxLabelZoomKey, m_maxLabelZoom);
  obj.setEntry(m_maxGrayscaleZoomKey, getMaxGrayscaleZoom());
  obj.setNonEmptyEntry(m_grayScaleNameKey, m_grayscaleName);
  obj.setNonEmptyEntry(m_synapseLabelszKey, m_synapseLabelszName);
  obj.setNonEmptyEntry(m_roiNameKey, m_roiName);
  obj.setNonEmptyEntry(m_todoListNameKey, m_todoListName);
  obj.setEntry(m_proofreadingKey, !m_readOnly);
  obj.setNonEmptyEntry(m_adminTokenKey, m_adminToken);

  ZJsonArray jsonArray;
  for (std::vector<std::string>::const_iterator iter = m_roiList.begin();
       iter != m_roiList.end(); ++iter) {
    jsonArray.append(*iter);
  }
  obj.setEntry(m_roiListKey, jsonArray);

  obj.setEntry(m_multiscale2dNameKey, m_multiscale2dName);

  if (!m_tileConfig.empty()) {
    /*
    ZJsonObject tileConfigJson;
    for (const auto& tg : m_tileConfig) {
      ZJsonObject subjson = tg.second.toJsonObject();
      tileConfigJson.setEntry(tg.first.c_str(), subjson);
    }
    */
    ZJsonObject tileConfigJson = MakeJsonObject(m_tileConfig);
    obj.setEntry(m_tileConfigKey, tileConfigJson);
  }
  /*
  if (!m_tileConfig.isEmpty()) {
    obj.setEntry(m_tileConfigKey, const_cast<ZJsonObject&>(m_tileConfig));
  }
  */

  if (!m_sourceConfig.empty()) {
    ZJsonObject sourceConfigJson = MakeJsonObject(m_sourceConfig);
    obj.setEntry(m_sourceConfigKey, sourceConfigJson);
//    obj.setEntry(m_sourceConfigKey, const_cast<ZJsonObject&>(m_sourceConfig));
  }

  obj.setEntry(m_synapseNameKey, m_synapseName);
  obj.setEntry(m_supervisorKey, m_isSupervised);
  obj.setNonEmptyEntry(m_supervisorServerKey, m_supervisorServer);
  obj.setEntry(m_defaultSettingKey, usingDefaultDataSetting());

  return obj;
}

std::string ZDvidTarget::getSynapseLabelszName() const
{
  if (!m_synapseLabelszName.empty()) {
    return m_synapseLabelszName;
  }

  if (getSynapseName().empty()) {
    return "";
  }

  return ZDvidData::GetName(ZDvidData::ERole::LABELSZ,
                            ZDvidData::ERole::SYNAPSE,
                            getSynapseName());
}

void ZDvidTarget::setSynapseLabelszName(const std::string &name)
{
  m_synapseLabelszName = name;
}

void ZDvidTarget::loadDvidDataSetting(const ZJsonObject &obj)
{
  if (obj.hasKey("segmentation")) {
    setSegmentationName(ZJsonParser::stringValue(obj["segmentation"]));
  }
  if (obj.hasKey("bodies")) {
    setBodyLabelName(ZJsonParser::stringValue(obj["bodies"]));
  }
  if (obj.hasKey("synapses")) {
    setSynapseName(ZJsonParser::stringValue(obj["synapses"]));
  }
  if (obj.hasKey("grayscale")) {
    setGrayScaleName(ZJsonParser::stringValue(obj["grayscale"]));
  }

  if (obj.hasKey("todos")) {
    setTodoListName(ZJsonParser::stringValue(obj["todos"]));
  }
}

ZJsonObject ZDvidTarget::toDvidDataSetting() const
{
  ZJsonObject obj;

  obj.setEntry("segmentation", getSegmentationName());
  obj.setEntry("synapses", getSynapseName());
  obj.setEntry("bodies", getBodyLabelName());
  obj.setEntry("skeletons", getSkeletonName());
  obj.setEntry("grayscale", getGrayScaleName());
  obj.setEntry("body_annotations", getBodyAnnotationName());
  obj.setEntry("todos", getTodoListName());
  obj.setEntry("sequencer_info", getSynapseLabelszName());
  obj.setEntry("bookmarks", getBookmarkName());

  return obj;
}

namespace {

template<typename T>
void LoadJsonConfig(const ZJsonObject &json, std::map<std::string, T> &config)
{
  const char *key;
  json_t *value;
  ZJsonObject_foreach(json, key, value) {
    T tg;
    tg.loadJsonObject(ZJsonObject(value, ZJsonValue::SET_INCREASE_REF_COUNT));
    config[key] = tg;
  }
}

} //namespace

namespace {
template<typename T>
std::string GetTestJsonString(const T &json)
{
  return json.dumpJanssonString(JSON_INDENT(0) | JSON_SORT_KEYS);
}
}
bool ZDvidTarget::Test()
{
#ifdef _QT_GUI_USED_
  std::cout << "Testing private functions ..." << std::endl;
  {
    TileConfig tg;
    tg.setLowQuality(true);
    std::map<std::string, TileConfig> config;
    config["d1"] = tg;
    tg.setLowQuality(false);
    config["t2"] = tg;

    ZJsonObject json = MakeJsonObject(config);
    if (GetTestJsonString(json) !=
        "{\"d1\": {\"low_quality\": true}, \"t2\": {\"low_quality\": false}}") {
      std::cout << GetTestJsonString(json) << std::endl;
      LERROR() << "Test failed";
      return false;
    }
  }

  {
    ZDvidNode node("emdata2.int.janelia.org", "5678", 7000);
    std::map<std::string, ZDvidNode> config;
    config["n1"] = node;
    node.setUuid("abcd");
    config["n2"] = node;
    ZJsonObject json = MakeJsonObject(config);
    if (GetTestJsonString(json) !=
        "{\"n1\": {\"host\": \"emdata2.int.janelia.org\", \"port\": 7000, "
        "\"uuid\": \"5678\"}, \"n2\": {\"host\": "
        "\"emdata2.int.janelia.org\", \"port\": 7000, \"uuid\": \"abcd\"}}") {
      std::cout << GetTestJsonString(json) << std::endl;
      LERROR() << "Test failed";
      return false;
    }
  }

  {
    std::map<std::string, TileConfig> config;
    ZJsonObject obj;
    obj.decodeString("{\"d1\": {\"low_quality\": true}, \"t2\": {\"low_quality\": false}}");
    LoadJsonConfig(obj, config);
    if (config.count("d1") == 0) {
      LERROR() << "Test failed";
    } else {
      if (!config.at("d1").isLowQuality()) {
        LERROR() << "Test failed";
      }
    }
    if (config.count("t2") == 0) {
      LERROR() << "Test failed";
    } else {
      if (config.at("t2").isLowQuality()) {
        LERROR() << "Test failed";
      }
    }
  }

  {
    std::map<std::string, ZDvidNode> config;
    ZJsonObject obj;
    obj.decodeString(
          "{\"n1\": {\"address\": \"emdata2.int.janelia.org\", \"port\": 7000, "
          "\"uuid\": \"5678\"}, \"n2\": {\"address\": "
          "\"emdata2.int.janelia.org\", \"port\": 7000, \"uuid\": \"abcd\"}}");
    LoadJsonConfig(obj, config);
    if (config.count("n1") == 0) {
      LERROR() << "Test failed";
    } else {
      if (config.at("n1").getSourceString(false) !=
          "emdata2.int.janelia.org:7000:5678") {
        LERROR() << "Test failed";
      }
    }
    if (config.count("n2") == 0) {
      LERROR() << "Test failed";
    } else {
      if (config.at("n2").getSourceString(false) !=
          "emdata2.int.janelia.org:7000:abcd") {
        LERROR() << "Test failed";
      }
    }
  }
#endif
//  std::cout << json.dumpString(0) << std::endl;

  return true;
}

void ZDvidTarget::updateData(const ZJsonObject &obj)
{
  if (obj.hasKey("uuid")) {
    setUuid(ZJsonParser::stringValue(obj["uuid"]));
  }

  if (obj.hasKey(m_bgValueKey)) {
    m_bgValue = ZJsonParser::integerValue(obj[m_bgValueKey]);
  }
  if (obj.hasKey(m_bodyLabelNameKey)) {
    setBodyLabelName(ZJsonParser::stringValue(obj[m_bodyLabelNameKey]));
  }
  if (obj.hasKey(m_newSegmentationNameKey)) {
    setSegmentationName(ZJsonParser::stringValue(obj[m_newSegmentationNameKey]));
  } else if (obj.hasKey(m_segmentationNameKey)) {
    setSegmentationName(ZJsonParser::stringValue(obj[m_segmentationNameKey]));
  }
  if (obj.hasKey(m_grayScaleNameKey)) {
    setGrayScaleName(ZJsonParser::stringValue(obj[m_grayScaleNameKey]));
  }
  if (obj.hasKey(m_multiscale2dNameKey)) {
    setMultiscale2dName(
          ZJsonParser::stringValue(obj[m_multiscale2dNameKey]));
  }
  if (obj.hasKey(m_tileConfigKey)) {
    ZJsonObject tileConfigJson = ZJsonObject(obj.value(m_tileConfigKey));
    LoadJsonConfig(tileConfigJson, m_tileConfig);
  }

  if (obj.hasKey(m_roiListKey)) {
    ZJsonArray jsonArray(obj.value(m_roiListKey));
    for (size_t i = 0; i < jsonArray.size(); ++i) {
      addRoiName(ZJsonParser::stringValue(jsonArray.getData(), i));
    }
  }
  if (obj.hasKey(m_roiNameKey)) {
    setRoiName(ZJsonParser::stringValue(obj[m_roiNameKey]));
  }
  if (obj.hasKey(m_synapseNameKey)) {
    setSynapseName(ZJsonParser::stringValue(obj[m_synapseNameKey]));
  }

  if (obj.hasKey(m_todoListNameKey)) {
    setTodoListName(ZJsonParser::stringValue(obj[m_todoListNameKey]));
  }

  if (obj.hasKey(m_defaultSettingKey)) {
    useDefaultDataSetting(ZJsonParser::booleanValue(obj[m_defaultSettingKey]));
  }

  if (obj.hasKey(m_userNameKey)) {
    ZJsonValue value = obj.value(m_userNameKey);
    if (value.isString()) {
      m_userList.insert(ZJsonParser::stringValue(value.getData()));
    } else if (value.isArray()) {
      ZJsonArray nameArray(value);
      for (size_t i = 0; i < nameArray.size(); ++i) {
        m_userList.insert(ZJsonParser::stringValue(nameArray.at(i)));
      }
    }
  }
  if (obj.hasKey(m_supervisorKey)) {
    ZJsonValue value = obj.value(m_supervisorKey);
    if (value.isBoolean()) {
      m_isSupervised= ZJsonParser::booleanValue(value.getData());
    }
  }
  if (obj.hasKey(m_supervisorServerKey)) {
    m_supervisorServer = ZJsonParser::stringValue(obj[m_supervisorServerKey]);
  }

  if (obj.hasKey(m_sourceConfigKey)) {
    LoadJsonConfig(ZJsonObject(obj.value(m_sourceConfigKey)), m_sourceConfig);
  }
}

void ZDvidTarget::loadJsonObject(const ZJsonObject &obj)
{
  clear();

  bool isValidJson = true;

  if (obj.hasKey(m_debugKey)) {
#ifndef _DEBUG_
    isValidJson = !ZJsonParser::booleanValue(obj[m_debugKey]);
#endif
  }

  if (isValidJson) {
    m_node.loadJsonObject(obj);
    m_comment = ZJsonParser::stringValue(obj[m_commentKey]);
    m_name = ZJsonParser::stringValue(obj[m_nameKey]);
    m_localFolder = ZJsonParser::stringValue(obj[m_localKey]);
    m_readOnly = !ZJsonParser::booleanValue(
          obj[m_proofreadingKey], /*default=*/true);
    m_adminToken = ZJsonParser::stringValue(obj[m_adminTokenKey]);
    updateData(obj);
  }
}

std::string ZDvidTarget::getUrl() const
{
  return m_node.getUrl();
}

/*
std::string ZDvidTarget::getLocalLowResGrayScalePath(
    int xintv, int yintv, int zintv) const
{
  if (getLocalFolder().empty()) {
    return "";
  }

  ZString path = getLocalFolder() + "/grayscale/";
  path.appendNumber(xintv);
  path += "_";
  path.appendNumber(yintv);
  path += "_";
  path.appendNumber(zintv);

  return path;
}


std::string ZDvidTarget::getLocalLowResGrayScalePath(
    int xintv, int yintv, int zintv, int z) const
{
  if (getLocalFolder().empty()) {
    return "";
  }

  const int padding = 6;

  ZString path = getLocalLowResGrayScalePath(xintv, yintv, zintv);
  path += "/";
  path.appendNumber(z, padding);
  path += ".tif";

  return path;
}
*/

bool ZDvidTarget::isInferred() const
{
  return m_isInferred;
}

void ZDvidTarget::setInferred(bool status)
{
  m_isInferred = status;
}

void ZDvidTarget::setMappedUuid(
    const std::string &original, const std::string &mapped)
{
  m_node.setMappedUuid(original, mapped);
//  m_orignalUuid = original;
//  setUuid(mapped);
}

std::string ZDvidTarget::getOriginalUuid() const
{
  return m_node.getOriginalUuid();
}

std::string ZDvidTarget::getBodyLabelName() const
{
  if (m_bodyLabelName.empty()) {
    if (getSegmentationType() != ZDvidData::EType::LABELBLK) {
      return getSegmentationName();
    }
//    return ZDvidData::GetName(ZDvidData::ERole::BODY_LABEL);
  } else if (ZDvidData::IsNullName(m_bodyLabelName)) {
    return "";
  }

  return m_bodyLabelName;
}

std::string ZDvidTarget::getBodyLabelName(int zoom) const
{
  std::string name;
  if (zoom == 0) {
    return getBodyLabelName();
  } else if (zoom > 0) {
    name = getBodyLabelName() + "_" + ZString::num2str(zoom);
  }

  return name;
}

bool ZDvidTarget::usingMulitresBodylabel() const
{
  if (m_usingMultresBodyLabel) {
    return getMaxLabelZoom() > 3;
  }

  return false;
}

bool ZDvidTarget::hasBodyLabel() const
{
  return !getBodyLabelName().empty();
}

bool ZDvidTarget::hasSegmentation() const
{
  return !getSegmentationName().empty();
}

bool ZDvidTarget::hasLabelMapData() const
{
  if (hasSegmentation()) {
    if (getSegmentationType() == ZDvidData::EType::LABELMAP) {
      return true;
    }
  }

  return false;
}

bool ZDvidTarget::hasBlockCoding() const
{
  return hasLabelMapData();
}

bool ZDvidTarget::hasSupervoxel() const
{
  return hasLabelMapData();
}

bool ZDvidTarget::isSegmentationSyncable() const
{
  if (hasSegmentation()) {
    return true;
    /*
    if (getSegmentationType() != ZDvidData::EType::TYPE_LABELMAP) {
      return true;
    }
    */
  }

  return false;
}

bool ZDvidTarget::segmentationAsBodyLabel() const
{
  return getSegmentationType() == ZDvidData::EType::LABELARRAY ||
      getSegmentationType() == ZDvidData::EType::LABELMAP;
}

bool ZDvidTarget::hasSparsevolSizeApi() const
{
  return getSegmentationType() == ZDvidData::EType::LABELARRAY ||
      getSegmentationType() == ZDvidData::EType::LABELMAP;
}

bool ZDvidTarget::hasMultiscaleSegmentation() const
{
  if (hasSegmentation()) {
    return getSegmentationType() == ZDvidData::EType::LABELARRAY ||
        getSegmentationType() == ZDvidData::EType::LABELMAP;
  }

  return false;
}

bool ZDvidTarget::hasCoarseSplit() const
{
  if (hasSegmentation()) {
    return getSegmentationType() != ZDvidData::EType::LABELMAP;
  }

  return false;
}

void ZDvidTarget::setSegmentationType(ZDvidData::EType type)
{
  m_segmentationType = type;
}

ZDvidData::EType ZDvidTarget::getSegmentationType() const
{
  return m_segmentationType;
}

std::string ZDvidTarget::getSegmentationName() const
{ 
  if (m_segmentationName.empty()) {
    return ZDvidData::GetName(ZDvidData::ERole::SEGMENTATION);
  } else if (ZDvidData::IsNullName(m_segmentationName)) {
    return "";
  }

  return m_segmentationName;
}

std::string ZDvidTarget::GetMultiscaleDataName(
    const std::string &dataName, int zoom)
{
  std::string name = dataName;

  if (!name.empty() && zoom > 0) {
    name = name + "_" + ZString::num2str(zoom);
  }

  return name;
}

std::string ZDvidTarget::getSegmentationName(int zoom) const
{
  return GetMultiscaleDataName(getSegmentationName(), zoom);
  /*
  std::string name = getLabelBlockName();
  if (!name.empty() && zoom > 0) {
    name = name + "_" + ZString::num2str(zoom);
  }

  return name;
  */
}

std::string ZDvidTarget::getGrayScaleName(int zoom) const
{
  return GetMultiscaleDataName(getGrayScaleName(), zoom);
}

std::string ZDvidTarget::getValidGrayScaleName(int zoom) const
{
  if (zoom < 0 || zoom > getMaxGrayscaleZoom()) {
    return "";
  }

  return getGrayScaleName(zoom);
}

std::string ZDvidTarget::getValidSegmentationName(int zoom) const
{
  if (zoom < 0 || zoom > getMaxLabelZoom()) {
    return "";
  }

  if (hasMultiscaleSegmentation()) {
    return getSegmentationName();
  }

  return getSegmentationName(zoom);
}

void ZDvidTarget::setSegmentationName(const std::string &name)
{
  m_segmentationName = name;
}

std::string ZDvidTarget::getMultiscale2dName() const
{
  return m_multiscale2dName;
}

bool ZDvidTarget::hasTileData() const
{
  return !getMultiscale2dName().empty();
}

bool ZDvidTarget::isTileLowQuality() const
{
  return isLowQualityTile(getMultiscale2dName());
}

std::string ZDvidTarget::getBodyInfoName() const
{
  return ZDvidData::GetName(ZDvidData::ERole::BODY_INFO,
                            ZDvidData::ERole::SPARSEVOL,
                            getBodyLabelName());
}

bool ZDvidTarget::hasSynapse() const
{
  return !getSynapseName().empty();
}

bool ZDvidTarget::hasSynapseLabelsz() const
{
  return m_hasSynapseLabelsz && !getSynapseLabelszName().empty();
}

void ZDvidTarget::enableSynapseLabelsz(bool on)
{
  m_hasSynapseLabelsz = on;
}

void ZDvidTarget::TileConfig::loadJsonObject(const ZJsonObject &jsonObj)
{
  if (jsonObj.hasKey("low_quality")) {
    m_lowQuality = ZJsonParser::booleanValue(jsonObj["low_quality"]);
  } else {
    m_lowQuality = false;
  }
}

ZJsonObject ZDvidTarget::TileConfig::toJsonObject() const
{
  ZJsonObject obj;
  obj.setEntry("low_quality", isLowQuality());

  return obj;
}

bool ZDvidTarget::isLowQualityTile(const std::string &name) const
{
  bool lowQuality = false;

  if (!name.empty()) {
    if (m_tileConfig.count(name) > 0) {
      lowQuality = m_tileConfig.at(name).isLowQuality();
    }
    /*
    if (m_tileConfig.hasKey(name.c_str())) {
      ZJsonObject obj(m_tileConfig.value(name.c_str()));
      if (obj.hasKey("low_quality")) {
        lowQuality = ZJsonParser::booleanValue(obj["low_quality"]);
      }
    }
    */
  }

  return lowQuality;
}

bool ZDvidTarget::hasGrayScaleData() const
{
  if (getGrayScaleName() == "*" || getGrayScaleName().empty()) {
    return false;
  }

  return true;
}

std::string ZDvidTarget::getGrayScaleName() const
{
//  if (m_grayScaleName.empty()) {
//    return ZDvidData::GetName(ZDvidData::ROLE_GRAY_SCALE);
//  }

  return m_grayscaleName;
}

void ZDvidTarget::setGrayScaleName(const std::string &name)
{
  m_grayscaleName = name;
}

void ZDvidTarget::setBodyLabelName(const std::string &name)
{
  m_bodyLabelName = name;
}

void ZDvidTarget::setNullBodyLabelName()
{
  setBodyLabelName("*");
}

void ZDvidTarget::setNullSegmentationName()
{
  setSegmentationName("*");
}

void ZDvidTarget::setMultiscale2dName(const std::string &name)
{
  m_multiscale2dName = name;
}

void ZDvidTarget::setDefaultMultiscale2dName()
{
  m_multiscale2dName = ZDvidData::GetName(ZDvidData::ERole::MULTISCALE_2D);
}

void ZDvidTarget::configTile(const std::string &name, bool lowQuality)
{
  if (!name.empty()) {
    if (m_tileConfig.count(name) == 0) {
      m_tileConfig[name] = TileConfig();
    }
    m_tileConfig[name].setLowQuality(lowQuality);
#if 0
    if (lowQuality) {
      ZJsonObject obj;
      if (m_tileConfig.hasKey(name.c_str())) {
        obj = ZJsonObject(m_tileConfig.value(name.c_str()));
      } else {
        m_tileConfig.setEntry(name.c_str(), obj);
      }
      obj.setEntry("low_quality", true);
    } else {
      if (m_tileConfig.hasKey(name.c_str())) {
        ZJsonObject obj(m_tileConfig.value(name.c_str()));
        if (obj.hasKey("low_quality")) {
          obj.setEntry("low_quality", false);
        }
      }
    }
#endif
  }
}

std::string ZDvidTarget::getRoiName(size_t index) const
{
  return m_roiList[index];
}

std::string ZDvidTarget::getRoiName() const
{
  return m_roiName;
}

void ZDvidTarget::setRoiName(const std::string &name)
{
  m_roiName = name;
}

void ZDvidTarget::addRoiName(const std::string &name)
{
  m_roiList.push_back(name);
//  m_roiName = name;
}

std::string ZDvidTarget::getSynapseName() const
{
  if (m_synapseName.empty()) {
    return ZDvidData::GetName(ZDvidData::ERole::SYNAPSE);
  }

  return m_synapseName;
}

std::string ZDvidTarget::getBookmarkName() const
{
  return ZDvidData::GetName(ZDvidData::ERole::BOOKMARK);
}

std::string ZDvidTarget::getBookmarkKeyName() const
{
  return ZDvidData::GetName(ZDvidData::ERole::BOOKMARK_KEY);
}

std::string ZDvidTarget::getSkeletonName() const
{
  return ZDvidData::GetName(
        ZDvidData::ERole::SKELETON, ZDvidData::ERole::SPARSEVOL,
        getBodyLabelName());
}

std::string ZDvidTarget::getMeshName() const
{
  return ZDvidData::GetName(ZDvidData::ERole::MESH,
                            ZDvidData::ERole::SPARSEVOL,
                            getBodyLabelName());
}

std::string ZDvidTarget::getMeshName(int zoom) const
{
  ZString name = getMeshName();
  if (!name.empty() && zoom > 0) {
    name += "_";
    name.appendNumber(zoom);
  }

  return name;
}

std::string ZDvidTarget::getThumbnailName() const
{
  return ZDvidData::GetName(ZDvidData::ERole::THUMBNAIL, ZDvidData::ERole::SPARSEVOL,
                            getBodyLabelName());
}

void ZDvidTarget::setTodoListName(const std::string &name)
{
  m_todoListName = name;
}

std::string ZDvidTarget::getTodoListName() const
{
  if (!m_todoListName.empty()) {
    return m_todoListName;
  }

  return ZDvidData::GetName(ZDvidData::ERole::TODO_LIST,
                            ZDvidData::ERole::SPARSEVOL,
                            getBodyLabelName());
}

bool ZDvidTarget::isDefaultTodoListName() const
{
  return m_todoListName.empty();
}

bool ZDvidTarget::isDefaultBodyLabelName() const
{
  return m_isDefaultBodyLabel;
}

void ZDvidTarget::setDefaultBodyLabelFlag(bool on)
{
  m_isDefaultBodyLabel = on;
}

std::string ZDvidTarget::getBodyAnnotationName() const
{
  return ZDvidData::GetName(ZDvidData::ERole::BODY_ANNOTATION,
                            ZDvidData::ERole::SPARSEVOL, getBodyLabelName());
}

void ZDvidTarget::setSynapseName(const std::string &name)
{
  m_synapseName = name;
}


std::string ZDvidTarget::getSplitLabelName() const
{
  return ZDvidData::GetName(
        ZDvidData::ERole::SPLIT_LABEL, ZDvidData::ERole::SPARSEVOL,
        getBodyLabelName());
}
/*
void ZDvidTarget::setUserName(const std::string &name)
{
  m_userName = name;
}
*/

const std::set<std::string>& ZDvidTarget::getUserNameSet() const
{
  return m_userList;
}


void ZDvidTarget::setSourceConfig(const ZJsonObject &config)
{
  LoadJsonConfig(config, m_sourceConfig);
//  m_sourceConfig = config;
}

ZJsonObject ZDvidTarget::getSourceConfigJson() const
{
  return MakeJsonObject(m_sourceConfig);
//  return m_sourceConfig;
}

void ZDvidTarget::setSource(const char *key, const ZDvidNode &node)
{
  if (node == m_node || !node.isValid()) {
    m_sourceConfig.erase(key);
//    m_sourceConfig.removeKey(key);
  } else {
    m_sourceConfig[key] = node;
//    m_sourceConfig.setEntry(key, node.toJsonObject().getData());
  }
}

void ZDvidTarget::setGrayScaleSource(const ZDvidNode &node)
{
  setSource(m_grayScaleNameKey, node);
}

void ZDvidTarget::setAdminToken(const std::string &token)
{
  m_adminToken = token;
}

std::string ZDvidTarget::getAdminToken() const
{
  return m_adminToken;
}

bool ZDvidTarget::hasAdminToken() const
{
  return !m_adminToken.empty();
}

ZDvidNode ZDvidTarget::getSource(const char *key) const
{
  if (m_sourceConfig.count(key) > 0) {
    ZDvidNode node = m_sourceConfig.at(key);
//    ZJsonObject obj(m_sourceConfig.value(key));
//    node.loadJsonObject(obj);
    if (node.isValid()) {
      return node;
    }
  }

  return m_node;
}

ZDvidNode ZDvidTarget::getGrayScaleSource() const
{
  return getSource(m_grayScaleNameKey);
}


ZDvidTarget ZDvidTarget::getGrayScaleTarget() const
{
  ZDvidNode node = getGrayScaleSource();

  ZDvidTarget target(node);
  target.setGrayScaleName(getGrayScaleName());

  return target;
}

std::vector<ZDvidTarget> ZDvidTarget::getGrayScaleTargetList() const
{
  std::vector<ZDvidTarget> targetList;
  targetList.push_back(getGrayScaleTarget());

  return targetList;
}

void ZDvidTarget::clearGrayScale()
{
  setGrayScaleName("");
  m_sourceConfig.erase(m_grayScaleNameKey);
}

ZDvidTarget ZDvidTarget::getTileTarget() const
{
  ZDvidNode node = getTileSource();

  ZDvidTarget target(node);
  target.setMultiscale2dName(getMultiscale2dName());

  return target;
}

ZDvidNode ZDvidTarget::getTileSource() const
{
  return getSource(m_multiscale2dNameKey);
}

void ZDvidTarget::setTileSource(const ZDvidNode &node)
{
  setSource(m_multiscale2dNameKey, node);
}


void ZDvidTarget::prepareGrayScale()
{
  if (m_sourceConfig.count(m_grayScaleNameKey) > 0) {
    m_node = m_sourceConfig[m_grayScaleNameKey];
//    ZJsonObject nodeJson(m_sourceConfig.value(m_grayScaleNameKey));
//    m_node.loadJsonObject(nodeJson);
  }
}

void ZDvidTarget::prepareTile()
{
  if (m_sourceConfig.count(m_multiscale2dNameKey) > 0) {
    m_node = m_sourceConfig[m_multiscale2dNameKey];
//    ZJsonObject nodeJson(m_sourceConfig.value(m_multiscale2dNameKey));
//    m_node.loadJsonObject(nodeJson);
  }
}

bool ZDvidTarget::IsDvidTarget(const std::string &source)
{
  return ZString(source).startsWith("http:");
}
