#ifndef ZDVIDDIALOG_H
#define ZDVIDDIALOG_H

#include <QDialog>
#include <string>

namespace Ui {
class ZDvidDialog;
}

class ZDvidDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZDvidDialog(QWidget *parent = 0);
  ~ZDvidDialog();

  int getPort() const;
  QString getAddress() const;
  QString getUuid() const;

public slots:
  void setServer(int index);

private:
  Ui::ZDvidDialog *ui;
  std::string m_customString;
};

#endif // ZDVIDDIALOG_H
