#include "zsegmentationprojectmodel.h"
#include "zstackframe.h"
#include "zstackdoc.h"
#include "zobject3dscanarray.h"
#include "zobject3dfactory.h"

ZSegmentationProjectModel::ZSegmentationProjectModel(QObject *parent) :
  QAbstractItemModel(parent), m_data(NULL)
{
}

ZSegmentationProjectModel::~ZSegmentationProjectModel()
{
}

void ZSegmentationProjectModel::clear()
{
  beginResetModel();
//  reset();
  //removeAllRows();
  if (m_data != NULL) {
    m_data->clear();
  }
  endResetModel();
}

int ZSegmentationProjectModel::columnCount(const QModelIndex &/*parent*/) const
{
  return 1;
}

int ZSegmentationProjectModel::rowCount(const QModelIndex &parent) const
{
  ZTreeNode<ZObject3dScan> *parentLabel = getLabelNode(parent);
  int count = 1;
  if (parentLabel != NULL) {
    count = parentLabel->childNumber();
  }

  return count;
}

void ZSegmentationProjectModel::removeAllRows()
{
  removeRows(0, rowCount(index(0, 0)), index(0, 0));
  removeRows(0, 1, QModelIndex());
}

QVariant ZSegmentationProjectModel::data(const QModelIndex &index, int role) const
{
  switch (role) {
  case Qt::DisplayRole:
    switch (index.column()) {
    case 0:
    {
      ZTreeNode<ZObject3dScan> *labelNode = getLabelNode(index);
      if (labelNode != NULL) {
        return (int) labelNode->data().getLabel();
      }
    }
    default:
      break;
    }
  default:
    break;
  }

  return QVariant();
}

QModelIndex ZSegmentationProjectModel::index(
    int row, int column, const QModelIndex &parent) const
{
  if (parent.isValid()) {
    ZTreeNode<ZObject3dScan> *parentLabel =
        static_cast<ZTreeNode<ZObject3dScan>*>(parent.internalPointer());
    if (parentLabel != NULL) {
      return createIndex(row, column, parentLabel->getChild(row));
    }
  }

  if (m_data->getRootLabel() == NULL) {
    return QModelIndex();
  }

  return createIndex(row, column, m_data->getRootLabel());
}

QModelIndex ZSegmentationProjectModel::parent(const QModelIndex &index) const
{
  if (index.isValid()) {
    ZTreeNode<ZObject3dScan> *labelNode = getLabelNode(index);
    if (labelNode != NULL) {
      ZTreeNode<ZObject3dScan> *parentLabel = labelNode->parent();
      if (parentLabel != NULL) {
        return createIndex(parentLabel->getSiblingIndex(), 0, parentLabel);
      }
    }
  }

  return QModelIndex();
}

ZTreeNode<ZObject3dScan>* ZSegmentationProjectModel::getLabelNode(
    const QModelIndex &index) const
{
  return static_cast<ZTreeNode<ZObject3dScan>*>(index.internalPointer());
}

void ZSegmentationProjectModel::updateSegmentation()
{
  ZTreeNode<ZObject3dScan> *labelNode = getLabelNode(m_currentIndex);
  if (labelNode != NULL) {
    removeRows(0, labelNode->childNumber(), m_currentIndex);
    labelNode->killDownstream();

    const ZStack *stack =
        getProject()->getDataFrame()->document()->getLabelField();
    ZObject3dScanArray *objArray = NULL;
    if (stack != NULL) {
      objArray = ZObject3dFactory::MakeObject3dScanArray(*stack);

      if (objArray != NULL) {
        if (!objArray->empty()) {
          //beginInsertRows(m_currentIndex, 0, objArray->size());
          int row = 0;
          for (ZObject3dScanArray::iterator iter = objArray->begin();
               iter != objArray->end(); ++iter) {
            ZObject3dScan &obj = *iter;
            ZTreeNode<ZObject3dScan> *childNode = new ZTreeNode<ZObject3dScan>;
            childNode->setData(obj);
            childNode->setParent(labelNode);
            insertRow(row++, m_currentIndex);
          }
          //endInsertRows();
        }
      }
      delete objArray;
    }
  }
  //emit dataChanged(index(0, 0, ));
}

QVariant ZSegmentationProjectModel::headerData(
    int /*section*/, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    return "Label";
  }

  return QVariant();
}

void ZSegmentationProjectModel::loadSegmentationTarget(const QModelIndex &index)
{
  ZTreeNode<ZObject3dScan> *labelNode = getLabelNode(index);
  m_data->loadSegmentationTarget(labelNode);
  m_currentIndex = index;
}

bool ZSegmentationProjectModel::insertRows(
    int row, int count, const QModelIndex &parent)
{
  beginInsertRows(parent, row, row + count - 1);
  endInsertRows();

  return true;
}

void ZSegmentationProjectModel::generateTestData()
{
  getProject()->generateTestData();
  m_currentIndex = index(0, 0);
}

bool ZSegmentationProjectModel::removeRows(
    int row, int count, const QModelIndex &parent)
{
  if (count > 0) {
    beginRemoveRows(parent, row, row + count - 1);
    endRemoveRows();

    return true;
  }

  return false;
}

void ZSegmentationProjectModel::loadStack(const QString &fileName)
{
  getProject()->loadStack(fileName);
  removeRows(0, rowCount(index(0, 0)), index(0, 0));
  m_currentIndex = index(0, 0);
}
