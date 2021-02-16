#include "zflyembookmarkwidget.h"

#include <iostream>

#include "common/utilities.h"
#include "ui_zflyembookmarkwidget.h"

ZFlyEmBookmarkWidget::ZFlyEmBookmarkWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ZFlyEmBookmarkWidget)
{
  ui->setupUi(this);

  connect(ui->bookmarkTab, SIGNAL(currentChanged(int)),
          this, SLOT(onTabChanged(int)));
  connect(ui->importPushButton, &QPushButton::clicked,
          this, &ZFlyEmBookmarkWidget::importBookmark);
  connect(ui->exportPushButton, &QPushButton::clicked,
          this, &ZFlyEmBookmarkWidget::exportBookmark);
}

ZFlyEmBookmarkWidget::~ZFlyEmBookmarkWidget()
{
  delete ui;
}

void ZFlyEmBookmarkWidget::setBookmarkModel(
    ZFlyEmBookmarkListModel *model,
    EBookmarkSource source)
{
  getBookmarkView(source)->setBookmarkModel(model);
}

ZFlyEmBookmarkView* ZFlyEmBookmarkWidget::getBookmarkView(EBookmarkSource source)
{
  switch (source) {
  case EBookmarkSource::ASSIGNED:
    return qobject_cast<ZFlyEmBookmarkView*>(ui->assignedBookmarkView);
  case EBookmarkSource::USER:
    return qobject_cast<ZFlyEmBookmarkView*>(ui->userBookmarkView);
  }

  return NULL;
}

ZFlyEmBookmarkWidget::EBookmarkSource ZFlyEmBookmarkWidget::getCurrentSource() {
    int index = ui->bookmarkTab->currentIndex();
    return static_cast<EBookmarkSource>(index);
}

void ZFlyEmBookmarkWidget::importBookmark()
{
#ifdef _DEBUG_
  std::cout << __func__ << ": " << neutu::ToString(getCurrentSource()) << std::endl;
#endif

  switch (getCurrentSource()) {
  case EBookmarkSource::USER:
    emit importingUserBookmark();
    break;
  case EBookmarkSource::ASSIGNED:
    emit importingAssignedBookmark();
    break;
  }
}

void ZFlyEmBookmarkWidget::exportBookmark()
{
#ifdef _DEBUG_
  std::cout << __func__ << ": " << neutu::ToString(getCurrentSource()) << std::endl;
#endif

  switch (getCurrentSource()) {
  case EBookmarkSource::USER:
    emit exportingUserBookmark();
    break;
  case EBookmarkSource::ASSIGNED:
    emit exportingAssignedBookmark();
    break;
  }
}

void ZFlyEmBookmarkWidget::onTabChanged(int index)
{
  ui->bookmarkFilter->onTabChanged(index);

  ui->importPushButton->setEnabled(index == 1);
  ui->exportPushButton->setEnabled(index == 1);
}

void ZFlyEmBookmarkWidget::hideFileButtons()
{
  ui->importPushButton->hide();
  ui->exportPushButton->hide();
}
