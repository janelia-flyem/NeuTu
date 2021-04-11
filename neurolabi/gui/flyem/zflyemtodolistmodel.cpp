#include "zflyemtodolistmodel.h"

#include <QItemSelectionModel>

#include "zflyemproofdoc.h"
#include "zflyemtodoitem.h"

ZFlyEmTodoListModel::ZFlyEmTodoListModel(QObject *parent) :
  ZSortFilterTableModel(parent)
{
  init();
}

void ZFlyEmTodoListModel::init()
{
  connectSignalSlot();

//  m_proxy = new ZSortFilterProxyModel;
//  m_proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
//  m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
//  m_proxy->setFilterKeyColumn(-1);
//  m_proxy->setSourceModel(this);
}

void ZFlyEmTodoListModel::connectSignalSlot()
{
}

//QModelIndex ZFlyEmTodoListModel::getMappedIndex(const QModelIndex &index)
//{
//  if (m_proxy != NULL) {
//    return m_proxy->mapToSource(index);
//  }

//  return index;
//}

void ZFlyEmTodoListModel::processDoubleClick(const QModelIndex &index)
{
#ifdef _DEBUG_
  std::cout << "Index id: " << index.internalId() << std::endl;
#endif
  ZFlyEmToDoItem *item = getItem(getMappedIndex(index));
  if (item != NULL) {
    if (getDocument() != NULL) {
      getDocument()->notifyZoomingTo(item->getPosition());
    }
  }
}

QVariant ZFlyEmTodoListModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
  return getPresenter()->headerData(section, orientation, role);
}

const ZFlyEmTodoPresenter *ZFlyEmTodoListModel::getPresenter() const
{
  if (m_presenter.get() == NULL) {
    return &m_defaultPresenter;
  }

  return m_presenter.get();
}

ZFlyEmTodoPresenter *ZFlyEmTodoListModel::getPresenter()
{
  if (m_presenter.get() == NULL) {
    return &m_defaultPresenter;
  }

  return m_presenter.get();
}

int ZFlyEmTodoListModel::rowCount(const QModelIndex &/*parent*/) const
{
  return m_itemList.size();
}

int ZFlyEmTodoListModel::columnCount(const QModelIndex &/*parent*/) const
{
  return getPresenter()->columnCount();
}

QVariant ZFlyEmTodoListModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  if (index.row() >= rowCount(index) || index.row() < 0) {
    return QVariant();
  }

  const ZFlyEmToDoItem *item = getItem(index.row());

  if (item != NULL) {
    return getPresenter()->data(*item, index.column(), role);
  }

  return QVariant();
}

QModelIndex ZFlyEmTodoListModel::index(
    int row, int column, const QModelIndex &/*parent*/) const
{
  return createIndex(row, column, row);
}

bool ZFlyEmTodoListModel::insertRows(
    int row, int count, const QModelIndex &parent)
{
  if (count > 0) {
    beginInsertRows(parent, row, row + count - 1);
    endInsertRows();

    return true;
  }

  return false;
}

bool ZFlyEmTodoListModel::insertColumns(
    int col, int count, const QModelIndex &parent)
{
  if (count > 0) {
    beginInsertColumns(parent, col, col + count - 1);
    endInsertColumns();

    return true;
  }

  return false;
}

bool ZFlyEmTodoListModel::removeRows(
    int row, int count, const QModelIndex &parent)
{
  if (count > 0) {
    beginRemoveRows(parent, row, row + count - 1);

    for (int i = row; i < count; ++i) {
      m_itemList.removeOne(getItem(i));
    }

    endRemoveRows();

    return true;
  }

  return false;
}

bool ZFlyEmTodoListModel::removeColumns(
    int col, int count, const QModelIndex &parent)
{
  if (count > 0){
    beginRemoveColumns(parent, col, col + count - 1);
    endRemoveColumns();

    return true;
  }

  return false;
}

const ZFlyEmToDoItem* ZFlyEmTodoListModel::getItem(const QModelIndex &index) const
{
  QModelIndex mappedIndex = index;

  return getItem(mappedIndex.row());
}


ZFlyEmToDoItem *ZFlyEmTodoListModel::getItem(const QModelIndex &index)
{
  return const_cast<ZFlyEmToDoItem*>(
        static_cast<const ZFlyEmTodoListModel&>(*this).getItem(index));
}

const ZFlyEmToDoItem* ZFlyEmTodoListModel::getItem(int index) const
{
  if (index < 0 || index >= (int) m_itemList.size()) {
    return NULL;
  }

  return m_itemList[index];
}

