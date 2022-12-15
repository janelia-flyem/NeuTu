#ifndef ZFLYEMTODOANNOTATIONDIALOG_H
#define ZFLYEMTODOANNOTATIONDIALOG_H

#include <QDialog>

#include "flyem/zflyemtodoitem.h"

namespace Ui {
class ZFlyEmTodoAnnotationDialog;
}

class ZFlyEmTodoAnnotationDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZFlyEmTodoAnnotationDialog(QWidget *parent = 0);
  ~ZFlyEmTodoAnnotationDialog();

  void init(const ZFlyEmToDoItem &item);
  void annotate(ZFlyEmToDoItem *item);

private:
  void initActionBox();
  void initCheckedCombo();
  int getPriority() const;
  QString getComment() const;

private slots:
  void updateWidget();

private:
  Ui::ZFlyEmTodoAnnotationDialog *ui;
  std::string m_bufferCheckedString = ZFlyEmToDoItem::TODO_STATE_UNCHECKED;
};

#endif // ZFLYEMTODOANNOTATIONDIALOG_H
