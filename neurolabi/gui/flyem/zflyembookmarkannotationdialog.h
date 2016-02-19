#ifndef ZFLYEMBOOKMARKANNOTATIONDIALOG_H
#define ZFLYEMBOOKMARKANNOTATIONDIALOG_H

#include <QDialog>

#include "zflyembookmark.h"

namespace Ui {
class ZFlyEmBookmarkAnnotationDialog;
}

class ZFlyEmBookmarkAnnotationDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZFlyEmBookmarkAnnotationDialog(QWidget *parent = 0);
  ~ZFlyEmBookmarkAnnotationDialog();

  void annotate(ZFlyEmBookmark *bookmark);
  void setFrom(const ZFlyEmBookmark *bookmark);

  void setBodyId(uint64_t bodyId);
  void setType(ZFlyEmBookmark::EBookmarkType type);
  void setComment(const QString &comment);

  ZFlyEmBookmark::EBookmarkType getType() const;

private:
  Ui::ZFlyEmBookmarkAnnotationDialog *ui;
};

#endif // ZFLYEMBOOKMARKANNOTATIONDIALOG_H
