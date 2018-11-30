#ifndef NEUPRINTQUERYDIALOG_H
#define NEUPRINTQUERYDIALOG_H

#include <QDialog>

namespace Ui {
class NeuPrintQueryDialog;
}

class NeuPrintQueryDialog : public QDialog
{
  Q_OBJECT

public:
  explicit NeuPrintQueryDialog(QWidget *parent = 0);
  ~NeuPrintQueryDialog();

  QList<QString> getInputRoi() const;
  QList<QString> getOutputRoi() const;

private:
  Ui::NeuPrintQueryDialog *ui;
};

#endif // NEUPRINTQUERYDIALOG_H
