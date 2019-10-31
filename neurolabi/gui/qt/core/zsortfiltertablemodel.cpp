#include "zsortfiltertablemodel.h"

#include <QItemSelectionModel>

ZSortFilterTableModel::ZSortFilterTableModel(QObject *parent) :
  QAbstractTableModel(parent)
{
  m_proxy = new ZSortFilterProxyModel;
  m_proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
  m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
  m_proxy->setFilterKeyColumn(-1);
  m_proxy->setSourceModel(this);
}

QModelIndex ZSortFilterTableModel::getMappedIndex(const QModelIndex &index)
{
  if (m_proxy != NULL) {
    return m_proxy->mapToSource(index);
  }

  return index;
}

QModelIndex ZSortFilterTableModel::index(
    int row, int column, const QModelIndex &/*parent*/) const
{
  return createIndex(row, column, row);
}


QModelIndexList ZSortFilterTableModel::getSelected(QItemSelectionModel *sel) const
{
//  return sel->selection().indexes();

  return getProxy()->mapSelectionToSource(sel->selection()).indexes();
}

void ZSortFilterTableModel::sortTable()
{
  getProxy()->sort(getProxy()->sortColumn(), getProxy()->sortOrder());
}

void ZSortFilterTableModel::updateRow(int row)
{
  emit dataChanged(index(row, 0), index(row, columnCount() - 1));
}
