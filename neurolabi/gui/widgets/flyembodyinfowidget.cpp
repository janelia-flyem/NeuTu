#include "flyembodyinfowidget.h"

#include <QHBoxLayout>

#include "ui_flyembodyinfowidget.h"
#include "dialogs/flyembodyinfodialog.h"

FlyEmBodyInfoWidget::FlyEmBodyInfoWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::FlyEmBodyInfoWidget)
{
  ui->setupUi(this);

  m_mainWidget = new FlyEmBodyInfoDialog(
        FlyEmBodyInfoDialog::EMode::SEQUENCER, this);
  m_mainWidget->setSizePolicy(
        QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
//  setWidget(m_mainWidget);

  QHBoxLayout *layout = new QHBoxLayout(this);
  setLayout(layout);
  layout->addWidget(m_mainWidget);
}

FlyEmBodyInfoWidget::~FlyEmBodyInfoWidget()
{
  delete ui;
}
