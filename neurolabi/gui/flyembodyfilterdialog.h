#ifndef FLYEMBODYFILTERDIALOG_H
#define FLYEMBODYFILTERDIALOG_H

#include <QDialog>

namespace Ui {
class FlyEmBodyFilterDialog;
}

class FlyEmBodyFilterDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlyEmBodyFilterDialog(QWidget *parent = 0);
  ~FlyEmBodyFilterDialog();

private:
  Ui::FlyEmBodyFilterDialog *ui;
};

#endif // FLYEMBODYFILTERDIALOG_H
