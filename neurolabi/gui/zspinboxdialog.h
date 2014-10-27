#ifndef ZSPINBOXDIALOG_H
#define ZSPINBOXDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QSpinBox>

class ZSpinBoxDialog : public QDialog
{
  Q_OBJECT
public:
  explicit ZSpinBoxDialog(QWidget *parent = 0);

  int getValue() const;

signals:

public slots:

private:
  QLabel *m_label;
  QSpinBox *m_spinBox;
};

#endif // ZSPINBOXDIALOG_H
