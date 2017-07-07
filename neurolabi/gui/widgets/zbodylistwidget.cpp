#include "zbodylistwidget.h"
#include "ui_zbodylistwidget.h"

ZBodyListWidget::ZBodyListWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ZBodyListWidget)
{
  ui->setupUi(this);
}

ZBodyListWidget::~ZBodyListWidget()
{
  delete ui;
}
