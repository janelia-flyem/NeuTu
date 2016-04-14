#ifndef FLYEMSETTINGDIALOG_H
#define FLYEMSETTINGDIALOG_H

#include <QDialog>

namespace Ui {
class FlyEmSettingDialog;
}

class FlyEmSettingDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlyEmSettingDialog(QWidget *parent = 0);
  ~FlyEmSettingDialog();

private:
  Ui::FlyEmSettingDialog *ui;
};

#endif // FLYEMSETTINGDIALOG_H
