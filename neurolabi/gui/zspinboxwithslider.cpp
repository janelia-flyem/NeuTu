#include "zspinboxwithslider.h"

#include "zspinbox.h"
#include "QsLog.h"
#include "zutils.h"
#include <QHBoxLayout>
#include <QEvent>
#include <limits>

ZSliderEventFilter::ZSliderEventFilter(QObject* parent)
  : QObject(parent)
{
}

bool ZSliderEventFilter::eventFilter(QObject* obj, QEvent* event)
{
  if (event->type() == QEvent::Wheel) {
    if (auto qas = qobject_cast<QAbstractSlider*>(obj)) {
      if (qas->focusPolicy() == Qt::WheelFocus) {
        event->accept();
        return false;
      }
      event->ignore();
      return true;
    }
  }
  return QObject::eventFilter(obj, event);
}

ZSlider2::ZSlider2(QWidget* parent)
  : QSlider(Qt::Horizontal, parent)
{
  installEventFilter(new ZSliderEventFilter(this));
  setFocusPolicy(Qt::StrongFocus);
}

ZSlider2::ZSlider2(Qt::Orientation ori, QWidget* parent)
  : QSlider(ori, parent)
{
}

void ZSlider2::focusInEvent(QFocusEvent* e)
{
  QSlider::focusInEvent(e);
  setFocusPolicy(Qt::WheelFocus);
}

void ZSlider2::focusOutEvent(QFocusEvent* e)
{
  QSlider::focusOutEvent(e);
  setFocusPolicy(Qt::StrongFocus);
}

ZSpinBoxWithSlider::ZSpinBoxWithSlider(int value, int min, int max, int step, bool tracking,
                                       const QString& prefix, const QString& suffix, QWidget* parent)
  : QWidget(parent)
{
  createWidget(value, min, max, step, tracking, prefix, suffix);
}

void ZSpinBoxWithSlider::setValue(int v)
{
  m_slider->setValue(v);
  m_spinBox->setValue(v);
}

void ZSpinBoxWithSlider::valueChangedFromSlider(int v)
{
  m_spinBox->blockSignals(true);
  m_spinBox->setValue(v);
  m_spinBox->blockSignals(false);
  emit valueChanged(v);
}

void ZSpinBoxWithSlider::valueChangedFromSpinBox(int v)
{
  m_slider->blockSignals(true);
  m_slider->setValue(v);
  m_slider->blockSignals(false);
  emit valueChanged(v);
}

void ZSpinBoxWithSlider::setDataRange(int min, int max)
{
  m_slider->setRange(min, max);
  m_spinBox->setRange(min, max);
}

void ZSpinBoxWithSlider::createWidget(int value, int min, int max, int step, bool tracking, const QString& prefix,
                                      const QString& suffix)
{
  m_slider = new ZSlider2();
  m_slider->setRange(min, max);
  m_slider->setValue(value);
  m_slider->setSingleStep(step);
  m_slider->setTracking(tracking);
//  m_slider->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
  m_spinBox = new ZSpinBox();
  m_spinBox->setRange(min, max);
  m_spinBox->setValue(value);
  m_spinBox->setSingleStep(step);
  m_spinBox->setPrefix(prefix);
  m_spinBox->setSuffix(suffix);
  QHBoxLayout* lo = new QHBoxLayout(this);
  lo->setContentsMargins(0, 0, 0, 0);
  lo->addWidget(m_spinBox);
  lo->addWidget(m_slider);
  connect(m_slider, &ZSlider2::valueChanged, this, &ZSpinBoxWithSlider::valueChangedFromSlider);
#if __cplusplus > 201103L
  connect(m_spinBox, qOverload<int>(&ZSpinBox::valueChanged), this, &ZSpinBoxWithSlider::valueChangedFromSpinBox);
#else
  connect(m_spinBox, QOverload<int>::of(&ZSpinBox::valueChanged), this, &ZSpinBoxWithSlider::valueChangedFromSpinBox);
#endif

#ifdef _DEBUG_0
  qDebug() << "slider size: " << m_slider->sizeHint() << m_slider->size();
#endif
}

