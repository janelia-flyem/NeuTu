#ifndef SHAPEPAPERDIALOG_H
#define SHAPEPAPERDIALOG_H

#include <QDialog>
#include <QString>
#include <QPointer>
#include <QVector>
#include <map>
#include "zparameter.h"
#include "zstringparameter.h"

class ZFlyEmDataFrame;
class MainWindow;
class ZFlyEmNeuronArray;
class ZFlyEmDataBundle;

namespace Ui {
class ShapePaperDialog;
}

class ShapePaperDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ShapePaperDialog(QWidget *parent = 0);
  ~ShapePaperDialog();

  enum EResult {
    RESULT_SIMMAT, RESULT_FEATMAT, RESULT_TRUE_CLASS_LABEL,
    RESULT_PRED_CLASS_LABEL, RESULT_DENDROGRAM,
    RESULT_CONFMAT, RESULT_LAYER_FEATMAT, RESULT_MODEL_SOURCE,
    RESULT_CLUSTERING, RESULT_NEURON_INFO
  };

  QString getSparseObjectDir() const;
  QString getObjectStackDir() const;
  QString getDataBundlePath() const;
  QString getSimmatPath() const;
  QString getConfmatPath() const;
  QString getDendrogramPath() const;
  QString getFeaturePath() const;
  QString getTrueClassLabelPath() const;
  QString getPredicatedClassLabelPath() const;
  QString getModelSourcePath() const;

  QString getPath(EResult result) const;
  bool exists(EResult result) const;
  void tryOutput(EResult result);

  MainWindow* getMainWindow();

  void dump(const QString &str, bool appending = false);

  void updateButtonState();

private:
  double computeRatioDiff(double x, double y, double mu1, double var1,
                          double mu2, double var2);
  void predictFromOrtAdjustment();
  void computeFeatureMatrix();
  void exportClassLabel();
  void computeConfusionMatrix();
  void computeLayerFeature();
  ZFlyEmNeuronArray* getNeuronArray();
  ZFlyEmDataBundle* getDataBundle();
  std::map<std::string, int> getClassIdMap();
  void exportModelSource();
  void exportNeuronInfo();

private slots:
  void on_configPushButton_clicked();

  void on_sparseObjectPushButton_clicked();

  void on_dataBundlePushButton_clicked();

  void detachFrame();

  void on_simmatPushButton_clicked();

  void on_dendrogramPushButton_clicked();

  void on_predictPushButton_clicked();

  void on_featurePushButton_clicked();

  void on_allResultPushButton_clicked();

  void on_clusteringPushButton_clicked();

private:
  Ui::ShapePaperDialog *ui;

  ZFlyEmDataFrame *m_frame;

  ZStringParameter *m_objectStackDir;
  ZStringParameter *m_sparseObjectDir;
  ZStringParameter *m_resultDir;
  ZStringParameter *m_bundleDir;
};

#endif // SHAPEPAPERDIALOG_H
