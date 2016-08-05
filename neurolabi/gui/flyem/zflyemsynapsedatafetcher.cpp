#include "zflyemsynapsedatafetcher.h"

#include <QMutexLocker>
#include <QtConcurrentRun>

#include "zjsonobject.h"
#include "dvid/zdvidsynapse.h"
#include "dvid/zdvidsynapseensenmble.h"

ZFlyEmSynapseDataFetcher::ZFlyEmSynapseDataFetcher(QObject *parent) :
  QObject(parent)
{
  init();
}

ZFlyEmSynapseDataFetcher::~ZFlyEmSynapseDataFetcher()
{
  qDebug() << "ZFlyEmSynapseDataFetcher destroyed";

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

  m_fetchingRegion.reset();
}

ZIntCuboid ZFlyEmSynapseDataFetcher::takeRegion()
{
  QMutexLocker locker(&m_regionMutex);

  ZIntCuboid region = m_fetchingRegion;

  m_fetchingRegion.reset();

  return region;
}

void ZFlyEmSynapseDataFetcher::setRegion(const ZIntCuboid &box)
{
  QMutexLocker locker(&m_regionMutex);

  m_fetchingRegion = box;
}

void ZFlyEmSynapseDataFetcher::submit(const ZIntCuboid &box)
{
  setRegion(box);
}

void ZFlyEmSynapseDataFetcher::startFetching()
{
  const QString threadId = "ZFlyEmSynapseDataFetcher::fetchFunc";

  if (!m_futureMap.isAlive(threadId)) {
    QtConcurrent::run(this, &ZFlyEmSynapseDataFetcher::fetchFunc);
  }
}


void ZFlyEmSynapseDataFetcher::fetchFunc()
{
  QMutexLocker locker(&m_dataMutex);

  ZIntCuboid dataBox = takeRegion();

  while (!dataBox.isEmpty() && !m_dataRange.contains(dataBox)) {
    ZDvidUrl dvidUrl(m_dvidTarget);
    m_data = m_reader.readJsonArray(dvidUrl.getSynapseUrl(dataBox));
    m_dataRange = dataBox;
    emit dataFetched(this);

    dataBox = takeRegion();
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
        synapse.loadJsonObject(synapseJson, NeuTube::FlyEM::LOAD_NO_PARTNER);
        se->addSynapse(synapse, ZDvidSynapseEnsemble::DATA_LOCAL);
      }
    }

    se->setReady(m_dataRange);
  }
}