ZDoubleSpinBoxWithSlider::ZDoubleSpinBoxWithSlider(double value, double min, double max, double step,
                                                   int decimal, bool tracking, const QString& prefix,
                                                   const QString& suffix, QWidget* parent)
  : QWidget(parent), m_value(value), m_min(min), m_max(max), m_step(step), m_decimal(decimal), m_tracking(tracking)
{
  double sliderMaxValue = (m_max - m_min) / m_step;
  if (sliderMaxValue > std::numeric_limits<int>::max())
    m_sliderMaxValue = std::numeric_limits<int>::max();
  else
    m_sliderMaxValue = static_cast<int>(sliderMaxValue);
  createWidget(prefix, suffix);
}

void ZDoubleSpinBoxWithSlider::setValue(double v)
{
  m_spinBox->setValue(v);
}

void ZDoubleSpinBoxWithSlider::valueChangedFromSlider(int v)
{
  m_value = static_cast<double>(v) / m_sliderMaxValue * (m_max - m_min) + m_min;
  m_spinBox->blockSignals(true);
  m_spinBox->setValue(m_value);
  m_spinBox->blockSignals(false);
  emit valueChanged(m_value);
}

void ZDoubleSpinBoxWithSlider::valueChangedFromSpinBox(double v)
{
  m_value = v;
  int sliderPos = static_cast<int>((m_value - m_min) / (m_max - m_min) * m_sliderMaxValue);
  m_slider->blockSignals(true);
  m_slider->setValue(sliderPos);
  m_slider->blockSignals(false);
  emit valueChanged(m_value);
}

void ZDoubleSpinBoxWithSlider::setDataRange(double min, double max)
{
  m_min = min;
  m_max = max;
  double sliderMaxValue = (m_max - m_min) / m_step;
  if (sliderMaxValue > std::numeric_limits<int>::max())
    m_sliderMaxValue = std::numeric_limits<int>::max();
  else
    m_sliderMaxValue = static_cast<int>(sliderMaxValue);
  m_slider->setRange(0, m_sliderMaxValue);
  m_spinBox->setRange(m_min, m_max);
}

void ZDoubleSpinBoxWithSlider::createWidget(const QString& prefix,
                                            const QString& suffix)
{
  m_slider = new ZSlider2();
  m_slider->setRange(0, m_sliderMaxValue);
  m_slider->setValue(static_cast<int>((m_value - m_min) / (m_max - m_min) * m_sliderMaxValue));
  m_slider->setSingleStep(std::max(1, static_cast<int>(m_step * m_sliderMaxValue / (m_max - m_min))));
  m_slider->setTracking(m_tracking);
  m_spinBox = new ZDoubleSpinBox();
  m_spinBox->setRange(m_min, m_max);
  m_spinBox->setValue(m_value);
  m_spinBox->setSingleStep(m_step);
  m_spinBox->setDecimals(m_decimal);
  m_spinBox->setPrefix(prefix);
  m_spinBox->setSuffix(suffix);
  QHBoxLayout* lo = new QHBoxLayout(this);
  lo->setContentsMargins(0, 0, 0, 0);
  lo->addWidget(m_spinBox);
  lo->addWidget(m_slider);
  connect(m_slider, &ZSlider2::valueChanged, this, &ZDoubleSpinBoxWithSlider::valueChangedFromSlider);
#if __cplusplus > 201103L
  connect(m_spinBox, qOverload<double>(&ZDoubleSpinBox::valueChanged), this,
          &ZDoubleSpinBoxWithSlider::valueChangedFromSpinBox);
#else
  connect(m_spinBox, QOverload<double>::of(&ZDoubleSpinBox::valueChanged), this,
          &ZDoubleSpinBoxWithSlider::valueChangedFromSpinBox);
#endif
}
