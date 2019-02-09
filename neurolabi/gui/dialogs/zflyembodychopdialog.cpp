#include "zflyembodychopdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

#include "widgets/zaxiswidget.h"
#include "zwidgetfactory.h"
#include "zbuttonbox.h"

ZFlyEmBodyChopDialog::ZFlyEmBodyChopDialog(QWidget *parent) :
  QDialog(parent)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  m_axisWidget = new ZAxisWidget(this);
  m_axisWidget->setToolTip("Select the axis along which to chop");
  layout->addWidget(m_axisWidget);

  setWindowTitle("Chopping Options");

  QHBoxLayout *buttonLayout = new QHBoxLayout;
  buttonLayout->addSpacerItem(ZWidgetFactory::MakeHSpacerItem());

  m_buttonBox = ZWidgetFactory::makeButtonBox(
        ZButtonBox::ROLE_YES | ZButtonBox::ROLE_NO,
        this);
  buttonLayout->addWidget(m_buttonBox);

  layout->addLayout(buttonLayout);
}

neutu::EAxis ZFlyEmBodyChopDialog::getAxis() const
{
  return m_axisWidget->getAxis();
}


