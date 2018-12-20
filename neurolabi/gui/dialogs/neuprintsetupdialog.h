#ifndef NEUPRINTSETUPDIALOG_H
#define NEUPRINTSETUPDIALOG_H

#include <QDialog>

class QAbstractButton;

namespace Ui {
class NeuprintSetupDialog;
}

class NeuprintSetupDialog : public QDialog
{
  Q_OBJECT

public:
  explicit NeuprintSetupDialog(QWidget *parent = 0);
  ~NeuprintSetupDialog();

  QString getAuthToken() const;

private slots:
  bool apply();
  void processButtonClick(QAbstractButton*);

private:
  Ui::NeuprintSetupDialog *ui;
};

#endif // NEUPRINTSETUPDIALOG_H
