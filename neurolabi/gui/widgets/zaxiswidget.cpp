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


neutu::EAxis ZAxisWidget::getAxis() const
{
  switch (ui->axisComboBox->currentIndex()) {
  case 0:
    return neutu::EAxis::X;
  case 1:
    return neutu::EAxis::Y;
  case 2:
    return neutu::EAxis::Z;
  }

  return neutu::EAxis::X;
}
