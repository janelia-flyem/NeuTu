#include "zdvidserverwidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

ZDvidServerWidget::ZDvidServerWidget(QWidget *parent) :
  QWidget(parent)
{
  createWidgets();
}

void ZDvidServerWidget::createWidgets()
{
  QVBoxLayout *topLayout = new QVBoxLayout(this);

  m_addressWidget = ZWidgetFactory::MakeLabledEditWidget(
        "Address", ZWidgetFactory::SPACER_NONE, this);
  topLayout->addWidget(m_addressWidget);

  m_portWidget = ZWidgetFactory::MakeLabledEditWidget(
        "Port", ZWidgetFactory::SPACER_RIGHT, this);
  topLayout->addWidget(m_portWidget);
}
