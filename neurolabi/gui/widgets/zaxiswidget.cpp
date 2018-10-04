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
    return neutube::EAxis::X;
  case 1:
    return neutube::EAxis::Y;
  case 2:
    return neutube::EAxis::Z;
  }

  return neutube::EAxis::X;
}
