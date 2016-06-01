#include "zflyemsynapseannotationdialog.h"
#include "ui_zflyemsynapseannotationdialog.h"
#include "tz_math.h"

ZFlyEmSynapseAnnotationDialog::ZFlyEmSynapseAnnotationDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmSynapseAnnotationDialog)
{
  ui->setupUi(this);
}

ZFlyEmSynapseAnnotationDialog::~ZFlyEmSynapseAnnotationDialog()
{
  delete ui;
}

void ZFlyEmSynapseAnnotationDialog::setConfidence(double c)
{
  if (c > 1.0) {
    c = 1.0;
  }

  if (c < 0.0) {
    c = 0.0;
  }

  ui->confComboBox->setCurrentIndex(iround((1.0 - c) / 0.5));
}

double ZFlyEmSynapseAnnotationDialog::getConfidence() const
{
  double conf = 1.0;
  switch (ui->confComboBox->currentIndex()) {
  case 0:
    conf = 1.0;
    break;
  case 1:
    conf = 0.5;
    break;
  case 2:
    conf = 0.1;
    break;
  default:
    break;
  }

  return conf;
}
