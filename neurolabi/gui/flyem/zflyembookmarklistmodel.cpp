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

const ZFlyEmBookmark* ZFlyEmBookmarkListModel::getBookmark(int index) const
{
  if (index < 0 || index >= (int) m_bookmarkArray.size()) {
    return NULL;
  }

  return m_bookmarkArray[index];
}

ZFlyEmBookmark *ZFlyEmBookmarkListModel::getBookmark(int index)
{
  return const_cast<ZFlyEmBookmark*>(
        static_cast<const ZFlyEmBookmarkListModel&>(*this).getBookmark(index));
}

const ZFlyEmBookmarkPtrArray& ZFlyEmBookmarkListModel::getBookmarkArray() const
{
  return m_bookmarkArray;
}

QVariant ZFlyEmBookmarkListModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  if (index.row() >= rowCount(index) || index.row() < 0) {
    return QVariant();
  }

  const ZFlyEmBookmark *bookmark = getBookmark(index.row());
  return m_presenter->data(*bookmark, index.column(), role);
}

QModelIndex ZFlyEmBookmarkListModel::index(
    int row, int column, const QModelIndex &/*parent*/) const
{
  return createIndex(row, column, row);
}

/*
void ZFlyEmBookmarkListModel::load(const QString &filePath)
{
  m_bookmarkArray.importJsonFile(filePath.toStdString());
}
*/
void ZFlyEmBookmarkListModel::clear()
{
  removeRows(0, rowCount());
  m_bookmarkArray.clear();
}

void ZFlyEmBookmarkListModel::append(const ZFlyEmBookmark *bookmark)
{
  m_bookmarkArray.append(const_cast<ZFlyEmBookmark*>(bookmark));
  insertRow(rowCount() - 1);
  //QModelIndex topLeft = createIndex(rowCount() - 1, 0);
  //QModelIndex bottomRight = createIndex(rowCount() - 1, columnCount() - 1);

  //emit dataChanged(topLeft, bottomRight);
}

bool ZFlyEmBookmarkListModel::insertRows(
    int row, int count, const QModelIndex &parent)
{
  if (count > 0) {
    beginInsertRows(parent, row, row + count - 1);
    endInsertRows();

    return true;
  }

  return false;
}

bool ZFlyEmBookmarkListModel::insertColumns(
    int col, int count, const QModelIndex &parent)
{
  if (count > 0) {
    beginInsertColumns(parent, col, col + count - 1);
    endInsertColumns();

    return true;
  }

  return false;
}

bool ZFlyEmBookmarkListModel::removeRows(
    int row, int count, const QModelIndex &parent)
{
  if (count > 0) {
    beginRemoveRows(parent, row, row + count - 1);

    QVector<ZFlyEmBookmark*> removing;
    for (int i = row; i < count; ++i) {
      ZFlyEmBookmark *bookmark = getBookmark(i);
      removing.append(bookmark);
    }
    m_bookmarkArray.remove(removing);

    endRemoveRows();

    return true;
  }

  return false;
}

bool ZFlyEmBookmarkListModel::removeColumns(
    int col, int count, const QModelIndex &parent)
{
  if (count > 0){
    beginRemoveColumns(parent, col, col + count - 1);
    endRemoveColumns();

    return true;
  }

  return false;
}

void ZFlyEmBookmarkListModel::update(int row)
{
  emit dataChanged(index(row, 0), index(row, columnCount() - 1));
}

void ZFlyEmBookmarkListModel::removeBookmark(ZFlyEmBookmark *bookmark)
{
  int index = m_bookmarkArray.findFirstIndex(bookmark);
  if (index>= 0) {
    removeRow(index);
  }
}

/*
void ZFlyEmBookmarkListModel::append(const ZFlyEmBookmark &bookmark)
{
  m_bookmarkArray.push_back(bookmark);
  insertRows(rowCount() - 1, 1);
}
*/
