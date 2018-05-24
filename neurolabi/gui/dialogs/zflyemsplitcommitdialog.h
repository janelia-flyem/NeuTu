#ifndef ZFLYEMSPLITCOMMITDIALOG_H
#define ZFLYEMSPLITCOMMITDIALOG_H

#include <QDialog>

namespace Ui {
class ZFlyEmSplitCommitDialog;
}

class ZFlyEmSplitCommitDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZFlyEmSplitCommitDialog(QWidget *parent = 0);
  ~ZFlyEmSplitCommitDialog();

  int getGroupSize() const;

  bool keepingMainSeed() const;
  bool runningCca() const;

private:
  Ui::ZFlyEmSplitCommitDialog *ui;
};

#endif // ZFLYEMSPLITCOMMITDIALOG_H
