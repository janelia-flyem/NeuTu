#include "flyemsynapsefilterdialog.h"
#include "ui_flyemsynapsefilterdialog.h"

FlyEmSynapseFilterDialog::FlyEmSynapseFilterDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmSynapseFilterDialog)
{
  ui->setupUi(this);
}

FlyEmSynapseFilterDialog::~FlyEmSynapseFilterDialog()
{
  delete ui;
}
