#ifndef ZFLYEMTODOFILTERDIALOG_H
#define ZFLYEMTODOFILTERDIALOG_H

#include <QDialog>

namespace Ui {
class ZFlyEmTodoFilterDialog;
}

class ZFlyEmToDoItem;
class ZStackObject;

class ZFlyEmTodoFilterDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZFlyEmTodoFilterDialog(QWidget *parent = 0);
  ~ZFlyEmTodoFilterDialog();

  bool passed(const ZFlyEmToDoItem &item) const;
  bool passed(const ZFlyEmToDoItem *item) const;


private:
  Ui::ZFlyEmTodoFilterDialog *ui;
};

#endif // ZFLYEMTODOFILTERDIALOG_H
