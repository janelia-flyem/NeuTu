#include "neuprintsetupdialog.h"

#include <fstream>

#include "zjsonparser.h"
#include "ui_neuprintsetupdialog.h"
#include "neutubeconfig.h"

#include "qt/core/utilities.h"
#include "zdialogfactory.h"
#include "zglobal.h"
#include "service/neuprintreader.h"
#include "neuprintdatasetdialog.h"

NeuprintSetupDialog::NeuprintSetupDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::NeuprintSetupDialog)
{
  ui->setupUi(this);

  ui->serverLineEdit->setText(ZGlobal::GetInstance().getNeuPrintServer());
  QString auth = ZGlobal::GetInstance().getNeuPrintAuth();
  m_auth.decode(auth.toStdString(), false);
  if (m_auth.isEmpty()) {
    m_auth.setNonEmptyEntry("token", auth.toStdString());
  }
  ui->tokenTextEdit->setPlainText(getDefaultAuthToken());
  connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)),
          this, SLOT(processButtonClick(QAbstractButton*)));
}

NeuprintSetupDialog::~NeuprintSetupDialog()
{
  delete ui;
}

QString NeuprintSetupDialog::getDefaultAuthToken() const
{
  QString server = neutu::NormalizeServerAddress(ui->serverLineEdit->text());
  if (m_auth.hasKey(server.toStdString())) {
    return ZJsonParser::stringValue(m_auth[server.toStdString().c_str()]).c_str();
  }

  return ZJsonParser::stringValue(m_auth["token"]).c_str();
//  return ui->tokenTextEdit->toPlainText();
}

QString NeuprintSetupDialog::getAuthToken() const
{
  return ui->tokenTextEdit->toPlainText();
}

void NeuprintSetupDialog::processButtonClick(QAbstractButton *button)
{
  if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole) {
    if (apply()) {
      accept();
    }
  }
}

void NeuprintSetupDialog::setUuid(const QString &uuid)
{
  m_uuid = uuid;
}

std::unique_ptr<NeuPrintReader> NeuprintSetupDialog::takeNeuPrintReader()
{
  return std::move(m_reader);
}

void NeuprintSetupDialog::storeToken(const QString &server, const QString &token)
{
  m_auth.setNonEmptyEntry(server.toStdString().c_str(), token.toStdString());
  m_auth.dump(NeutubeConfig::getInstance().getPath(
                NeutubeConfig::EConfigItem::NEUPRINT_AUTH));
}

bool NeuprintSetupDialog::makeNeuPrintReader(
    const QString &normalizedServer, const QString &token)
{
  ZGlobal::GetInstance().setNeuPrintServer(normalizedServer);
//    NeuPrintReader *reader = ZGlobal::GetInstance().getNeuPrintReader();

  if (m_reader) {
    if (m_reader->getServer() != normalizedServer) {
      m_reader.reset();
    }
  }

  if (!m_reader) {
    m_reader = std::unique_ptr<NeuPrintReader>(new NeuPrintReader(normalizedServer));
  }

  std::string rawToken = token.toStdString();
  ZJsonObject tokenObj;
  tokenObj.decodeString(token.toStdString().c_str());
  if (tokenObj.hasKey("token")) {
    rawToken = ZJsonParser::stringValue(tokenObj["token"]);
  }

  m_reader->authorize(rawToken.c_str());
  if (m_reader->isConnected()) {
    storeToken(normalizedServer, rawToken.c_str());

    if (m_reader->hasDataset(m_uuid)) {
      m_reader->setCurrentDataset(m_reader->getDataset(m_uuid));
      return true;
    } else {
      QStringList datasetList = m_reader->getDatasetList();
      if (!m_reader->getDatasetList().isEmpty()) {
        NeuprintDatasetDialog datasetDlg;
        datasetDlg.setDatasetList(datasetList);
        datasetDlg.setHintLabel(
              "No matched dataset found. Select one to continue:");
        if (datasetDlg.exec()) {
          m_reader->setCurrentDataset(datasetDlg.getDataset());
          return true;
        }
      } else {
        QString details;
        for (const QString &dataset : m_reader->getDatasetList()) {
          details += dataset + " ";
        }
        details += token;

        ZDialogFactory::Warn(
              "NeuPrint Not Supported",
              "Cannot use NeuPrint at " + normalizedServer +
              " because no dataset is found on the server\n"
              "Details: " + details,
              this);
      }
    }
  } else {
    if (m_reader->getStatus() == neutu::EServerStatus::NOAUTH) {
      ZDialogFactory::Warn(
            "Authorization Failed",
            "Failed to get authorization from NeuPrint at " + normalizedServer +
            " with token \n" + rawToken.c_str(),
            this);
    } else if (m_reader->getStatus() == neutu::EServerStatus::OFFLINE) {
      ZDialogFactory::Warn(
            "No Connect", "Cannot connect to " + normalizedServer,
            this);
    } else {
      ZDialogFactory::Warn(
            "Config Failure",
            "Cannot use the NeuPrint server because of some unknown reason.",
            this);
    }
  }

  return false;
}

bool NeuprintSetupDialog::apply()
{
  QString server = ui->serverLineEdit->text();
  if (server.isEmpty()) {
    ZDialogFactory::Warn(
          "No Server Specified", "Please specify the NeuPrint server", this);
  } else {
    QString token = getAuthToken();
    if (token.isEmpty()) {
      ZDialogFactory::Warn(
            "No Authorization", "Please specify the authorization token",
            this);
      m_auth.removeKey(server.toStdString().c_str());
      m_auth.removeKey("token");
    } else {
      return makeNeuPrintReader(
            neutu::NormalizeServerAddress(server, "https"),
            token);
    }
  }

  return false;
}
