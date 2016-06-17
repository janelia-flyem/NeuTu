#ifndef ZCONTRASTPROTOCALDIALOG_H
#define ZCONTRASTPROTOCALDIALOG_H

#include <QDialog>

class ZJsonObject;

namespace Ui {
class ZContrastProtocalDialog;
}

class ZContrastProtocalDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZContrastProtocalDialog(QWidget *parent = 0);
  ~ZContrastProtocalDialog();

  ZJsonObject getContrastProtocal() const;

private:
  Ui::ZContrastProtocalDialog *ui;
};

#endif // ZCONTRASTPROTOCALDIALOG_H
