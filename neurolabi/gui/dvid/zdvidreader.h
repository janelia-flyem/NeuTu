#ifndef ZDVIDREADER_H
#define ZDVIDREADER_H

#include <QObject>
#include <QEventLoop>
#include <QTimer>
#include <QImage>
#include <QMutex>
#include <QMutexLocker>
#include <QByteArray>

#include <string>
#include <vector>

//#include "zdvidclient.h"
#include "flyem/zflyem.h"
#include "zclosedcurve.h"
#include "dvid/zdvidinfo.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidsynapse.h"
#include "dvid/zdvidbufferreader.h"
#include "dvid/zdvidurl.h"


#if defined(_ENABLE_LOWTIS_)
#include <lowtis/LowtisConfig.h>
#endif

class ZDvidFilter;
class ZArray;
class ZJsonObject;
class ZFlyEmNeuronBodyInfo;
class ZDvidTile;
class ZDvidTileInfo;
class ZSwcTree;
class ZObject3dScan;
class ZSparseStack;
class ZDvidVersionDag;
class ZDvidSparseStack;
class ZFlyEmBodyAnnotation;
class ZFlyEmBookmark;
class ZFlyEmToDoItem;
class ZDvidRoi;
class ZObject3dScanArray;
class ZMesh;
class ZStack;
class ZAffineRect;

struct archive;

namespace libdvid{
class DVIDNodeService;
class DVIDConnection;
}

namespace lowtis {
class ImageService;
}

/*!
 * \brief The class for reading data from DVID
 */
class ZDvidReader/* : public QObject*/
{
//  Q_OBJECT
public:
  explicit ZDvidReader(/*QObject *parent = 0*/);
  ~ZDvidReader();

  /*!
   * \brief Open a dvid node to read.
   *
   * It returns true iff the node is opened correctly. The user can also use
   * \a good() or \a isReady() to check if the node is opened later.
   *
   * \param serverAddress Host name of the server.
   * \param uuid UUID of the node
   * \param port Port of the server.
   *
   * \return true iff the node is opened correctly
   */
  bool open(const QString &serverAddress, const QString &uuid,
            int port = -1);

  /*!
   * \brief Open a dvid node to read.
   *
   * This function will try to infer the real node and data instances from the
   * settings in \a target.
   */
  bool open(const ZDvidTarget &target);

  /*!
   * \brief Open a dvid node defined by a source string
   *
   * \param sourceString the format of the source string is defined in the ZDvidTarget class.
   */
  bool open(const QString &sourceString);

  /*!
   * \brief Open a target as it is.
   *
   * No inference is applied.
   */
  bool openRaw(const ZDvidTarget &target);

  void clear();

  int getStatusCode() const;
  void setStatusCode(int code) const;

  /*!
   * \brief Check if the reader is ready to use
   *
   * \return true iff the reader is ready.
   */
  bool isReady() const;

  std::string readNodeInfo() const;

  std::string getErrorMsg() const {
    return m_errorMsg;
  }

  ZDvid::ENodeStatus getNodeStatus() const;
  void updateNodeStatus();

  ZDvidBufferReader& getBufferReader() const {
    return m_bufferReader;
  }

  std::vector<std::string> readDataInstances(const std::string &type);

  //ZSwcTree* readSwc(const QString &key);
  ZSwcTree *readSwc(uint64_t bodyId) const;
//  ZObject3dScan readBody(uint64_t bodyId, bool canonizing);


  uint64_t readParentBodyId(uint64_t spId) const;

  ZObject3dScan* readBody(
      uint64_t bodyId, bool canonizing, ZObject3dScan *result) const;
  ZObject3dScan* readBody(
      uint64_t bodyId, flyem::EBodyLabelType labelType,
      bool canonizing, ZObject3dScan *result) const;

  ZObject3dScan* readBodyDs(
      uint64_t bodyId, bool canonizing, ZObject3dScan *result);

  ZObject3dScan* readBodyDs(
      uint64_t bodyId, int xIntv, int yIntv, int zIntv,
      bool canonizing, ZObject3dScan *result);

