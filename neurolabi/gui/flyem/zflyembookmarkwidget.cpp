#include "zflyembookmarkwidget.h"
#include "ui_zflyembookmarkwidget.h"

ZFlyEmBookmarkWidget::ZFlyEmBookmarkWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ZFlyEmBookmarkWidget)
{
  ui->setupUi(this);
}

ZFlyEmBookmarkWidget::~ZFlyEmBookmarkWidget()
{
  delete ui;
}
