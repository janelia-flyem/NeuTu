#include "zflyembodylistmodel.h"

#include <QDebug>

#include "zstring.h"

ZFlyEmBodyListModel::ZFlyEmBodyListModel(QObject *parent) :
  QStringListModel(parent)
{
  connectSignalSlot();
}

void ZFlyEmBodyListModel::connectSignalSlot()
{
  connect(this, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
          this, SLOT(processChangedRows(QModelIndex,QModelIndex)));
//  connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
//          this, SLOT(processInsertedRows(QModelIndex,int,int)));
//  connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)),
//          this, SLOT(processRemovedRows(QModelIndex,int,int)));
}

#if 0
Qt::ItemDataRole ZFlyEmBodyListModel::getBackupRole()
{
  return Qt::UserRole;
}
#endif

uint64_t ZFlyEmBodyListModel::getBodyId(int row) const
{
  QModelIndex modelIndex = index(row);

#ifdef _DEBUG_2
  qDebug() << modelIndex << " from " << rowCount();
#endif

  return getBodyId(modelIndex);
}

uint64_t ZFlyEmBodyListModel::getBodyId(const QModelIndex &index) const
{
  if (!index.isValid()) {
    return 0;
  }

  QString bodyIdStr = this->data(index, Qt::DisplayRole).toString();

  uint64_t bodyId = bodyIdStr.toULongLong();

  return bodyId;
}

int ZFlyEmBodyListModel::getRow(uint64_t bodyId) const
{
  int row = -1;

  for (int i = 0; i < rowCount(); ++i) {
    if (getBodyId(i) == bodyId) {
      row = i;
      break;
    }
  }

  return row;
}

void ZFlyEmBodyListModel::removeBody(uint64_t bodyId)
{
  int row = getRow(bodyId);
  if (row >= 0) {
    removeRow(0);
  }
}

void ZFlyEmBodyListModel::removeAllBodies()
{
  removeRows(0, rowCount(), QModelIndex());
}

QModelIndex ZFlyEmBodyListModel::getIndex(uint64_t bodyId) const
{
  return index(getRow(bodyId));
}

void ZFlyEmBodyListModel::removeRowList(const QList<int> &rowList)
{
  QList<int> sortedList = rowList;
  qSort(sortedList);
  for (int i = sortedList.size() - 1; i >= 0; --i) {
    int row = sortedList[i];
    removeRow(row);
  }
}

void ZFlyEmBodyListModel::addBody(uint64_t bodyId)
{
  if (!m_bodySet.contains(bodyId)) {
    insertRow(rowCount());
    QModelIndex modelIndex = index(rowCount() - 1);
    setData(modelIndex, QString("%1").arg(bodyId), Qt::DisplayRole);
  }
}

void ZFlyEmBodyListModel::processChangedRows(
    const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
  QList<int> removingList;

#ifdef _DEBUG_2
    qDebug() << data(topLeft, Qt::DisplayRole) << data(topLeft, getBackupRole());
#endif

  QSet<uint64_t> newSet; //The set of bodies to be added

  int first = topLeft.row();
  int last = bottomRight.row();
  for (int r = first; r <= last; ++r) {
    uint64_t bodyId = getBodyId(first);
    if (bodyId <= 0) {
      removingList.append(r);
    } else {
      newSet.insert(bodyId);
      m_bodySet.insert(bodyId);
      emit bodyAdded(bodyId);
    }
  }

  foreach (uint64_t bodyId, m_backupSet) {
    if (!newSet.contains(bodyId) && m_bodySet.contains(bodyId)) {
      m_bodySet.remove(bodyId);
      emit bodyRemoved(bodyId);
    }
  }
  m_backupSet.clear();

  removeRowList(removingList);
}

void ZFlyEmBodyListModel::backupBody(int row, bool appending)
{
  if (!appending) {
    m_backupSet.clear();
  }

  uint64_t bodyId = getBodyId(row);

  if (bodyId > 0) {
    m_backupSet.insert(bodyId);
  }
}

void ZFlyEmBodyListModel::processInsertedRows(
    const QModelIndex &/*modelIndex*/, int first, int last)
{
  QList<int> removingList;

  for (int r = first; r <= last; ++r) {
    uint64_t bodyId = getBodyId(first);
    if (m_bodySet.contains(bodyId) || bodyId <= 0) {
      removingList.append(r);
    } else {
      m_bodySet.insert(bodyId);
      emit bodyAdded(bodyId);
    }
  }

  removeRowList(removingList);
}

bool ZFlyEmBodyListModel::removeRows(int row, int count, const QModelIndex &parent)
{
  for (int i = 0; i < count; ++i) {
    uint64_t bodyId = getBodyId(row + i);
    if (m_bodySet.contains(bodyId)) {
      m_bodySet.remove(bodyId);
      emit bodyRemoved(bodyId);
    }
  }

  return QStringListModel::removeRows(row, count, parent);
}

bool ZFlyEmBodyListModel::setData(
    const QModelIndex &index, const QVariant &value, int role)
{
  QVariant newValue = value;

  if(role == Qt::DisplayRole || role == Qt::EditRole) {
    //Backup the old body ID so that it can be removed later if necessary
    uint64_t oldBodyId = getBodyId(index.row());
    if (oldBodyId > 0) {
      m_backupSet.insert(oldBodyId);
    }

    //Set the value to empty if it's a duplication
    QString bodyIdStr = value.toString();

    uint64_t bodyId = bodyIdStr.toULongLong();

    if (oldBodyId == bodyId) {
      return false;
    }

    if (m_bodySet.contains(bodyId)) {
      newValue.setValue(QString());
    }
  }

  return QStringListModel::setData(index, newValue, role);
}
