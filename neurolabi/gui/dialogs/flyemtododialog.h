#ifndef FLYEMTODODIALOG_H
#define FLYEMTODODIALOG_H

#include <QDialog>

#include "common/zsharedpointer.h"

class ZFlyEmTodoListModel;
class ZStackDoc;

namespace Ui {
class FlyEmTodoDialog;
}

class FlyEmTodoDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlyEmTodoDialog(QWidget *parent = 0);
  ~FlyEmTodoDialog();

  void setDocument(ZSharedPointer<ZStackDoc> doc);

public slots:
  void updateTable();

signals:
  void checkingTodoItem(int x, int y, int z, bool checked);

private:
  void init();
  void setChecked(bool checked);

private slots:
  void updateTextFilter(QString text);
  void updateVisibility(bool on);
  void checkSelected();
  void uncheckSelected();

private:
  Ui::FlyEmTodoDialog *ui;
  ZFlyEmTodoListModel *m_model;
};

#endif // FLYEMTODODIALOG_H