  ZObject3dScan* readBody(uint64_t bodyId, int z, neutube::EAxis axis,
                          bool canonizing, ZObject3dScan *result) const;
  ZObject3dScan* readBody(uint64_t bodyId, flyem::EBodyLabelType labelType,
                          int z, neutube::EAxis axis,
                          bool canonizing, ZObject3dScan *result) const;


  ZObject3dScan* readBody(uint64_t bodyId, int minZ, int maxZ,
                          bool canonizing,
                          neutube::EAxis axis, ZObject3dScan *result) const;
  ZObject3dScan* readBody(uint64_t bodyId, flyem::EBodyLabelType labelType,
                          int minZ, int maxZ, bool canonizing,
                          neutube::EAxis axis, ZObject3dScan *result) const;


  ZObject3dScan* readBody(uint64_t bodyId, const ZIntCuboid &box, bool canonizing,
      ZObject3dScan *result) const;
  ZObject3dScan* readBody(uint64_t bodyId, flyem::EBodyLabelType labelType,
                          const ZIntCuboid &box, bool canonizing,
                          ZObject3dScan *result) const;

  ZObject3dScan* readBody(uint64_t bodyId, flyem::EBodyLabelType labelType,
                          int zoom, const ZIntCuboid &box, bool canonizing,
                          ZObject3dScan *result) const;

  ZObject3dScan* readBodyWithPartition(uint64_t bodyId, ZObject3dScan *result) const;
  ZObject3dScan* readBodyWithPartition(
      uint64_t bodyId, flyem::EBodyLabelType labelType, ZObject3dScan *result) const;

  ZObject3dScan* readMultiscaleBody(
      uint64_t bodyId, int zoom, bool canonizing, ZObject3dScan *result) const;

  ZObject3dScanArray* readBody(const std::set<uint64_t> &bodySet) const;

  /*!
   * \brief Check the number of blocks of a body
   *
   * \return The number of blocks of a body. It returns -1 if the count cannot
   *         be determined.
   */
  int readBodyBlockCount(uint64_t bodyId) const;

  ZObject3dScan* readSupervoxel(
      uint64_t bodyId, bool canonizing, ZObject3dScan *result) const;

  ZMesh* readMesh(uint64_t bodyId, int zoom) const;
  ZMesh* readMesh(const std::string &data, const std::string &key) const;
  ZMesh* readMeshFromUrl(const std::string &url) const;

  /*!
   * \brief Read meshes from a key-value instance whose values are tar archives of
   * Draco-compressed meshes
   */
  struct archive *readMeshArchiveStart(uint64_t bodyId) const;
  struct archive *readMeshArchiveStart(uint64_t bodyId, size_t &bytesTotal) const;
  ZMesh *readMeshArchiveNext(struct archive *arc) const;
  ZMesh *readMeshArchiveNext(struct archive *arc, size_t &bytesJustRead) const;

  /*!
   * \brief An alternative to repeated calls to readMeshArchiveNext(), which uses
   * std::async and std::future to decompress the meshes in parallel.
   */
  void readMeshArchiveAsync(struct archive *arc, std::vector<ZMesh*>& results,
                            const std::function<void(size_t, size_t)>& progress = {}) const;

  void readMeshArchiveEnd(struct archive *arc) const;

  ZStack* readThumbnail(uint64_t bodyId);

  ZSparseStack* readSparseStack(uint64_t bodyId) const;
  ZDvidSparseStack* readDvidSparseStack(
      uint64_t bodyId, flyem::EBodyLabelType labelType = flyem::LABEL_BODY) const;
//  ZDvidSparseStack* readDvidSparseStack(uint64_t bodyId) const;
  ZDvidSparseStack* readDvidSparseStack(uint64_t bodyId, const ZIntCuboid &range) const;
  ZDvidSparseStack* readDvidSparseStackAsync(
      uint64_t bodyId, flyem::EBodyLabelType labelType = flyem::LABEL_BODY) const;
  ZStack* readGrayScale(
      int x0, int y0, int z0, int width, int height, int depth) const;
  ZStack* readGrayScale(
      const std::string &dataName,
      int x0, int y0, int z0, int width, int height, int depth) const;

