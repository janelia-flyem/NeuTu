#include "zobjsmodelmanager.h"

#include "mvc/zstackdoc.h"
#include "zobjsmodelfactory.h"
#include "zobjsmodel.h"

ZObjsModelManager::ZObjsModelManager(ZStackDoc *parent) : QObject(parent),
  m_doc(parent)
{
  connect(parent, SIGNAL(objectModified(ZStackObjectInfoSet)),
          this, SLOT(processObjectModified(ZStackObjectInfoSet)));
}

ZStackDoc* ZObjsModelManager::getDocument() const
{
  return m_doc;
}

void ZObjsModelManager::updateData(const ZStackObject *obj)
{
  if (obj != NULL) {
    getObjsModel(obj->getType())->updateData(obj);
  }
}

void ZObjsModelManager::processObjectModified(
    const ZStackObjectInfoSet &infoSet)
{
  auto typeSet = infoSet.getType();
  for (auto type : typeSet) {
    ZObjsModel *model = getObjsModel(type);
    if (model != NULL) {
      model->processObjectModified(infoSet);
    }
  }
}

ZObjsModel* ZObjsModelManager::getObjsModel(ZStackObject::EType type)
{
  type = ZObjsModelFactory::GetCanonicalType(type);

  if (!m_modelMap.contains(type)) {
    ZObjsModel *model = ZObjsModelFactory::Make(type, getDocument(), this);
    m_modelMap[type] = model;
  }

  return m_modelMap[type];
}

ZObjsModel* ZObjsModelManager::getObjsModel(ZStackObjectRole::TRole role)
{
  if (!m_roleModelMap.contains(role)) {
    ZObjsModel *model = ZObjsModelFactory::Make(role, getDocument(), this);
    m_roleModelMap[role] = model;
  }

  return m_roleModelMap[role];
}
