#include "zgraphobjsmodel.h"

#include <QFileInfo>

#include "zstackdoc.h"
#include "z3dgraph.h"
#include "zobjsitem.h"


ZGraphObjsModel::ZGraphObjsModel(ZStackDoc *doc, QObject *parent) :
  ZObjsModel(parent), m_doc(doc)
{
  updateModelData();
}

ZGraphObjsModel::~ZGraphObjsModel()
{

}

QModelIndex ZGraphObjsModel::getIndex(Z3DGraph *graph, int col) const
{
  std::map<Z3DGraph*, int>::const_iterator pun2rIt = m_graphToRow.find(graph);
  if (pun2rIt != m_graphToRow.end()) {
    std::map<QString, ZObjsItem*>::const_iterator s2pIt =
        m_graphSourceToParent.find(graph->getSource().c_str());
    std::map<ZObjsItem*, int>::const_iterator p2rIt =
        m_graphSourceParentToRow.find(s2pIt->second);
    return index(pun2rIt->second, col, index(p2rIt->second, 0));
  }
  return QModelIndex();
}

Z3DGraph *ZGraphObjsModel::getGraph(const QModelIndex &index) const
{
  if (!index.isValid())
    return NULL;

  ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());

  if (item->parent() && item->parent()->parent() == m_rootItem)
    return ZStackObject::CastVoidPointer<Z3DGraph>(item->getActuralData());
//    return static_cast<ZPunctum*>(item->getObj());
  else
    return NULL;
}

const std::vector<Z3DGraph *> *ZGraphObjsModel::getGraphList(
    const QModelIndex &index) const
{
  if (!index.isValid())
    return NULL;

  ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());

  if (item->parent() == m_rootItem) {
    std::map<ZObjsItem*, int>::const_iterator it;
    it = m_graphSourceParentToRow.find(item);
    if (it == m_graphSourceParentToRow.end()) {
      LERROR() << "Wrong Index";
    } else
      return &(m_graphSeparatedByFile[it->second]);
  }

  return NULL;
}

void ZGraphObjsModel::updateData(Z3DGraph *graph)
{
  QModelIndex index = getIndex(graph);
  if (!index.isValid())
    return;
  ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());
  QList<QVariant> &data = item->getItemData();
  Z3DGraph *p = graph;
  QList<QVariant>::iterator beginit = data.begin();
  beginit++;
  data.erase(beginit, data.end());
  data << p->getSource().c_str();
  emit dataChanged(index, getIndex(graph, item->parent()->columnCount()-1));
}

void ZGraphObjsModel::updateModelData()
{
  beginResetModel();
  delete m_rootItem;
  QList<QVariant> rootData;
  rootData << "Graph" << "Source";

  m_rootItem = new ZObjsItem(
        rootData, &(m_doc->getObjectList(ZStackObject::TYPE_3D_GRAPH)));
  setupModelData(m_rootItem);
  endResetModel();
}

void ZGraphObjsModel::setupModelData(ZObjsItem *parent)
{
  QList<QVariant> data;

  m_graphSourceToParent.clear();
  m_graphSourceToCount.clear();
  m_graphToRow.clear();
  m_graphSourceParentToRow.clear();
  m_graphSeparatedByFile.clear();
  int sourceParentRow = 0;
  QList<ZStackObject*> graphList =
      m_doc->getObjectList(ZStackObject::TYPE_3D_GRAPH);
  int numDigit = numDigits(graphList.size()+1);
  for (int i=0; i<graphList.size(); i++) {
    data.clear();
    Z3DGraph *p = dynamic_cast<Z3DGraph*>(graphList.at(i));
    QFileInfo sourceInfo(p->getSource().c_str());
    if (m_graphSourceToParent.find(p->getSource().c_str()) !=
        m_graphSourceToParent.end()) {
      ZObjsItem *sourceParent = m_graphSourceToParent[p->getSource().c_str()];
      data << QString("Graph %1").
              arg(m_graphSourceToCount[p->getSource().c_str()] + 1,
          numDigit, 10, QLatin1Char('0')) << sourceInfo.fileName();
      m_graphToRow[p] = m_graphSourceToCount[p->getSource().c_str()];
      m_graphSourceToCount[p->getSource().c_str()]++;
      ZObjsItem *graph = new ZObjsItem(data, p, sourceParent);
      graph->setCheckState(p->isVisible() ? Qt::Checked : Qt::Unchecked);
      graph->setToolTip(QString("Graph from: %1").arg(p->getSource().c_str()));
      sourceParent->appendChild(graph);
      m_graphSeparatedByFile[m_graphSourceParentToRow[sourceParent]].push_back(
            dynamic_cast<Z3DGraph*>(graphList.at(i)));
    } else {
      data << sourceInfo.fileName() << "source";
      m_graphSeparatedByFile.push_back(std::vector<Z3DGraph*>());
      ZObjsItem *sourceParent = new ZObjsItem(data, NULL, parent);
      sourceParent->setToolTip(QString("Graph source: %1").arg(p->getSource().c_str()));
      m_graphSourceToParent[p->getSource().c_str()] = sourceParent;
      m_graphSourceToCount[p->getSource().c_str()] = 0;
      parent->appendChild(sourceParent);
      m_graphSourceParentToRow[sourceParent] = sourceParentRow++;

      data.clear();
      data << QString("graph %1").arg(m_graphSourceToCount[p->getSource().c_str()] + 1, numDigit, 10, QLatin1Char('0'))
           << sourceInfo.fileName();
      m_graphToRow[p] = m_graphSourceToCount[p->getSource().c_str()];
      m_graphSourceToCount[p->getSource().c_str()]++;
      ZObjsItem *graph = new ZObjsItem(data, p, sourceParent);
      graph->setCheckState(p->isVisible() ? Qt::Checked : Qt::Unchecked);
      sourceParent->setCheckState(p->isVisible() ? Qt::Checked : Qt::Unchecked);
      graph->setToolTip(QString("graph from: %1").arg(p->getSource().c_str()));
      sourceParent->appendChild(graph);
      m_graphSeparatedByFile[m_graphSourceParentToRow[sourceParent]].push_back(
            dynamic_cast<Z3DGraph*>(graphList.at(i)));
    }
  }
}

void ZGraphObjsModel::setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs)
{
  ZObjsModel::setModelIndexCheckState(index, cs);
  if (getGraph(index) != NULL)
    m_doc->setGraphVisible(getGraph(index), cs == Qt::Checked);
}

bool ZGraphObjsModel::needCheckbox(const QModelIndex &index) const
{
  if (index.isValid()) {
    return true;
  }

  QModelIndex idx = parent(index);
  if (idx.isValid() && static_cast<ZObjsItem*>(idx.internalPointer()) == m_rootItem) {
    return true;
  }
  return getGraph(index) != NULL;
}



