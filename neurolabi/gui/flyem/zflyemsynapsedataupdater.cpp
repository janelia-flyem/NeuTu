#include "zflyemsynapsedataupdater.h"
#include "zflyemsynapsedatafetcher.h"
#include "zstackdoc.h"
#include "dvid/zdvidsynapseensenmble.h"

ZFlyEmSynapseDataUpdater::ZFlyEmSynapseDataUpdater(QObject *parent) :
  QObject(parent)
{
  m_se = NULL;
}

void ZFlyEmSynapseDataUpdater::setData(
    ZDvidSynapseEnsemble *se, ZSharedPointer<ZStackDoc> doc)
{
  m_se = se;
  m_doc = doc;
}

void ZFlyEmSynapseDataUpdater::updateData(ZFlyEmSynapseDataFetcher *fetcher)
{
  fetcher->addSynapse(m_se);
  m_doc->processObjectModified(m_se);
  m_doc->notifyObjectModified();
}
