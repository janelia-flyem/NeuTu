#include "userfeedbackdialog.h"

#include <QPushButton>

#include "ui_userfeedbackdialog.h"

#include "logging/zlog.h"

UserFeedbackDialog::UserFeedbackDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::UserFeedbackDialog)
{
  ui->setupUi(this);
  ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Send");
}

UserFeedbackDialog::~UserFeedbackDialog()
{
  delete ui;
}

QString UserFeedbackDialog::getFeedback() const
{
  return ui->feedbackTextEdit->toPlainText();
}

QString UserFeedbackDialog::getAction() const
{
  return "complain";
}

bool UserFeedbackDialog::isAnonymous() const
{
  return ui->anonymousCheckBox->isChecked();
}

bool UserFeedbackDialog::send(
    std::function<void(const QString&)> msgHandler) const
{
  QString fb = getFeedback();
  if (!fb.isEmpty()) {
    KLog() << ZLog::Feedback() << ZLog::Action(getAction().toStdString())
           << ZLog::Description(fb.toStdString())
           << (isAnonymous() ? ZLog::AnonymousUser() : ZLog::Tag());

    msgHandler(isAnonymous() ? "Thank you for your anonymous feedback!" :
                               "Thank you for your feedback!");

    return true;
  }

  return false;
}
