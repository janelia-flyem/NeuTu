#include "zsynapsepropertydialog.h"
#include "ui_zsynapsepropertydialog.h"

#include "dvid/zdvidannotation.h"

ZSynapsePropertyDialog::ZSynapsePropertyDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZSynapsePropertyDialog)
{
  ui->setupUi(this);
  ui->preSynRadiusDoubleSpinBox->setValue(
        ZDvidAnnotation::DEFAULT_PRE_SYN_RADIUS);
  ui->postSynRadiusDoubleSpinBox->setValue(
        ZDvidAnnotation::DEFAULT_POST_SYN_RADIUS);

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
