#ifndef DVIDOBJECTDIALOG_H
#define DVIDOBJECTDIALOG_H

#include <QDialog>
#include <vector>

namespace Ui {
class DvidObjectDialog;
}

class DvidObjectDialog : public QDialog
{
  Q_OBJECT

public:
  explicit DvidObjectDialog(QWidget *parent = 0);
  ~DvidObjectDialog();

  void setAddress(const QString &address);
  QString getAddress() const;
  std::vector<int> getBodyId() const;

  bool retrievingObject() const;
  bool retrievingSkeleton() const;

private:
  Ui::DvidObjectDialog *ui;
};

#endif // DVIDOBJECTDIALOG_H
