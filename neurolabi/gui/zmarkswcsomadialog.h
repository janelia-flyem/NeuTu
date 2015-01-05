#ifndef ZMARKSWCSOMADIALOG_H
#define ZMARKSWCSOMADIALOG_H

#include <QDialog>

namespace Ui {
class ZMarkSwcSomaDialog;
}

class ZMarkSwcSomaDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZMarkSwcSomaDialog(QWidget *parent = 0);
  ~ZMarkSwcSomaDialog();

  double getRadiusThre() const;
  int getSomaType() const;
  int getOtherType() const;

private:
  Ui::ZMarkSwcSomaDialog *ui;
};

#endif // ZMARKSWCSOMADIALOG_H
