#include "zflyembodylistmodel.h"

#include <QDebug>
#include <QRegularExpression>
#include <iostream>

#include "zstring.h"
#include "zflyembodystateaccessor.h"
#include "zflyembodymanager.h"

ZFlyEmBodyListModel::ZFlyEmBodyListModel(QObject *parent) :
  QStringListModel(parent)
{
  connectSignalSlot();
}

ZFlyEmBodyListModel::~ZFlyEmBodyListModel()
{
  delete m_stateAccessor;
}

void ZFlyEmBodyListModel::setBodyStateAccessor(ZFlyEmBodyStateAccessor *sa)
{
  delete m_stateAccessor;
  m_stateAccessor = sa;
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

QVariant ZFlyEmBodyListModel::data(const QModelIndex &index, int role) const
{
  switch (role) {
  case Qt::ForegroundRole:
    if (ZFlyEmBodyManager::encodingSupervoxel(getBodyId(index))) {
      return QColor(0, 128, 0);
    }
    break;
  case Qt::DisplayRole:
  {
    uint64_t bodyId = getBodyId(index);
    if (bodyId > 0) {
      uint64_t decodedBodyId = ZFlyEmBodyManager::decode(bodyId);
      if (ZFlyEmBodyManager::encodingSupervoxel(bodyId)) {
        return QString("%1 (sv)").arg(decodedBodyId);
      } else if (ZFlyEmBodyManager::encodesTar(bodyId) &&
                 ZFlyEmBodyManager::encodedLevel(bodyId) == 0) {
        return QString("%1 (c)").arg(decodedBodyId);
      } else {
        return QString("%1").arg(decodedBodyId);
      }
    } else {
      return QString();
    }
  }
    break;
  default:
    break;
  }

  return QStringListModel::data(index, role);
}

uint64_t ZFlyEmBodyListModel::getBodyId(int row) const
{
  QModelIndex modelIndex = index(row);

#ifdef _DEBUG_2
  qDebug() << modelIndex << " from " << rowCount();
#endif

  return getBodyId(modelIndex);
}

uint64_t ZFlyEmBodyListModel::GetBodyId(const QString &str)
{
  uint64_t bodyId = str.toULongLong();
  if (bodyId == 0) {
    QRegularExpression regexp("^(supervoxel|sv|c)[:\\s]*([0-9]+)",
                              QRegularExpression::CaseInsensitiveOption);
    qDebug() << "Input body str:" << str;
    QRegularExpressionMatch match = regexp.match(str);
    if (match.hasMatch()) {
      bodyId = match.captured(2).toULongLong();
      if (bodyId > 0) {
        if (match.captured(1) == "c") {
          bodyId = ZFlyEmBodyManager::encode(bodyId);
        } else {
          bodyId = ZFlyEmBodyManager::EncodeSupervoxel(bodyId);
        }
      }
    } else {

    }
  }

  return bodyId;
}

uint64_t ZFlyEmBodyListModel::getBodyId(const QModelIndex &index) const
{
  if (!index.isValid()) {
    return 0;
  }

  QString bodyIdStr = this->data(index, Qt::EditRole).toString();

//  if (bodyIdStr.startsWith())

  uint64_t bodyId = GetBodyId(bodyIdStr);

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
    removeRow(row);
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
    setData(modelIndex, QString("%1").arg(bodyId), Qt::EditRole);
  }
}

void ZFlyEmBodyListModel::addBodySliently(uint64_t bodyId)
{
  if (m_bodySet.contains(bodyId)) {
    insertRow(rowCount());
    QModelIndex modelIndex = index(rowCount() - 1);
    setData(modelIndex, QString("%1").arg(bodyId), Qt::EditRole);
  }
}

bool ZFlyEmBodyListModel::isBodyProtected(uint64_t bodyId) const
{
#ifdef _DEBUG_2
  return bodyId > 0;
#endif

  bool captured = false;
  if (m_stateAccessor != NULL) {
    captured = m_stateAccessor->isProtected(bodyId);
  }

  return captured;
}

