#ifndef ZAUTOTRACEDIALOG_H
#define ZAUTOTRACEDIALOG_H

#include <QDialog>

namespace Ui {
class ZAutoTraceDialog;
}

class ZAutoTraceDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZAutoTraceDialog(QWidget *parent = 0);
  ~ZAutoTraceDialog();

  bool resampling() const;
  int getTraceLevel() const;

  void setChannelCount(int count);
  int getChannel() const;

  bool diagnosis() const;
  bool overTracing() const;
  bool screenSeed() const;

private:
  bool usingDefaultLevel() const;

private slots:
  void updateWidget();

private:
  Ui::ZAutoTraceDialog *ui;

};

#endif // ZAUTOTRACEDIALOG_H
