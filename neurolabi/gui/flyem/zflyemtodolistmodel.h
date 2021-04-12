#ifndef ZFLYEMTODOLISTMODEL_H
#define ZFLYEMTODOLISTMODEL_H

//#include <QAbstractTableModel>

#include <functional>

#include "common/zsharedpointer.h"
#include "qt/core/zsortfilterproxymodel.h"
#include "qt/core/zsortfiltertablemodel.h"
#include "flyem/zflyemtodopresenter.h"

class ZStackDoc;
class ZFlyEmProofDoc;
class QItemSelectionModel;
class ZIntPoint;

class ZFlyEmTodoListModel : public ZSortFilterTableModel
{
  Q_OBJECT
public:
  explicit ZFlyEmTodoListModel(QObject *parent = 0);

  const ZFlyEmTodoPresenter *getPresenter() const;
  ZFlyEmTodoPresenter *getPresenter();

  int rowCount( const QModelIndex & parent = QModelIndex() ) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data( const QModelIndex & index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
  bool insertColumns(int col, int count, const QModelIndex &parent = QModelIndex());
  bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
  bool removeColumns(int col, int count, const QModelIndex &parent = QModelIndex());

  QModelIndex index(int row, int column,
                    const QModelIndex &parent = QModelIndex()) const;

  void clear();
  void append(const ZFlyEmToDoItem *item);
  template <typename InputIterator>
  void append(const InputIterator &first, const InputIterator &last);
//  void update(int row);

  const ZFlyEmToDoItem* getItem(const QModelIndex &index) const;
  ZFlyEmToDoItem* getItem(const QModelIndex &index);

  const ZFlyEmToDoItem* getItem(int index) const;
  ZFlyEmToDoItem* getItem(int index);

  void removeItem(ZFlyEmToDoItem *item);

  ZFlyEmProofDoc* getDocument() const;

  void setDocument(ZSharedPointer<ZStackDoc> doc);

  void update();

  /*
  QSortFilterProxyModel* getProxy() const {
    return m_proxy;
  }
  */

//  void sortTodoList();

  void setVisibleTest(std::function<bool(const ZFlyEmToDoItem&)> f);

  void setCheckedVisibleOnly();
  void setUncheckedVisibleOnly();
  void setAllVisible();

  void setSelectedChecked(QItemSelectionModel *sel, bool checked);

  void deleteSelected(QItemSelectionModel *sel, bool allowingUndo);
  void deleteSelected(
      QItemSelectionModel *sel, std::function<bool(int)> confirm,
      std::function<bool(int)> undoAllowed);
  QList<ZFlyEmToDoItem*> getSelectedTodoList(QItemSelectionModel *sel) const;
  QList<ZIntPoint> getSelectedTodoPosList(QItemSelectionModel *sel) const;

signals:
  void locatingItem(ZFlyEmToDoItem *item);
  void doubleClicked(QModelIndex index);
  void checkingTodoItem(int x, int y, int z, bool checked);

public slots:
  void processDoubleClick(const QModelIndex &index);

private:
  void init();
  void connectSignalSlot();
//  void setChecked(const QModelIndex &index, bool checked);
  void setChecked(int row, bool checked);
  void setChecked(const QModelIndexList &indexList, bool checked);
//  QModelIndexList getSelected(QItemSelectionModel *sel) const;
//  QModelIndex getMappedIndex(const QModelIndex &index);

private:
  QList<ZFlyEmToDoItem*> m_itemList;
  ZSharedPointer<ZStackDoc> m_doc;
  ZFlyEmTodoPresenter m_defaultPresenter;
  ZSharedPointer<ZFlyEmTodoPresenter> m_presenter;
//  ZSortFilterProxyModel* m_proxy = nullptr;
};

template <typename InputIterator>
void ZFlyEmTodoListModel::append(
    const InputIterator &first, const InputIterator &last)
{
  int oldRowCount = rowCount();
  int count = 0;
  for (InputIterator iter = first; iter != last; ++iter) {
    m_itemList.append(*iter);
    ++count;
  }
  insertRows(oldRowCount, count);
}

#endif // ZFLYEMTODOLISTMODEL_H
