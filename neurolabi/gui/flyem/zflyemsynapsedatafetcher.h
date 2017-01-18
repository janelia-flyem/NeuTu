#ifndef ZFLYEMSYNAPSEDATAFETCHER_H
#define ZFLYEMSYNAPSEDATAFETCHER_H

#include <QObject>
#include <QMutex>
#include <QVector>

#include "zintcuboid.h"
#include "zjsonarray.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidreader.h"
#include "zthreadfuturemap.h"

class ZDvidSynapseEnsemble;

class ZFlyEmSynapseDataFetcher : public QObject
{
  Q_OBJECT
public:
  explicit ZFlyEmSynapseDataFetcher(QObject *parent = 0);
  ~ZFlyEmSynapseDataFetcher();

  void setDvidTarget(const ZDvidTarget &dvidTarget);

  void setRegion(const QVector<ZIntCuboid> &region);
  void resetRegion();
  QVector<ZIntCuboid> takeRegion();

  void submit(const ZIntCuboid &box);
  void submit(const QVector<ZIntCuboid> &region);
  void addSynapse(ZDvidSynapseEnsemble *se);

private:
  void fetchFunc();
  void init();
  void connectSignalSlot();

signals:
  void dataFetched(ZFlyEmSynapseDataFetcher*);

public slots:
  void startFetching();

private:
  QVector<ZIntCuboid> m_fetchingRegion;
  QMutex m_regionMutex;
  QMutex m_dataMutex;

  ZDvidTarget m_dvidTarget;
  ZDvidReader m_reader;


  QVector<ZIntCuboid> m_lastFetchingRegion;
  ZJsonArray m_data;
  ZIntCuboid m_dataRange;

  ZThreadFutureMap m_futureMap;
  QTimer *m_timer;
};

#endif // ZFLYEMSYNAPSEDATAFETCHER_H
