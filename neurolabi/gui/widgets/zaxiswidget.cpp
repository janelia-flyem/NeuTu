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


neutube::EAxis ZAxisWidget::getAxis() const
{
  switch (ui->axisComboBox->currentIndex()) {
  case 0:
    return neutube::X_AXIS;
  case 1:
    return neutube::Y_AXIS;
  case 2:
    return neutube::Z_AXIS;
  }

  return neutube::X_AXIS;
}
