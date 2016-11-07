#include "zaxiswidget.h"
#include "ui_zaxiswidget.h"

ZAxisWidget::ZAxisWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ZAxisWidget)
{
  ui->setupUi(this);
}

ZAxisWidget::~ZAxisWidget()
{
  delete ui;
}


NeuTube::EAxis ZAxisWidget::getAxis() const
{
  switch (ui->axisComboBox->currentIndex()) {
  case 0:
    return NeuTube::X_AXIS;
  case 1:
    return NeuTube::Y_AXIS;
  case 2:
    return NeuTube::Z_AXIS;
  }

  return NeuTube::X_AXIS;
}
