#ifndef ZDVIDREADER_H
#define ZDVIDREADER_H

#include <QObject>
#include <QEventLoop>
#include <QTimer>
#include <string>
#include <vector>
#include "zobject3dscan.h"
#include "zswctree.h"
#include "zstack.hxx"
#include "zdvidclient.h"
#include "flyem/zflyem.h"
#include "zintcuboid.h"
#include "zsparsestack.h"
#include "zclosedcurve.h"
#include "dvid/zdvidinfo.h"
#include "zintcuboid.h"
#include "dvid/zdvidtarget.h"

class ZDvidFilter;
class ZArray;
class ZJsonObject;
class ZFlyEmNeuronBodyInfo;

class ZDvidReader : public QObject
{
  Q_OBJECT
public:
  explicit ZDvidReader(QObject *parent = 0);

  bool open(const QString &serverAddress, const QString &uuid,
            int port = -1);
  bool open(const ZDvidTarget &target);
  bool open(const QString &sourceString);

  ZSwcTree *readSwc(int bodyId);
  ZObject3dScan readBody(int bodyId);
  ZObject3dScan* readBody(int bodyId, ZObject3dScan *result);

  ZStack* readThumbnail(int bodyId);

  ZSparseStack* readSparseStack(int bodyId);
  ZStack* readGrayScale(
      int x0, int y0, int z0, int width, int height, int depth);
  ZStack* readGrayScale(const ZIntCuboid &cuboid);

  ZStack* readBodyLabel(
      int x0, int y0, int z0, int width, int height, int depth);
  QString readInfo(const QString &dataType);

  std::set<int> readBodyId(
      int x0, int y0, int z0, int width, int height, int depth);
  std::set<int> readBodyId(const ZIntPoint &firstCorner,
                              const ZIntPoint &lastCorner);
  std::set<int> readBodyId(size_t minSize);
  std::set<int> readBodyId(size_t minSize, size_t maxSize);
  std::set<int> readBodyId(const ZDvidFilter &filter);

  QByteArray readKeyValue(const QString &dataName, const QString &key);
  QStringList readKeys(const QString &dataName,
                       const QString &minKey, const QString &maxKey);

  ZClosedCurve* readRoiCurve(const std::string &key, ZClosedCurve *result);
  ZIntCuboid readBoundBox(int z);

  ZDvidInfo readGrayScaleInfo();

  bool hasData(const std::string &key) const;

  ZArray* readLabels64(const std::string &dataName, int x0, int y0, int z0,
                       int width, int height, int depth) const;

  bool hasSparseVolume() const;
  bool hasBodyInfo(int bodyId) const;

  ZFlyEmNeuronBodyInfo readBodyInfo(int bodyId);

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  int readMaxBodyId();

signals:
  void readingDone();

public slots:
  void slotTest();
  void startReading();
  void endReading();

  std::set<int> readBodyId(const QString sizeRange);

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
