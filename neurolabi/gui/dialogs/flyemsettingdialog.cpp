#include "flyemsettingdialog.h"
#include "ui_flyemsettingdialog.h"
#include "neutubeconfig.h"

FlyEmSettingDialog::FlyEmSettingDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmSettingDialog)
{
  ui->setupUi(this);

  init();
}

FlyEmSettingDialog::~FlyEmSettingDialog()
{
  delete ui;
}

void FlyEmSettingDialog::init()
{
  loadSetting();
  connectSignalSlot();
}

void FlyEmSettingDialog::loadSetting()
{
  ui->configLineEdit->setText(NeutubeConfig::GetFlyEmConfigPath());
  ui->servicelineEdit->setText(NeutubeConfig::GetNeuTuServer());
  ui->statusLabel->setText(
        GET_FLYEM_CONFIG.getNeutuService().isNormal() ? "Normal" : "Down");
  ui->profilingCheckBox->setChecked(NeutubeConfig::LoggingProfile());
  ui->autoStatuscCheckBox->setChecked(NeutubeConfig::AutoStatusCheck());
}

void FlyEmSettingDialog::connectSignalSlot()
{
  connect(ui->updatePushButton, SIGNAL(clicked()), this, SLOT(update()));
  connect(ui->closePushButton, SIGNAL(clicked()), this, SLOT(close()));
}

std::string FlyEmSettingDialog::getNeuTuServer() const
{
  return ui->servicelineEdit->text().trimmed().toStdString();
}

std::string FlyEmSettingDialog::getConfigPath() const
{
  return ui->configLineEdit->text().toStdString();
}

void FlyEmSettingDialog::update()
{
  GET_FLYEM_CONFIG.setServer(getNeuTuServer());
  if (GET_FLYEM_CONFIG.getNeutuService().isNormal()) {
    ui->statusLabel->setText("Normal");
  } else {
    ui->statusLabel->setText("Down");
  }
  GET_FLYEM_CONFIG.loadConfig(getConfigPath());
  NeutubeConfig::EnableProfileLogging(ui->profilingCheckBox->isChecked());
  NeutubeConfig::EnableAutoStatusCheck(ui->autoStatuscCheckBox->isChecked());
}
