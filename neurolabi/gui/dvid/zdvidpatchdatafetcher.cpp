#include "zdvidpatchdatafetcher.h"
#include "neutubeconfig.h"
#include "zdvidtileensemble.h"
#include "zstackview.h"

ZDvidPatchDataFetcher::ZDvidPatchDataFetcher(QObject *parent) :
  ZDvidDataFetcher(parent)
{
  init();
}

ZDvidPatchDataFetcher::~ZDvidPatchDataFetcher()
{
  ZOUT(LTRACE(), 5) << "ZDvidPatchDataFetcher destroyed";
  delete m_patch;
}

void ZDvidPatchDataFetcher::init()
{
  m_patch = NULL;
}

void ZDvidPatchDataFetcher::fetch(const ZIntCuboid &region)
{
  ZStack *stack = m_reader.readGrayScale(region);

  if (stack != NULL) {
    int width = region.getWidth();
    int height = region.getHeight();
    {
      QMutexLocker locker(&m_dataMutex);
      m_dataRange = region;
      if (m_patch != NULL) {
        if (m_patch->width() != width || m_patch->height() != height) {
          delete m_patch;
          m_patch = NULL;
        }
      }

      if (m_patch == NULL) {
        m_patch = new ZImage(width, height, QImage::Format_Indexed8);
      }

      m_patch->setData(stack->array8());
      m_patch->setOffset(region.getFirstCorner().getX(),
                         region.getFirstCorner().getY());
      m_patch->setZ(region.getFirstCorner().getZ());
    }
    delete stack;

    emit dataFetched(this);
  }
}

bool ZDvidPatchDataFetcher::updatePatch(ZDvidTileEnsemble *slice, int z)
{
  QMutexLocker locker(&m_dataMutex);
  if (slice != NULL && m_dataRange.getFirstCorner().getZ() == z) {
    if (m_patch != NULL) {
      slice->updatePatch(m_patch, m_dataRange);
      return true;
    }
  }

  return false;
}

bool ZDvidPatchDataFetcher::updatePatch(ZDvidTileEnsemble *slice)
{
  return updatePatch(slice, slice->getView()->getCurrentZ());
}
