#ifndef FLYEMSETTINGDIALOG_H
#define FLYEMSETTINGDIALOG_H

#include <QDialog>

namespace Ui {
class FlyEmSettingDialog;
}

class FlyEmSettingDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlyEmSettingDialog(QWidget *parent = 0);
  ~FlyEmSettingDialog();

  void loadSetting();

public slots:
  void update();
  void updateDefaultConfigChecked(bool on);

private:
  std::string getNeuTuServer() const;
  std::string getTaskServer() const;
  std::string getConfigPath() const;

  bool usingDefaultConfig() const;
  bool namingSynapse() const;

  void init();

  void connectSignalSlot();
  QString shrink(const QString &str, int len);

private:
  Ui::FlyEmSettingDialog *ui;
};

#endif // FLYEMSETTINGDIALOG_H
