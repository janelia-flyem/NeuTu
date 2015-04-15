#include "flyemproofcontrolform.h"
#include "ui_flyemproofcontrolform.h"

FlyEmProofControlForm::FlyEmProofControlForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::FlyEmProofControlForm)
{
  ui->setupUi(this);
  setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);

  connect(ui->segmentCheckBox, SIGNAL(clicked(bool)),
          this, SIGNAL(segmentVisibleChanged(bool)));
  connect(ui->mergeSegmentPushButton, SIGNAL(clicked()),
          this, SIGNAL(mergingSelected()));
}

FlyEmProofControlForm::~FlyEmProofControlForm()
{
  delete ui;
}
