#ifndef ZAUTOTRACEDIALOG_H
#define ZAUTOTRACEDIALOG_H

#include <QDialog>

class QCheckBox;
class ZLabeledSpinBoxWidget;
class ZLabeledComboWidget;

class ZAutoTraceDialog : public QDialog
{
  Q_OBJECT
public:
  explicit ZAutoTraceDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
  ~ZAutoTraceDialog();

  bool getDoResample() const;

  int getTraceLevel() const;

  void setChannelNumber(int count);
  int getChannel() const;

signals:

public slots:

private:
  QCheckBox *m_resampleCheckbox;
  ZLabeledSpinBoxWidget *m_levelSpinBox;
  ZLabeledComboWidget *m_channelWidget;
};

#endif // ZAUTOTRACEDIALOG_H
