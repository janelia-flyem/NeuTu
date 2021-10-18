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
  void updateDefaultNeuTuServerChecked(bool on);
  void updateDefaultTaskServerChecked(bool on);

private slots:
  void onConfigPushButton();

private:
  std::string getNeuTuServer() const;
  std::string getTaskServer() const;
  std::string getConfigPath() const;

  bool usingDefaultConfig() const;
  bool usingDefaultService() const;
  bool usingDefaultTaskServer() const;
  bool namingSynapse() const;
  bool namingPsd() const;

  void init();
  void updateNeutuseWidget();
  void updateCleavingServerWidget();

  void connectSignalSlot();
  static QString Shrink(const QString &str, int len);

private:
  Ui::FlyEmSettingDialog *ui;
};

#endif // FLYEMSETTINGDIALOG_H
