#include "helpdialog.h"

#include<QFileInfo>
#include <QWebEngineView>

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
  } else {
    ui->textBrowser->hide();
    QWebEngineView *view = new QWebEngineView(this);
    view->setUrl(QUrl("https://app.gitbook.com/@janelia-flyem/s/neutu/"));
    ui->verticalLayout->addWidget(view);
  }
}

HelpDialog::~HelpDialog()
{
  delete ui;
}

void HelpDialog::setSource(const QString &source)
{
  ui->textBrowser->setSource(source);
}
