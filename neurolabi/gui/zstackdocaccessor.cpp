#include "zstackdocaccessor.h"

#include "zstackdoc.h"
#include "flyem/zstackwatershedcontainer.h"
#include "zobject3dscanarray.h"
#include "neutubeconfig.h"

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

void ZStackDocAccessor::ParseWatershedContainer(
    ZStackDoc *doc, ZStackWatershedContainer *container)
{
  if (doc != NULL && container != NULL) {
    ZObject3dScanArray result;
    container->makeSplitResult(1, &result);

    ZOUT(LTRACE(), 5) << result.size() << "split generated.";
    QList<ZStackObject *> objList;

    for (ZObject3dScanArray::iterator iter = result.begin();
         iter != result.end(); ++iter) {
      ZObject3dScan *obj = *iter;
      obj->addRole(ZStackObjectRole::ROLE_3DMESH_DECORATOR); //For 3D visualization
      objList.append(obj);
//      getDataBuffer()->addUpdate(
//            obj, ZStackDocObjectUpdate::ACTION_ADD_NONUNIQUE);
    }
//    getDataBuffer()->deliver();
    AddObject(doc, objList);
    result.shallowClear();
  }
}
