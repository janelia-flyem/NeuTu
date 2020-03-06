#include "zpaintbundle.h"

#include <QMutexLocker>

#include "data3d/zstackobjecthelper.h"

ZPaintBundle::ZPaintBundle()
{
//  m_swcNodes = &m_emptyNodeSet;
  clearAllDrawableLists();
//  m_sliceAxis = sliceAxis;
}

ZPaintBundle::~ZPaintBundle()
{
  clearDynamicObjectList();
}

neutu::EAxis ZPaintBundle::getSliceAxis() const
{
  return m_viewParam.getSliceAxis();
}

void ZPaintBundle::addDynamicObject(ZStackObject *obj)
{
  if (obj) {
    QMutexLocker locker(&m_dynamicObjectListMutex);
    m_dynamicObjectList.append(std::shared_ptr<ZStackObject>(obj));
  }
}

/*
void ZPaintBundle::cloneDynamicObject(ZStackObject *obj)
{

}
*/

void ZPaintBundle::clearAllDrawableLists()
{
  m_objList.clear();
  clearDynamicObjectList();
}

void ZPaintBundle::clearDynamicObjectList()
{
  /*
  for (ZStackObject *obj : m_dynamicObjectList) {
    delete obj;
  }
  */
  QMutexLocker locker(&m_dynamicObjectListMutex);
  m_dynamicObjectList.clear();
}

void ZPaintBundle::addDrawableList(const QList<ZStackObject*>& lst)
{
  for (ZStackObject *obj : lst) {
    if (obj->getTarget() == ZStackObject::ETarget::WIDGET_CANVAS) {
      addDynamicObject(obj->clone());
    } else {
      m_objList.append(obj);
    }
  }
}

void ZPaintBundle::setViewParam(const ZStackViewParam &param)
{
  m_viewParam = param;
}

bool ZPaintBundle::hasDynamicObject() const
{
  QMutexLocker locker(&m_dynamicObjectListMutex);
  return !m_dynamicObjectList.empty();
}

void ZPaintBundle::alignToCutPlane(
    const QList<std::shared_ptr<ZStackObject> > &objList) const
{
  for (auto obj : objList) {
    ZStackObjectHelper::Align(obj.get(), m_viewParam.getArbSlicePlane());
  }
}

QList<std::shared_ptr<ZStackObject>>
ZPaintBundle::getVisibleDynamicObjectList() const
{
  QList<std::shared_ptr<ZStackObject>> objList;
  {
    QMutexLocker locker(&m_dynamicObjectListMutex);
    for (auto obj : m_dynamicObjectList) {
      if (obj->isSliceVisible(
            getZ(), m_viewParam.getSliceAxis(), m_viewParam.getArbSlicePlane())) {
        objList.append(obj);
      }
    }
  }
  std::sort(objList.begin(), objList.end(), ZStackObject::ZOrderLessThan());

  return objList;
}

QList<ZStackObject*> ZPaintBundle::getVisibleObjectList() const
{
  QList<ZStackObject*> objList;
  for (ZStackObject *obj : m_objList) {
    if (obj->getTarget() == m_target &&
        obj->isSliceVisible(getZ(), m_viewParam.getSliceAxis())) {
      objList.append(obj);
    }
  }
//  objList.append(m_objList);
//  objList.append(m_dynamicObjectList);

  std::sort(objList.begin(), objList.end(), ZStackObject::ZOrderLessThan());

  return objList;
}
