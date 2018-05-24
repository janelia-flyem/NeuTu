#include "zspinboxwithscrollbar.h"

#include "zspinbox.h"
#include "zutils.h"
#include <QHBoxLayout>
#include <QScrollBar>
#include <QLabel>

ZSpinBoxWithScrollBar::ZSpinBoxWithScrollBar(int value, int min, int max, int step, bool tracking,
                                             const QString& prefix, const QString& suffix, QWidget* parent)
  : QWidget(parent)
{
  createWidget(value, min, max, step, tracking, prefix, suffix);
}

void ZSpinBoxWithScrollBar::setValue(int v)
{
  m_scrollBar->setValue(v);
  m_spinBox->setValue(v);
}

void ZSpinBoxWithScrollBar::valueChangedFromScrollBar(int v)
{
  m_spinBox->blockSignals(true);
  m_spinBox->setValue(v);
  m_spinBox->blockSignals(false);
  emit valueChanged(v);
}

void ZSpinBoxWithScrollBar::valueChangedFromSpinBox(int v)
{
  m_scrollBar->blockSignals(true);
  m_scrollBar->setValue(v);
  m_scrollBar->blockSignals(false);
  emit valueChanged(v);
}

void ZSpinBoxWithScrollBar::setDataRange(int min, int max)
{
  m_label->setText(QString("/ (%1 to %2)").arg(min).arg(max));
  m_scrollBar->setRange(min, max);
  m_spinBox->setRange(min, max);
}

void ZSpinBoxWithScrollBar::createWidget(int value, int min, int max, int step, bool tracking, const QString& prefix,
                                         const QString& suffix)
{
  m_scrollBar = new QScrollBar(Qt::Horizontal, this);
  m_scrollBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  m_scrollBar->setFocusPolicy(Qt::NoFocus);
  m_scrollBar->setRange(min, max);
  m_scrollBar->setValue(value);
  m_scrollBar->setSingleStep(step);
  m_scrollBar->setTracking(tracking);
  m_spinBox = new ZSpinBox();
  m_spinBox->setRange(min, max);
  m_spinBox->setValue(value);
  m_spinBox->setSingleStep(step);
  m_spinBox->setPrefix(prefix);
  m_spinBox->setSuffix(suffix);
  QHBoxLayout* lo = new QHBoxLayout(this);
  lo->setContentsMargins(0, 0, 0, 0);
  lo->addWidget(m_spinBox);
  m_label = new QLabel(QString("/ (%1 to %2)").arg(min).arg(max));
  m_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
  lo->addWidget(m_label);
  lo->addWidget(m_scrollBar);
  connect(m_scrollBar, &QScrollBar::valueChanged, this, &ZSpinBoxWithScrollBar::valueChangedFromScrollBar);
#if __cplusplus > 201103L
  connect(m_spinBox, qOverload<int>(&ZSpinBox::valueChanged), this, &ZSpinBoxWithScrollBar::valueChangedFromSpinBox);
#else
  connect(m_spinBox, QOverload<int>::of(&ZSpinBox::valueChanged), this, &ZSpinBoxWithScrollBar::valueChangedFromSpinBox);
#endif
}
