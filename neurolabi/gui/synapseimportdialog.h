#ifndef SYNAPSEIMPORTDIALOG_H
#define SYNAPSEIMPORTDIALOG_H

#include <QDialog>

namespace Ui {
class SynapseImportDialog;
}

class SynapseImportDialog : public QDialog
{
  Q_OBJECT

public:
  explicit SynapseImportDialog(QWidget *parent = 0);
  ~SynapseImportDialog();

  int getSynapseSelection() const;

private:
  Ui::SynapseImportDialog *ui;
};

#endif // SYNAPSEIMPORTDIALOG_H
