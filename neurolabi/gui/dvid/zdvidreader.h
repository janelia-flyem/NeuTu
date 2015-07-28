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

class ZDvidReader : public QObject
{
  Q_OBJECT
public:
  explicit ZDvidReader(QObject *parent = 0);

  bool open(const QString &serverAddress, const QString &uuid,
            int port = -1);
  bool open(const ZDvidTarget &target);
  bool open(const QString &sourceString);

  //ZSwcTree* readSwc(const QString &key);
  ZSwcTree *readSwc(int bodyId);
  ZObject3dScan readBody(int bodyId);
  ZObject3dScan* readBody(int bodyId, ZObject3dScan *result);
  ZObject3dScan* readBody(int bodyId, int z, ZObject3dScan *result);

  ZStack* readThumbnail(int bodyId);

  ZSparseStack* readSparseStack(int bodyId);
  ZDvidSparseStack* readDvidSparseStack(int bodyId);
  ZStack* readGrayScale(
      int x0, int y0, int z0, int width, int height, int depth);
  ZStack* readGrayScale(const ZIntCuboid &cuboid);
  ZStack* readGrayScaleBlock(
      const ZIntPoint &blockIndex, const ZDvidInfo &dvidInfo);
  std::vector<ZStack*> readGrayScaleBlock(
      const ZIntPoint &blockIndex, const ZDvidInfo &dvidInfo,
      int blockNumber);


  /*!
   * \brief Read a stack of labels (Obsolete)
   *
   *  Obsolete function. Use readLabels64() instead.
   */
  ZStack* readBodyLabel(
      int x0, int y0, int z0, int width, int height, int depth);

  QString readInfo(const QString &dataName) const;

  std::set<int> readBodyId(
      int x0, int y0, int z0, int width, int height, int depth);
  std::set<int> readBodyId(const ZIntPoint &firstCorner,
                              const ZIntPoint &lastCorner);
  std::set<uint64_t> readBodyId(size_t minSize);
  std::set<uint64_t> readBodyId(size_t minSize, size_t maxSize);
  std::set<uint64_t> readBodyId(const ZDvidFilter &filter);

  QByteArray readKeyValue(const QString &dataName, const QString &key);
  QStringList readKeys(const QString &dataName);
  QStringList readKeys(const QString &dataName, const QString &minKey);
  QStringList readKeys(const QString &dataName,
                       const QString &minKey, const QString &maxKey);

  ZClosedCurve* readRoiCurve(const std::string &key, ZClosedCurve *result);
  ZIntCuboid readBoundBox(int z);

  ZDvidInfo readGrayScaleInfo() const;

  bool hasData(const std::string &key) const;

  ZArray* readLabels64(const std::string &dataName, int x0, int y0, int z0,
                       int width, int height, int depth) const;
  ZArray* readLabels64(int x0, int y0, int z0,
                       int width, int height, int depth) const;

  bool hasSparseVolume() const;
  bool hasSparseVolume(int bodyId) const;
  bool hasBodyInfo(int bodyId) const;

  bool hasCoarseSparseVolume(int bodyId) const;

  ZFlyEmNeuronBodyInfo readBodyInfo(int bodyId);

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  int readMaxBodyId();

  uint64_t readBodyIdAt(int x, int y, int z);

  ZDvidTileInfo readTileInfo(const std::string &dataName) const;

  //ZDvidTile *readTile(const std::string &dataName, int resLevel,
  //                   int xi0, int yi0, int z0) const;
  ZDvidTile *readTile(int resLevel, int xi0, int yi0, int z0) const;

  ZDvidVersionDag readVersionDag(const std::string &uuid) const;
  ZDvidVersionDag readVersionDag() const;

  ZObject3dScan readCoarseBody(uint64_t bodyId);

  ZObject3dScan readRoi(const std::string dataName);

  ZFlyEmBodyAnnotation readBodyAnnotation(uint64_t bodyId) const;

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

protected:
  QEventLoop *m_eventLoop;
  ZDvidClient *m_dvidClient;
  QTimer *m_timer;
  bool m_isReadingDone;
  ZDvidTarget m_dvidTarget;
};

#endif // ZDVIDREADER_H
