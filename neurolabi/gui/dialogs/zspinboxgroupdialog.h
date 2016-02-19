#ifndef ZSPINBOXGROUPDIALOG_H
#define ZSPINBOXGROUPDIALOG_H

#include <QDialog>
#include <QMap>
#include <QVBoxLayout>
#include "zlabeledspinboxwidget.h"

class ZSpinBoxGroupDialog : public QDialog
{
  Q_OBJECT
public:
  explicit ZSpinBoxGroupDialog(QWidget *parent = 0);

  int getValue(const QString &name) const;
  bool addSpinBox(const QString &name, int vmin, int vmax, int defaultValue,
                  int skipValue);

signals:

public slots:
  void skip();

private:
  QVBoxLayout *m_mainLayout;
  QVBoxLayout *m_spinBoxLayout;
  QMap<QString, ZLabeledSpinBoxWidget*> m_spinBoxMap;
  int m_isSkipped;
};

#endif // ZSPINBOXGROUPDIALOG_H
