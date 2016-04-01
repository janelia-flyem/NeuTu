#ifndef FLYEMTODODIALOG_H
#define FLYEMTODODIALOG_H

#include <QDialog>

#include "zsharedpointer.h"

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

private:
  void init();

private:
  Ui::FlyEmTodoDialog *ui;
  ZFlyEmTodoListModel *m_model;
};

#endif // FLYEMTODODIALOG_H
