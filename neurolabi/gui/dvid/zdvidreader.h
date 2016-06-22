#ifndef ZDVIDREADER_H
#define ZDVIDREADER_H

#include <QObject>
#include <QEventLoop>
#include <QTimer>
#include <QImage>

#include <string>
#include <vector>

#include "zstack.hxx"
#include "zdvidclient.h"
#include "flyem/zflyem.h"
#include "zintcuboid.h"
#include "zclosedcurve.h"
#include "dvid/zdvidinfo.h"
#include "zintcuboid.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidsynapse.h"

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

namespace libdvid{
class DVIDNodeService;
}

class ZDvidReader : public QObject
{
  Q_OBJECT
public:
  explicit ZDvidReader(QObject *parent = 0);
  ~ZDvidReader();

  bool open(const QString &serverAddress, const QString &uuid,
            int port = -1);
  bool open(const ZDvidTarget &target);
  bool open(const QString &sourceString);

  /*!
   * \brief Get the status code of the latest request (NOT functioning yet)
   * \return
   */
  int getStatusCode() const;

  /*!
   * \brief Check if the reader is ready to use
   *
   * \return true iff the reader is ready.
   */
  bool isReady() const;

  //ZSwcTree* readSwc(const QString &key);
  ZSwcTree *readSwc(uint64_t bodyId);
  ZObject3dScan readBody(uint64_t bodyId);
  ZObject3dScan* readBody(uint64_t bodyId, ZObject3dScan *result);
  ZObject3dScan* readBody(uint64_t bodyId, int z, NeuTube::EAxis axis,
                          ZObject3dScan *result);

  ZStack* readThumbnail(uint64_t bodyId);

  ZSparseStack* readSparseStack(uint64_t bodyId);
  ZDvidSparseStack* readDvidSparseStack(uint64_t bodyId);
  ZDvidSparseStack* readDvidSparseStackAsync(uint64_t bodyId);
  ZStack* readGrayScale(
      int x0, int y0, int z0, int width, int height, int depth);
  ZStack* readGrayScale(const ZIntCuboid &cuboid);
  ZStack* readGrayScaleBlock(
      const ZIntPoint &blockIndex, const ZDvidInfo &dvidInfo);
  std::vector<ZStack*> readGrayScaleBlock(
      const ZIntPoint &blockIndex, const ZDvidInfo &dvidInfo,
      int blockNumber);

  QString readInfo(const QString &dataName) const;

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

  bool hasKey(const QString &dataName, const QString &key);
  QByteArray readKeyValue(const QString &dataName, const QString &key);
  QStringList readKeys(const QString &dataName);
  QStringList readKeys(const QString &dataName, const QString &minKey);
  QStringList readKeys(const QString &dataName,
                       const QString &minKey, const QString &maxKey);

  ZClosedCurve* readRoiCurve(const std::string &key, ZClosedCurve *result);
  ZIntCuboid readBoundBox(int z);

  ZDvidInfo readGrayScaleInfo() const;

  ZJsonObject readInfo() const;

  bool hasData(const std::string &dataName) const;

  ZArray* readLabels64(const std::string &dataName, int x0, int y0, int z0,
                       int width, int height, int depth) const;
  ZArray* readLabels64(int x0, int y0, int z0,
                       int width, int height, int depth) const;
  ZArray* readLabels64(const ZIntCuboid &box);
  /*
  ZArray* readLabelSlice(const std::string &dataName, int x0, int y0, int z0,
                         int dim1, int dim2, int width, int height);
                         */

  bool hasSparseVolume() const;
  bool hasSparseVolume(uint64_t bodyId) const;
  bool hasBodyInfo(uint64_t bodyId) const;
  bool hasBody(uint64_t bodyId) const;

  ZIntPoint readBodyLocation(uint64_t bodyId) const;

  bool hasCoarseSparseVolume(uint64_t bodyId) const;

  ZFlyEmNeuronBodyInfo readBodyInfo(uint64_t bodyId);

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  uint64_t readMaxBodyId();

  uint64_t readBodyIdAt(int x, int y, int z);
  uint64_t readBodyIdAt(const ZIntPoint &pt);
  std::vector<uint64_t> readBodyIdAt(const std::vector<ZIntPoint> &ptArray);

  ZDvidTileInfo readTileInfo(const std::string &dataName) const;

  //ZDvidTile *readTile(const std::string &dataName, int resLevel,
  //                   int xi0, int yi0, int z0) const;
  ZDvidTile *readTile(int resLevel, int xi0, int yi0, int z0) const;

  ZDvidVersionDag readVersionDag(const std::string &uuid) const;
  ZDvidVersionDag readVersionDag() const;

  ZObject3dScan readCoarseBody(uint64_t bodyId) const;

  ZObject3dScan readRoi(const std::string &dataName);

  ZFlyEmBodyAnnotation readBodyAnnotation(uint64_t bodyId) const;

  ZJsonObject readJsonObject(const std::string &url) const;
  ZJsonArray readJsonArray(const std::string &url) const;

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

  std::vector<ZIntPoint> readSynapsePosition(const ZIntCuboid &box) const;
  std::vector<ZDvidSynapse> readSynapse(
      const ZIntCuboid &box,
      NeuTube::FlyEM::ESynapseLoadMode mode = NeuTube::FlyEM::LOAD_NO_PARTNER) const;
  std::vector<ZDvidSynapse> readSynapse(
      uint64_t label,
      NeuTube::FlyEM::ESynapseLoadMode mode = NeuTube::FlyEM::LOAD_NO_PARTNER) const;
  ZDvidSynapse readSynapse(
      int x, int y, int z,
      NeuTube::FlyEM::ESynapseLoadMode mode = NeuTube::FlyEM::LOAD_NO_PARTNER) const;
  ZJsonObject readSynapseJson(int x, int y, int z) const;
  ZJsonObject readSynapseJson(const ZIntPoint &pt) const;
  template <typename InputIterator>
  ZJsonArray readSynapseJson(
      const InputIterator &first, const InputIterator &last);

  std::vector<ZFlyEmToDoItem> readToDoItem(const ZIntCuboid &box) const;
  ZFlyEmToDoItem readToDoItem(int x, int y, int z) const;

  void setVerbose(bool verbose) { m_verbose = verbose; }
  bool isVerbose() const { return m_verbose; }

  ZIntPoint readBodyBottom(uint64_t bodyId) const;
  ZIntPoint readBodyTop(uint64_t bodyId) const;
  ZIntCuboid readBodyBoundBox(uint64_t bodyId) const;

  bool good() const;

#if defined(_ENABLE_LIBDVIDCPP_)
  ZSharedPointer<libdvid::DVIDNodeService> getService() const {
    return m_service;
  }
#endif

signals:
  void readingDone();

public slots:
  void slotTest();
  void startReading();
  void endReading();

  std::set<uint64_t> readBodyId(const QString sizeRange);

private:
  static std::vector<std::pair<int, int> > partitionStack(
      int x0, int y0, int z0, int width, int height, int depth);
  bool isReadingDone();
  void waitForReading();
  bool startService();

  void init();

protected:
  QEventLoop *m_eventLoop;
//  ZDvidClient *m_dvidClient;
  QTimer *m_timer;
  bool m_isReadingDone;
  ZDvidTarget m_dvidTarget;
  bool m_verbose;
  int m_statusCode;
#if defined(_ENABLE_LIBDVIDCPP_)
  ZSharedPointer<libdvid::DVIDNodeService> m_service;
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

#endif // ZDVIDREADER_H
