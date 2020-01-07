#ifndef NEUPRINTDATASETDIALOG_H
#define NEUPRINTDATASETDIALOG_H

#include <QDialog>

namespace Ui {
class NeuprintDatasetDialog;
}

class NeuprintDatasetDialog : public QDialog
{
  Q_OBJECT

public:
  explicit NeuprintDatasetDialog(QWidget *parent = nullptr);
  ~NeuprintDatasetDialog();

  void setDatasetList(const QStringList &datasets);
  QString getDataset() const;

private:
  Ui::NeuprintDatasetDialog *ui;
};

#endif // NEUPRINTDATASETDIALOG_H
