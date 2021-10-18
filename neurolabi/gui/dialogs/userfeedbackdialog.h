#ifndef USERFEEDBACKDIALOG_H
#define USERFEEDBACKDIALOG_H

#include <functional>

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

  bool send(std::function<void(const QString&)> msgHandler) const;

private:
  bool isAnonymous() const;

private:
  Ui::UserFeedbackDialog *ui;
};

#endif // USERFEEDBACKDIALOG_H
