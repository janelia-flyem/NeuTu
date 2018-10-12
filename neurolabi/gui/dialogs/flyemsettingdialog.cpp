#include "flyemsettingdialog.h"
#include <QFileInfo>
#include "QsLog/QsLog.h"

#include "ui_flyemsettingdialog.h"
#include "neutubeconfig.h"
#include "flyem/flyemdef.h"

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

void FlyEmSettingDialog::updateNeutuseWidget()
{
  QPalette *palette = new QPalette();
  palette->setColor(
        QPalette::Text,
        GET_FLYEM_CONFIG.hasNormalService() ? Qt::darkGreen : Qt::darkRed);
  ui->servicelineEdit->setPalette(*palette);
}

void FlyEmSettingDialog::init()
{
  loadSetting();
  connectSignalSlot();
  ui->configPushButton->hide();
  ui->dataDirPushButton->hide();
  ui->defaultConfigLineEdit->setText(
        Shrink(GET_FLYEM_CONFIG.getDefaultConfigPath().c_str(), 40));
  ui->defaultConfigLineEdit->setToolTip(
        GET_FLYEM_CONFIG.getDefaultConfigPath().c_str());

  updateDefaultConfigChecked(usingDefaultConfig());
  updateDefaultNeuTuServerChecked(usingDefaultService());
  updateDefaultTaskServerChecked(usingDefaultTaskServer());
}

void FlyEmSettingDialog::loadSetting()
{
  QString configPath = NeutubeConfig::GetFlyEmConfigPath();
  if (!configPath.isEmpty()) {
    ui->configLineEdit->setText(configPath);
  } else {
#if defined(_FLYEM_)
    ui->configLineEdit->setText(GET_FLYEM_CONFIG.getConfigPath().c_str());
#endif
  }
  ui->configLineEdit->setToolTip(ui->configLineEdit->text());

#if defined(_FLYEM_)
  updateNeutuseWidget();

  ui->servicelineEdit->setText(GET_FLYEM_CONFIG.getNeuTuServer().c_str());

//  ui->servicelineEdit->setText(
//        GET_FLYEM_CONFIG.getNeutuService().getServer().c_str());

//  ui->statusLabel->setText(
//        GET_FLYEM_CONFIG.hasNormalService() ? "Normal" : "Down");
#ifdef _DEBUG_
  std::cout << "Current task server: " << GET_FLYEM_CONFIG.getTaskServer() << std::endl;
#endif
  ui->taskServerLineEdit->setText(GET_FLYEM_CONFIG.getTaskServer().c_str());
#endif
  ui->profilingCheckBox->setChecked(NeutubeConfig::LoggingProfile());
  ui->autoStatuscCheckBox->setChecked(NeutubeConfig::AutoStatusCheck());
  ui->verboseSpinBox->setValue(NeutubeConfig::GetVerboseLevel());
  ui->parallelTileCheckBox->setChecked(NeutubeConfig::ParallelTileFetching());
  ui->prefetchCheckBox->setChecked(NeutubeConfig::LowtisPrefetching());
  std::string dataDir = GET_DATA_DIR;
  ui->dataDirLineEdit->setText(QFileInfo(dataDir.c_str()).absoluteFilePath());
#if defined(_FLYEM_)
  ui->defaultConfigFileCheckBox->setChecked(
        GET_FLYEM_CONFIG.usingDefaultConfig());
  ui->defaultConfigFileCheckBox->setToolTip(
        GET_FLYEM_CONFIG.getDefaultConfigPath().c_str());
  ui->defaultServiceCheckBox->setChecked(
        GET_FLYEM_CONFIG.usingDefaultNeuTuServer());
  ui->defaultTaskServerCheckBox->setChecked(
        GET_FLYEM_CONFIG.usingDefaultTaskServer());
  ui->synapseNameCheckBox->setChecked(NeutubeConfig::NamingSynapse());

  std::pair<int,int> centerCut = GET_FLYEM_CONFIG.getCenterCut(
        flyem::key::GRAYSCALE);
  ui->grayCxSpinBox->setValue(centerCut.first);
  ui->grayCySpinBox->setValue(centerCut.second);

  centerCut = GET_FLYEM_CONFIG.getCenterCut(flyem::key::SEGMENTATION);
  ui->segCxSpinBox->setValue(centerCut.first);
  ui->segCySpinBox->setValue(centerCut.second);
  ui->psdNameCheckBox->setChecked(NeutubeConfig::NamingPsd());
#endif
  ui->meshThreSpinBox->setValue(
        NeutubeConfig::GetMeshSplitThreshold() / 1000000);
}

void FlyEmSettingDialog::connectSignalSlot()
{
  connect(ui->updatePushButton, SIGNAL(clicked()), this, SLOT(update()));
  connect(ui->closePushButton, SIGNAL(clicked()), this, SLOT(close()));
  connect(ui->defaultConfigFileCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(updateDefaultConfigChecked(bool)));
  connect(ui->defaultServiceCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(updateDefaultNeuTuServerChecked(bool)));
  connect(ui->defaultTaskServerCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(updateDefaultTaskServerChecked(bool)));
}

