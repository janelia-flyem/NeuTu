#ifndef USERFEEDBACKDIALOG_H
#define USERFEEDBACKDIALOG_H

#include <QDialog>

namespace Ui {
class UserFeedbackDialog;
}

class UserFeedbackDialog : public QDialog
{
  Q_OBJECT

public:
  explicit UserFeedbackDialog(QWidget *parent = nullptr);
  ~UserFeedbackDialog();

  QString getFeedback() const;
  //Purpose
  QString getAction() const;

private:
  Ui::UserFeedbackDialog *ui;
};

#endif // USERFEEDBACKDIALOG_H
