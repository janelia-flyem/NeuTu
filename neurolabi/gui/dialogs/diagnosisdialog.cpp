#include "diagnosisdialog.h"
#include "ui_diagnosisdialog.h"
#include <QTextBrowser>
#include <QFile>
#include <QTextStream>
#include <QScrollBar>
#include <iostream>

#include "neutube.h"
#include "neutubeconfig.h"
#include "flyem/zflyemmisc.h"

DiagnosisDialog::DiagnosisDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DiagnosisDialog)
{
  ui->setupUi(this);
  loadErrorFile();
  loadWarnFile();
  loadInfoFile();
  connect(ui->tabWidget, SIGNAL(currentChanged(int)), SLOT(scrollToBottom(int)));
}

DiagnosisDialog::~DiagnosisDialog()
{
  delete ui;
}

void DiagnosisDialog::LoadFile(
    const std::string &filePath, QTextBrowser *browser)
{
  if (!filePath.empty()) {
    QFile file(filePath.c_str());
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      browser->setPlainText(flyem::ReadLastLines(filePath.c_str(), 1000));
      file.close();
    }
  }
}

void DiagnosisDialog::loadErrorFile()
{
  LoadFile(neutube::getErrorFile(), ui->errorTextBrowser);
}

void DiagnosisDialog::loadWarnFile()
{
  LoadFile(neutube::getWarnFile(), ui->warnTextBrowser);
}

void DiagnosisDialog::loadInfoFile()
{
  LoadFile(NeutubeConfig::getInstance().getPath(NeutubeConfig::EConfigItem::LOG_FILE),
           ui->infoTextBrowser);
}

void DiagnosisDialog::scrollToBottom(int index)
{
  switch (index) {
  case 0:
    ui->errorTextBrowser->verticalScrollBar()->setValue(
          ui->errorTextBrowser->verticalScrollBar()->maximum());
    break;
  case 1:
    ui->warnTextBrowser->verticalScrollBar()->setValue(
          ui->warnTextBrowser->verticalScrollBar()->maximum());
    break;
  case 2:
    ui->infoTextBrowser->verticalScrollBar()->setValue(
          ui->infoTextBrowser->verticalScrollBar()->maximum());
    break;
  default:
    break;
  }
}

void DiagnosisDialog::scrollToBottom()
{
  ui->errorTextBrowser->verticalScrollBar()->setValue(
        ui->errorTextBrowser->verticalScrollBar()->maximum());
  ui->warnTextBrowser->verticalScrollBar()->setValue(
        ui->warnTextBrowser->verticalScrollBar()->maximum());
  ui->infoTextBrowser->verticalScrollBar()->setValue(
        ui->infoTextBrowser->verticalScrollBar()->maximum());
}

void DiagnosisDialog::setSystemInfo(const QString &str)
{
  ui->videoCardInfoTextEdit->setPlainText(str);
}

void DiagnosisDialog::setSystemInfo(const QStringList &info)
{
  setSystemInfo(info.join("\n"));
}
