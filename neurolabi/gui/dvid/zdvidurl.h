#ifndef ZDVIDURL_H
#define ZDVIDURL_H

#include <string>

#include "common/neutube_def.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdviddata.h"
#include "dvid/zdviddef.h"
#include "geometry/zintcuboid.h"

class ZIntPoint;

class ZDvidUrl
{
public:
  ZDvidUrl();
  ZDvidUrl(const std::string &serverAddress, const std::string &uuid, int port);
  ZDvidUrl(const ZDvidTarget &target);
  ZDvidUrl(const ZDvidTarget &target, const std::string &uuid);


  void setDvidTarget(const ZDvidTarget &target);
  void setDvidTarget(const ZDvidTarget &target, const std::string &uuid);

  std::string getApiLoadUrl() const;
  std::string getNodeUrl() const;
  std::string getDataUrl(const std::string &dataName) const;
  std::string getDataUrl(ZDvidData::ERole role) const;
  std::string getDataUrl(
      ZDvidData::ERole role, ZDvidData::ERole prefixRole,
      const std::string &prefixName);
  std::string getInfoUrl(const std::string &dataName) const;
  std::string getInfoUrl() const;
  std::string getHelpUrl() const;
  std::string getServerInfoUrl() const;
  std::string getApiUrl() const;
  std::string getRepoUrl() const;
  std::string getInstanceUrl() const;
  std::string getOldMasterUrl() const;
  std::string getMasterUrl() const;
  std::string getMirrorInfoUrl() const;
  std::string getDefaultDataInstancesUrl() const;
  std::string getDataMapUrl() const;
  std::string getDataConfigUrl(const std::string &userName) const;

  std::string getCommitInfoUrl() const;

  std::string getSkeletonUrl(const std::string &bodyLabelName) const;
  std::string getSkeletonUrl(
      uint64_t bodyId, const std::string &bodyLabelName) const;
  std::string getSkeletonUrl() const;
  std::string getSkeletonUrl(uint64_t bodyId) const;

  std::string getSkeletonConfigUrl(const std::string &bodyLabelName);

  std::string getMeshUrl();
  std::string getMeshUrl(uint64_t bodyId, int zoom);
  std::string getMeshInfoUrl(uint64_t bodyId, int zoom);
  static std::string GetMeshInfoUrl(const std::string &meshUrl);
//  std::string getThumbnailUrl(const std::string &bodyLableName) const;
//  std::string getThumbnailUrl(int bodyId) const;

  std::string getMeshesTarsUrl();
  std::string getMeshesTarsUrl(uint64_t bodyId);
  std::string getMeshesTarsKeyRangeUrl(uint64_t bodyId1, uint64_t bodyId2);

  std::string getTarSupervoxelsUrl();
  std::string getTarSupervoxelsUrl(uint64_t bodyId);
  std::string getSupervoxelMeshUrl(uint64_t bodyId);
  std::string getSupervoxelMapUrl(uint64_t bodyId);

  std::string getThumbnailUrl(const std::string &bodyLabelName) const;
  std::string
  getThumbnailUrl(uint64_t bodyId, const std::string &bodyLabelName) const;
  std::string getThumbnailUrl(uint64_t bodyId) const;

  std::string getSp2bodyUrl() const;
  std::string getSp2bodyUrl(const std::string &suffix) const;

//  std::string getSparsevolUrl() const;
//  std::string getSparsevolUrl(int bodyId) const;

  std::string getSupervoxelUrl(const std::string &dataName) const;
  std::string getSupervoxelUrl(uint64_t bodyId, const std::string &dataName) const;
  std::string getSupervoxelUrl(uint64_t bodyId) const;
  std::string getSupervoxelUrl(uint64_t bodyId, int z, neutu::EAxis axis) const;
  std::string getSupervoxelUrl(
      uint64_t bodyId, int minZ, int maxZ, neutu::EAxis axis) const;
  std::string getSupervoxelUrl(uint64_t bodyId, const ZIntCuboid &box) const;
  std::string getSupervoxelUrl(uint64_t bodyId, int zoom, const ZIntCuboid &box) const;
  std::string getMultiscaleSupervoxelUrl(uint64_t bodyId, int zoom) const;
//  std::string getSupervoxelSizeUrl(uint64_t bodyId) const;

  struct SparsevolConfig {
    uint64_t bodyId = 0;
    ZIntCuboid range;
    int zoom = 0;
    std::string format;
    neutu::EBodyLabelType labelType;
  };

