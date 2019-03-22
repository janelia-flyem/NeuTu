#include "flyemorthocontrolform.h"
#include "ui_flyemorthocontrolform.h"

#include "logging/utilities.h"
#include "zwidgetmessage.h"

FlyEmOrthoControlForm::FlyEmOrthoControlForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::FlyEmOrthoControlForm)
{
  ui->setupUi(this);

  connectSignalSlot();
}

FlyEmOrthoControlForm::~FlyEmOrthoControlForm()
{
  delete ui;
}

void FlyEmOrthoControlForm::connectSignalSlot()
{
  connect(ui->locateToPushButton, SIGNAL(clicked()),
          this, SIGNAL(locatingMain()));
  connect(ui->resetCrosshairPushButton, SIGNAL(clicked()),
          this, SIGNAL(resettingCrosshair()));
  connect(ui->reloadPushButton, SIGNAL(clicked()),
          this, SIGNAL(reloading()));
  connect(ui->showSegCheckBox, SIGNAL(toggled(bool)),
          this, SIGNAL(showingSeg(bool)));
  connect(ui->dataCheckBox, SIGNAL(toggled(bool)),
          this, SIGNAL(showingData(bool)));
  connect(ui->contrastCheckBox, SIGNAL(toggled(bool)),
          this, SIGNAL(settingHighContrast(bool)));
  connect(ui->smoothCheckBox, SIGNAL(toggled(bool)),
          this, SIGNAL(settingSmooth(bool)));
  connect(ui->crosshairCheckBox, SIGNAL(toggled(bool)),
          this, SIGNAL(showingCrosshair(bool)));
}

void FlyEmOrthoControlForm::toggleShowingSeg()
{
  ui->showSegCheckBox->toggle();
}

void FlyEmOrthoControlForm::toggleData()
{
  ui->dataCheckBox->toggle();
}

bool FlyEmOrthoControlForm::isShowingSeg() const
{
  return ui->showSegCheckBox->isChecked();
}

bool FlyEmOrthoControlForm::isDataVisible() const
{
  return ui->dataCheckBox->isChecked();
}

ZFlyEmMessageWidget* FlyEmOrthoControlForm::getMessageWidget() const
{
  return ui->messageWidget;
}

void FlyEmOrthoControlForm::dump(const ZWidgetMessage &message)
{
  neutu::LogMessage(message);
  if (message.hasTarget(ZWidgetMessage::TARGET_TEXT)) {
    ui->messageWidget->dump(message.toHtmlString(), message.isAppending());
  }
}