ZFlyEmToDoItem *ZFlyEmTodoListModel::getItem(int index)
{
  return const_cast<ZFlyEmToDoItem*>(
        static_cast<const ZFlyEmTodoListModel&>(*this).getItem(index));
}

void ZFlyEmTodoListModel::clear()
{
  removeRows(0, rowCount());
  m_itemList.clear();
}

void ZFlyEmTodoListModel::append(const ZFlyEmToDoItem *item)
{
  m_itemList.append(const_cast<ZFlyEmToDoItem*>(item));
  insertRow(rowCount() - 1);
}

void ZFlyEmTodoListModel::setDocument(ZSharedPointer<ZStackDoc> doc)
{
  clear();
  m_doc = doc;
}

ZFlyEmProofDoc* ZFlyEmTodoListModel::getDocument() const
{
  return qobject_cast<ZFlyEmProofDoc*>(m_doc.get());
}

void ZFlyEmTodoListModel::update()
{
  clear();

  if (getDocument() != NULL) {
    std::set<uint64_t> bodySet =
        getDocument()->getSelectedBodySet(neutu::ELabelSource::ORIGINAL);
    for (std::set<uint64_t>::const_iterator iter = bodySet.begin();
         iter != bodySet.end(); ++iter) {
      uint64_t bodyId = *iter;
      std::vector<ZFlyEmToDoItem*> itemList =
          getDocument()->getTodoItem(bodyId);
      append(itemList.begin(), itemList.end());
    }
  }
}

void ZFlyEmTodoListModel::setChecked(int row, bool checked)
{
  ZFlyEmToDoItem *item = getItem(index(row, 0));
  if (item) {
    item->setChecked(checked);
    updateRow(row);
    emit checkingTodoItem(item->getX(), item->getY(), item->getZ(), checked);
  }
}

void ZFlyEmTodoListModel::setChecked(
    const QModelIndexList &indexList, bool checked)
{
  for (const QModelIndex &index : indexList) {
    setChecked(index.row(), checked);
  }
}

void ZFlyEmTodoListModel::deleteSelected(
    QItemSelectionModel *sel, bool allowingUndo)
{
  getDocument()->removeTodoList(getSelectedTodoPosList(sel), allowingUndo);
}

void ZFlyEmTodoListModel::deleteSelected(
    QItemSelectionModel *sel, std::function<bool(int)> confirm,
    std::function<bool(int)> undoAllowed)
{
  auto todoList = getSelectedTodoPosList(sel);
  if (confirm(todoList.size())) {
    getDocument()->removeTodoList(todoList, undoAllowed(todoList.size()));
  }
}

QList<ZIntPoint> ZFlyEmTodoListModel::getSelectedTodoPosList(QItemSelectionModel *sel) const
{
  QList<ZIntPoint> itemList;
  foreach (const QModelIndex &index, getSelected(sel)) {
    itemList.append(getItem(index)->getPosition());
  }

  return itemList;
}
QList<ZFlyEmToDoItem*> ZFlyEmTodoListModel::getSelectedTodoList(
    QItemSelectionModel *sel) const
{
  QList<ZFlyEmToDoItem*> itemList;
  foreach (const QModelIndex &index, getSelected(sel)) {
    itemList.append(const_cast<ZFlyEmToDoItem*>(getItem(index)));
  }

  return itemList;
}

//QModelIndexList ZFlyEmTodoListModel::getSelected(QItemSelectionModel *sel) const
//{
////  return sel->selection().indexes();

//  return getProxy()->mapSelectionToSource(sel->selection()).indexes();
//}

/*
void ZFlyEmTodoListModel::update(int row)
{
  emit dataChanged(index(row, 0), index(row, columnCount() - 1));
}
*/

/*
void ZFlyEmTodoListModel::sortTodoList()
{
  getProxy()->sort(getProxy()->sortColumn(), getProxy()->sortOrder());
}
*/

void ZFlyEmTodoListModel::setSelectedChecked(
    QItemSelectionModel *sel, bool checked)
{
  setChecked(getSelected(sel), checked);
}

void ZFlyEmTodoListModel::setVisibleTest(
    std::function<bool(const ZFlyEmToDoItem&)> f)
{
  getPresenter()->setVisibleTest(f);
  getProxy()->invalidate();
}

void ZFlyEmTodoListModel::setCheckedVisibleOnly()
{
  setVisibleTest([](const ZFlyEmToDoItem &item) { return item.isChecked(); });

}

void ZFlyEmTodoListModel::setUncheckedVisibleOnly()
{
  setVisibleTest([](const ZFlyEmToDoItem &item) { return !item.isChecked(); });
}

void ZFlyEmTodoListModel::setAllVisible()
{
  setVisibleTest([](const ZFlyEmToDoItem&) { return true; });
}
