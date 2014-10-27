#ifndef ZLABELEDCOMBOWIDGET_H
#define ZLABELEDCOMBOWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>

class ZLabeledComboWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ZLabeledComboWidget(QWidget *parent = 0);

  void addSpacer();
  void setLabel(const QString &label);

  inline QComboBox* getComboBox() {
    return m_mainWidget;
  }

signals:

public slots:

private:
  QLabel *m_label;
  QComboBox *m_mainWidget;
  QHBoxLayout *m_layout;
};

#endif // ZLABELEDCOMBOWIDGET_H
