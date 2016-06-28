#include "zstackdocselector.h"

#include "zstackdoc.h"

ZStackDocSelector::ZStackDocSelector(const ZSharedPointer<ZStackDoc> &doc)
{
  m_doc = doc;
}

void ZStackDocSelector::deselectAll()
{
  m_doc->deselectAllObject(false);

  for (std::map<ZStackObject::EType, ESelectOption>::const_iterator
       iter = m_option.begin(); iter != m_option.end(); ++iter) {
    ZStackObject::EType type = iter->first;
    ESelectOption option = iter->second;
    if (option == SELECT_RECURSIVE) {
      QList<ZStackObject*> objList = m_doc->getObjectList(type);
      for (QList<ZStackObject*>::iterator iter = objList.begin();
           iter != objList.end(); ++iter) {
        ZStackObject *obj = *iter;
        obj->deselect(true);
      }
    }
  }
}

void ZStackDocSelector::setSelectOption(
    ZStackObject::EType type, ESelectOption option)
{
  m_option[type] = option;
}


