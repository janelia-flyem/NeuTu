#ifndef ZFLYEMBODYSPLITDIALOG_H
#define ZFLYEMBODYSPLITDIALOG_H

#include <QDialog>

namespace Ui {
class ZFlyEmBodySplitDialog;
}

class ZFlyEmBodySplitDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZFlyEmBodySplitDialog(QWidget *parent = 0);
  ~ZFlyEmBodySplitDialog();

public:
  uint64_t getBodyId() const;
  bool isOfflineSplit() const;

  void setBodyId(uint64_t bodyId);

private:
  Ui::ZFlyEmBodySplitDialog *ui;
};

#endif // ZFLYEMBODYSPLITDIALOG_H
