#include "zflyemsplituploadoptiondialog.h"
#include "ui_zflyemsplituploadoptiondialog.h"

ZFlyEmSplitUploadOptionDialog::ZFlyEmSplitUploadOptionDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmSplitUploadOptionDialog)
{
  ui->setupUi(this);
}

ZFlyEmSplitUploadOptionDialog::~ZFlyEmSplitUploadOptionDialog()
{
  delete ui;
}
