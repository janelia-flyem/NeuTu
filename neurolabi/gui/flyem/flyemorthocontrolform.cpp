#include "flyemorthocontrolform.h"
#include "ui_flyemorthocontrolform.h"
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
  /*
  connect(ui->upPushButton, SIGNAL(clicked()),
          this, SIGNAL(movingUp()));
  connect(ui->downPushButton, SIGNAL(clicked()),
          this, SIGNAL(movingDown()));
  connect(ui->leftPushButton, SIGNAL(clicked()),
          this, SIGNAL(movingLeft()));
  connect(ui->rightPushButton, SIGNAL(clicked()),
          this, SIGNAL(movingRight()));
          */
  connect(ui->locateToPushButton, SIGNAL(clicked()),
          this, SIGNAL(locatingMain()));
  connect(ui->showSegCheckBox, SIGNAL(toggled(bool)),
          this, SIGNAL(showingSeg(bool)));
  connect(ui->dataCheckBox, SIGNAL(toggled(bool)),
          this, SIGNAL(showingData(bool)));
  connect(ui->contrastCheckBox, SIGNAL(toggled(bool)),
          this, SIGNAL(settingHighContrast(bool)));
  connect(ui->smoothCheckBox, SIGNAL(toggled(bool)),
          this, SIGNAL(settingSmooth(bool)));
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
  ui->messageWidget->dump(message.toHtmlString(), message.isAppending());
}
