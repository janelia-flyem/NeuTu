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
  return m_displayConfig.getSliceAxis();
//  return m_viewParam.getSliceAxis();
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

/*
void ZPaintBundle::setViewParam(const ZStackViewParam &param)
{
  m_viewParam = param;
}
*/

void ZPaintBundle::setSliceViewTransform(const ZSliceViewTransform &t)
{
  m_displayConfig.setTransform(t);
}

bool ZPaintBundle::hasDynamicObject() const
{
  QMutexLocker locker(&m_dynamicObjectListMutex);
  return !m_dynamicObjectList.empty();
}

void ZPaintBundle::setDisplayStyle(ZStackObject::EDisplayStyle style)
{
  m_displayConfig.setStyle(style);
}

ZStackObject::EDisplayStyle ZPaintBundle::getDisplayStyle() const
{
  return m_displayConfig.getStyle();
}

void ZPaintBundle::setDisplaySliceMode(neutu::data3d::EDisplaySliceMode mode)
{
  m_displayConfig.setSliceMode(mode);
}

neutu::data3d::EDisplaySliceMode ZPaintBundle::getDisplaySliceMode() const
{
  return m_displayConfig.getSliceMode();
}

/*
void ZPaintBundle::alignToCutPlane(
    const QList<std::shared_ptr<ZStackObject> > &objList) const
{
  for (auto obj : objList) {
    ZStackObjectHelper::Align(obj.get(), m_viewParam.getArbSlicePlane());
  }
}
*/

QList<std::shared_ptr<ZStackObject>>
ZPaintBundle::getVisibleDynamicObjectList() const
{
  QList<std::shared_ptr<ZStackObject>> objList;
  {
    QMutexLocker locker(&m_dynamicObjectListMutex);
    for (auto obj : m_dynamicObjectList) {
      if (obj->isSliceVisible(m_displayConfig)) {
        objList.append(obj);
      }
    }
  }
  std::sort(objList.begin(), objList.end(), ZStackObject::ZOrderLessThan());

  return objList;
}

QList<ZStackObject*> ZPaintBundle::getVisibleObjectList(ZStackObject::ETarget target) const
{
  QList<ZStackObject*> objList;
  for (ZStackObject *obj : m_objList) {
    if (obj->getTarget() == target && obj->isSliceVisible(m_displayConfig)) {
      objList.append(obj);
    }
  }
//  objList.append(m_objList);
//  objList.append(m_dynamicObjectList);

  std::sort(objList.begin(), objList.end(), ZStackObject::ZOrderLessThan());

  return objList;
}
