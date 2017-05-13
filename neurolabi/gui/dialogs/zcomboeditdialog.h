#ifndef ZCOMBOEDITDIALOG_H
#define ZCOMBOEDITDIALOG_H

#include <QDialog>

namespace Ui {
class ZComboEditDialog;
}

class ZComboEditDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZComboEditDialog(QWidget *parent = 0);
  ~ZComboEditDialog();

  QString getText() const;

  void setStringList(const QStringList &stringList);
  void setStringList(const std::vector<std::string> &stringList);
  void setCurrentIndex(int index);

private:
  Ui::ZComboEditDialog *ui;
};

#endif // ZCOMBOEDITDIALOG_H
