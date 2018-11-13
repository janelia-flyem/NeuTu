#ifndef INFORMATIONDIALOG_H
#define INFORMATIONDIALOG_H

#include <QDialog>

#include <string>

namespace Ui {
class InformationDialog;
}

class InformationDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit InformationDialog(QWidget *parent = 0);
  ~InformationDialog();

  void setText(const std::string &text);
  void setText(const QString &text);
  void setText(const char *text);
  
private:
  Ui::InformationDialog *ui;
};

#endif // INFORMATIONDIALOG_H