  ZStack* readGrayScale(
      int x0, int y0, int z0, int width, int height, int depth, int zoom) const;

#if 0
  ZStack* readGrayScaleOld(
      int x0, int y0, int z0, int width, int height, int depth) const;
#endif
  ZStack* readGrayScale(const ZIntCuboid &cuboid) const;
  ZStack* readGrayScaleBlock(
      const ZIntPoint &blockIndex, const ZDvidInfo &dvidInfo) const;
  std::vector<ZStack*> readGrayScaleBlock(
      const ZIntPoint &blockIndex, const ZDvidInfo &dvidInfo,
      int blockNumber, int zoom = 0) const;

//  QString readInfo(const QString &dataName) const;

  std::set<uint64_t> readBodyId(
      int x0, int y0, int z0, int width, int height, int depth,
      bool ignoringZero = true);
  std::set<uint64_t> readBodyId(const ZIntPoint &firstCorner,
                                const ZIntPoint &lastCorner,
                                bool ignoringZero = true);
  std::set<uint64_t> readBodyId(size_t minSize);
  std::set<uint64_t> readBodyId(size_t minSize, size_t maxSize);
  std::set<uint64_t> readBodyId(const ZDvidFilter &filter);
  std::set<uint64_t> readAnnnotatedBodySet();

  bool hasKey(const QString &dataName, const QString &key) const;

  QByteArray readKeyValue(const QString &dataName, const QString &key) const;
  QStringList readKeys(const QString &dataName) const;
  QStringList readKeys(const QString &dataName, const QString &minKey);
  QStringList readKeys(const QString &dataName,
                       const QString &minKey, const QString &maxKey) const;
  ZJsonObject readJsonObjectFromKey(
      const QString &dataName, const QString &key) const;

  ZClosedCurve* readRoiCurve(const std::string &key, ZClosedCurve *result);
  ZIntCuboid readBoundBox(int z);

  ZDvidInfo readGrayScaleInfo() const;
  ZDvidInfo readLabelInfo() const;

  ZIntPoint readRoiBlockSize(const std::string &dataName) const;

  ZJsonObject readInfo() const;
  ZJsonObject readInfo(const std::string &dataName) const;

  ZDvidInfo readDataInfo(const std::string &dataName) const;

  bool hasData(const std::string &dataName) const;
  std::string getType(const std::string &dataName) const;

  ZArray* readLabels64(const std::string &dataName, int x0, int y0, int z0,
                       int width, int height, int depth) const;
  ZArray* readLabels64(int x0, int y0, int z0,
                       int width, int height, int depth, int zoom = 0) const;
  ZArray* readLabels64(const ZIntCuboid &box, int zoom = 0) const;

  ZIntPoint readPosition(uint64_t bodyId, int x, int y, int z) const;
  ZIntPoint readPosition(uint64_t bodyId, const ZIntPoint &pt) const;

  /*!
   * \brief Read labels in the zoomed space
   *
   * \param x0 zoomed X
   * \param y0 zoomed Y
   * \param z0 zoomed Z
   * \param width zoomed width
   * \param height zoomed height
   * \param depth zoomed depth
   * \param zoom zoom level
   * \return
   */
  ZArray* readLabels64Raw(
      int x0, int y0, int z0,
      int width, int height, int depth, int zoom = 0) const;

#if defined(_ENABLE_LOWTIS_)
  //Read label data
  ZArray* readLabels64Lowtis(int x0, int y0, int z0,
                             int width, int height, int zoom = 0) const;

  ZArray* readLabels64Lowtis(
      int x0, int y0, int z0,
      int width, int height, int zoom, int cx, int cy, bool centerCut) const;

