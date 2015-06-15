#ifndef ZSPINBOXDIALOG_H
#define ZSPINBOXDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include "zbuttonbox.h"

class ZSpinBoxDialog : public QDialog
{
  Q_OBJECT
public:
  explicit ZSpinBoxDialog(QWidget *parent = 0);

  int getValue() const;
  void setValue(int v);
  void setValueLabel(const QString &label);
  QPushButton* getButton(ZButtonBox::TRole role);

  bool isSkipped() const { return m_isSkipped; }

signals:

public slots:
  void skip();
  void accept();

private:
  bool m_isSkipped;
  int m_skippedValue;
  QLabel *m_label;
  QSpinBox *m_spinBox;
  ZButtonBox *m_buttonBox;
};

#endif // ZSPINBOXDIALOG_H
