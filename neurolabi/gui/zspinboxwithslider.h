#ifndef ZSPINBOXWITHSLIDER_H
#define ZSPINBOXWITHSLIDER_H

#include <QWidget>
#include <QSlider>

class ZSpinBox;

class ZDoubleSpinBox;

class ZSliderEventFilter : public QObject
{
public:
  explicit ZSliderEventFilter(QObject* parent = nullptr);

protected:
  virtual bool eventFilter(QObject* obj, QEvent* event) override;
};

class ZSlider2 : public QSlider
{
Q_OBJECT
public:
  explicit ZSlider2(QWidget* parent = nullptr);

  explicit ZSlider2(Qt::Orientation ori, QWidget* parent = nullptr);

protected:
  virtual void focusInEvent(QFocusEvent* e) override;

  virtual void focusOutEvent(QFocusEvent* e) override;
};

class ZSpinBoxWithSlider : public QWidget
{
Q_OBJECT
public:
  explicit ZSpinBoxWithSlider(int value, int min, int max, int step = 1,
                              bool tracking = true, const QString& prefix = "",
                              const QString& suffix = "", QWidget* parent = nullptr);

  void setValue(int v);

  void setDataRange(int min, int max);

signals:

  void valueChanged(int);

protected:
  void createWidget(int value, int min, int max, int step, bool tracking, const QString& prefix,
                    const QString& suffix);

private:
  void valueChangedFromSlider(int v);

  void valueChangedFromSpinBox(int v);

  ZSlider2* m_slider;
  ZSpinBox* m_spinBox;
};

class ZDoubleSpinBoxWithSlider : public QWidget
{
Q_OBJECT
public:
  explicit ZDoubleSpinBoxWithSlider(double value, double min, double max, double step = .01,
                                    int decimal = 3, bool tracking = true, const QString& prefix = "",
                                    const QString& suffix = "", QWidget* parent = nullptr);

  void setValue(double v);

  void setDataRange(double min, double max);

signals:

  void valueChanged(double);

protected:
  void createWidget(const QString& prefix,
                    const QString& suffix);

private:
  void valueChangedFromSlider(int v);

  void valueChangedFromSpinBox(double v);

private:
  ZSlider2* m_slider;
  ZDoubleSpinBox* m_spinBox;
  double m_value;
  double m_min;
  double m_max;
  double m_step;
  int m_decimal;
  bool m_tracking;
  int m_sliderMaxValue;
};

#endif // ZSPINBOXWITHSLIDER_H