  /*!
   * (\a x0, \a y0, \a z0) is the retrieval center.
   */
  ZArray *readLabels64Lowtis(int x0, int y0, int z0, double vx1, double vy1, double vz1,
      double vx2, double vy2, double vz2,
      int width, int height, int zoom, int cx, int cy, bool centerCut) const;
  ZArray *readLabels64Lowtis(
      const ZIntPoint &center, const ZPoint &v1, const ZPoint &v2,
      int width, int height, int zoom, int cx, int cy, bool centerCut) const;
  ZArray *readLabels64Lowtis(
      const ZAffineRect &ar, int zoom, int cx, int cy, bool centerCut) const;


  //Read grayscale data
  ZStack *readGrayScaleLowtis(int x0, int y0, int z0,
                              int width, int height, int zoom = 0) const;
  ZStack *readGrayScaleLowtis(
      int x0, int y0, int z0,
      int width, int height, int zoom, int cx, int cy, bool centerCut) const;

  /*!
   * (\a x0, \a y0, \a z0) is the retrieval center.
   */
  ZStack *readGrayScaleLowtis(
      int x0, int y0, int z0, double vx1, double vy1, double vz1,
      double vx2, double vy2, double vz2,
      int width, int height, int zoom, int cx, int cy, bool centerCut) const;

  ZStack *readGrayScaleLowtis(
      const ZAffineRect &ar, int zoom, int cx, int cy, bool centerCut) const;

  ZStack *readGrayScaleLowtis(
      const ZIntPoint &center, const ZPoint &v1, const ZPoint &v2,
      int width, int height, int zoom, int cx, int cy, bool centerCut) const;
#endif
  /*
  ZArray* readLabelSlice(const std::string &dataName, int x0, int y0, int z0,
                         int dim1, int dim2, int width, int height);
                         */

  int readSynapseLabelszBody(
      uint64_t bodyId, ZDvid::ELabelIndexType index) const;
  ZJsonArray readSynapseLabelsz(int n, ZDvid::ELabelIndexType index) const;
  ZJsonArray readSynapseLabelszThreshold(
      int threshold, ZDvid::ELabelIndexType index) const;
  ZJsonArray readSynapseLabelszThreshold(
      int threshold, ZDvid::ELabelIndexType index, int offset, int number) const;

  bool hasSparseVolume() const;
  bool hasSparseVolume(uint64_t bodyId) const;
  bool hasBodyInfo(uint64_t bodyId) const;
  bool hasBody(uint64_t bodyId) const;
//  bool hasSupervoxel(uint64_t bodyId) const;
  size_t readBodySize(uint64_t bodyId) const;

  bool hasGrayscale() const;

  ZIntPoint readBodyLocation(uint64_t bodyId) const;

  bool hasCoarseSparseVolume(uint64_t bodyId) const;

  ZFlyEmNeuronBodyInfo readBodyInfo(uint64_t bodyId);

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  ZDvidTarget& getDvidTarget() {
    return m_dvidTarget;
  }

  uint64_t readMaxBodyId();

  void updateMaxLabelZoom();
  void updateMaxLabelZoom(
      const ZJsonObject &infoJson, const ZDvidVersionDag &dag);
  void updateMaxGrayscaleZoom();
  void updateMaxGrayscaleZoom(
      const ZJsonObject &infoJson, const ZDvidVersionDag &dag);

  uint64_t readBodyIdAt(int x, int y, int z) const;
  uint64_t readBodyIdAt(const ZIntPoint &pt) const;
  std::vector<uint64_t> readBodyIdAt(
      const std::vector<ZIntPoint> &ptArray) const;
  std::vector<std::vector<uint64_t> > readBodyIdAt(
      const std::vector<std::vector<ZIntPoint> > &ptArray) const;
  template <typename InputIterator>
  std::vector<uint64_t> readBodyIdAt(
      const InputIterator &first, const InputIterator &last) const;

  uint64_t readSupervoxelIdAt(int x, int y, int z) const;
  uint64_t readSupervoxelIdAt(const ZIntPoint &pt) const;

  ZDvidTileInfo readTileInfo(const std::string &dataName) const;

