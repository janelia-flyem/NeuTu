#ifndef ZDVIDADVANCEDDIALOG_H
#define ZDVIDADVANCEDDIALOG_H

#include <QDialog>
#include <string>

namespace Ui {
class ZDvidAdvancedDialog;
}

class ZDvidAdvancedDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZDvidAdvancedDialog(QWidget *parent = 0);
  ~ZDvidAdvancedDialog();

  void setDvidServer(const QString &str);
  void updateWidgetForEdit(bool editable);
  void updateWidgetForDefaultSetting(bool usingDefault);

  void setTodoName(const std::string &name);
  std::string getTodoName() const;
  bool isSupervised() const;
  std::string getSupervisorServer() const;
  void setSupervised(bool supervised);
  void setSupervisorServer(const std::string &server);

private:
  Ui::ZDvidAdvancedDialog *ui;
};

#endif // ZDVIDADVANCEDDIALOG_H
