#include "zsynapsepropertydialog.h"
#include "ui_zsynapsepropertydialog.h"

ZSynapsePropertyDialog::ZSynapsePropertyDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZSynapsePropertyDialog)
{
  ui->setupUi(this);

  connect(ui->preSynRadiusDoubleSpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(notifyRadiusChanged()));
  connect(ui->postSynRadiusDoubleSpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(notifyRadiusChanged()));
}

ZSynapsePropertyDialog::~ZSynapsePropertyDialog()
{
  delete ui;
}

void ZSynapsePropertyDialog::notifyRadiusChanged()
{
  emit synapseRadiusChanged(ui->preSynRadiusDoubleSpinBox->value(),
                            ui->postSynRadiusDoubleSpinBox->value());
}

void ZSynapsePropertyDialog::setRadius(double pre, double post)
{
  ui->preSynRadiusDoubleSpinBox->setValue(pre);
  ui->postSynRadiusDoubleSpinBox->setValue(post);
}