  //ZDvidTile *readTile(const std::string &dataName, int resLevel,
  //                   int xi0, int yi0, int z0) const;
  ZDvidTile *readTile(int resLevel, int xi0, int yi0, int z0) const;

  ZDvidVersionDag readVersionDag(const std::string &uuid) const;
  ZDvidVersionDag readVersionDag() const;

  ZObject3dScan readCoarseBody(uint64_t bodyId) const;
  ZObject3dScan readCoarseBody(
      uint64_t bodyId, flyem::EBodyLabelType labelType) const;

  ZObject3dScan* readCoarseBody(uint64_t bodyId, ZObject3dScan *obj) const;
  ZObject3dScan* readCoarseBody(
      uint64_t bodyId, flyem::EBodyLabelType labelType, ZObject3dScan *obj) const;
  ZObject3dScan* readCoarseBody(
      uint64_t bodyId, flyem::EBodyLabelType labelType, const ZIntCuboid &box,
      ZObject3dScan *obj) const;

  int readCoarseBodySize(uint64_t bodyId) const;

  ZObject3dScan readRoi(const std::string &dataName) const;
  ZObject3dScan* readRoi(
      const std::string &dataName, ZObject3dScan *result, bool appending = false) const;
  ZDvidRoi* readRoi(const std::string &dataName, ZDvidRoi *roi);
  ZJsonArray readRoiJson(const std::string &dataName);

  ZFlyEmBodyAnnotation readBodyAnnotation(uint64_t bodyId) const;
  ZJsonObject readBodyAnnotationJson(uint64_t bodyId) const;

  bool hasBodyAnnotation() const;

  ZJsonObject readJsonObject(const std::string &url) const;
  ZJsonArray readJsonArray(const std::string &url) const;
  ZJsonArray readJsonArray(
      const std::string &url, const QByteArray &payload) const;

  ZJsonArray readAnnotation(
      const std::string &dataName, const std::string &tag) const;
  /*!
   * \brief Read all point annotations within the given label.
   * \param dataName Annotation data name
   * \param label Annotation label
   */
  ZJsonArray readAnnotation(const std::string &dataName, uint64_t label) const;


  ZJsonArray readTaggedBookmark(const std::string &tag) const;
  ZJsonObject readBookmarkJson(int x, int y, int z) const;
  ZJsonObject readBookmarkJson(const ZIntPoint &pt) const;
  bool isBookmarkChecked(int x, int y, int z) const;
  bool isBookmarkChecked(const ZIntPoint &pt) const;

  ZJsonObject readAnnotationJson(
      const std::string &dataName, const ZIntPoint &pt) const;
  ZJsonObject readAnnotationJson(
      const std::string &dataName, int x, int y, int z) const;

  std::vector<ZIntPoint> readSynapsePosition(const ZIntCuboid &box) const;
  std::vector<ZDvidSynapse> readSynapse(
      const ZIntCuboid &box,
      flyem::EDvidAnnotationLoadMode mode = flyem::LOAD_NO_PARTNER) const;
  std::vector<ZDvidSynapse> readSynapse(
      uint64_t label,
      flyem::EDvidAnnotationLoadMode mode = flyem::LOAD_NO_PARTNER) const;
  std::vector<ZDvidSynapse> readSynapse(
      uint64_t label, const ZDvidRoi &roi,
      flyem::EDvidAnnotationLoadMode mode) const;

  ZDvidSynapse readSynapse(
      int x, int y, int z,
      flyem::EDvidAnnotationLoadMode mode = flyem::LOAD_NO_PARTNER) const;
  ZDvidSynapse readSynapse(
      const ZIntPoint &pt,
      flyem::EDvidAnnotationLoadMode mode = flyem::LOAD_NO_PARTNER) const;
  ZJsonObject readSynapseJson(int x, int y, int z) const;
  ZJsonObject readSynapseJson(const ZIntPoint &pt) const;
  template <typename InputIterator>
  ZJsonArray readSynapseJson(
      const InputIterator &first, const InputIterator &last);