void ZFlyEmBodyListModel::processChangedRows(
    const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
  QList<int> removingList;

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
    }
  }

  QSet<uint64_t> protectedSet; //For debugging purpose
  foreach (uint64_t bodyId, m_backupSet) {
    if (!newSet.contains(bodyId)) {
      if (isBodyProtected(bodyId)) {
        protectedSet.insert(bodyId);
      } else {
        m_bodySet.remove(bodyId);
#ifdef _DEBUG_
        std::cout << "emit bodyRemoved: " << bodyId << std::endl;
#endif
        emit bodyRemoved(bodyId);
      }
    }
  }

  foreach (uint64_t bodyId, newSet) {
#ifdef _DEBUG_
      std::cout << "emit bodyAdded: " << bodyId << std::endl;
#endif
    emit bodyAdded(bodyId);
  }

  m_backupSet.clear();

  removeRowList(removingList);

#if 0
  m_ignoreDuplicate = true;
  foreach (uint64_t bodyId, protectedSet) {
    addBodySliently(bodyId);
  }
  m_ignoreDuplicate = false;
#endif
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

#if 0
void ZFlyEmBodyListModel::processInsertedRows(
    const QModelIndex &/*modelIndex*/, int first, int last)
{
  QList<int> removingList;

  for (int r = first; r <= last; ++r) {
    uint64_t bodyId = getBodyId(first);
    if (m_bodySet.contains(bodyId) || bodyId <= 0) {
      if (m_ignoreDuplicate == false) {
        removingList.append(r);
      }
    } else {
      m_bodySet.insert(bodyId);
#ifdef _DEBUG_
      std::cout << "emit bodyRemoved: " << bodyId << std::endl;
#endif
      emit bodyAdded(bodyId);
    }
  }

  removeRowList(removingList);
}
#endif

bool ZFlyEmBodyListModel::removeRows(int row, int count, const QModelIndex &parent)
{
  bool removed = false;

  int currentRow = 0;
  int currentCount = 0;

  for (int i = count - 1; i >= 0; --i) {
    currentRow = row + i;
    uint64_t bodyId = getBodyId(currentRow);
    if (!isBodyProtected(bodyId)) {
      if (m_bodySet.contains(bodyId)) {
        m_bodySet.remove(bodyId);
      }
      ++currentCount;
#ifdef _DEBUG_
      std::cout << "emit bodyRemoved: " << bodyId << std::endl;
#endif
      emit bodyRemoved(bodyId);
    } else {
      if (currentCount > 0) {
        removed = removed || QStringListModel::removeRows(
              currentRow, currentCount, parent);
      }
      currentCount = 0;
    }
  }


  if (currentCount > 0) {
    removed = removed || QStringListModel::removeRows(
          currentRow, currentCount, parent);
  }

//  bool removed = QStringListModel::removeRows(row, count, parent);

  return removed;
}

bool ZFlyEmBodyListModel::setData(
    const QModelIndex &index, const QVariant &value, int role)
{
  QVariant newValue = value;

  qDebug() << "Item value:" << value;

  if(role == Qt::EditRole) {
    //Backup the old body ID so that it can be removed later if necessary
    uint64_t oldBodyId = getBodyId(index.row());
    uint64_t bodyId = GetBodyId(value.toString());

    if (oldBodyId == bodyId || isBodyProtected(oldBodyId)) {
      return false;
    }

    if (oldBodyId > 0) {
      m_backupSet.insert(oldBodyId);
    }

    //Set the value to empty if it's a duplication
//    QString bodyIdStr = value.toString();

//    uint64_t bodyId = bodyIdStr.toULongLong();


    if (m_bodySet.contains(bodyId)) {
      newValue.setValue(QString());
    } else {
      newValue.setValue(QString("%1").arg(bodyId));
    }
  }

  return QStringListModel::setData(index, newValue, role);
}

QSet<uint64_t> ZFlyEmBodyListModel::getBodySet() const
{
  return m_bodySet;
}
