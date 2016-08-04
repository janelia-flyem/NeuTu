#include "zflyemsynapsedatafetcher.h"

#include <QMutexLocker>

#include "zjsonobject.h"
#include "dvid/zdvidsynapse.h"
#include "dvid/zdvidsynapseensenmble.h"

ZFlyEmSynapseDataFetcher::ZFlyEmSynapseDataFetcher(QObject *parent) :
  QObject(parent)
{
}

void ZFlyEmSynapseDataFetcher::resetRegion()
{
  QMutexLocker locker(&m_regionMutex);

  m_dataRegion.reset();
}

ZIntCuboid ZFlyEmSynapseDataFetcher::takeRegion()
{
  QMutexLocker locker(&m_regionMutex);

  ZIntCuboid region = m_dataRegion;

  m_dataRegion.reset();

  return region;
}

void ZFlyEmSynapseDataFetcher::setRegion(const ZIntCuboid &box)
{
  QMutexLocker locker(&m_regionMutex);

  m_dataRegion = box;
}

void ZFlyEmSynapseDataFetcher::fetch()
{

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
  }
}
