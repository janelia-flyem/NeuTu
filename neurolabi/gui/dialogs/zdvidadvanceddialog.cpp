#include "zdvidadvanceddialog.h"
#include "ui_zdvidadvanceddialog.h"

ZDvidAdvancedDialog::ZDvidAdvancedDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZDvidAdvancedDialog)
{
  ui->setupUi(this);
  m_oldSupervised = true;
}

ZDvidAdvancedDialog::~ZDvidAdvancedDialog()
{
  delete ui;
}

void ZDvidAdvancedDialog::backup()
{
  m_oldSupervised = isSupervised();
  m_oldSupervisorServer = getSupervisorServer();
  m_oldTodoName = getTodoName();
}

void ZDvidAdvancedDialog::recover()
{
  setSupervised(m_oldSupervised);
  setSupervisorServer(m_oldSupervisorServer);
  setTodoName(m_oldTodoName);
}

void ZDvidAdvancedDialog::setDvidServer(const QString &str)
{
  ui->serverLabel->setText(str);
}

void ZDvidAdvancedDialog::updateWidgetForEdit(bool editable)
{
  ui->librarianCheckBox->setEnabled(editable);
  ui->librarianLineEdit->setEnabled(editable);
  ui->todoLineEdit->setEnabled(editable);
}

void ZDvidAdvancedDialog::updateWidgetForDefaultSetting(bool usingDefault)
{
  ui->todoLineEdit->setVisible(!usingDefault);
  if (usingDefault) {
    ui->todoLabel->setText("Todo Name <default>");
  } else {
    ui->todoLabel->setText("Todo Name");
  }
}

void ZDvidAdvancedDialog::setTodoName(const std::string &name)
{
  ui->todoLineEdit->setText(name.c_str());
}

std::string ZDvidAdvancedDialog::getTodoName() const
{
  return ui->todoLineEdit->text().toStdString();
}

bool ZDvidAdvancedDialog::isSupervised() const
{
  return ui->librarianCheckBox->isChecked();
}

void ZDvidAdvancedDialog::setSupervised(bool supervised)
{
  ui->librarianCheckBox->setChecked(supervised);
}

std::string ZDvidAdvancedDialog::getSupervisorServer() const
{
  return ui->librarianLineEdit->text().toStdString();
}

void ZDvidAdvancedDialog::setSupervisorServer(const std::string &server)
{
  ui->librarianLineEdit->setText(server.c_str());
}
