#include "zdvidpatchdataupdater.h"

#include "mvc/zstackdoc.h"
#include "mvc/zstackdocdatabuffer.h"

#include "zdvidtileensemble.h"
#include "zdvidpatchdatafetcher.h"


ZDvidPatchDataUpdater::ZDvidPatchDataUpdater(QObject *parent) :
  QObject(parent)
{
  m_se = NULL;
}

void ZDvidPatchDataUpdater::setData(
    ZDvidTileEnsemble *se, ZSharedPointer<ZStackDoc> doc)
{
  m_se = se;
  m_doc = doc;
}

void ZDvidPatchDataUpdater::updateData(ZDvidPatchDataFetcher *fetcher)
{
  if (fetcher->updatePatch(m_se)) {
    m_doc->getDataBuffer()->addUpdate(
          m_se, ZStackDocObjectUpdate::EAction::UPDATE);
    m_doc->getDataBuffer()->deliver();
//    m_doc->processObjectModified(m_se);
//    m_doc->notifyObjectModified();
  }
}
