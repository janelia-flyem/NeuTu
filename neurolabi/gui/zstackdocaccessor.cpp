#include "zstackdocaccessor.h"

#include <QsLog.h>
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
    QMutexLocker locker(doc->getObjectGroup().getMutex());
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

void ZStackDocAccessor::RemoveSplitSeed(ZStackDoc *doc, uint64_t label)
{
  if (doc != NULL) {
    QMutexLocker locker(doc->getObjectGroup().getMutex());
    TStackObjectList objList = doc->getObjectGroup().getObjectListUnsync(
          ZStackObjectRole::ROLE_SEED);
    for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
         ++iter) {
      if ((*iter)->getLabel() == label) {
        doc->getDataBuffer()->addUpdate(*iter, GetRemoveAction(true));
      }
    }
    if (!objList.isEmpty()) {
      doc->getDataBuffer()->deliver();
    }
  }
}

void ZStackDocAccessor::RemoveSplitSeed(ZStackDoc *doc)
{
  if (doc != NULL) {
    QMutexLocker locker(doc->getObjectGroup().getMutex());
    TStackObjectList objList = doc->getObjectGroup().getObjectListUnsync(
          ZStackObjectRole::ROLE_SEED);
    for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
         ++iter) {
      doc->getDataBuffer()->addUpdate(*iter, GetRemoveAction(true));
    }
    if (!objList.isEmpty()) {
      doc->getDataBuffer()->deliver();
    }
  }
}

void ZStackDocAccessor::RemoveSplitSeed(ZStackDoc *doc, std::string objectClass)
{
  if (doc != NULL) {
    QMutexLocker locker(doc->getObjectGroup().getMutex());
    TStackObjectList objList = doc->getObjectGroup().getObjectListUnsync(
          ZStackObjectRole::ROLE_SEED);
    bool updated = false;
    for (ZStackObject *obj : objList) {
      if (obj->getObjectClass() == objectClass) {
        doc->getDataBuffer()->addUpdate(obj, GetRemoveAction(true));
        updated = true;
      }
    }
    if (updated) {
      doc->getDataBuffer()->deliver();
    }
  }
}

void ZStackDocAccessor::RemoveSideSplitSeed(ZStackDoc *doc)
{
  if (doc != NULL) {
    QMutexLocker locker(doc->getObjectGroup().getMutex());
    TStackObjectList objList = doc->getObjectGroup().getObjectListUnsync(
          ZStackObjectRole::ROLE_SEED);
    for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
         ++iter) {
      if ((*iter)->getLabel() != 1) {
        doc->getDataBuffer()->addUpdate(*iter, GetRemoveAction(true));
      }
    }
    if (!objList.isEmpty()) {
      doc->getDataBuffer()->deliver();
    }
  }
}

void ZStackDocAccessor::RemoveObject(
    ZStackDoc *doc, ZStackObject::EType type, const std::string &source,
    bool deleteObject)
{
  if (doc != NULL) {
    QMutexLocker locker(doc->getObjectGroup().getMutex());
    TStackObjectList objList =
        doc->getObjectGroup().findSameSourceUnsync(type, source);
    for (TStackObjectList::iterator iter = objList.begin(); iter != objList.end();
         ++iter) {
      doc->getDataBuffer()->addUpdate(*iter, GetRemoveAction(deleteObject));
    }
    if (!objList.isEmpty()) {
      doc->getDataBuffer()->deliver();
    }
  }
}

void ZStackDocAccessor::RemoveObject(
    ZStackDoc *doc, ZStackObjectRole::TRole role, bool deleteObject)
{
  if (doc != NULL) {
    QMutexLocker locker(doc->getObjectGroup().getMutex());
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

void ZStackDocAccessor::RemoveAllSwcTree(ZStackDoc *doc, bool deleteObject)
{
  RemoveObject(doc, ZStackObject::TYPE_SWC, deleteObject);
}

void ZStackDocAccessor::SetObjectVisible(
    ZStackDoc *doc, ZStackObject::EType type, const std::string &source, bool on)
{
  if (doc != NULL) {
    QMutexLocker locker(doc->getObjectGroup().getMutex());
    TStackObjectList objList =
        doc->getObjectGroup().findSameSourceUnsync(type, source);
    for (ZStackObject *obj : objList) {
      if (obj->isVisible() != on) {
        obj->setVisible(on);
        doc->bufferObjectVisibilityChanged(obj);
      }
    }
    doc->processObjectModified();
  }
}

void ZStackDocAccessor::AddObjectUnique(ZStackDoc *doc, ZStackObject *obj)
{
  if (doc != NULL && obj != NULL) {
    doc->getDataBuffer()->addUpdate(
          obj, ZStackDocObjectUpdate::ACTION_ADD_UNIQUE);
    doc->getDataBuffer()->deliver();
  }
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

//Parse watershed results
//Each region will be assigned an ID for finding mesh correspondence
void ZStackDocAccessor::ParseWatershedContainer(
    ZStackDoc *doc, ZStackWatershedContainer *container)
{
  if (doc != NULL && container != NULL) {
    ZObject3dScanArray result;
    container->makeSplitResult(1, &result, NULL);

    ZOUT(LTRACE(), 5) << result.size() << "split generated.";
    QList<ZStackObject *> objList;

    int id = 1;
    for (ZObject3dScanArray::iterator iter = result.begin();
         iter != result.end(); ++iter) {
      ZObject3dScan *obj = *iter;
      obj->setObjectId(std::to_string(id++));
      obj->addRole(ZStackObjectRole::ROLE_3DMESH_DECORATOR); //For 3D visualization
      objList.append(obj);
    }

    AddObject(doc, objList);
    result.shallowClear();
  }
}
