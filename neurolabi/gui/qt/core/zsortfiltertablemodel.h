#ifndef ZSORTFILTERTABLEMODEL_H
#define ZSORTFILTERTABLEMODEL_H

#include <QAbstractTableModel>

#include "zsortfilterproxymodel.h"

class QItemSelectionModel;

/*!
 * \brief The  class for building a sortable/filterable model
 *
 * It enhances QAbstractTableModel by including a ZSortFilterProxyModel object
 * as its proxy.
 */
class ZSortFilterTableModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit ZSortFilterTableModel(QObject *parent = nullptr);

  QSortFilterProxyModel* getProxy() const {
    return m_proxy;
  }

  QModelIndex index(int row, int column,
                    const QModelIndex &parent = QModelIndex()) const;

  QModelIndexList getSelected(QItemSelectionModel *sel) const;

  void sortTable();
  void updateRow(int row);

signals:

public slots:

protected:
  QModelIndex getMappedIndex(const QModelIndex &index);

private:
  ZSortFilterProxyModel* m_proxy = nullptr;
};

#endif // ZSORTFILTERTABLEMODEL_H
