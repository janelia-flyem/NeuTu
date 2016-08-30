#include "flyemsettingdialog.h"
#include <QFileInfo>
#include "QsLog/QsLog.h"

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
  ui->configPushButton->hide();
  ui->dataDirPushButton->hide();
}

void FlyEmSettingDialog::loadSetting()
{
  QString configPath = NeutubeConfig::GetFlyEmConfigPath();
  if (!configPath.isEmpty()) {
    ui->configLineEdit->setText(configPath);
  } else {
    ui->configLineEdit->setText(GET_FLYEM_CONFIG.getConfigPath().c_str());
  }
  ui->servicelineEdit->setText(
        GET_FLYEM_CONFIG.getNeutuService().getServer().c_str());
  ui->statusLabel->setText(
        GET_FLYEM_CONFIG.getNeutuService().isNormal() ? "Normal" : "Down");
  ui->profilingCheckBox->setChecked(NeutubeConfig::LoggingProfile());
  ui->autoStatuscCheckBox->setChecked(NeutubeConfig::AutoStatusCheck());
  ui->verboseSpinBox->setValue(NeutubeConfig::GetVerboseLevel());
  ui->parallelTileCheckBox->setChecked(NeutubeConfig::ParallelTileFetching());
  std::string dataDir = GET_DATA_DIR;
  ui->dataDirLineEdit->setText(QFileInfo(dataDir.c_str()).absoluteFilePath());
  ui->defaultConfigFileCheckBox->setChecked(GET_FLYEM_CONFIG.usingDefaultConfig());
}

void FlyEmSettingDialog::connectSignalSlot()
{
  connect(ui->updatePushButton, SIGNAL(clicked()), this, SLOT(update()));
  connect(ui->closePushButton, SIGNAL(clicked()), this, SLOT(close()));
}

bool FlyEmSettingDialog::usingDefaultConfig() const
{
  return ui->defaultConfigFileCheckBox->isChecked();
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
  GET_FLYEM_CONFIG.setConfigPath(getConfigPath());
  GET_FLYEM_CONFIG.useDefaultConfig(usingDefaultConfig());
  GET_FLYEM_CONFIG.loadConfig();

  NeutubeConfig::EnableProfileLogging(ui->profilingCheckBox->isChecked());
  NeutubeConfig::EnableAutoStatusCheck(ui->autoStatuscCheckBox->isChecked());
  NeutubeConfig::SetVerboseLevel(ui->verboseSpinBox->value());
  if (NeutubeConfig::GetVerboseLevel() >= 5) {
    QsLogging::Logger::instance().setLoggingLevel(QsLogging::TraceLevel);
  }
  NeutubeConfig::SetParallelTileFetching(ui->parallelTileCheckBox->isChecked());
  NeutubeConfig::SetDataDir(ui->dataDirLineEdit->text());
  NeutubeConfig::SetFlyEmConfigPath(getConfigPath().c_str());
  NeutubeConfig::UseDefaultFlyEmConfig(usingDefaultConfig());
}
