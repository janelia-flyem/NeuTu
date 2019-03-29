#ifndef ZFLYEMTODOLISTVIEW_H
#define ZFLYEMTODOLISTVIEW_H

#include <QTableView>

class QSortFilterProxyModel;

class ZFlyEmTodoListView : public QTableView
{
  Q_OBJECT

public:
  ZFlyEmTodoListView(QWidget *parent = Q_NULLPTR);

  void setStringFilter(const QString &text);
  void sort();

  QSortFilterProxyModel* getModel() const;
};

#endif // ZFLYEMTODOLISTVIEW_H