  std::vector<ZFlyEmToDoItem> readToDoItem(const ZIntCuboid &box) const;
  ZFlyEmToDoItem readToDoItem(int x, int y, int z) const;
  ZJsonObject readToDoItemJson(int x, int y, int z);
  ZJsonObject readToDoItemJson(const ZIntPoint &pt);

  ZJsonObject readContrastProtocal() const;

  void setVerbose(bool verbose) { m_verbose = verbose; }
  bool isVerbose() const { return m_verbose; }

  ZIntPoint readBodyBottom(uint64_t bodyId) const;
  ZIntPoint readBodyTop(uint64_t bodyId) const;
  ZIntCuboid readBodyBoundBox(uint64_t bodyId) const;
  ZIntPoint readBodyPosition(uint64_t bodyId) const;

  ZJsonObject readSkeletonConfig() const;

  int64_t getReadingTime() const {
    return m_readingTime;
  }

  bool good() const;

  std::string readMasterNode() const;
  std::vector<std::string> readMasterList() const;

  enum EReadOption {
    READ_CURRENT, READ_TRACE_BACK
  };

  ZJsonObject readDefaultDataSetting(
      EReadOption option = ZDvidReader::READ_CURRENT) const;

  ZJsonObject readDataMap() const;

  std::string readBodyLabelName() const;
  void syncBodyLabelName();

//  std::vector<std::string> readMasterList() const;
  static std::string ReadMasterNode(const ZDvidTarget &target);
  static std::vector<std::string> ReadMasterList(const ZDvidTarget &target);

#if defined(_ENABLE_LIBDVIDCPP_)
  ZSharedPointer<libdvid::DVIDNodeService> getService() const {
    return m_service;
  }
#endif

  QByteArray readBuffer(const std::string &url) const;
  QByteArray readDataFromEndpoint(
      const std::string &endPoint, bool tryingCompress = false) const;

  bool refreshLabelBuffer() const;

  void testApiLoad();

  int checkProofreadingData() const;

  QByteArray readServiceResult(
      const std::string &group, const std::string &key) const;
  ZJsonObject readServiceTask(
      const std::string &group, const std::string &key) const;
  std::map<std::string, ZJsonObject> readSplitTaskMap() const;
  QList<ZStackObject*> readSeedFromSplitTask(
      const std::string &taskKey, uint64_t bodyId);
  QList<ZStackObject*> readSeedFromSplitTask(
      const ZDvidTarget &target, uint64_t bodyId);

//  ZJsonObject readTestTask() const;
  ZJsonObject readTestTask(const std::string &key) const;

  bool hasSplitTask(const QString &key) const;

  void setGrayCenterCut(int cx, int cy);
  void setLabelCenterCut(int cx, int cy);

  class PauseVerbose {
  public:
    PauseVerbose(ZDvidReader *reader) : m_reader(reader) {
      m_verbose = m_reader->isVerbose();
      m_reader->setVerbose(false);
    }

    ~PauseVerbose() {
      m_reader->setVerbose(m_verbose);
    }

  private:
    ZDvidReader *m_reader;
    bool m_verbose;
  };

//public slots:
//  void slotTest();
//  void startReading();
//  void endReading();

//  std::set<uint64_t> readBodyId(const QString sizeRange);

private:
  ZDvidReader(const ZDvidReader&);
  ZDvidReader& operator=(const ZDvidReader&);

  static std::vector<std::pair<int, int> > partitionStack(
      int x0, int y0, int z0, int width, int height, int depth);
//  bool isReadingDone();
//  void waitForReading();
  bool startService();

  void init();

  void updateSegmentationData();

  std::vector<ZStack*> readGrayScaleBlockOld(
      const ZIntPoint &blockIndex, const ZDvidInfo &dvidInfo,
      int blockNumber) const;

  void clearBuffer() const;

  static std::string GetMasterNodeFromBuffer(
      const ZDvidBufferReader &bufferReader);
  static std::vector<std::string> GetMasterListFromBuffer(
      const ZDvidBufferReader &bufferReader);
  ZJsonObject readDefaultDataSettingCurrent() const;
  ZJsonObject readDefaultDataSettingTraceBack() const;
  void loadDefaultDataSetting();
  void loadDvidDataSetting(const ZJsonObject obj);

