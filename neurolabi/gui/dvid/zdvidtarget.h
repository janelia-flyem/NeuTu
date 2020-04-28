#ifndef ZDVIDTARGET_H
#define ZDVIDTARGET_H

#include <string>
#include <set>

#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zdviddata.h"
#include "zdvidnode.h"
#include "zdviddef.h"

/*!
 * \brief The class of representing a dvid node.
 */
class ZDvidTarget
{
public:
  ZDvidTarget();
  ZDvidTarget(const std::string &address, const std::string &uuid, int port = -1);
  ZDvidTarget(const ZDvidNode &node);

  void clear();

  void set(const std::string &address, const std::string &uuid, int port = -1);
  void setServer(const std::string &address);
  void setUuid(const std::string &uuid);
  void setPort(int port);

  void setMappedUuid(const std::string &original, const std::string &mapped);

  bool hasDvidUuid() const;

  /*!
   * \brief Set dvid target from source string
   *
   * The old settings will be cleared after the call.
   *
   * \param sourceString Format: http:host:port:node:segmentation_name:grayscale.
   */
  void setFromSourceString(const std::string &sourceString);


  /*!
   * \brief Set dvid target from source string
   *
   * The old settings will be cleared after the call.
   *
   * \param sourceString Format: http:host:port:node:<dataType>_name.
   */
  void setFromSourceString(
      const std::string &sourceString, dvid::EDataType dataType);

  void setFromUrl_deprecated(const std::string &url);

  std::string getAddress() const {
    return m_node.getHost();
  }

  std::string getHostWithScheme() const;
  std::string getRootUrl() const;

  /*!
   * \brief Get the address with port
   *
   * \return "address[:port]" or empty if the address is empty.
   */
  std::string getAddressWithPort() const;

  inline const std::string& getUuid() const {
    return m_node.getUuid();
  }

  inline const std::string& getComment() const {
    return m_comment;
  }

  inline const std::string& getName() const {
    return m_name;
  }

  inline int getPort() const {
    return m_node.getPort();
  }

  const ZDvidNode& getNode() const {
    return m_node;
  }

  /*!
   * \brief Check if there is a port
   *
   * A valid port is any non-negative port number.
   *
   * \return true iff the port is available.
   */
  bool hasPort() const;

  inline void setName(const std::string &name) {
    m_name = name;
  }
  inline void setComment(const std::string &comment) {
    m_comment = comment;
  }

  std::string getUrl() const;
//  std::string getUrl(const std::string &dataName) const;

  /*!
   * \brief Get a single string to represent the target
   *
   * \a withHttpPrefix specifies whether the source string contains the "http:"
   * prefix or not. \a uuidBrief specifies the max number of characters of the
   * uuid used in the source string, except when its no greater than 0, the
   * intrinsic uuid will be used.
   *
   * \return "[http:]address:port:uuid". Return empty if the address is empty.
   */
  std::string getSourceString(
      bool withHttpPrefix = true, int uuidBrief = 0) const;

  std::string getGrayscaleSourceString() const;

  /*!
   * \brief Get the API path of a given body
   *
   * The functions does not check if a body exists.
   *
   * \return The path of a certain body.
   */
//  std::string getBodyPath(uint64_t bodyId) const;

  /*!
   * \brief Test if the target is valid
   *
   * \return true iff the target is valid.
   */
  bool isValid() const;

  /*!
   * \brief Get the status of the node.
   *
   * The status of the node is related to the status of the database.
   */
  dvid::ENodeStatus getNodeStatus() const;

  /*!
   * \brief Set the status of the node.
   */
  void setNodeStatus(dvid::ENodeStatus status);

  /*!
   * \brief Load json object
   */
  void loadJsonObject(const ZJsonObject &obj);
  ZJsonObject toJsonObject() const;

  void setFromJson(const std::string &jsonSpec);

  void updateData(const ZJsonObject &obj);

  void loadDvidDataSetting(const ZJsonObject &obj);
  ZJsonObject toDvidDataSetting() const;

  void print() const;

  void setScheme(const std::string &scheme);
  std::string getScheme() const;

  void setMock(bool on);
  bool isMock() const;

  //Special functions
  inline const std::string& getLocalFolder() const {
    return m_localFolder;
  }

  inline void setLocalFolder(const std::string &folder) {
    m_localFolder = folder;
  }

  /*
  std::string getLocalLowResGrayScalePath(
      int xintv, int yintv, int zintv) const;
  std::string getLocalLowResGrayScalePath(
      int xintv, int yintv, int zintv, int z) const;
*/

