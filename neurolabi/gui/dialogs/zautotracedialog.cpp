#include "zautotracedialog.h"
#include "ui_zautotracedialog.h"

ZAutoTraceDialog::ZAutoTraceDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZAutoTraceDialog)
{
  ui->setupUi(this);

  setChannelCount(1);

  updateWidget();

  connect(ui->defaultLevelCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(updateWidget()));
  connect(ui->levelSlider, SIGNAL(valueChanged(int)),
          this, SLOT(updateWidget()));

#ifndef _ADVANCED_
  ui->diagnosisCheckBox->hide();
  ui->overTraceCheckBox->hide();
  ui->seedScreenCheckBox->hide();
#endif
}

ZAutoTraceDialog::~ZAutoTraceDialog()
{
  delete ui;
}

bool ZAutoTraceDialog::usingDefaultLevel() const
{
  return ui->defaultLevelCheckBox->isChecked();
}

void ZAutoTraceDialog::setChannelCount(int count)
{
  if (ui->channelComboBox->count() != count) {
    ui->channelComboBox->clear();
    for (int i = 1; i <= count; ++i) {
      ui->channelComboBox->addItem(QString("Ch %1").arg(i));
    }
  }

  ui->channelComboBox->setEnabled(count > 1);
}

int ZAutoTraceDialog::getChannel() const
{
  return ui->channelComboBox->currentIndex();
}

bool ZAutoTraceDialog::resampling() const
{
  return ui->resampleCheckBox->isChecked();
}

bool ZAutoTraceDialog::diagnosis() const
{
  return ui->diagnosisCheckBox->isChecked();
}

bool ZAutoTraceDialog::overTracing() const
{
  return ui->overTraceCheckBox->isChecked();
}

bool ZAutoTraceDialog::screenSeed() const
{
  return ui->seedScreenCheckBox->isChecked();
}

int ZAutoTraceDialog::getTraceLevel() const
{
  if (usingDefaultLevel()) {
    return 0;
  }

  return ui->levelSlider->value();
}

void ZAutoTraceDialog::updateWidget()
{
  ui->levelSlider->setDisabled(usingDefaultLevel());
  if (usingDefaultLevel()) {
    ui->levelLabel->setText("");
  } else {
    ui->levelLabel->setText(QString("<font face=courier>(%1/6)</font>").arg(ui->levelSlider->value()));
  }
}
