#include "zstackdocselector.h"

#include "mvc/zstackdoc.h"

ZStackDocSelector::ZStackDocSelector()
{

}

ZStackDocSelector::ZStackDocSelector(const std::shared_ptr<ZStackDoc> &doc)
{
  setDocument(doc);
}

void ZStackDocSelector::setDocument(const std::shared_ptr<ZStackDoc> &doc)
{
  if (m_doc.get() != doc.get()) {
    m_doc = doc;
  }
}

void ZStackDocSelector::deselectAll()
{
  if (m_doc) {
    for (std::map<ZStackObject::EType, ESelectOption>::const_iterator
         iter = m_option.begin(); iter != m_option.end(); ++iter) {
      ZStackObject::EType type = iter->first;
      ESelectOption option = iter->second;
      if (option == SELECT_RECURSIVE) {
        QList<ZStackObject*> objList = m_doc->getObjectList(type);
        for (QList<ZStackObject*>::iterator iter = objList.begin();
             iter != objList.end(); ++iter) {
          ZStackObject *obj = *iter;
          obj->deselectSub();
          m_doc->bufferObjectModified(
                obj, ZStackObjectInfo::STATE_SELECTION_CHANGED, true);
        }
      }
    }
    m_doc->deselectAllObject(false);
  }
}

void ZStackDocSelector::setSelectOption(
    ZStackObject::EType type, ESelectOption option)
{
  m_option[type] = option;
}


