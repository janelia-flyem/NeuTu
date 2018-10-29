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

  enum class EMode {
    SELECTED, TOP
  };

  void setComputingServer(const QString &address);
  bool isOverwriting() const;

  void setMode(EMode mode);
  int getTopCount() const;

private:
  void updateWidget();

private:
  Ui::ZFlyEmSkeletonUpdateDialog *ui;
  EMode m_mode = EMode::SELECTED;
};

#endif // ZFLYEMSKELETONUPDATEDIALOG_H
