#include "zstackobjectconfig.h"

#include "zstackobject.h"
#include "zlabelcolortable.h"

ZStackObjectConfig::ZStackObjectConfig()
{

}


void ZStackObjectConfig::configure(ZStackObject *obj)
{
  if (obj != NULL) {
    obj->setSource(m_source);
    obj->setObjectClass(m_objectClass);
  }
}

void ZStackObjectConfig::configureSeed(ZStackObject *obj, uint64_t label)
{
  if (obj != NULL) {
    configure(obj);
    obj->setLabel(label);
    ZLabelColorTable colorTable;
    obj->setColor(colorTable.getColor(obj->getLabel()));
    obj->setRole(ZStackObjectRole::ROLE_SEED |
                 ZStackObjectRole::ROLE_3DGRAPH_DECORATOR);
  }
}
