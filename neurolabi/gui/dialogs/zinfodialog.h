#ifndef ZINFODIALOG_H
#define ZINFODIALOG_H

#include <QDialog>

namespace Ui {
class ZInfoDialog;
}

class ZInfoDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZInfoDialog(QWidget *parent = 0);
  ~ZInfoDialog();

  void setText(const QString &text);

private:
  Ui::ZInfoDialog *ui;
};

#endif // ZINFODIALOG_H
