#include "zdvidpatchdataupdater.h"
#include "zdvidtileensemble.h"
#include "zstackdoc.h"
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
    m_doc->processObjectModified(m_se);
    m_doc->notifyObjectModified();
  }
}