  std::string getSparsevolUrl(const std::string &dataName) const;
  std::string getSparsevolUrl(uint64_t bodyId, const std::string &dataName) const;
  std::string getSparsevolUrl(uint64_t bodyId) const;
  std::string getSparsevolUrl(uint64_t bodyId, int z, neutu::EAxis axis) const;
  std::string getSparsevolUrl(
      uint64_t bodyId, int minZ, int maxZ, neutu::EAxis axis) const;
  std::string getSparsevolUrl(uint64_t bodyId, const ZIntCuboid &box) const;
  std::string getSparsevolUrl(uint64_t bodyId, int zoom, const ZIntCuboid &box) const;
  std::string getMultiscaleSparsevolUrl(uint64_t bodyId, int zoom) const;
  std::string getSparsevolSizeUrl(uint64_t bodyId, neutu::EBodyLabelType labelType) const;

  std::string getSparsevolUrl(const SparsevolConfig &config);

  std::string getSparsevolLastModUrl(uint64_t bodyId);

//  std::string getCoarseSparsevolUrl() const;
//  std::string getCoarseSparsevolUrl(int bodyId) const;

  std::string getCoarseSupervoxelUrl(const std::string &dataName) const;
  std::string getCoarseSupervoxelUrl(uint64_t bodyId, const std::string &dataName) const;
  std::string getCoarseSupervoxelUrl(uint64_t bodyId) const;

  std::string getCoarseSparsevolUrl(const std::string &dataName) const;
  std::string getCoarseSparsevolUrl(uint64_t bodyId, const std::string &dataName) const;
  std::string getCoarseSparsevolUrl(uint64_t bodyId) const;

  std::string getCoarseSparsevolUrl(
      uint64_t bodyId, const std::string &dataName, neutu::EBodyLabelType labelType) const;


  std::string getGrayscaleUrl() const;
  std::string getGrayscaleUrl(int sx, int sy, int x0, int y0, int z0,
                              const std::string &format = "") const;
  std::string getGrayscaleUrl(int sx, int sy, int sz, int x0, int y0, int z0)
   const;
  std::string getGrayScaleBlockUrl(
      int ix, int iy, int iz, int blockNumber = 1) const;

  std::string getSegmentationUrl() const;
  std::string getLabels64Url() const;
  std::string getLabels64Url(int zoom) const;
  std::string getLabels64Url(const std::string &name,
      int sx, int sy, int sz, int x0, int y0, int z0) const;
  std::string getLabels64Url(
      int sx, int sy, int sz, int x0, int y0, int z0, int zoom = 0) const;
  /*
  std::string getLabelSliceUrl(const std::string &name, int dim1, int dim2,
                               int )
                               */

  std::string getKeyUrl(const std::string &name, const std::string &key) const;
  std::string getKeyRangeUrl(
      const std::string &name,
      const std::string &key1, const std::string &key2) const;
  std::string getAllKeyUrl(const std::string &name) const;
  std::string getKeyValuesUrl(const std::string &name) const;

  std::string getBodyAnnotationUrl(const std::string &bodyLabelName) const;
  std::string getBodyAnnotationUrl(
      uint64_t bodyId, const std::string &bodyLabelName) const;
  std::string getBodyAnnotationUrl(uint64_t bodyId) const;

  std::string getBodyInfoUrl(const std::string &dataName) const;
  std::string getBodyInfoUrl(uint64_t bodyId, const std::string &dataName) const;
  std::string getBodyInfoUrl(uint64_t bodyId) const;

  std::string getBodySizeUrl(uint64_t bodyId) const;
  std::string getSupervoxelSizeUrl(uint64_t bodyId) const;

  std::string getBoundBoxUrl() const;
  std::string getBoundBoxUrl(int z) const;

  std::string getLocalBodyIdUrl(int x, int y, int z) const;
  std::string getLocalBodyIdArrayUrl() const;

  std::string getLocalSupervoxelIdUrl(int x, int y, int z) const;

  std::string getBodyLabelUrl() const;
  std::string getBodyLabelUrl(const std::string &dataName) const;
    /*
  std::string getBodyLabelUrl(const std::string &dataName,
      int x0, int y0, int z0, int width, int height, int depth) const;
  std::string getBodyLabelUrl(
      int x0, int y0, int z0, int width, int height, int depth) const;
      */

