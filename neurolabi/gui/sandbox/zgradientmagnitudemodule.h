#ifndef ZGRADIENTMAGNITUDEMODULE_H
#define ZGRADIENTMAGNITUDEMODULE_H
#include <map>
#include <string>
#include <QWidget>
#include <QString>
#include <QMessageBox>
#include "zsandboxmodule.h"
#include "mvc/zstackdoc.h"
#include "imgproc/zstackgradient.h"


class QComboBox;
class QCheckBox;
class QPushButton;
class QDoubleSpinBox;

class ZSelectGradientStrategyWindow:public QWidget
{
  Q_OBJECT
public:
  ZSelectGradientStrategyWindow(QWidget *parent = 0);
private slots:
  void onStart();
  void onReset();
  void onUseSameSigmaChanged(int);
private:
  QPushButton* start_gradient_magnitude;
  QComboBox*   strategies;
  QCheckBox*   reverse;
  QCheckBox*   ignore_background;
  QDoubleSpinBox*     gaussin_sigma_x;
  QDoubleSpinBox*     gaussin_sigma_y;
  QDoubleSpinBox*     gaussin_sigma_z;
  QCheckBox*          gaussin_use_same_sigma;
  QDoubleSpinBox*     edge_enhance;
  std::map<QString,GradientStrategyContext::StrategyType>strategy_map;
};



class ZGradientMagnitudeModule : public ZSandboxModule
{
  Q_OBJECT
public:
  explicit ZGradientMagnitudeModule(QObject *parent = 0);
  ~ZGradientMagnitudeModule();
signals:

public slots:

private slots:
  void execute();
private:
  void init();

private:
  ZSelectGradientStrategyWindow* select_strategy_window;
};

#endif // ZGRADIENTMAGNITUDEMODULE_H
