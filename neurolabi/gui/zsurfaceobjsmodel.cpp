#include "zsurfaceobjsmodel.h"

#include <QFileInfo>

#include "zstackdoc.h"
#include "zcubearray.h"
#include "zobjsitem.h"
#include "neutubeconfig.h"

ZSurfaceObjsModel::ZSurfaceObjsModel(ZStackDoc *doc, QObject *parent) :
  ZObjsModel(parent), m_doc(doc)
{
  updateModelData();
}

ZSurfaceObjsModel::~ZSurfaceObjsModel()
{

}

QModelIndex ZSurfaceObjsModel::getIndex(ZCubeArray *cubearray, int col) const
{
  std::map<ZCubeArray*, int>::const_iterator pun2rIt = m_surfaceToRow.find(cubearray);
  if (pun2rIt != m_surfaceToRow.end()) {
    std::map<QString, ZObjsItem*>::const_iterator s2pIt =
        m_surfaceSourceToParent.find(cubearray->getSource().c_str());
    std::map<ZObjsItem*, int>::const_iterator p2rIt =
        m_surfaceSourceParentToRow.find(s2pIt->second);
    return index(pun2rIt->second, col, index(p2rIt->second, 0));
  }
  return QModelIndex();
}

ZCubeArray *ZSurfaceObjsModel::getSurface(const QModelIndex &index) const
{
  if (!index.isValid())
    return NULL;

  ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());

  if (item->parent() && item->parent()->parent() == m_rootItem)
    return ZStackObject::CastVoidPointer<ZCubeArray>(item->getActuralData());
//    return static_cast<ZPunctum*>(item->getObj());
  else
    return NULL;
}

const std::vector<ZCubeArray *> *ZSurfaceObjsModel::getSurfaceList(
    const QModelIndex &index) const
{
  if (!index.isValid())
    return NULL;

  ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());

  if (item->parent() == m_rootItem) {
    std::map<ZObjsItem*, int>::const_iterator it;
    it = m_surfaceSourceParentToRow.find(item);
    if (it == m_surfaceSourceParentToRow.end()) {
      LERROR() << "Wrong Index";
    } else
      return &(m_surfaceSeparatedByFile[it->second]);
  }

  return NULL;
}

void ZSurfaceObjsModel::updateData(ZCubeArray *cubearray)
{
  QModelIndex index = getIndex(cubearray);
  if (!index.isValid())
    return;
  ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());
  QList<QVariant> &data = item->getItemData();
  ZCubeArray *p = cubearray;
  QList<QVariant>::iterator beginit = data.begin();
  beginit++;
  data.erase(beginit, data.end());
  data << p->getSource().c_str();
  emit dataChanged(index, getIndex(cubearray, item->parent()->columnCount()-1));
}

void ZSurfaceObjsModel::updateModelData()
{
  beginResetModel();
  delete m_rootItem;
  QList<QVariant> rootData;
  rootData << "Surface" << "Source";

  ZOUT(LTRACE(), 5) << "Update surface object";
  m_rootItem = new ZObjsItem(
        rootData, &(m_doc->getObjectList(ZStackObject::TYPE_3D_CUBE)));
  setupModelData(m_rootItem);
  endResetModel();
}

void ZSurfaceObjsModel::setupModelData(ZObjsItem *parent)
{
  QList<QVariant> data;

  m_surfaceSourceToParent.clear();
  m_surfaceSourceToCount.clear();
  m_surfaceToRow.clear();
  m_surfaceSourceParentToRow.clear();
  m_surfaceSeparatedByFile.clear();
  int sourceParentRow = 0;
  ZOUT(LTRACE(), 5) << "Setup surface model";
  QList<ZStackObject*> surfaceList =
      m_doc->getObjectList(ZStackObject::TYPE_3D_CUBE);
  int numDigit = numDigits(surfaceList.size()+1);
  for (int i=0; i<surfaceList.size(); i++) {
    data.clear();
    ZCubeArray *p = dynamic_cast<ZCubeArray*>(surfaceList.at(i));
    QFileInfo sourceInfo(p->getSource().c_str());
    if (m_surfaceSourceToParent.find(p->getSource().c_str()) !=
        m_surfaceSourceToParent.end()) {
      ZObjsItem *sourceParent = m_surfaceSourceToParent[p->getSource().c_str()];
      data << QString("Surface %1").
              arg(m_surfaceSourceToCount[p->getSource().c_str()] + 1,
          numDigit, 10, QLatin1Char('0')) << sourceInfo.fileName();
      m_surfaceToRow[p] = m_surfaceSourceToCount[p->getSource().c_str()];
      m_surfaceSourceToCount[p->getSource().c_str()]++;
      ZObjsItem *cubearray = new ZObjsItem(data, p, sourceParent);
      cubearray->setCheckState(p->isVisible() ? Qt::Checked : Qt::Unchecked);
      cubearray->setToolTip(QString("Surface from: %1").arg(p->getSource().c_str()));
      sourceParent->appendChild(cubearray);
      m_surfaceSeparatedByFile[m_surfaceSourceParentToRow[sourceParent]].push_back(
            dynamic_cast<ZCubeArray*>(surfaceList.at(i)));
    } else {
      data << sourceInfo.fileName() << "source";
      m_surfaceSeparatedByFile.push_back(std::vector<ZCubeArray*>());
      ZObjsItem *sourceParent = new ZObjsItem(data, NULL, parent);
      sourceParent->setToolTip(QString("Surface source: %1").arg(p->getSource().c_str()));
      m_surfaceSourceToParent[p->getSource().c_str()] = sourceParent;
      m_surfaceSourceToCount[p->getSource().c_str()] = 0;
      parent->appendChild(sourceParent);
      m_surfaceSourceParentToRow[sourceParent] = sourceParentRow++;

      data.clear();
      data << QString("surface %1").arg(m_surfaceSourceToCount[p->getSource().c_str()] + 1, numDigit, 10, QLatin1Char('0'))
           << sourceInfo.fileName();
      m_surfaceToRow[p] = m_surfaceSourceToCount[p->getSource().c_str()];
      m_surfaceSourceToCount[p->getSource().c_str()]++;
      ZObjsItem *cubearray = new ZObjsItem(data, p, sourceParent);
      cubearray->setCheckState(p->isVisible() ? Qt::Checked : Qt::Unchecked);
      sourceParent->setCheckState(p->isVisible() ? Qt::Checked : Qt::Unchecked);
      cubearray->setToolTip(QString("surface from: %1").arg(p->getSource().c_str()));
      sourceParent->appendChild(cubearray);
      m_surfaceSeparatedByFile[m_surfaceSourceParentToRow[sourceParent]].push_back(
            dynamic_cast<ZCubeArray*>(surfaceList.at(i)));
    }
  }
}

void ZSurfaceObjsModel::setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs)
{
  ZObjsModel::setModelIndexCheckState(index, cs);
  if (getSurface(index) != NULL)
    m_doc->setSurfaceVisible(getSurface(index), cs == Qt::Checked);
}

bool ZSurfaceObjsModel::needCheckbox(const QModelIndex &index) const
{
  if (index.isValid()) {
    return true;
  }

  QModelIndex idx = parent(index);
  if (idx.isValid() && static_cast<ZObjsItem*>(idx.internalPointer()) == m_rootItem) {
    return true;
  }
  return getSurface(index) != NULL;
}



