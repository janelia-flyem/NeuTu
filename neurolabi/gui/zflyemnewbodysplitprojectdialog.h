#ifndef ZFLYEMNEWBODYSPLITPROJECTDIALOG_H
#define ZFLYEMNEWBODYSPLITPROJECTDIALOG_H

#include <QDialog>
#include "dvid/zdvidtarget.h"

class ZDvidDialog;

namespace Ui {
class ZFlyEmNewBodySplitProjectDialog;
}

class ZFlyEmNewBodySplitProjectDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZFlyEmNewBodySplitProjectDialog(QWidget *parent = 0);
  ~ZFlyEmNewBodySplitProjectDialog();

  void setDvidDialog(ZDvidDialog *dlg);

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }
  void setDvidTarget(const ZDvidTarget &target);
  int getBodyId() const;

private slots:
  void showDvidDialog();

private:
  Ui::ZFlyEmNewBodySplitProjectDialog *ui;
  ZDvidDialog *m_dvidDlg;
  ZDvidTarget m_dvidTarget;
};

#endif // ZFLYEMNEWBODYSPLITPROJECTDIALOG_H
