#include "zdvidsourcewidget.h"
#include "ui_zdvidsourcewidget.h"

ZDvidSourceWidget::ZDvidSourceWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ZDvidSourceWidget)
{
  ui->setupUi(this);
}

ZDvidSourceWidget::~ZDvidSourceWidget()
{
  delete ui;
}

int ZDvidSourceWidget::getPort() const
{
  return ui->portSpinBox->value();
}

QString ZDvidSourceWidget::getAddress() const
{
  return ui->addressLineEdit->text();
}

QString ZDvidSourceWidget::getUuid() const
{
  return ui->uuidLineEdit->text();
}

void ZDvidSourceWidget::setPort(int port)
{
  ui->portSpinBox->setValue(port);
}

void ZDvidSourceWidget::setAddress(const std::string &address)
{
  ui->addressLineEdit->setText(address.c_str());
}

void ZDvidSourceWidget::setUuid(const std::string &uuid)
{
  ui->uuidLineEdit->setText(uuid.c_str());
}

void ZDvidSourceWidget::setReadOnly(bool readOnly)
{
  ui->addressLineEdit->setReadOnly(readOnly);
  ui->portSpinBox->setReadOnly(readOnly);
  ui->uuidLineEdit->setReadOnly(readOnly);
}
