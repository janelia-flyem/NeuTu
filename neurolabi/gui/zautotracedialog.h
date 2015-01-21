#ifndef ZAUTOTRACEDIALOG_H
#define ZAUTOTRACEDIALOG_H

#include <QDialog>

class QCheckBox;

class ZAutoTraceDialog : public QDialog
{
  Q_OBJECT
public:
  explicit ZAutoTraceDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
  ~ZAutoTraceDialog();

  bool getDoResample() const;

signals:

public slots:

private:
  QCheckBox *m_resampleCheckbox;
};

#endif // ZAUTOTRACEDIALOG_H
