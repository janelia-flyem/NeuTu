#include "zstackdocaccessor.h"

#include "zstackdoc.h"

ZStackDocAccessor::ZStackDocAccessor()
{
}

ZStackDocObjectUpdate::EAction ZStackDocAccessor::GetRemoveAction(bool deleteObject)
{
  if (deleteObject) {
    return ZStackDocObjectUpdate::ACTION_KILL;
  } else {
    return ZStackDocObjectUpdate::ACTION_EXPEL;
  }
}

void ZStackDocAccessor::RemoveObject(
    ZStackDoc *doc, ZStackObject *obj, bool deleteObject)
{
  if (doc != NULL && obj != NULL) {
    if (deleteObject) {
      doc->getDataBuffer()->addUpdate(obj, GetRemoveAction(deleteObject));
    }
    doc->getDataBuffer()->deliver();
  }
}

void ZStackDocAccessor::RemoveObject(
    ZStackDoc *doc, ZStackObject::EType type, bool deleteObject)
{
  if (doc != NULL) {
    {
      QMutexLocker(doc->getObjectGroup().getMutex());
      TStackObjectList objList = doc->getObjectGroup().getObjectListUnsync(type);
      for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
           ++iter) {
        doc->getDataBuffer()->addUpdate(*iter, GetRemoveAction(deleteObject));
      }
      if (!objList.isEmpty()) {
        doc->getDataBuffer()->deliver();
      }
    }
  }
}

void ZStackDocAccessor::RemoveObject(
    ZStackDoc *doc, ZStackObjectRole::TRole role, bool deleteObject)
{
  if (doc != NULL) {
    {
      QMutexLocker(doc->getObjectGroup().getMutex());
      TStackObjectList objList = doc->getObjectGroup().getObjectListUnsync(role);
      for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
           ++iter) {
        doc->getDataBuffer()->addUpdate(*iter, GetRemoveAction(deleteObject));
      }
      if (!objList.isEmpty()) {
        doc->getDataBuffer()->deliver();
      }
    }
  }
}

void ZStackDocAccessor::RemoveAllSwcTree(ZStackDoc *doc, bool deleteObject)
{
  RemoveObject(doc, ZStackObject::TYPE_SWC, deleteObject);
}

void ZStackDocAccessor::AddObject(ZStackDoc *doc, ZStackObject *obj)
{
  if (doc != NULL && obj != NULL) {
    doc->getDataBuffer()->addUpdate(
          obj, ZStackDocObjectUpdate::ACTION_ADD_NONUNIQUE);
    doc->getDataBuffer()->deliver();
  }
}

void ZStackDocAccessor::AddObject(ZStackDoc *doc, const QList<ZStackObject *> &objList)
{
  if (doc != NULL && !objList.isEmpty()) {
    doc->getDataBuffer()->addUpdate(
          objList, ZStackDocObjectUpdate::ACTION_ADD_NONUNIQUE);
    doc->getDataBuffer()->deliver();
  }
}
