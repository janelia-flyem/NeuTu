#include "zlabeledspinboxwidget.h"
#include "zspinbox.h"

ZLabeledSpinBoxWidget::ZLabeledSpinBoxWidget(QWidget *parent) :
  QWidget(parent)
{
  m_layout = new QHBoxLayout(this);

  m_label = new QLabel(this);
  m_layout->addWidget(m_label);

  m_mainWidget = new ZSpinBox(this);
  m_layout->addWidget(m_mainWidget);
  m_mainWidget->setFocusPolicy(Qt::ClickFocus);

  connect(m_mainWidget, SIGNAL(valueConfirmed(int)),
          this, SIGNAL(valueConfirmed(int)));
  connect(m_mainWidget, SIGNAL(valueChanged(int)),
          this, SIGNAL(valueChanged(int)));
}

void ZLabeledSpinBoxWidget::setRange(int vmin, int vmax)
{
  m_mainWidget->setRange(vmin, vmax);
}

void ZLabeledSpinBoxWidget::setLabel(const QString &label)
{
  m_label->setText(label);
}

void ZLabeledSpinBoxWidget::addSpacer()
{
  QSpacerItem *item =
      new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

  m_layout->addItem(item);
}

int ZLabeledSpinBoxWidget::getSkipValue() const
{
  return m_skipValue;
}

int ZLabeledSpinBoxWidget::getValue() const
{
  return m_mainWidget->value();
}

void ZLabeledSpinBoxWidget::setValueQuietly(int v)
{
  if (m_mainWidget->value() != v) {
    m_mainWidget->blockSignals(true);
    m_mainWidget->setValue(v);
    m_mainWidget->blockSignals(false);
  }
}

void ZLabeledSpinBoxWidget::setRangeQuietly(int vmin, int vmax)
{
  if (m_mainWidget->minimum() != vmin || m_mainWidget->maximum() != vmax) {
    m_mainWidget->blockSignals(true);
    m_mainWidget->setRange(vmin, vmax);
    m_mainWidget->blockSignals(false);
  }
}

void ZLabeledSpinBoxWidget::setValue(int v)
{
  m_mainWidget->setValue(v);
}

void ZLabeledSpinBoxWidget::setSkipValue(int v)
{
  m_skipValue = v;
}
