#include "neuprintsetupdialog.h"

#include <fstream>

#include "zjsonparser.h"
#include "ui_neuprintsetupdialog.h"
#include "neutubeconfig.h"

#include "qt/core/utilities.h"
#include "zdialogfactory.h"
#include "zglobal.h"
#include "service/neuprintreader.h"

NeuprintSetupDialog::NeuprintSetupDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::NeuprintSetupDialog)
{
  ui->setupUi(this);

  ui->serverLineEdit->setText(ZGlobal::GetInstance().getNeuPrintServer());
  connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)),
          this, SLOT(processButtonClick(QAbstractButton*)));
}

NeuprintSetupDialog::~NeuprintSetupDialog()
{
  delete ui;
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

bool NeuprintSetupDialog::apply()
{
  QString server = ui->serverLineEdit->text();
  if (server.isEmpty()) {
    ZDialogFactory::Warn(
          "No Server Specified", "Please specify the NeuPrint server", this);
  } else {
    server = neutu::NormalizeServerAddress(server, "https");

    ZGlobal::GetInstance().setNeuPrintServer(server);
    NeuPrintReader *reader = ZGlobal::GetInstance().getNeuPrintReader();
    if (reader) {
      QString token = getAuthToken();
      if (token.isEmpty()) {
        ZDialogFactory::Warn(
              "No Authorization", "Please specify the authorization token",
              this);
      } else {
        if (ZJsonParser::IsObject(token.toStdString()) == false) {
          token = "{\"token\":\"" + token + "\"" + "}";
        }

        reader->authorizeFromJson(token);
        if (reader->hasAuthCode()) {
          QString authFile = NeutubeConfig::getInstance().getPath(
                NeutubeConfig::EConfigItem::NEUPRINT_AUTH).c_str();
          std::ofstream stream(authFile.toStdString());
          if (stream.good()) {
            stream << token.toStdString();
            stream.close();
          }

          reader->connect();
          if (reader->hasDataset(m_uuid)) {
            return true;
          } else {
            QString details;
            for (const QString &dataset : reader->getDatasetList()) {
              details += dataset + " ";
            }
            details += token;

            ZDialogFactory::Warn(
                  "NeuPrint Not Supported",
                  "Cannot use NeuPrint at " + server +
                  " because this dataset is not supported by the server"
                  " or th token is wrong.\n\n"
                  "Details: " + details,
                  this);
          }
        } else {
          ZDialogFactory::Warn(
                "Authorization Failed",
                "Failed to get authorization from NeuPrint at " + server +
                " with token \n" + token,
                this);
        }
      }
    } else {
      ZDialogFactory::Warn(
            "Config Failure",
            "Cannot use the NeuPrint server because of some unknown reason.",
            this);
    }
  }

  return false;
}
