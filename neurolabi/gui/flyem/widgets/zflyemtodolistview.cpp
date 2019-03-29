#include "zflyemtodolistview.h"

#include <QSortFilterProxyModel>

ZFlyEmTodoListView::ZFlyEmTodoListView(QWidget *parent) : QTableView(parent)
{
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
