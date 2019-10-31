#include "zflyemsynapsedataupdater.h"

#include "mvc/zstackdoc.h"
#include "mvc/zstackdocdatabuffer.h"

#include "zflyemsynapsedatafetcher.h"
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
  m_doc->getDataBuffer()->addUpdate(
        m_se, ZStackDocObjectUpdate::EAction::UPDATE);
  m_doc->getDataBuffer()->deliver();
//  m_doc->processObjectModified(m_se);
//  m_doc->notifyObjectModified();
}
