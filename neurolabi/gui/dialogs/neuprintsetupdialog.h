#ifndef NEUPRINTSETUPDIALOG_H
#define NEUPRINTSETUPDIALOG_H

#include <QDialog>

namespace Ui {
class NeuprintSetupDialog;
}

class NeuprintSetupDialog : public QDialog
{
  Q_OBJECT

public:
  explicit NeuprintSetupDialog(QWidget *parent = 0);
  ~NeuprintSetupDialog();

private:
  Ui::NeuprintSetupDialog *ui;
};

#endif // NEUPRINTSETUPDIALOG_H