  std::string getBodyListUrl(int minSize) const;
  std::string getBodyListUrl(int minSize, int maxSize) const;

  //std::string getMaxBodyIdUrl() const;

  std::string getSynapseListUrl() const;
  std::string getSynapseAnnotationUrl(const std::string &name) const;
  std::string getSynapseAnnotationUrl() const;

  std::string getMergeUrl(const std::string &dataName) const;
  std::string getSplitUrl(
      const std::string &dataName, uint64_t originalLabel) const;
  std::string getSplitSupervoxelUrl(
      const std::string &dataName, uint64_t originalLabel) const;
  std::string getSplitUrl(
      const std::string &dataName, uint64_t originalLabel,
      uint64_t newLabel) const;
  std::string getCoarseSplitUrl(
      const std::string &dataName, uint64_t originalLabel) const;

  //std::string getMergeOperationUrl(const std::string &dataName) const;
  std::string getMergeOperationUrl(const std::string &userName) const;

  std::string getTileUrl(const std::string &dataName) const;
  std::string getTileUrl(const std::string &dataName, int resLevel) const;
  std::string getTileUrl(
      const std::string &dataName, int resLevel,
      int xi0, int yi0, int z0) const;

  std::string getRepoInfoUrl() const;
  std::string getLockUrl() const;
  std::string getBranchUrl() const;

  std::string getRoiUrl(const std::string &dataName) const;
  std::string getRoiMeshUrl(const std::string &key) const;

  /*!
   * \brief Get the url for an roi that will be listed in GUI.
   */
  std::string getManagedRoiUrl(const std::string &key) const;

  std::string getBookmarkKeyUrl() const;
  std::string getBookmarkKeyUrl(int x, int y, int z) const;
  std::string getBookmarkKeyUrl(const ZIntPoint &pt) const;

  std::string getBookmarkUrl() const;
  std::string getBookmarkUrl(int x, int y, int z) const;
  std::string getBookmarkUrl(int x, int y, int z,
                             int width, int height, int depth) const;
  std::string getBookmarkUrl(
      const ZIntPoint &pt, int width, int height, int depth) const;
   std::string getBookmarkUrl(const ZIntCuboid &box) const;

  std::string getCustomBookmarkUrl(const std::string &userName) const;

  std::string getBodyAnnotationName() const;

  std::string getAnnotationUrl(const std::string &dataName) const;
  std::string getAnnotationUrl(
      const std::string &dataName, const std::string tag) const;
  std::string getAnnotationUrl(
      const std::string &dataName, uint64_t label) const;
  std::string getAnnotationUrl(
      const std::string &dataName, int x, int y, int z) const;
  std::string getAnnotationElementsUrl(const std::string &dataName) const;
  std::string getAnnotationDeleteUrl(const std::string &dataName) const;
  std::string getAnnotationDeleteUrl(const std::string &dataName,
                                     int x, int y, int z) const;
  std::string getAnnotationUrl(
      const std::string &dataName, int x, int y, int z,
      int width, int height, int depth) const;
  std::string getAnnotationUrl(
      const std::string &dataName, const ZIntCuboid &box) const;

  std::string getDataSyncUrl(
      const std::string &dataName, const std::string &queryString) const;
  std::string getAnnotationSyncUrl(const std::string &dataName) const;
  std::string getAnnotationSyncUrl(
      const std::string &dataName, const std::string &queryString) const;
  std::string getLabelszSyncUrl(const std::string &dataName) const;

  std::string getLabelMappingUrl() const;

  std::string getSynapseUrl() const;
  std::string getSynapseUrl(int x, int y, int z) const;
  std::string getSynapseUrl(const ZIntPoint &pos) const;
  std::string getSynapseUrl(const ZIntPoint &pos,
                            int width, int height, int depth) const;
  std::string getSynapseUrl(int x, int y, int z,
                            int width, int height, int depth) const;
  std::string getSynapseUrl(const ZIntCuboid &box) const;
  std::string getSynapseUrl(uint64_t label, bool relation) const;
  std::string getSynapseElementsUrl() const;
  std::string getSynapseMoveUrl(
      const ZIntPoint &from, const ZIntPoint &to) const;

  std::string getTodoListUrl() const;
  std::string getTodoListElementsUrl() const;
  std::string getTodoListDeleteUrl(int x, int y, int z) const;
  std::string getTodoListUrl(const ZIntCuboid &cuboid) const;
  std::string getTodoListUrl(int x, int y, int z,
                            int width, int height, int depth) const;
  std::string getTodoListUrl(int x, int y, int z) const;

