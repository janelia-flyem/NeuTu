#include "zsortfilterproxymodel.h"

ZSortFilterProxyModel::ZSortFilterProxyModel(QObject *parent) :
  QSortFilterProxyModel(parent)
{

}

bool ZSortFilterProxyModel::filterAcceptsRow(
    int source_row, const QModelIndex &source_parent) const
{
  QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

  return (sourceModel()->data(index).isNull() == false) &&
      QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

//To avoid shuffling while sorting the same values
bool ZSortFilterProxyModel::lessThan(
    const QModelIndex &source_left, const QModelIndex &source_right) const
{
  bool defaultCompare = QSortFilterProxyModel::lessThan(source_left, source_right);
  if (!defaultCompare &&
      !QSortFilterProxyModel::lessThan(source_right, source_left)) {
    return source_left.internalId() < source_right.internalId();
  }

  return defaultCompare;
}

//void ZSortFilterProxyModel::update()
//{
//  invalidateFilter();
//}
