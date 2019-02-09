#include "zstackdocloader.h"

#include "zstackdoc.h"
#include "zstackdocreader.h"
#include "zstackobjectgroup.h"

ZStackDocLoader::ZStackDocLoader() : m_isStackReserved(false),
  m_isSparseStackReserved(false), m_isStackChanged(false),
  m_isSparseStackChanged(false)
{
}


void ZStackDocLoader::load(ZStackDoc *doc, ZStackDocReader &reader)
{
  if (doc != NULL) {
    doc->blockSignals(true);
    QList<ZStackObject::EType> typeList = doc->getObjectGroup().getAllType();
    for (QList<ZStackObject::EType>::const_iterator iter = typeList.begin();
         iter != typeList.end(); ++iter) {
      ZStackObject::EType type = *iter;
      if (getLoadMode(type) == LOAD_OVERWRITE) {
//        doc->getObjectGroup().remove(type);
        m_typeRemoved.append(type);
      }
    }

    QList<ZStackObject*> objList = doc->getObjectGroup().getObjectList();
    for (QList<ZStackObject*>::iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      ZStackObject *obj = *iter;
      if (getLoadMode(obj->getType()) == LOAD_OVERWRITE) {
        m_roleRemoved.addRole(obj->getRole().getRole());
        doc->getObjectGroup().removeObject(obj);
      }
    }

    typeList = reader.getObjectGroup().getAllType();
    for (QList<ZStackObject::EType>::const_iterator iter = typeList.begin();
         iter != typeList.end(); ++iter) {
      ZStackObject::EType type = *iter;
//      doc->getObjectGroup().append(newObjectGroup.getObjectList(type));
      m_typeAdded.append(type);
    }

    const QList<ZStackObject*> &newObjList =
        reader.getObjectGroup().getObjectList();
    for (QList<ZStackObject*>::const_iterator iter = newObjList.begin();
         iter != newObjList.end(); ++iter) {
      const ZStackObject *obj = *iter;
      m_roleAdded.addRole(obj->getRole().getRole());
      doc->getObjectGroup().add(obj, true);
    }
    reader.getPlayerList().moveTo(doc->getPlayerList());
//    doc->getPlayerList().append(reader.getPlayerList());


    if (reader.getStack() != NULL) {
      doc->loadStack(reader.getStack());
      doc->setStackSource(reader.getStackSource());
      m_isStackChanged = true;
    }
    if (reader.getSparseStack() != NULL) {
      doc->setSparseStack(reader.getSparseStack());
      m_isSparseStackChanged = true;
    }

    processChanged(doc);
    doc->blockSignals(false);
  }
}

void ZStackDocLoader::processChanged(ZStackDoc *doc)
{
  std::set<ZStackObject::EType> changedTypeSet;
  changedTypeSet.insert(m_typeRemoved.begin(), m_typeRemoved.end());
  changedTypeSet.insert(m_typeAdded.begin(), m_typeAdded.end());
//  changedTypeSet.fromList(m_typeRemoved);
//  changedTypeSet += m_typeAdded.toSet();

  for (auto iter = changedTypeSet.begin();
       iter != changedTypeSet.end(); ++iter) {
    processObjectChanged(doc, *iter);
  }

  if (m_isStackChanged) {
    //doc->initNeuronTracer();
    doc->notifyStackModified(true);
  }
}

void ZStackDocLoader::processObjectChanged(
    ZStackDoc *doc, ZStackObject::EType type)
{
  switch (type) {
  case ZStackObject::EType::SWC:
    doc->notifySwcModified();
    break;
  case ZStackObject::EType::LOCSEG_CHAIN:
    doc->notifyChainModified();
    break;
  case ZStackObject::EType::PUNCTUM:
    doc->notifyPunctumModified();
    break;
  case ZStackObject::EType::MESH:
    doc->notifyMeshModified();
    break;
  case ZStackObject::EType::SPARSE_OBJECT:
    doc->notifySparseObjectModified();
    break;
  default:
    break;
  }
}

ZStackDocLoader::ELoadMode ZStackDocLoader::getLoadMode(
    ZStackObject::EType type) const
{
  if (m_loadType.contains(type)) {
    return m_loadType[type];
  }

  return LOAD_OVERWRITE;
}