  std::string getConfigUrl() const;
  std::string getContrastUrl() const;

  static std::string GetBodyKey(uint64_t bodyId);
  static std::string GetSkeletonKey(uint64_t bodyId);
  static std::string GetMeshKey(uint64_t bodyId);
  static std::string GetMeshInfoKey(uint64_t bodyId);
  static std::string GetTaskKey();

  void setUuid(const std::string &uuid);


  std::string getSynapseLabelszUrl(int n) const;
  std::string getSynapseLabelszBodyUrl(uint64_t bodyId) const;
  std::string getSynapseLabelszBodiesUrl() const;

  static std::string GetLabelszIndexTypeStr(dvid::ELabelIndexType type);
  std::string getSynapseLabelszUrl(int n, dvid::ELabelIndexType indexType) const;
  std::string getSynapseLabelszBodyUrl(
      uint64_t bodyId, dvid::ELabelIndexType indexType) const;
  std::string getSynapseLabelszBodiesUrl(dvid::ELabelIndexType indexType) const;

  std::string getSynapseLabelszThresholdUrl(int threshold) const;
  std::string getSynapseLabelszThresholdUrl(
      int threshold, dvid::ELabelIndexType indexType) const;
  std::string getSynapseLabelszThresholdUrl(
      int threshold, dvid::ELabelIndexType indexType, int offset, int number) const;

public:
  static std::string GetPath(const std::string &url);
  static std::string GetFullUrl(
      const std::string &prefix, const std::string &path);
  /*!
   * \brief Get entry point of getting key value entries
   */
  static std::string GetKeyCommandUrl(const std::string &dataUrl);

  static std::string GetKeyValuesCommandUrl(const std::string &dataUrl);

  static std::string GetTarfileCommandUrl(const std::string &dataUrl);

  static uint64_t GetBodyId(const std::string &url);

  static std::string ExtractSplitTaskKey(const std::string &url);
  static std::string GetResultKeyFromTaskKey(const std::string &key);

  std::string getSplitTaskKey(const uint64_t bodyId) const;
  std::string getSplitResultKey(const uint64_t bodyId) const;

  std::string getTestTaskUrl(const std::string &key);
//  std::string getTestTaskUrl() const;

//  static bool IsSplitTask(const std::string &url);

  static std::string AppendRangeQuery(
      const std::string &url, const ZIntCuboid &box);
  static std::string AppendRangeQuery(
      const std::string &url, int minZ, int maxZ, neutu::EAxis axis, bool exact);

private:
  std::string getSplitUrl(
      const std::string &dataName, uint64_t originalLabel,
      const std::string &command) const;
  static std::string GetServiceResultEndPoint();
//  static std::string AppendQuery(const std::string &url, const std::string query);
//  template<typename T>
//  static std::string AppendQuery(
//      const std::string &url, const std::pair<std::string,T> &query);

//  template<>
//  static std::string AppendQuery<bool>(
//      const std::string &url, const std::pair<std::string, bool> &query);

  static std::string AppendQueryM(
      const std::string &url,
      const std::vector<std::pair<std::string, int>> &query);

public:
  static const std::string SUPERVOXEL_FLAG;

private:
  ZDvidTarget m_dvidTarget;

  static const std::string m_keyCommand;
  static const std::string m_keysCommand;
  static const std::string m_keyRangeCommand;
  static const std::string m_keyValuesCommand;
  static const std::string m_infoCommand;
  static const std::string m_sparsevolCommand;
  static const std::string m_coarseSparsevolCommand;
  static const std::string m_supervoxelCommand;
  static const std::string m_coarseSupervoxelCommand;
  static const std::string m_splitCommand;
  static const std::string m_coarseSplitCommand;
  static const std::string m_splitSupervoxelCommand;
  static const std::string m_labelCommand;
  static const std::string m_labelArrayCommand;
  static const std::string m_roiCommand;
  static const std::string m_annotationElementCommand;
  static const std::string m_annotationElementsCommand;
  static const std::string m_annotationLabelCommand;
  static const std::string m_annotationMoveCommand;
  static const std::string m_annotationTagCommand;
  static const std::string m_labelMappingCommand;
  static const std::string m_tarfileCommand;
};

#endif // ZDVIDURL_H
