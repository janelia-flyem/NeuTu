#include "zdviddatafetcher.h"
#include "neutubeconfig.h"

#include <QtConcurrentRun>

ZDvidDataFetcher::ZDvidDataFetcher(QObject *parent) :
  QObject(parent)
{
  init();
}

ZDvidDataFetcher::~ZDvidDataFetcher()
{
  ZOUT(LTRACE(), 5)<< "ZDvidDataFetcher destroyed";

  m_futureMap.waitForFinished();
}

void ZDvidDataFetcher::init()
{
  m_timer = new QTimer(this);
}

void ZDvidDataFetcher::start(int msec)
{
  m_timer->setInterval(msec);
  m_timer->start();
  connectSignalSlot();
}

void ZDvidDataFetcher::connectSignalSlot()
{
  connect(m_timer, SIGNAL(timeout()), this, SLOT(startFetching()));
}


void ZDvidDataFetcher::setDvidTarget(const ZDvidTarget &dvidTarget)
{
  m_dvidTarget = dvidTarget;
  m_reader.open(m_dvidTarget);
}

void ZDvidDataFetcher::resetRegion()
{
  QMutexLocker locker(&m_regionMutex);

  m_fetchingRegion.clear();
}

QVector<ZIntCuboid> ZDvidDataFetcher::takeRegion()
{
  QMutexLocker locker(&m_regionMutex);

  QVector<ZIntCuboid> region = m_fetchingRegion;

  m_fetchingRegion.clear();

  return region;
}

void ZDvidDataFetcher::setRegion(const QVector<ZIntCuboid> &region)
{
  QMutexLocker locker(&m_regionMutex);

  m_fetchingRegion = region;
}

void ZDvidDataFetcher::submit(const QVector<ZIntCuboid> &region)
{
  setRegion(region);
}

void ZDvidDataFetcher::submit(const ZIntCuboid &box)
{
  QMutexLocker locker(&m_regionMutex);
  m_fetchingRegion.clear();
  m_fetchingRegion.append(box);
}

void ZDvidDataFetcher::startFetching()
{
  const QString threadId = "ZDvidDataFetcher::fetchFunc";

  if (!m_futureMap.isAlive(threadId)) {
    QFuture<void> future =
        QtConcurrent::run(this, &ZDvidDataFetcher::fetchFunc);
    m_futureMap[threadId] = future;
  }
}

void ZDvidDataFetcher::fetchFunc()
{
  QVector<ZIntCuboid> region = takeRegion();

  while (!region.isEmpty()) {
    foreach (const ZIntCuboid &dataBox, region) {
      bool fetching = true;
      foreach (const ZIntCuboid &lastBox, m_lastFetchingRegion) {
        if (lastBox.contains(dataBox)) {
          fetching = false;
          break;
        }
      }

      if (!dataBox.isEmpty() && fetching) {
        fetch(dataBox);
      }
    }

    m_lastFetchingRegion = region;
    region = takeRegion();
  }
}


