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

void ZFlyEmBookmarkWidget::setBookmarkModel(
    ZFlyEmBookmarkListModel *model, EBookmarkSource source)
{
  getBookmarkView(source)->setBookmarkModel(model);
}

ZFlyEmBookmarkView* ZFlyEmBookmarkWidget::getBookmarkView(EBookmarkSource source)
{
  switch (source) {
  case SOURCE_ASSIGNED:
    return qobject_cast<ZFlyEmBookmarkView*>(ui->assignedBookmarkView);
  case SOURCE_USER:
    return qobject_cast<ZFlyEmBookmarkView*>(ui->userBookmarkView);
  }

  return NULL;
}