  bool reportMissingData(const std::string dataName) const;

  static ZIntCuboid GetStackBox(
      int x0, int y0, int z0, int width, int height, int zoom);
  static ZIntCuboid GetStackBoxAtCenter(
      int x0, int y0, int z0, int width, int height, int zoom);
  static std::vector<int> GetOffset(int x0, int y0, int z0);
  static std::vector<int> GetOffset(int cx, int cy, int cz, int width, int height);


  lowtis::ImageService* getLowtisServiceGray(int cx, int cy) const;
  lowtis::ImageService* getLowtisServiceLabel(int cx, int cy) const;

  void prepareLowtisService(
      ZSharedPointer<lowtis::ImageService> &service, const std::string &dataName,
      lowtis::DVIDConfig &config, int cx, int cy) const;

  template<typename T>
  void configureLowtis(T *config, const std::string &dataName) const;

protected:
  ZDvidTarget m_dvidTarget;
  bool m_verbose;

  std::string m_errorMsg;

  mutable int m_statusCode;
  mutable int64_t m_readingTime;

  mutable ZDvidBufferReader m_bufferReader;
#if defined(_ENABLE_LIBDVIDCPP_)
  ZSharedPointer<libdvid::DVIDNodeService> m_service;
  ZSharedPointer<libdvid::DVIDConnection> m_connection;
  QMutex m_serviceMutex;
#endif

#if defined(_ENABLE_LOWTIS_)
  mutable lowtis::DVIDLabelblkConfig m_lowtisConfig;
  mutable ZSharedPointer<lowtis::ImageService> m_lowtisService;
  mutable lowtis::DVIDGrayblkConfig m_lowtisConfigGray;
  mutable ZSharedPointer<lowtis::ImageService> m_lowtisServiceGray;
#endif

};

template <typename InputIterator>
ZJsonArray ZDvidReader::readSynapseJson(
    const InputIterator &first, const InputIterator &last)
{
  ZJsonArray synapseJsonArray;
  for (InputIterator iter = first; iter != last; ++iter) {
    const ZIntPoint &pt = *iter;
    ZJsonObject obj = readSynapseJson(pt);
    synapseJsonArray.append(obj);
  }

  return synapseJsonArray;
}

template <typename InputIterator>
std::vector<uint64_t> ZDvidReader::readBodyIdAt(
    const InputIterator &first, const InputIterator &last) const
{
  std::vector<uint64_t> bodyArray;

  if (first != last) {
    ZDvidBufferReader &bufferReader = m_bufferReader;
#if defined(_ENABLE_LIBDVIDCPP_)
    bufferReader.setService(m_service);
#endif
    ZDvidUrl dvidUrl(m_dvidTarget);

    ZJsonArray queryObj;

    for (InputIterator iter = first; iter != last; ++iter) {
      const ZIntPoint &pt = *iter;
      ZJsonArray coordObj;
      coordObj.append(pt.getX());
      coordObj.append(pt.getY());
      coordObj.append(pt.getZ());

      queryObj.append(coordObj);
    }

    QString queryForm = queryObj.dumpString(0).c_str();

#ifdef _DEBUG_
    std::cout << "Payload: " << queryForm.toStdString() << std::endl;
#endif

    QByteArray payload;
    payload.append(queryForm);

    bufferReader.read(
          dvidUrl.getLocalBodyIdArrayUrl().data(), payload, "GET", true);
    setStatusCode(bufferReader.getStatusCode());

    ZJsonArray infoJson;
    infoJson.decodeString(bufferReader.getBuffer().data());

    for (size_t i = 0; i < infoJson.size(); ++i) {
      uint64_t bodyId = (uint64_t) ZJsonParser::integerValue(infoJson.at(i));
      bodyArray.push_back(bodyId);
    }
  }

  return bodyArray;
}

#endif // ZDVIDREADER_H
