#ifndef DVIDOPERATEDIALOG_H
#define DVIDOPERATEDIALOG_H

#include <QDialog>
#include "zdviddialog.h"

namespace Ui {
class DvidOperateDialog;
}

class DvidOperateDialog : public QDialog
{
  Q_OBJECT

public:
  explicit DvidOperateDialog(QWidget *parent = 0);
  ~DvidOperateDialog();

  void setDvidDialog(ZDvidDialog *dlg);

private slots:
  void on_dvidPushButton_clicked();

  void on_creatDataPushButton_clicked();

private:
  Ui::DvidOperateDialog *ui;

  ZDvidDialog *m_dvidDlg;
};

#endif // DVIDOPERATEDIALOG_H
