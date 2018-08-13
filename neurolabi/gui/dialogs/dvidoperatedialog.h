#ifndef DVIDOPERATEDIALOG_H
#define DVIDOPERATEDIALOG_H

#include <QDialog>
#include "zdvidtargetproviderdialog.h"

namespace Ui {
class DvidOperateDialog;
}

class ZContrastProtocalDialog;

class DvidOperateDialog : public QDialog
{
  Q_OBJECT

public:
  explicit DvidOperateDialog(QWidget *parent = 0);
  ~DvidOperateDialog();

  void setDvidDialog(ZDvidTargetProviderDialog *dlg);

  const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

private slots:
  void on_dvidPushButton_clicked();

  void on_creatDataPushButton_clicked();

  void on_contrastPushButton_clicked();

  void on_addMasterPushButton_clicked();

private:
  void updateDvidTarget();

private:
  Ui::DvidOperateDialog *ui;

  ZDvidTarget m_dvidTarget;

  ZDvidTargetProviderDialog *m_dvidDlg;
  ZContrastProtocalDialog *m_contrastDlg;
};

#endif // DVIDOPERATEDIALOG_H
