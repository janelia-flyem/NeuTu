#ifndef ZFLYEMSYNAPSEDATAFETCHER_H
#define ZFLYEMSYNAPSEDATAFETCHER_H

#include <QObject>
#include <QMutex>

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

  void setRegion(const ZIntCuboid &box);
  void resetRegion();
  ZIntCuboid takeRegion();

  void submit(const ZIntCuboid &box);
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
  ZIntCuboid m_fetchingRegion;
  QMutex m_regionMutex;
  QMutex m_dataMutex;

  ZDvidTarget m_dvidTarget;
  ZDvidReader m_reader;

  ZJsonArray m_data;
  ZIntCuboid m_dataRange;

  ZThreadFutureMap m_futureMap;
  QTimer *m_timer;
};

#endif // ZFLYEMSYNAPSEDATAFETCHER_H
