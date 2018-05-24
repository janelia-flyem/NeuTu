#ifndef ZDVIDDATAFETCHER_H
#define ZDVIDDATAFETCHER_H

#include <QObject>
#include <QMutex>
#include <QVector>

#include "zintcuboid.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidreader.h"
#include "zthreadfuturemap.h"

class ZDvidDataFetcher : public QObject
{
  Q_OBJECT
public:
  explicit ZDvidDataFetcher(QObject *parent = 0);
  virtual ~ZDvidDataFetcher();

  void setDvidTarget(const ZDvidTarget &dvidTarget);

  void setRegion(const QVector<ZIntCuboid> &region);
  void resetRegion();
  QVector<ZIntCuboid> takeRegion();

  void submit(const ZIntCuboid &box);
  void submit(const QVector<ZIntCuboid> &region);

  virtual void start(int msec);

private:
  void fetchFunc();
  void init();
  virtual void connectSignalSlot();

  virtual void fetch(const ZIntCuboid &region) = 0;

signals:

public slots:
  void startFetching();

protected:
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

#endif // ZDVIDDATAFETCHER_H