bool FlyEmSettingDialog::usingDefaultConfig() const
{
  return ui->defaultConfigFileCheckBox->isChecked();
}

bool FlyEmSettingDialog::usingDefaultService() const
{
  return ui->defaultServiceCheckBox->isChecked();
}

bool FlyEmSettingDialog::usingDefaultTaskServer() const
{
  return ui->defaultTaskServerCheckBox->isChecked();
}

bool FlyEmSettingDialog::namingSynapse() const
{
  return ui->synapseNameCheckBox->isChecked();
}

bool FlyEmSettingDialog::namingPsd() const
{
  return ui->psdNameCheckBox->isChecked();
}

std::string FlyEmSettingDialog::getNeuTuServer() const
{
  return ui->servicelineEdit->text().trimmed().toStdString();
}

std::string FlyEmSettingDialog::getTaskServer() const
{
  return ui->taskServerLineEdit->text().trimmed().toStdString();
}

std::string FlyEmSettingDialog::getConfigPath() const
{
  return ui->configLineEdit->text().toStdString();
}

void FlyEmSettingDialog::updateDefaultConfigChecked(bool on)
{
  ui->defaultConfigLineEdit->setVisible(on);
  ui->configLineEdit->setVisible(!on);
  ui->configPushButton->setVisible(!on);
}

void FlyEmSettingDialog::updateDefaultNeuTuServerChecked(bool on)
{
  ui->servicelineEdit->setText(GET_FLYEM_CONFIG.getNeuTuServer(on).c_str());
  ui->servicelineEdit->setReadOnly(on);
  ui->servicelineEdit->setFrame(!on);
}

void FlyEmSettingDialog::updateDefaultTaskServerChecked(bool on)
{
  ui->taskServerLineEdit->setText(GET_FLYEM_CONFIG.getTaskServer(on).c_str());
  ui->taskServerLineEdit->setReadOnly(on);
  ui->taskServerLineEdit->setFrame(!on);
}

void FlyEmSettingDialog::update()
{
#if defined(_FLYEM_)
  GET_FLYEM_CONFIG.setConfigPath(getConfigPath());
  GET_FLYEM_CONFIG.useDefaultConfig(usingDefaultConfig());
  GET_FLYEM_CONFIG.loadConfig();

  GET_FLYEM_CONFIG.useDefaultNeuTuServer(usingDefaultService());
  if (!usingDefaultService()) {
    GET_FLYEM_CONFIG.setCustomNeuTuServer(getNeuTuServer());
  }

  GET_FLYEM_CONFIG.activateNeuTuServer();
  updateNeutuseWidget();

  /*
  if (GET_FLYEM_CONFIG.hasNormalService()) {
    ui->statusLabel->setText("Normal");
  } else {
    ui->statusLabel->setText("Down");
  }
  */
  GET_FLYEM_CONFIG.useDefaultTaskServer(usingDefaultTaskServer());
  if (!usingDefaultTaskServer()) {
    GET_FLYEM_CONFIG.setCustomTaskServer(getTaskServer());
  }

  GET_FLYEM_CONFIG.setAnalyzingMb6(namingSynapse());

  GET_FLYEM_CONFIG.setCenterCut(
        flyem::key::GRAYSCALE, ui->grayCxSpinBox->value(),
        ui->grayCySpinBox->value());
  GET_FLYEM_CONFIG.setCenterCut(
        flyem::key::SEGMENTATION, ui->segCxSpinBox->value(),
        ui->segCySpinBox->value());
  GET_FLYEM_CONFIG.setPsdNameDetail(namingPsd());
#endif

  NeutubeConfig::EnableProfileLogging(ui->profilingCheckBox->isChecked());
  NeutubeConfig::EnableAutoStatusCheck(ui->autoStatuscCheckBox->isChecked());
  NeutubeConfig::SetVerboseLevel(ui->verboseSpinBox->value());
  if (NeutubeConfig::GetVerboseLevel() >= 5) {
    QsLogging::Logger::instance().setLoggingLevel(QsLogging::TraceLevel);
  }
  NeutubeConfig::SetParallelTileFetching(ui->parallelTileCheckBox->isChecked());
  NeutubeConfig::EnableLowtisPrefetching(ui->prefetchCheckBox->isChecked());
  NeutubeConfig::SetDataDir(ui->dataDirLineEdit->text());
  NeutubeConfig::SetFlyEmConfigPath(getConfigPath().c_str());
  NeutubeConfig::UseDefaultFlyEmConfig(usingDefaultConfig());
  NeutubeConfig::UseDefaultNeuTuServer(usingDefaultService());
  NeutubeConfig::UseDefaultTaskServer(usingDefaultTaskServer());
  NeutubeConfig::SetNamingSynapse(namingSynapse());
  NeutubeConfig::SetNamingPsd(namingPsd());
  NeutubeConfig::SetMeshSplitThreshold(ui->meshThreSpinBox->value() * 1000000);
}

QString FlyEmSettingDialog::Shrink(const QString &str, int len)
{
  QString newStr = str;
  if (str.size() > len) {
    newStr = str.left(len / 2 - 1) + "..." + str.right(len / 2 - 1);
  }

  return newStr;
}
