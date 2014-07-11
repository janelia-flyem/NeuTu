#include "zflyembookmarklistmodel.h"

ZFlyEmBookmarkListModel::ZFlyEmBookmarkListModel(QObject *parent) :
  QAbstractTableModel(parent)
{
  m_defaultPresenter = new ZFlyEmBookmarkPresenter(this);
  m_presenter = m_defaultPresenter;
}

QVariant ZFlyEmBookmarkListModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
  return m_presenter->headerData(section, orientation, role);
}

int ZFlyEmBookmarkListModel::rowCount(const QModelIndex &/*parent*/) const
{
  return m_bookmarkArray.size();
}

int ZFlyEmBookmarkListModel::columnCount(const QModelIndex &/*parent*/) const
{
  return m_presenter->columnCount();
}

const ZFlyEmBookmark& ZFlyEmBookmarkListModel::getBookmark(int row) const
{
  return m_bookmarkArray[row];
}

QVariant ZFlyEmBookmarkListModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  if (index.row() >= rowCount(index) || index.row() < 0) {
    return QVariant();
  }

  const ZFlyEmBookmark &bookmark = getBookmark(index.row());
  return m_presenter->data(bookmark, index.column(), role);
}

void ZFlyEmBookmarkListModel::load(const QString &filePath)
{
  m_bookmarkArray.importJsonFile(filePath.toStdString());
}

void ZFlyEmBookmarkListModel::clear()
{
  removeRows(0, rowCount());
  m_bookmarkArray.clear();
}

void ZFlyEmBookmarkListModel::append(const ZFlyEmBookmark &bookmark)
{
  m_bookmarkArray.append(bookmark);
  insertRow(rowCount() - 1);
  //QModelIndex topLeft = createIndex(rowCount() - 1, 0);
  //QModelIndex bottomRight = createIndex(rowCount() - 1, columnCount() - 1);

  //emit dataChanged(topLeft, bottomRight);
}

bool ZFlyEmBookmarkListModel::insertRows(
    int row, int count, const QModelIndex &parent)
{
  beginInsertRows(parent, row, row + count - 1);
  endInsertRows();

  return true;
}

bool ZFlyEmBookmarkListModel::insertColumns(
    int col, int count, const QModelIndex &parent)
{
  beginInsertColumns(parent, col, col + count - 1);
  endInsertColumns();

  return true;
}

bool ZFlyEmBookmarkListModel::removeRows(
    int row, int count, const QModelIndex &parent)
{
  beginRemoveRows(parent, row, row + count - 1);
  endRemoveRows();

  return true;
}

bool ZFlyEmBookmarkListModel::removeColumns(
    int col, int count, const QModelIndex &parent)
{
  beginRemoveColumns(parent, col, col + count - 1);
  endRemoveColumns();

  return true;
}
