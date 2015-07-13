#include "zflyembookmarkannotationdialog.h"
#include "ui_zflyembookmarkannotationdialog.h"

ZFlyEmBookmarkAnnotationDialog::ZFlyEmBookmarkAnnotationDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmBookmarkAnnotationDialog)
{
  ui->setupUi(this);
//  ui->commentLabel->hide();
//  ui->commentLineEdit->hide();
}

ZFlyEmBookmarkAnnotationDialog::~ZFlyEmBookmarkAnnotationDialog()
{
  delete ui;
}

void ZFlyEmBookmarkAnnotationDialog::annotate(ZFlyEmBookmark *bookmark)
{
  if (bookmark != NULL) {
    bookmark->setBookmarkType(getType());
    bookmark->setComment(ui->commentLineEdit->text());
  }
}

void ZFlyEmBookmarkAnnotationDialog::setFrom(const ZFlyEmBookmark *bookmark)
{
  if (bookmark != NULL) {
    setBodyId(bookmark->getBodyId());
    setType(bookmark->getBookmarkType());
    setComment(bookmark->getComment());
  }
}

void ZFlyEmBookmarkAnnotationDialog::setComment(const QString &comment)
{
  ui->commentLineEdit->setText(comment);
}

void ZFlyEmBookmarkAnnotationDialog::setBodyId(uint64_t bodyId)
{
  ui->bodyIdLabel->setText(QString("%1").arg(bodyId));
}

void ZFlyEmBookmarkAnnotationDialog::setType(ZFlyEmBookmark::EBookmarkType type)
{
  switch (type) {
  case ZFlyEmBookmark::TYPE_FALSE_SPLIT:
    ui->typeComboBox->setCurrentIndex(0);
    break;
  case ZFlyEmBookmark::TYPE_FALSE_MERGE:
    ui->typeComboBox->setCurrentIndex(1);
    break;
  case ZFlyEmBookmark::TYPE_LOCATION:
    ui->typeComboBox->setCurrentIndex(2);
    break;
  }
}

ZFlyEmBookmark::EBookmarkType ZFlyEmBookmarkAnnotationDialog::getType() const
{
  switch (ui->typeComboBox->currentIndex()) {
  case 0:
    return ZFlyEmBookmark::TYPE_FALSE_SPLIT;
  case 1:
    return ZFlyEmBookmark::TYPE_FALSE_MERGE;
  case 2:
    return ZFlyEmBookmark::TYPE_LOCATION;
  default:
    break;
  }

  return ZFlyEmBookmark::TYPE_LOCATION;
}
