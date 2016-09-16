#ifndef ZDVIDBODYPOSITIONDIALOG_H
#define ZDVIDBODYPOSITIONDIALOG_H

#include <QDialog>
#include <QList>

namespace Ui {
class ZDvidBodyPositionDialog;
}

class ZDvidDialog;

class ZDvidBodyPositionDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZDvidBodyPositionDialog(QWidget *parent = 0);
  ~ZDvidBodyPositionDialog();

  void setDvidDialog(ZDvidDialog *dlg);

private slots:
  void loadInputFile();
  void generatePosition();
  void setDvid();

private:
  Ui::ZDvidBodyPositionDialog *ui;

  ZDvidDialog *m_dvidDlg;
};

#endif // ZDVIDBODYPOSITIONDIALOG_H
