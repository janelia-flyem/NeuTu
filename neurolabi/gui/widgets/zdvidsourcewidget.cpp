#include "zdvidsourcewidget.h"
#include "ui_zdvidsourcewidget.h"
#include "dvid/zdvidnode.h"

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
  return ui->addressLineEdit->text().trimmed();
}

QString ZDvidSourceWidget::getUuid() const
{
  return ui->uuidLineEdit->text().trimmed();
}

ZDvidNode ZDvidSourceWidget::getNode() const
{
  ZDvidNode node;
  node.set(getAddress().toStdString(), getUuid().toStdString(), getPort());

  return node;
}

void ZDvidSourceWidget::setNode(const ZDvidNode &node)
{
  setAddress(node.getHost());
  setPort(node.getPort());
  setUuid(node.getUuid());
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
