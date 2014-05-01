#ifndef ZDVIDDIALOG_H
#define ZDVIDDIALOG_H

#include <QDialog>

namespace Ui {
class ZDvidDialog;
}

class ZDvidDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZDvidDialog(QWidget *parent = 0);
  ~ZDvidDialog();

  int getPort() const;
  QString getAddress() const;
  QString getUuid() const;

private:
  Ui::ZDvidDialog *ui;
};

#endif // ZDVIDDIALOG_H
