#ifndef ZLABELEDEDITWIDGET_H
#define ZLABELEDEDITWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>

class ZLabeledEditWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ZLabeledEditWidget(QWidget *parent = 0);

  void addSpacer();
  void setLabel(const QString &label);

  inline QLineEdit* getEditWidget() {
    return m_mainWidget;
  }

signals:

public slots:

private:
  QLabel *m_label;
  QLineEdit *m_mainWidget;
  QHBoxLayout *m_layout;
};

#endif // ZLABELEDEDITWIDGET_H
