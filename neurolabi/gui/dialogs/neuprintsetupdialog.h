#ifndef NEUPRINTSETUPDIALOG_H
#define NEUPRINTSETUPDIALOG_H

#include <memory>
#include <QDialog>

#include "zjsonobject.h"

class QAbstractButton;
class NeuPrintReader;

namespace Ui {
class NeuprintSetupDialog;
}

class NeuprintSetupDialog : public QDialog
{
  Q_OBJECT

public:
  explicit NeuprintSetupDialog(QWidget *parent = nullptr);
  ~NeuprintSetupDialog();

  void setUuid(const QString &uuid);
  QString getAuthToken() const;
  QString getDataset() const {
    return m_dataset;
  }

  std::unique_ptr<NeuPrintReader> takeNeuPrintReader();

private slots:
  bool apply();
  void processButtonClick(QAbstractButton*);
  QString getDefaultAuthToken() const;

private:
  Ui::NeuprintSetupDialog *ui;
  QString m_uuid;
  QString m_dataset;
  ZJsonObject m_auth;
  std::unique_ptr<NeuPrintReader> m_reader;
};

#endif // NEUPRINTSETUPDIALOG_H
