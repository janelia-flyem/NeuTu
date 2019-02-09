#include "zflyemroiobjsmodel.h"

#include "zstackdoc.h"
#include "logging/zqslog.h"
#include "neutubeconfig.h"
#include "zobjsitem.h"

ZFlyEMRoiObjsModel::ZFlyEMRoiObjsModel(ZStackDocPtr doc, QObject *parent) :
  ZObjsModel(parent), m_doc(doc)
{

}

ZFlyEMRoiObjsModel::~ZFlyEMRoiObjsModel()
{

}

QVariant ZFlyEMRoiObjsModel::data(const QModelIndex &index, int role) const
{
  if (role == Qt::BackgroundColorRole) {
    ZStackObject *roi = getRoi(index);
    if (roi != NULL) {
      return roi->getColor();
    }
  } else {
    return ZObjsModel::data(index, role);
  }

  return QVariant();
}

ZStackDocPtr ZFlyEMRoiObjsModel::getDocument() const
{
  return m_doc;
}

QModelIndex ZFlyEMRoiObjsModel::getIndex(ZStackObject *roi, int col) const
{
  std::map<ZStackObject*, int>::const_iterator iter = m_roiToRow.find(roi);
  if (iter != m_roiToRow.end()) {
    return index(iter->second, col);
  }
  return QModelIndex();
}

ZStackObject* ZFlyEMRoiObjsModel::getRoi(const QModelIndex &index) const
{
  ZStackObject *obj = NULL;
  if (index.isValid()) {
    ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());
    if (item->parent() == m_rootItem) {
      obj = ZStackObject::CastVoidPointer<ZStackObject>(item->getActualData());
    }
  }

  return obj;
}

void ZFlyEMRoiObjsModel::updateModelData()
{
  beginResetModel();
  delete m_rootItem;
  QList<QVariant> rootData;
  rootData << "ROI" << "Color";

  ZOUT(LTRACE(), 5) << "Update roi model";
  m_rootItem = new ZObjsItem(rootData, &(m_roiList));
  setupModelData(m_rootItem);
  endResetModel();
}

void ZFlyEMRoiObjsModel::setupModelData(ZObjsItem *parent)
{
  QList<QVariant> data;

  m_roiList = m_doc->getObjectList(ZStackObjectRole::ROLE_ROI);
  for (int i = 0; i < m_roiList.size(); ++i) {
    data.clear();
    ZStackObject *roi = m_roiList.at(i);
    data << roi->getSource().c_str() << "Color";
    ZObjsItem *item = new ZObjsItem(data, roi, parent);
    item->setCheckState(roi->isVisible() ? Qt::Checked : Qt::Unchecked);
    parent->appendChild(item);
    m_roiToRow[roi] = i;
  }
}
bool ZFlyEMRoiObjsModel::needCheckbox(const QModelIndex &index) const
{
  return getRoi(index) != NULL;
}
