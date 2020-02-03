#include "userfeedbackdialog.h"

#include <QPushButton>

#include "ui_userfeedbackdialog.h"

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
