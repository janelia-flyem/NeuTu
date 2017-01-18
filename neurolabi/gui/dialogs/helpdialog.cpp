#include "helpdialog.h"

#include<QFileInfo>

#include "ui_helpdialog.h"
#include "neutubeconfig.h"

HelpDialog::HelpDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::HelpDialog)
{
  ui->setupUi(this);

  QFileInfo fileInfo(NeutubeConfig::getInstance().getHelpFilePath().c_str());

  if (fileInfo.exists()) {
    ui->textBrowser->setSource(QUrl(fileInfo.absoluteFilePath()));
  }
}

HelpDialog::~HelpDialog()
{
  delete ui;
}
