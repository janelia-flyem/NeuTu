#ifndef ZOBJSMODELMANAGER_H
#define ZOBJSMODELMANAGER_H

#include <QObject>
#include <QMap>

#include "zstackobject.h"

class ZStackDoc;
class ZStackObjectInfoSet;
class ZObjsModel;

class ZPunctum;

/*!
 * \brief The class for managing models of stack objects
 *
 * It should only be used as a member of ZStackDoc.
 */
class ZObjsModelManager : public QObject
{
  Q_OBJECT
public:
  explicit ZObjsModelManager(ZStackDoc *parent);

  ZStackDoc* getDocument() const;

  ZObjsModel* getObjsModel(ZStackObject::EType type);
  ZObjsModel* getObjsModel(ZStackObjectRole::TRole role);

  void updateData(const ZStackObject *obj);

  template<typename T>
  T* getObjsModel(ZStackObject::EType type);

  template<typename T>
  T* getObjsModel(ZStackObjectRole::TRole role);

signals:

public slots:
  void processObjectModified(const ZStackObjectInfoSet &infoSet);

private:
  ZStackDoc *m_doc = nullptr;

  QMap<ZStackObject::EType, ZObjsModel*> m_modelMap;
  QMap<ZStackObjectRole::TRole, ZObjsModel*> m_roleModelMap;
};

template<typename T>
T* ZObjsModelManager::getObjsModel(ZStackObject::EType type)
{
  return dynamic_cast<T*>(getObjsModel(type));
}

template<typename T>
T* ZObjsModelManager::getObjsModel(ZStackObjectRole::TRole role)
{
  return dynamic_cast<T*>(getObjsModel(role));
}


#endif // ZOBJSMODELMANAGER_H
