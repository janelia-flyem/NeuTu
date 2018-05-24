#ifndef ZFLYEMSKELETONUPDATEDIALOG_H
#define ZFLYEMSKELETONUPDATEDIALOG_H

#include <QDialog>

namespace Ui {
class ZFlyEmSkeletonUpdateDialog;
}

class ZFlyEmSkeletonUpdateDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZFlyEmSkeletonUpdateDialog(QWidget *parent = 0);
  ~ZFlyEmSkeletonUpdateDialog();

  void setComputingServer(const QString &address);
  bool isOverwriting() const;

private:
  Ui::ZFlyEmSkeletonUpdateDialog *ui;
};

#endif // ZFLYEMSKELETONUPDATEDIALOG_H
