#ifndef ZSTACKFRAMESETTINGDIALOG_H
#define ZSTACKFRAMESETTINGDIALOG_H

#include <QDialog>
#include "common/neutube_def.h"

namespace Ui {
class ZStackFrameSettingDialog;
}

class ZNeuronTracerConfig;

class ZStackFrameSettingDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZStackFrameSettingDialog(QWidget *parent = 0);
  ~ZStackFrameSettingDialog();

  void updateTracingConfig(ZNeuronTracerConfig *config);
  void setFromTracingConfig(const ZNeuronTracerConfig &config);

  void setMinAutoScore(double score);
  void setMinManualScore(double score);
  void setMinSeedScore(double score);
  void setRefit(bool on);
  void setSpTest(bool on);
  void setCrossoverTest(bool on);
  void setEnhancingMask(bool on);
  void setRecoverLevel(int level);
  void setMaxEucDist(double d);
  void setScale(double x, double y, double z);
  void setBackground(neutu::EImageBackground bg);

  double getMinAutoScore() const;
  double getMinManualScore() const;
  double getMinSeedScore() const;
  bool getRefit() const;
  bool getSpTest() const;
  bool getEnhancingMask() const;
  int getRecoverLevel() const;
  double getMaxEucDist() const;
  double getXScale() const;
  double getYScale() const;
  double getZScale() const;
  bool getCrossoverTest() const;

  /*!
   * \brief Get backgound setting.
   */
  neutu::EImageBackground getBackground() const;

private:
  Ui::ZStackFrameSettingDialog *ui;
};

#endif // ZSTACKFRAMESETTINGDIALOG_H
