#include "zstackframesettingdialog.h"
#include "ui_zstackframesettingdialog.h"
#include "zneurontracerconfig.h"

ZStackFrameSettingDialog::ZStackFrameSettingDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZStackFrameSettingDialog)
{
  ui->setupUi(this);
}

ZStackFrameSettingDialog::~ZStackFrameSettingDialog()
{
  delete ui;
}

void ZStackFrameSettingDialog::setMinAutoScore(double score)
{
  ui->autoScoreDoubleSpinBox->setValue(score);
}

void ZStackFrameSettingDialog::setMinManualScore(double score)
{
  ui->manualScoreDoubleSpinBox->setValue(score);
}

void ZStackFrameSettingDialog::setMinSeedScore(double score)
{
  ui->seedDoubleSpinBox->setValue(score);
}

void ZStackFrameSettingDialog::setRefit(bool on)
{
  if (on) {
    ui->hardRadioButton->setChecked(true);
  } else {
    ui->normalRadioButton->setChecked(true);
  }
}

void ZStackFrameSettingDialog::setSpTest(bool on)
{
  ui->spTestCheckBox->setChecked(on);
}

void ZStackFrameSettingDialog::setCrossoverTest(bool on)
{
  ui->crossoverCheckBox->setChecked(on);
}

void ZStackFrameSettingDialog::setEnhancingMask(bool on)
{
  ui->enhancementCheckBox->setChecked(on);
}

void ZStackFrameSettingDialog::setRecoverLevel(int level)
{
  ui->recoverSpinBox->setValue(level);
}

void ZStackFrameSettingDialog::setMaxEucDist(double d)
{
  ui->maxDistDoubleSpinBox->setValue(d);
}

double ZStackFrameSettingDialog::getMinAutoScore() const
{
  return ui->autoScoreDoubleSpinBox->value();
}

double ZStackFrameSettingDialog::getMinManualScore() const
{
  return ui->manualScoreDoubleSpinBox->value();
}

double ZStackFrameSettingDialog::getMinSeedScore() const
{
  return ui->seedDoubleSpinBox->value();
}

bool ZStackFrameSettingDialog::getRefit() const
{
  return ui->hardRadioButton->isChecked();
}

bool ZStackFrameSettingDialog::getSpTest() const
{
  return ui->spTestCheckBox->isChecked();
}

bool ZStackFrameSettingDialog::getEnhancingMask() const
{
  return ui->enhancementCheckBox->isChecked();
}

int ZStackFrameSettingDialog::getRecoverLevel() const
{
  return ui->recoverSpinBox->value();
}

double ZStackFrameSettingDialog::getMaxEucDist() const
{
  return ui->maxDistDoubleSpinBox->value();
}

void ZStackFrameSettingDialog::updateTracingConfig(ZNeuronTracerConfig *config)
{
  if (config != NULL) {
    config->setEnhancingMask(getEnhancingMask());
    config->setMaxEucDist(getMaxEucDist());
    config->setMinAutoScore(getMinAutoScore());
    config->setMinManualScore(getMinManualScore());
    config->setMinSeedScore(getMinSeedScore());
    config->setRecoverLevel(getRecoverLevel());
    config->setRefit(getRefit());
    config->setSpTest(getSpTest());
    config->setCrossoverTest(getCrossoverTest());
  }
}

void ZStackFrameSettingDialog::setFromTracingConfig(
    const ZNeuronTracerConfig &config)
{
  setEnhancingMask(config.enhancingMask());
  setMaxEucDist(config.getMaxEucDist());
  setMinAutoScore(config.getMinAutoScore());
  setMinManualScore(config.getMinManualScore());
  setMinSeedScore(config.getMinSeedScore());
  setRecoverLevel(config.getRecoverLevel());
  setRefit(config.isRefit());
  setSpTest(config.spTest());
  setCrossoverTest(config.crossoverTest());
}

double ZStackFrameSettingDialog::getXScale() const
{
  return ui->xDoubleSpinBox->value();
}

double ZStackFrameSettingDialog::getYScale() const
{
  return ui->yDoubleSpinBox->value();
}

double ZStackFrameSettingDialog::getZScale() const
{
  return ui->zDoubleSpinBox->value();
}

bool ZStackFrameSettingDialog::getCrossoverTest() const
{
  return ui->crossoverCheckBox->isChecked();
}

neutube::EImageBackground ZStackFrameSettingDialog::getBackground() const
{
  if (ui->darkRadioButton->isChecked()) {
    return neutube::EImageBackground::DARK;
  }

  return neutube::EImageBackground::BRIGHT;
}

void ZStackFrameSettingDialog::setScale(double x, double y, double z)
{
  ui->xDoubleSpinBox->setValue(x);
  ui->yDoubleSpinBox->setValue(y);
  ui->zDoubleSpinBox->setValue(z);
}

void ZStackFrameSettingDialog::setBackground(neutube::EImageBackground bg)
{
  if (bg == neutube::EImageBackground::DARK) {
    ui->darkRadioButton->setChecked(true);
  }
}
