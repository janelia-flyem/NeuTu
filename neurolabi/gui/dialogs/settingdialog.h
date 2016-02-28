/**@file settingdialog.h
 * @brief Overall setting dialog
 * @author Ting Zhao
 */
#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>

#include "ui_settingdialog.h"
#include "neutube.h"

class ZResolution;
class ZNeuronTracerConfig;

class SettingDialog : public QDialog, public Ui_SettingDialog
{
  Q_OBJECT

public:
  SettingDialog(QWidget *parent = 0);

public:
  double xResolution();
  double yResolution();
  double zResolution();
  double xScale();
  double zScale();
  char unit();
  int traceEffort();
//  double traceMinScore();
  double autoTraceMinScore() const;
  double manualTraceMinScore() const;
  int receptor();
  bool useCone();
  inline bool traceMasked() { return m_traceMasked; }
  inline int rootOption() { return m_rootOption; }
  inline double distThre() { return m_distThre; }
  inline bool crossoverTest() { return m_crossoverTest; }
  inline bool singleTree() { return m_singleTree; }
  inline bool removeOvershoot() { return m_removeOvershoot; }
  inline int reconstructEffort() { return m_reconstructEffort; }

  /*!
   * \brief Get backgound setting.
   */
  NeuTube::EImageBackground getBackground() const;

  void setBackground(NeuTube::EImageBackground bg);

public:
  void setResolution(const ZResolution &res);
  void setResolution(double x, double y, double z, int unit = 1);
  void setUnit(char unit);
  void setTracingParameter(const ZNeuronTracerConfig &traceConfig);
  void setTracingParameter();

public slots:
  virtual void reject();
  virtual void accept();

private:
  void resetWidgetValue();
  void update();
  void updateOverview();

private:
  double m_resolution[3];
  double m_scales[3];
  int m_traceEffort;
  bool m_traceMasked;
  double m_autoTraceMinScore;
  double m_manualTraceMinScore;
  int m_receptor;
  bool m_useCone;
  int m_unit;
  double m_distThre;
  bool m_crossoverTest;
  bool m_singleTree;
  bool m_removeOvershoot;
  int m_rootOption;
  int m_reconstructEffort;
};

#endif
