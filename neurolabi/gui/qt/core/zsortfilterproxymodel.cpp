#include "zsortfilterproxymodel.h"

ZSortFilterProxyModel::ZSortFilterProxyModel(QObject *parent) :
  QSortFilterProxyModel(parent)
{

}

bool ZSortFilterProxyModel::filterAcceptsRow(
    int source_row, const QModelIndex &source_parent) const
{
  QModelIndex index = sourceModel()->index(source_row, 1, source_parent);

  return (sourceModel()->data(index).isNull() == false) &&
      QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

//void ZSortFilterProxyModel::update()
//{
//  invalidateFilter();
//}
