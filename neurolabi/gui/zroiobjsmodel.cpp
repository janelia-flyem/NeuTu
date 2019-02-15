#include "zroiobjsmodel.h"
#include "zstackobject.h"
#include "mvc/zstackdoc.h"
#include "zobjsitem.h"
#include "zstackobjectinfo.h"

ZRoiObjsModel::ZRoiObjsModel(ZStackDoc *doc, QObject *parent) :
  ZObjsModel(parent), m_doc(doc)
{
  updateModelData();
}

ZRoiObjsModel::~ZRoiObjsModel()
{
}

QModelIndex ZRoiObjsModel::getIndex(ZStackObject* roi, int col) const
{
  auto it = m_roiToRow.find(roi);
  if (it != m_roiToRow.end()) {
    return index(it->second, col);
  }
  return QModelIndex();
}

ZStackObject* ZRoiObjsModel::getRoi(const QModelIndex &index) const
{
  if (!index.isValid())
    return NULL;

  ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());

  if (item->parent() == m_rootItem)
    return ZStackObject::CastVoidPointer<ZStackObject>(item->getActualData());
  else
    return NULL;
}

void ZRoiObjsModel::updateModelData()
{
  beginResetModel();
  delete m_rootItem;
  QList<QVariant> rootData;

  rootData << "ROI" << "Source";

  m_rootItem = new ZObjsItem(rootData, NULL);
  setupModelData(m_rootItem);
  endResetModel();
}

void ZRoiObjsModel::processObjectModified(const ZStackObjectInfoSet &infoSet)
{
  foreach (const ZStackObjectInfo &info, infoSet.keys()) {
    if (info.getRole() == ZStackObjectRole::ROLE_ROI) {
      updateModelData();
      break;
    }
  }
}

void ZRoiObjsModel::setupModelData(ZObjsItem *parent)
{
  QList<QVariant> data;

  m_roiToRow.clear();
  QList<ZStackObject*> roiList =
      m_doc->getObjectList(ZStackObjectRole::ROLE_ROI);
  for (int i=0; i< roiList.size(); i++) {
    data.clear();
    ZStackObject *roi = roiList.at(i);

    data << QString("ROI %1").arg(i+1)
         << QString::fromStdString(roi->getSource());

    ZObjsItem *nodeParent = new ZObjsItem(data, roi, parent);
    nodeParent->setCheckState(roi->isVisible() ? Qt::Checked : Qt::Unchecked);
    nodeParent->setToolTip(QString("ROI %1: %2").arg(i + 1).arg(
                             QString::fromStdString(roi->getSource())));
    parent->appendChild(nodeParent);
    m_roiToRow[roi] = i;
  }
}

void ZRoiObjsModel::setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs)
{
  ZObjsModel::setModelIndexCheckState(index, cs);
  if (getRoi(index) != NULL)
    m_doc->setVisible(getRoi(index), cs == Qt::Checked);
}

bool ZRoiObjsModel::needCheckbox(const QModelIndex &index) const
{
  return getRoi(index) != NULL;
}

