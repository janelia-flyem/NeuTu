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
  explicit NeuPrintQueryDialog(QWidget *parent = nullptr);
  ~NeuPrintQueryDialog();

  void setRoiList(const QStringList roiList);

  QList<QString> getInputRoi() const;
  QList<QString> getOutputRoi() const;

private:
  Ui::NeuPrintQueryDialog *ui;
};

#endif // NEUPRINTQUERYDIALOG_H
