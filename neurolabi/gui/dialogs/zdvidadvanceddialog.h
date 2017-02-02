#ifndef ZDVIDADVANCEDDIALOG_H
#define ZDVIDADVANCEDDIALOG_H

#include <QDialog>
#include <string>

namespace Ui {
class ZDvidAdvancedDialog;
}

class ZJsonObject;
class QLabel;
class QLineEdit;

class ZDvidAdvancedDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZDvidAdvancedDialog(QWidget *parent = 0);
  ~ZDvidAdvancedDialog();

  void setDvidServer(const QString &str);
  void updateWidgetForEdit(bool editable);
  void updateWidgetForDefaultSetting(const ZJsonObject &obj);

  void setTodoName(const std::string &name);
  std::string getTodoName() const;
  bool isSupervised() const;
  std::string getSupervisorServer() const;
  void setSupervised(bool supervised);
  void setSupervisorServer(const std::string &server);

  void backup();
  void recover();

  static void UpdateWidget(QLabel *label, QLineEdit *lineEdit,
                    const QString &labelText, const QString &dataText);
  static void UpdateWidget(QLabel *label, QLineEdit *lineEdit,
                    const QString &labelText, const ZJsonObject &obj,
                    const char *key);

private:
  Ui::ZDvidAdvancedDialog *ui;

  bool m_oldSupervised;
  std::string m_oldSupervisorServer;
  std::string m_oldTodoName;
};

#endif // ZDVIDADVANCEDDIALOG_H
