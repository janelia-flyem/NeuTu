#ifndef ZDVIDADVANCEDDIALOG_H
#define ZDVIDADVANCEDDIALOG_H

#include <QDialog>

namespace Ui {
class ZDvidAdvancedDialog;
}

class ZDvidAdvancedDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZDvidAdvancedDialog(QWidget *parent = 0);
  ~ZDvidAdvancedDialog();

  void setDvidServer(const QString &str);

private:
  Ui::ZDvidAdvancedDialog *ui;
};

#endif // ZDVIDADVANCEDDIALOG_H
