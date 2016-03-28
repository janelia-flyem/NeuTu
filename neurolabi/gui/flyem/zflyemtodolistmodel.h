#ifndef ZFLYEMTODOLISTMODEL_H
#define ZFLYEMTODOLISTMODEL_H

#include <QAbstractTableModel>

#include "zsharedpointer.h"
#include "flyem/zflyemtodopresenter.h"

class ZStackDoc;
class ZFlyEmProofDoc;
class QSortFilterProxyModel;

class ZFlyEmTodoListModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit ZFlyEmTodoListModel(QObject *parent = 0);

  const ZFlyEmTodoPresenter *getPresenter() const;

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
  void update(int row);

  const ZFlyEmToDoItem* getItem(const QModelIndex &index) const;
  ZFlyEmToDoItem* getItem(const QModelIndex &index);

  const ZFlyEmToDoItem* getItem(int index) const;
  ZFlyEmToDoItem* getItem(int index);

  void removeItem(ZFlyEmToDoItem *item);

  ZFlyEmProofDoc* getDocument() const;

  void setDocument(ZSharedPointer<ZStackDoc> doc);

  void update();

  QSortFilterProxyModel* getSortProxy() const {
    return m_proxy;
  }

signals:
  void locatingItem(ZFlyEmToDoItem *item);
  void doubleClicked(QModelIndex index);

public slots:
  void processDoubleClick(const QModelIndex &index);

private:
  void init();
  void connectSignalSlot();

private:
  QList<ZFlyEmToDoItem*> m_itemList;
  ZSharedPointer<ZStackDoc> m_doc;
  ZFlyEmTodoPresenter m_defaultPresenter;
  ZSharedPointer<ZFlyEmTodoPresenter> m_presenter;
  QSortFilterProxyModel* m_proxy;
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
