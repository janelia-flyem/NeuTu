#include "zdvidversionmodel.h"
#include <queue>
#include <QColor>
#include <QBrush>
#include <QFont>

ZDvidVersionModel::ZDvidVersionModel(QObject *parent) :
  QAbstractItemModel(parent)
{
}

ZDvidVersionModel::~ZDvidVersionModel()
{
  clear();
}

void ZDvidVersionModel::clear()
{
  removeAllRows();
  m_dag.clear();
}

QVariant ZDvidVersionModel::data(
    const QModelIndex &index, int role) const
{
  std::string uuid = getVersionUuid(index);
  bool isLocked = m_dag.isLocked(uuid);
  bool isActive = m_dag.isActive(uuid);
  switch (role) {
  case Qt::DisplayRole:
    switch (index.column()) {
    case 0:
      return QString(getVersionUuid(index).c_str());
    case 1:
      if (isLocked) {
        return "Locked";
      }
      break;
    case 2:
    {
      std::vector<std::string> parentList =
          m_dag.getParentList(getVersionUuid(index));
      if (parentList.size() > 1) {
        std::string str = parentList[1];
        for (size_t i = 2; i < parentList.size(); ++i) {
          str += ", " + parentList[i];
        }
        return str.c_str();
      }
    }
    }
    break;
  case Qt::ForegroundRole:
    switch (index.column()) {
    case 0:
      if (isLocked) {
        return QColor(128, 128, 128);
      }
      break;
    }
    break;
  case Qt::FontRole:
    switch (index.column()) {
    case 0:
      if (isActive) {
        QFont font;
        font.setBold(true);
        return font;
      }
      break;
    }
    break;
  case Qt::BackgroundRole:
    switch (index.column()) {
    case 0:
      if (isActive) {
        return QBrush(QColor(0, 255, 0));
      }
      break;
    }
  default:
    break;
  }

  return QVariant();
}

QVariant ZDvidVersionModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
    case 0:
      return "uuid";
    case 1:
      return "Status";
    case 2:
      return "Other parents";
    default:
      break;
    }
  }

  return QVariant();
}

void ZDvidVersionModel::removeAllRows()
{
  removeRows(0, rowCount(index(0, 0)), index(0, 0));
  removeRows(0, 1, QModelIndex());
}

bool ZDvidVersionModel::removeRows(
    int row, int count, const QModelIndex &parent)
{
  if (count > 0) {
    beginRemoveRows(parent, row, row + count - 1);
    endRemoveRows();

    return true;
  }

  return false;
}


std::string ZDvidVersionModel::getVersionUuid(const QModelIndex &index) const
{
  std::string uuid;

  if (index.isValid()) {
    return getUuid((qint32) index.internalId());
  }

  return uuid;
}

int ZDvidVersionModel::columnCount(const QModelIndex &/*parent*/) const
{
  return 3;
}

int ZDvidVersionModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid()) {
    std::string uuid = getVersionUuid(parent);
    return m_dag.getChildList(uuid).size();
  }

  if (!m_dag.getRoot().empty()) {
    return 1;
  }

  return 0;
}

QModelIndex ZDvidVersionModel::parent(const QModelIndex &index) const
{
  if (index.isValid()) {
    std::string uuid = getVersionUuid(index);
    std::string parentUuid = m_dag.getFirstParent(uuid);
    if (!uuid.empty()) {
      return createIndex(
            m_dag.getSiblingIndex(uuid), 0, parentUuid);
    }
  }

  return QModelIndex();
}

bool ZDvidVersionModel::insertRows(
    int row, int count, const QModelIndex &parent)
{
  beginInsertRows(parent, row, row + count - 1);
  endInsertRows();

  return true;
}

std::string ZDvidVersionModel::getUuid(qint32 id)
{
  char uuid[5];
  memcpy(uuid, &id, 4);
  uuid[4] = '\0';

  return uuid;
}

qint32 ZDvidVersionModel::uuidToInternalId(const std::string &uuid)
{
  qint32 id = 0;
  if (!uuid.empty()) {
    memcpy(&id, uuid.c_str(), uuid.size());
  }

  return id;
}

QModelIndex ZDvidVersionModel::index(
    int row, int column, const QModelIndex &parent) const
{
  if (parent.isValid()) {
    std::string parentUuid = getUuid(parent.internalId());

    if (!parentUuid.empty()) {
      std::string uuid = m_dag.getChild(parentUuid, row);
      return createIndex(row, column, uuid);
    }
  }

  return createIndex(row, column, m_dag.getRoot());
}


QModelIndex ZDvidVersionModel::createIndex(
    int row, int column, const std::string &uuid) const
{
  if (!uuid.empty()) {
    return QAbstractItemModel::createIndex(row, column, uuidToInternalId(uuid));
  }

  return QModelIndex();
}

void ZDvidVersionModel::setRoot(const std::string &uuid)
{
  clear();
  m_dag.setRoot(uuid);
  insertRow(0, QModelIndex());
}

void ZDvidVersionModel::addNode(
    const std::string &uuid, const std::string &parentUuid)
{
  if (m_dag.addNode(uuid, parentUuid)) {
    int row = m_dag.getSiblingIndex(uuid);
    insertRow(row, createIndex(
                m_dag.getSiblingIndex(parentUuid), 0, parentUuid));
  }
}

void ZDvidVersionModel::setDag(const ZDvidVersionDag &dag)
{
  m_dag = dag;
  update();
}

void ZDvidVersionModel::update()
{
  removeAllRows();
  if (!m_dag.isEmpty()) {
    insertRow(0, QModelIndex());
  }

  std::vector<std::string> uuidList = m_dag.getBreadthFirstList();
  for (size_t i = 1; i < uuidList.size(); ++i) {
    std::vector<std::string> parentList = m_dag.getParentList(uuidList[i]);
    for (std::vector<std::string>::const_iterator iter = parentList.begin();
         iter != parentList.end(); ++iter) {
      insertRow(m_dag.getSiblingIndex(uuidList[i]),
                createIndex(m_dag.getSiblingIndex(*iter),0, *iter));
    }
  }
}

QModelIndex ZDvidVersionModel::getIndex(const std::string &uuid) const
{
  QModelIndex index = QModelIndex();

  if (!uuid.empty()) {
    index = createIndex(m_dag.getSiblingIndex(uuid), 0, uuid);
  }

  return index;
}

void ZDvidVersionModel::lockNode(const std::string &uuid)
{
  getDagRef()->lock(uuid);
  updateNodeStatus(uuid);
}

void ZDvidVersionModel::activateNode(const std::string &uuid)
{
  getDagRef()->activateNode(uuid);
  updateNodeStatus(uuid);
}

void ZDvidVersionModel::deactivateNode(const std::string &uuid)
{
  getDagRef()->deactivateNode(uuid);
  updateNodeStatus(uuid);
}

void ZDvidVersionModel::updateNodeStatus(const QModelIndex &modelIndex)
{
  const QModelIndex &topLeft = modelIndex;
  if (topLeft.isValid()) {
    QModelIndex bottomRight =
        index(topLeft.row(), columnCount(topLeft.parent()) - 1, topLeft.parent());
    emit dataChanged (topLeft, bottomRight);
  }
}

void ZDvidVersionModel::updateNodeStatus(const std::string &uuid)
{
  updateNodeStatus(getIndex(uuid));
}
