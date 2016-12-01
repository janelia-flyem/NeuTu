#ifndef ZTESTOPTIONDIALOG_H
#define ZTESTOPTIONDIALOG_H

#include <QDialog>

namespace Ui {
class ZTestOptionDialog;
}

class ZTestOptionDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZTestOptionDialog(QWidget *parent = 0);
  ~ZTestOptionDialog();

  enum EOption {
    OPTION_NORMAL, OPTION_STRESS
  };

  EOption getOption() const;

private:
  Ui::ZTestOptionDialog *ui;
};

#endif // ZTESTOPTIONDIALOG_H
