#include "zflyemtodolistview.h"

#include <QSortFilterProxyModel>
#include <QHeaderView>

ZFlyEmTodoListView::ZFlyEmTodoListView(QWidget *parent) : QTableView(parent)
{
  horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);
}

QSortFilterProxyModel* ZFlyEmTodoListView::getModel() const
{
  return qobject_cast<QSortFilterProxyModel*>(model());
}

void ZFlyEmTodoListView::setStringFilter(const QString &text)
{
  QSortFilterProxyModel *todoModel = getModel();
  if (todoModel) {
    todoModel->setFilterFixedString(text);
  }
}

void ZFlyEmTodoListView::sort()
{
  QSortFilterProxyModel *todoModel = getModel();
  if (todoModel) {
    todoModel->sort(todoModel->sortColumn(), todoModel->sortOrder());
  }
}
