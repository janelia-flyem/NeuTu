#ifndef ZLABELEDSPINBOXWIDGET_H
#define ZLABELEDSPINBOXWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QSpinBox>

class ZSpinBox;

class ZLabeledSpinBoxWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ZLabeledSpinBoxWidget(QWidget *parent = 0);

  void addSpacer();
  void setLabel(const QString &label);
  void setRange(int vmin, int vmax);
  void setValue(int v);
  void setSkipValue(int v);


  inline ZSpinBox* getEditWidget() {
    return m_mainWidget;
  }

  int getValue() const;
  int getSkipValue() const;

signals:
  void valueConfirmed(int);

public slots:

private:
  QLabel *m_label;
  ZSpinBox *m_mainWidget;
  QHBoxLayout *m_layout;
  int m_skipValue;
};

#endif // ZLABELEDSPINBOXWIDGET_H
