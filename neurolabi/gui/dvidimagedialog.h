#ifndef DVIDIMAGEDIALOG_H
#define DVIDIMAGEDIALOG_H

#include <QDialog>

namespace Ui {
class DvidImageDialog;
}

class DvidImageDialog : public QDialog
{
  Q_OBJECT

public:
  explicit DvidImageDialog(QWidget *parent = 0);
  ~DvidImageDialog();

  int getX() const;
  int getY() const;
  int getZ() const;
  int getWidth() const;
  int getHeight() const;
  int getDepth() const;
  QString getAddress() const;
  void setAddress(const QString address);

private:
  Ui::DvidImageDialog *ui;
};

#endif // DVIDIMAGEDIALOG_H
