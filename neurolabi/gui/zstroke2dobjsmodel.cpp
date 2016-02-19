#include "zstroke2dobjsmodel.h"
#include "zstackdoc.h"
#include "zobjsitem.h"
#include "zstroke2d.h"

ZStroke2dObjsModel::ZStroke2dObjsModel(ZStackDoc *doc, QObject *parent) :
  ZObjsModel(parent), m_doc(doc)
{
  updateModelData();
}

ZStroke2dObjsModel::~ZStroke2dObjsModel()
{

}

QModelIndex ZStroke2dObjsModel::getIndex(ZStroke2d *stroke, int col) const
{
  std::map<ZStroke2d*, int>::const_iterator iter = m_strokeToRow.find(stroke);

  QModelIndex modelIndex;

  if (iter != m_strokeToRow.end()) {
    modelIndex = index(iter->second, col);
  }

  return modelIndex;
}

ZStroke2d* ZStroke2dObjsModel::getStroke2d(const QModelIndex &index) const
{
  ZStroke2d *stroke = NULL;
  if (index.isValid()) {
    ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());

    if (item->parent() == m_rootItem) {
      stroke = ZStackObject::CastVoidPointer<ZStroke2d>(item->getActuralData());
    }
  }

  return stroke;
}

void ZStroke2dObjsModel::updateModelData()
{
  beginResetModel();
  delete m_rootItem;
  QList<QVariant> rootData;
  rootData << "ID" << "Role";
  m_rootItem = new ZObjsItem(rootData, NULL);
  setupModelData(m_rootItem);
  endResetModel();
}

void ZStroke2dObjsModel::setupModelData(ZObjsItem */*parent*/)
{
  //todo
}
