#ifndef ZFLYEMSPLITUPLOADOPTIONDIALOG_H
#define ZFLYEMSPLITUPLOADOPTIONDIALOG_H

#include <QDialog>

namespace Ui {
class ZFlyEmSplitUploadOptionDialog;
}

class ZFlyEmSplitUploadOptionDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZFlyEmSplitUploadOptionDialog(QWidget *parent = 0);
  ~ZFlyEmSplitUploadOptionDialog();

private:
  Ui::ZFlyEmSplitUploadOptionDialog *ui;
};

#endif // ZFLYEMSPLITUPLOADOPTIONDIALOG_H
