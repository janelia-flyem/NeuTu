#include "zstackdochelper.h"
#include "zstackdoc.h"
#include "dvid/zdvidlabelslice.h"

ZStackDocHelper::ZStackDocHelper()
{
  m_currentZ = 0;
  m_hasCurrentZ = false;
}

void ZStackDocHelper::extractCurrentZ(const ZStackDoc *doc)
{
  m_hasCurrentZ = false;
  if (doc != NULL) {
    const TStackObjectList &objList =
        doc->getObjectList(ZStackObject::TYPE_DVID_LABEL_SLICE);
    if (!objList.isEmpty()) {
      ZDvidLabelSlice *obj = dynamic_cast<ZDvidLabelSlice*>(objList.first());
      m_currentZ = obj->getViewParam().getZ();
      m_hasCurrentZ = true;
    }
  }
}

int ZStackDocHelper::getCurrentZ() const
{
  return m_currentZ;
}

bool ZStackDocHelper::hasCurrentZ() const
{
  return m_hasCurrentZ;
}
