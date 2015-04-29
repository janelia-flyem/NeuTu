#include "flyemsplitcontrolform.h"
#include "ui_flyemsplitcontrolform.h"

FlyEmSplitControlForm::FlyEmSplitControlForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::FlyEmSplitControlForm)
{
  ui->setupUi(this);

  setupWidgetBehavior();
}

FlyEmSplitControlForm::~FlyEmSplitControlForm()
{
  delete ui;
}

void FlyEmSplitControlForm::setupWidgetBehavior()
{
  connect(ui->exitPushButton, SIGNAL(clicked()), this, SIGNAL(exitingSplit()));
}
