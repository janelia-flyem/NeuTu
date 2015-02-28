#include "zflyemhackathonconfigdlg.h"
#include "ui_zflyemhackathonconfigdlg.h"
#include "zdialogfactory.h"

ZFlyEmHackathonConfigDlg::ZFlyEmHackathonConfigDlg(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmHackathonConfigDlg)
{
  ui->setupUi(this);

  connect(ui->sourceDirPushButton, SIGNAL(clicked()),
          this, SLOT(setSourceDir()));
  connect(ui->workDirPushButton, SIGNAL(clicked()),
          this, SLOT(setWorkDir()));
}

ZFlyEmHackathonConfigDlg::~ZFlyEmHackathonConfigDlg()
{
  delete ui;
}

QString ZFlyEmHackathonConfigDlg::getSourceDir() const
{
  return ui->sourceLineEdit->text();
}

QString ZFlyEmHackathonConfigDlg::setSourceDir()
{
  return ZDialogFactory::GetDirectory("Source Directory", "", this);
}

QString ZFlyEmHackathonConfigDlg::setWorkDir()
{
  return ZDialogFactory::GetDirectory("Working Directory", "", this);
}

QString ZFlyEmHackathonConfigDlg::getWorkDir() const
{
  return ui->workDirLineEdit->text();
}

bool ZFlyEmHackathonConfigDlg::usingInternalDvid() const
{
  return ui->internalCheckBox->isChecked();
}
