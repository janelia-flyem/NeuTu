#include "zswcobjsmodel.h"

#include "mvc/zstackdoc.h"
#include "zobjsitem.h"
#include "zswctree.h"
#include "tz_swc_tree.h"
#include "QsLog.h"
#include "neutubeconfig.h"
#include "zstackobjectinfo.h"

ZSwcObjsModel::ZSwcObjsModel(ZStackDoc *doc, QObject *parent) :
  ZObjsModel(parent), m_doc(doc)
{
  updateModelData();
}

ZSwcObjsModel::~ZSwcObjsModel()
{
}

#if 0
QVariant ZSwcObjsModel::data(const QModelIndex &index, int role) const
{
  if (role == Qt::ForegroundRole) {
    ZSwcTree *tree = getSwcTree(index);
    if (tree != NULL) {
      return tree->getColor();
    }
  } else {
    return ZObjsModel::data(index, role);
  }

  return QVariant();
}
#endif

QModelIndex ZSwcObjsModel::getIndex(ZSwcTree *tree, int col) const
{
  std::map<ZSwcTree*, int>::const_iterator swc2rIt = m_swcToRow.find(tree);
  if (swc2rIt != m_swcToRow.end()) {
    return index(swc2rIt->second, col);
  }
  return QModelIndex();
}

QModelIndex ZSwcObjsModel::getIndex(Swc_Tree_Node *tn, int col) const
{
  std::map<Swc_Tree_Node*, int>::const_iterator pun2rIt = m_swcTreeNodeToRow.find(tn);
  if (pun2rIt != m_swcTreeNodeToRow.end()) {
    std::map<Swc_Tree_Node*, ZSwcTree*>::const_iterator s2pIt = m_swcTreeNodeToSwc.find(tn);
    std::map<ZSwcTree*, int>::const_iterator p2rIt = m_swcToRow.find(s2pIt->second);
    return index(pun2rIt->second, col, index(p2rIt->second, 0));
  }
  return QModelIndex();
}

ZSwcTree *ZSwcObjsModel::getSwcTree(const QModelIndex &index) const
{
  if (!index.isValid())
    return NULL;

  ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());

  if (item->parent() == m_rootItem)
    return ZStackObject::CastVoidPointer<ZSwcTree>(item->getActualData());
  else
    return NULL;
}

Swc_Tree_Node *ZSwcObjsModel::getSwcTreeNode(const QModelIndex &index) const
{
  if (!index.isValid())
    return NULL;

  ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());

  if (item->parent() && item->parent()->parent() == m_rootItem)
    return static_cast<Swc_Tree_Node*>(item->getActualData());
  else
    return NULL;
}

void ZSwcObjsModel::processObjectModified(const ZStackObjectInfoSet &infoSet)
{
  if (infoSet.hasObjectModified(
        ZStackObject::EType::SWC,
        ZStackObjectInfo::STATE_ADDED | ZStackObjectInfo::STATE_REMOVED |
        ZStackObjectInfo::STATE_SOURCE_CHANGED)) {
    updateModelData();
  }
}

void ZSwcObjsModel::updateModelData()
{
  beginResetModel();
  delete m_rootItem;
  QList<QVariant> rootData;
  /*
  rootData << "swcs" << "id" << "type" << "radius" << "x" << "y" << "z" <<
              "parent_id" << "label" << "weight" << "feature" << "index" << "source";
              */
  rootData << "Neuron" << "Source";
  ZOUT(LTRACE(), 5) << "Update swc model";
  m_rootItem = new ZObjsItem(
        rootData, &(m_doc->getObjectList(ZStackObject::EType::SWC)));
  setupModelData(m_rootItem);
  endResetModel();
}

void ZSwcObjsModel::setupModelData(ZObjsItem *parent)
{
  QList<QVariant> data;

  m_swcToRow.clear();
  m_swcTreeNodeToRow.clear();
  m_swcTreeNodeToSwc.clear();
  QList<ZSwcTree*> swcList = m_doc->getSwcList();
  for (int i=0; i<swcList.size(); i++) {
    data.clear();
    ZSwcTree *swcTree = swcList.at(i);

    data << QString("Neuron %1").arg(i+1)
         << QString::fromStdString(swcTree->getSource());

    ZObjsItem *item = new ZObjsItem(data, swcTree, parent);
    item->setCheckState(swcTree->isVisible() ? Qt::Checked : Qt::Unchecked);
    item->setToolTip(QString("Neuron %1: %2").arg(i + 1).arg(
                             QString::fromStdString(swcTree->getSource())));
    parent->appendChild(item);
    m_swcToRow[swcTree] = i;
  }
}

void ZSwcObjsModel::setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs)
{
  ZObjsModel::setModelIndexCheckState(index, cs);
  ZSwcTree *tree = getSwcTree(index);
  if (tree != NULL) {
#ifdef _DEBUG_
    std::cout << "SWC " << tree->getSource()
              << ((cs == Qt::Checked) ? " checked" : " unchecked") << std::endl;
#endif
    m_doc->setSwcVisible(tree, cs == Qt::Checked);
  }
}

bool ZSwcObjsModel::needCheckbox(const QModelIndex &index) const
{
  return getSwcTree(index) != NULL;
}
