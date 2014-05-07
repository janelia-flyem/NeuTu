#include "zdviddialog.h"
#include "ui_zdviddialog.h"
#include "dvid/zdvidtarget.h"

ZDvidDialog::ZDvidDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZDvidDialog)
{
  ui->setupUi(this);
  ZDvidTarget target;
  target.set(getAddress().toStdString(), getUuid().toStdString(), getPort());
  m_customString = target.getSourceString();

  connect(ui->serverComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(setServer(int)));
}

ZDvidDialog::~ZDvidDialog()
{
  delete ui;
}

int ZDvidDialog::getPort() const
{
  return ui->portSpinBox->value();
}

QString ZDvidDialog::getAddress() const
{
  return ui->addressLineEdit->text();
}

QString ZDvidDialog::getUuid() const
{
  return ui->uuidLineEdit->text();
}

void ZDvidDialog::setServer(int index)
{
  ZDvidTarget dvidTarget;

  if (index == 0) {
    dvidTarget.set(m_customString);
  } else {
    dvidTarget.set("http:" + ui->serverComboBox->currentText().toStdString());
  }

  ui->addressLineEdit->setText(dvidTarget.getAddress().c_str());
  ui->portSpinBox->setValue(dvidTarget.getPort());
  ui->uuidLineEdit->setText(dvidTarget.getUuid().c_str());
}
