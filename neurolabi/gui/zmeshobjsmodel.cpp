#include "zmeshobjsmodel.h"

#include "mvc/zstackdoc.h"
#include "zobjsitem.h"
#include "zmesh.h"
#include "QsLog.h"
#include "neutubeconfig.h"
#include "zstackdocproxy.h"

ZMeshObjsModel::ZMeshObjsModel(ZStackDoc *doc, QObject *parent) :
  ZObjsModel(parent), m_doc(doc)
{
  updateModelData();
}

ZMeshObjsModel::~ZMeshObjsModel()
{
}

QModelIndex ZMeshObjsModel::getIndex(ZMesh* mesh, int col) const
{
  auto it = m_meshToRow.find(mesh);
  if (it != m_meshToRow.end()) {
    return index(it->second, col);
  }
  return QModelIndex();
}

ZMesh* ZMeshObjsModel::getMesh(const QModelIndex &index) const
{
  if (!index.isValid())
    return NULL;

  ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());

  if (item->parent() == m_rootItem)
    return ZStackObject::CastVoidPointer<ZMesh>(item->getActualData());
  else
    return NULL;
}

void ZMeshObjsModel::updateModelData()
{
  beginResetModel();
  delete m_rootItem;
  QList<QVariant> rootData;

  rootData << "Mesh" << "Source";
  ZOUT(LTRACE(), 5) << "Update mesh model";
  m_rootItem = new ZObjsItem(
        rootData, &(m_doc->getObjectList(ZStackObject::EType::MESH)));
  setupModelData(m_rootItem);
  endResetModel();
}

void ZMeshObjsModel::setupModelData(ZObjsItem *parent)
{
  QList<QVariant> data;

  m_meshToRow.clear();
  QList<ZMesh*> meshList = ZStackDocProxy::GetGeneralMeshList(m_doc);
//      m_doc->getMeshList();
  for (int i=0; i<meshList.size(); i++) {
    data.clear();
    ZMesh *mesh = meshList.at(i);

    data << QString("Mesh %1").arg(i+1)
         << QString::fromStdString(mesh->getSource());

    ZObjsItem *nodeParent = new ZObjsItem(data, mesh, parent);
    nodeParent->setCheckState(mesh->isVisible() ? Qt::Checked : Qt::Unchecked);
    nodeParent->setToolTip(QString("Mesh %1: %2").arg(i + 1).arg(
                             QString::fromStdString(mesh->getSource())));
    parent->appendChild(nodeParent);
    m_meshToRow[mesh] = i;
  }
}

void ZMeshObjsModel::setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs)
{
  ZObjsModel::setModelIndexCheckState(index, cs);
  if (getMesh(index) != NULL)
    m_doc->setMeshVisible(getMesh(index), cs == Qt::Checked);
}

bool ZMeshObjsModel::needCheckbox(const QModelIndex &index) const
{
  return getMesh(index) != NULL;
}

void ZMeshObjsModel::processObjectModified(const ZStackObjectInfoSet &infoSet)
{
  if (infoSet.contains(ZStackObject::EType::MESH) &&
      !infoSet.onlyVisibilityChanged(ZStackObject::EType::MESH)) {
    updateModelData();
  }
}
