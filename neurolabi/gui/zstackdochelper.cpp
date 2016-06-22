#include "zstackdochelper.h"
#include "zstackdoc.h"
#if defined(_FLYEM_)
#include "dvid/zdvidlabelslice.h"
#include "dvid/zdvidtileensemble.h"
#endif
#include "zstackview.h"

ZStackDocHelper::ZStackDocHelper()
{
  m_currentZ = 0;
  m_hasCurrentZ = false;
}
#if defined(_FLYEM_)
void ZStackDocHelper::extractCurrentZ(const ZStackDoc *doc)
{
  m_hasCurrentZ = false;
  if (doc != NULL) {
    {
      const TStackObjectList &objList =
          doc->getObjectList(ZStackObject::TYPE_DVID_TILE_ENSEMBLE);
      if (!objList.isEmpty()) {
        ZDvidTileEnsemble *obj =
            dynamic_cast<ZDvidTileEnsemble*>(objList.first());
        if (obj->isVisible()) {
          if (obj->getView() != NULL) {
            m_currentZ = obj->getView()->getCurrentZ();
            m_hasCurrentZ = true;
          }
        }
      }
    }

    if (!m_hasCurrentZ) {
      const TStackObjectList &objList =
          doc->getObjectList(ZStackObject::TYPE_DVID_LABEL_SLICE);
      if (!objList.isEmpty()) {
        ZDvidLabelSlice *obj = dynamic_cast<ZDvidLabelSlice*>(objList.first());
        if (obj->isVisible()) {
          m_currentZ = obj->getViewParam().getZ();
          m_hasCurrentZ = true;
        }
      }
    }
  }
}
#endif
int ZStackDocHelper::getCurrentZ() const
{
  return m_currentZ;
}

bool ZStackDocHelper::hasCurrentZ() const
{
  return m_hasCurrentZ;
}
