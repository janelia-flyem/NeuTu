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

  void setUuid(const QString &uuid);
  QString getAuthToken() const;
  QString getDataset() const {
    return m_dataset;
  }

private slots:
  bool apply();
  void processButtonClick(QAbstractButton*);

private:
  Ui::NeuprintSetupDialog *ui;
  QString m_uuid;
  QString m_dataset;
};

#endif // NEUPRINTSETUPDIALOG_H