  inline int getBgValue() const {
    return m_bgValue;
  }

  inline void setBgValue(int v) {
    m_bgValue = v;
  }

//  std::string getName(ZDvidData::ERole role) const;

  std::string getBodyLabelName() const;
  std::string getBodyLabelName(int zoom) const;
  void setBodyLabelName(const std::string &name);

  void setNullBodyLabelName();

  bool hasBodyLabel() const;
  bool hasSegmentation() const;
  bool hasLabelMapData() const;
  bool hasBlockCoding() const;
  bool hasSupervoxel() const;
  bool isSegmentationSyncable() const;
//  bool usingLabelArray() const;
//  bool usingLabelMap() const;
  ZDvidData::EType getSegmentationType() const;
  void setSegmentationType(ZDvidData::EType type);
  bool segmentationAsBodyLabel() const;
  bool hasSparsevolSizeApi() const;
  bool hasMultiscaleSegmentation() const;
  bool hasCoarseSplit() const;
//  void useLabelArray(bool on);
//  void useLabelMap(bool on);

  bool hasSynapse() const;
  bool hasSynapseLabelsz() const;
  void enableSynapseLabelsz(bool on);

  static std::string GetMultiscaleDataName(const std::string &dataName, int zoom);

  std::string getSegmentationName() const;
  std::string getSegmentationName(int zoom) const;
  std::string getValidSegmentationName(int zoom) const;
  void setSegmentationName(const std::string &name);

  void setNullSegmentationName();

  std::string getBodyInfoName() const;

  std::string getMultiscale2dName() const;
  bool isTileLowQuality() const;
  bool hasTileData() const;

  void setMultiscale2dName(const std::string &name);
  void setDefaultMultiscale2dName();
  void configTile(const std::string &name, bool lowQuality);
//  void setLossTileName(const std::string &name);
//  std::string getLosslessTileName() const;
//  std::string getLossTileName() const;
  bool isLowQualityTile(const std::string &name) const;

  bool hasGrayScaleData() const;
  std::string getGrayScaleName() const;
  std::string getGrayScaleName(int zoom) const;
  std::string getValidGrayScaleName(int zoom) const;
  void setGrayScaleName(const std::string &name);

  std::string getRoiName() const;
  void setRoiName(const std::string &name);

  std::string getRoiName(size_t index) const;
  void addRoiName(const std::string &name);

  const std::vector<std::string>& getRoiList() const {
    return m_roiList;
  }

  void setRoiList(const std::vector<std::string> &roiList) {
    m_roiList = roiList;
  }

  std::string getSynapseName() const;
  void setSynapseName(const std::string &name);

  std::string getBookmarkName() const;
  std::string getBookmarkKeyName() const;
  std::string getSkeletonName() const;
  std::string getMeshName() const;
//  std::string getMeshName(int zoom) const;
  std::string getThumbnailName() const;

  std::string getTodoListName() const;
  void setTodoListName(const std::string &name);
  bool isDefaultTodoListName() const;
  bool isDefaultBodyLabelName() const;
  void setDefaultBodyLabelFlag(bool on);

  std::string getBodyAnnotationName() const;

  std::string getSplitLabelName() const;

  const std::set<std::string>& getUserNameSet() const;
  //void setUserName(const std::string &name);

  static bool IsDvidTarget(const std::string &source);

  inline bool isSupervised() const { return m_isSupervised; }
  void enableSupervisor(bool on) {
    m_isSupervised = on;
  }
  const std::string& getSupervisor() const { return m_supervisorServer; }
  void setSupervisorServer(const std::string &server) {
    m_supervisorServer = server;
  }

  inline bool isEditable() const { return m_isEditable; }
  void setEditable(bool on) { m_isEditable = on; }

  bool readOnly() const;
  void setReadOnly(bool readOnly) {
    m_readOnly = readOnly;
  }
  bool isSynapseEditable() const {
    return m_isSynpaseEditable;
  }
  void setSynapseReadonly(bool on);

  bool isSupervoxelView() const;
  void setSupervoxelView(bool on);

  int getMaxLabelZoom() const {
    return m_maxLabelZoom;
  }

  void setMaxLabelZoom(int zoom) {
    m_maxLabelZoom = zoom;
  }

  int getMaxGrayscaleZoom() const {
    return m_maxGrayscaleZoom;
  }

  void setMaxGrayscaleZoom(int zoom) {
    m_maxGrayscaleZoom = zoom;
  }

  bool usingMulitresBodylabel() const;

  void setAdminToken(const std::string &token);
  std::string getAdminToken() const;
  bool hasAdminToken() const;

