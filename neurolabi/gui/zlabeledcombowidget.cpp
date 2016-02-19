#include "zlabeledcombowidget.h"

ZLabeledComboWidget::ZLabeledComboWidget(QWidget *parent) :
  QWidget(parent)
{
  m_layout = new QHBoxLayout(this);

  m_label = new QLabel(this);
  m_layout->addWidget(m_label);

  m_mainWidget = new QComboBox(this);
  m_layout->addWidget(m_mainWidget);
}

void ZLabeledComboWidget::addSpacer()
{
  QSpacerItem *item =
      new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

  m_layout->addItem(item);
}

void ZLabeledComboWidget::setLabel(const QString &label)
{
  m_label->setText(label);
}
