#include "zflyemsynapsedatafetcher.h"

#include <QMutexLocker>
#include <QtConcurrentRun>
#include <QElapsedTimer>

#include "zjsonobject.h"
#include "dvid/zdvidsynapse.h"
#include "dvid/zdvidsynapseensenmble.h"
#include "neutubeconfig.h"

ZFlyEmSynapseDataFetcher::ZFlyEmSynapseDataFetcher(QObject *parent) :
  QObject(parent)
{
  init();
}

ZFlyEmSynapseDataFetcher::~ZFlyEmSynapseDataFetcher()
{
  ZOUT(LTRACE(), 5) << "ZFlyEmSynapseDataFetcher destroyed";

  m_futureMap.waitForFinished();
}

void ZFlyEmSynapseDataFetcher::init()
{
  m_timer = new QTimer(this);
  m_timer->setInterval(200);
  m_timer->start();

  connectSignalSlot();
}

void ZFlyEmSynapseDataFetcher::connectSignalSlot()
{
  connect(m_timer, SIGNAL(timeout()), this, SLOT(startFetching()));
}

void ZFlyEmSynapseDataFetcher::setDvidTarget(const ZDvidTarget &dvidTarget)
{
  m_dvidTarget = dvidTarget;
  m_reader.open(m_dvidTarget);
}

void ZFlyEmSynapseDataFetcher::resetRegion()
{
  QMutexLocker locker(&m_regionMutex);

  m_fetchingRegion.clear();
}

QVector<ZIntCuboid> ZFlyEmSynapseDataFetcher::takeRegion()
{
  QMutexLocker locker(&m_regionMutex);

  QVector<ZIntCuboid> region = m_fetchingRegion;

  m_fetchingRegion.clear();

  return region;
}

void ZFlyEmSynapseDataFetcher::setRegion(const QVector<ZIntCuboid> &region)
{
  QMutexLocker locker(&m_regionMutex);

  m_fetchingRegion = region;
}

void ZFlyEmSynapseDataFetcher::submit(const QVector<ZIntCuboid> &region)
{
  setRegion(region);
}

void ZFlyEmSynapseDataFetcher::startFetching()
{
  const QString threadId = "ZFlyEmSynapseDataFetcher::fetchFunc";

  if (!m_futureMap.isAlive(threadId)) {
    QFuture<void> future =
        QtConcurrent::run(this, &ZFlyEmSynapseDataFetcher::fetchFunc);
    m_futureMap[threadId] = future;
  }
}


void ZFlyEmSynapseDataFetcher::fetchFunc()
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
        ZDvidUrl dvidUrl(m_dvidTarget);
        LINFO() << "Reading synapses: ";
        QElapsedTimer timer;
        timer.start();
        ZJsonArray data = m_reader.readJsonArray(dvidUrl.getSynapseUrl(dataBox));
        LINFO() << "Synapse reading time: " << timer.elapsed();

        {
          QMutexLocker locker(&m_dataMutex);
          m_data = data;
          m_dataRange = dataBox;
        }

        emit dataFetched(this);
      }
    }

    m_lastFetchingRegion = region;
    region = takeRegion();
  }
}

void ZFlyEmSynapseDataFetcher::addSynapse(ZDvidSynapseEnsemble *se)
{
  if (se != NULL) {
    QMutexLocker locker(&m_dataMutex);

    for (size_t i = 0; i < m_data.size(); ++i) {
      ZJsonObject synapseJson(m_data.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
      if (synapseJson.hasKey("Pos")) {
        ZDvidSynapse synapse;
        synapse.loadJsonObject(synapseJson, FlyEM::LOAD_NO_PARTNER);
        se->addSynapse(synapse, ZDvidSynapseEnsemble::DATA_LOCAL);
      }
    }

    se->setReady(m_dataRange);
  }
}
