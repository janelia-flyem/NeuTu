#include "zdvidadvanceddialog.h"
#include "ui_zdvidadvanceddialog.h"

ZDvidAdvancedDialog::ZDvidAdvancedDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZDvidAdvancedDialog)
{
  ui->setupUi(this);
  m_oldSupervised = true;

  if (ui->grayscaleMainCheckBox->isChecked()) {
    ui->grayscaleSourceWidget->setEnabled(false);
  }
  if (ui->tileMainCheckBox->isChecked()) {
    ui->tileSourceWidget->setEnabled(false);
  }

  connect(ui->grayscaleMainCheckBox, SIGNAL(toggled(bool)),
          ui->grayscaleSourceWidget, SLOT(setDisabled(bool)));
  connect(ui->tileMainCheckBox, SIGNAL(toggled(bool)),
          ui->tileSourceWidget, SLOT(setDisabled(bool)));
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
  m_oldGrayscaleSource = ui->grayscaleSourceWidget->getNode();
  m_oldTileSource = ui->tileSourceWidget->getNode();
}

void ZDvidAdvancedDialog::recover()
{
  setSupervised(m_oldSupervised);
  setSupervisorServer(m_oldSupervisorServer);
  setTodoName(m_oldTodoName);
  setGrayscaleSource(m_oldGrayscaleSource);
  setTileSource(m_oldTileSource);
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

void ZDvidAdvancedDialog::setGrayscaleSource(
    const ZDvidNode &node)
{
  ui->grayscaleSourceWidget->setNode(node);
}

void ZDvidAdvancedDialog::setTileSource(
    const ZDvidNode &node)
{
  ui->tileSourceWidget->setNode(node);
}


void ZDvidAdvancedDialog::setGrayscaleSource(
    const ZDvidNode &node, bool sameMainSource)
{
  ui->grayscaleSourceWidget->setNode(node);
  ui->grayscaleMainCheckBox->setChecked(sameMainSource);
}

void ZDvidAdvancedDialog::setTileSource(
    const ZDvidNode &node, bool sameMainSource)
{
  ui->tileSourceWidget->setNode(node);
  ui->tileMainCheckBox->setChecked(sameMainSource);
}

ZDvidNode ZDvidAdvancedDialog::getGrayscaleSource() const
{
  if (ui->grayscaleMainCheckBox->isChecked()) {
    return ZDvidNode();
  }

  return ui->grayscaleSourceWidget->getNode();
}

ZDvidNode ZDvidAdvancedDialog::getTileSource() const
{
  if (ui->tileMainCheckBox->isChecked()) {
    return ZDvidNode();
  }

  return ui->tileSourceWidget->getNode();
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
