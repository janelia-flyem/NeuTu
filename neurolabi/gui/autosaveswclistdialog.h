#ifndef AUTOSAVESWCLISTDIALOG_H
#define AUTOSAVESWCLISTDIALOG_H

#include <QDialog>
#include "zswcfilelistmodel.h"

namespace Ui {
class AutosaveSwcListDialog;
}

class AutosaveSwcListDialog : public QDialog
{
  Q_OBJECT

public:
  explicit AutosaveSwcListDialog(QWidget *parent = 0);
  ~AutosaveSwcListDialog();

  void updateFile();

public slots:
  void deleteSelected();
  void viewSwc(const QModelIndex &index);

private slots:
  void on_pushButton_clicked();

private:
  QModelIndexList getSelected() const;

private:
  Ui::AutosaveSwcListDialog *ui;
  ZSwcFileListModel m_fileList;
};

#endif // AUTOSAVESWCLISTDIALOG_H