  bool isInferred() const;
  void setInferred(bool status);

  std::string getOriginalUuid() const;

  std::string getSynapseLabelszName() const;
  void setSynapseLabelszName(const std::string &name);

  bool usingDefaultDataSetting() const {
    return m_usingDefaultSetting;
  }

  void useDefaultDataSetting(bool on) {
    m_usingDefaultSetting = on;
  }

  void setSourceConfig(const ZJsonObject &config);
  ZJsonObject getSourceConfigJson() const;

  /*!
   * \brief Set dvid source of grayscale data
   *
   * If \a node is invalid, the source will be set to the main source.
   *
   * \param node
   */
  void setGrayScaleSource(const ZDvidNode &node);
  void setTileSource(const ZDvidNode &node);
  void prepareGrayScale();
  void prepareTile();

  ZDvidNode getGrayScaleSource() const;
  ZDvidNode getTileSource() const;
  ZDvidTarget getGrayScaleTarget() const;
  ZDvidTarget getTileTarget() const;

  std::vector<ZDvidTarget> getGrayScaleTargetList() const;
  void clearGrayScale();

  static bool Test();

private:
//  void init();
  void setSource(const char *key, const ZDvidNode &node);
  ZDvidNode getSource(const char *key) const;

  class TileConfig {
  public:
    TileConfig() {}
    void loadJsonObject(const ZJsonObject &jsonObj);
    void setLowQuality(bool on) {
      m_lowQuality = on;
    }
    bool isLowQuality() const {
      return m_lowQuality;
    }
    ZJsonObject toJsonObject() const;

  private:
    bool m_lowQuality = false;
  };

public:
  const static char* m_commentKey;
  const static char* m_nameKey;
  const static char* m_localKey;
  const static char* m_debugKey;
  const static char* m_bgValueKey;
  const static char* m_grayScaleNameKey;
  const static char* m_bodyLabelNameKey;
  const static char* m_segmentationNameKey;
  const static char* m_newSegmentationNameKey;
  const static char* m_multiscale2dNameKey;
  const static char* m_tileConfigKey;
  const static char* m_roiListKey;
  const static char* m_roiNameKey;
  const static char* m_synapseNameKey;
  const static char* m_defaultSettingKey;
  const static char* m_userNameKey;
  const static char* m_supervisorKey;
  const static char* m_supervisorServerKey;
  const static char* m_maxLabelZoomKey;
  const static char* m_maxGrayscaleZoomKey;
  const static char* m_synapseLabelszKey;
  const static char* m_todoListNameKey;
  const static char* m_sourceConfigKey;
  const static char* m_proofreadingKey;
  const static char* m_adminTokenKey;

private:
  ZDvidNode m_node;
  std::string m_name;
  std::string m_comment;
  std::string m_localFolder;

  std::string m_bodyLabelName;
  std::string m_segmentationName;
  std::string m_multiscale2dName; //default lossless tile name
  std::string m_grayscaleName;
  bool m_hasSynapseLabelsz = true;
  std::string m_synapseLabelszName;
  std::string m_roiName;
  std::string m_todoListName;
  std::vector<std::string> m_roiList;
  std::string m_synapseName;
  std::string m_adminToken;

  std::map<std::string, TileConfig> m_tileConfig; //used when m_multiscale2dName is empty
//  ZJsonObject m_tileConfig;
  std::map<std::string, ZDvidNode> m_sourceConfig;
//  ZJsonObject m_sourceConfig;

  std::set<std::string> m_userList;
  bool m_isSupervised = true;
  std::string m_supervisorServer;
  int m_maxLabelZoom = 0;
  int m_maxGrayscaleZoom = 0;
  bool m_usingMultresBodyLabel = true;
  bool m_usingDefaultSetting = false;
  bool m_isSynpaseEditable = true;
  ZDvidData::EType m_segmentationType = ZDvidData::EType::LABELBLK;
//  bool m_usingLabelArray = false;
//  bool m_usingLabelMap = false;
  bool m_isDefaultBodyLabel = false;
  bool m_supervoxelView = false;
//  std::string m_userName;
//  std::string m_tileName;

  int m_bgValue = 255; //grayscale background

  bool m_isEditable = true; //if the configuration is editable
  bool m_readOnly = false; //if the database is readonly
  dvid::ENodeStatus m_nodeStatus = dvid::ENodeStatus::OFFLINE; //Status of the node
//  std::string m_orignalUuid;
  bool m_isInferred = false;
};

#endif // ZDVIDTARGET_H
