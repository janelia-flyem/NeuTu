#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>

namespace Ui {
class HelpDialog;
}

class HelpDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit HelpDialog(QWidget *parent = 0);
  ~HelpDialog();

  void setSource(const QString &source);
  
private:
  Ui::HelpDialog *ui;
};

#endif // HELPDIALOG_H
