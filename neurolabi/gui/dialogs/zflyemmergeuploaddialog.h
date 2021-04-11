#ifndef ZFLYEMMERGEUPLOADDIALOG_H
#define ZFLYEMMERGEUPLOADDIALOG_H

#include <QDialog>

namespace Ui {
class ZFlyEmMergeUploadDialog;
}

class ZFlyEmMergeUploadDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZFlyEmMergeUploadDialog(QWidget *parent = 0);
  ~ZFlyEmMergeUploadDialog();

  void setMessage(const QString &msg);
  bool mergingToLargest() const;
  bool mergingToHighestStatus() const;
//  void setBodyList(const QList<uint64_t> &bodyList);
//  uint64_t getMasterBody() const;

private:
  Ui::ZFlyEmMergeUploadDialog *ui;
};

#endif // ZFLYEMMERGEUPLOADDIALOG_H
