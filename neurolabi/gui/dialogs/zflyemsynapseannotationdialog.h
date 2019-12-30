#ifndef ZFLYEMSYNAPSEANNOTATIONDIALOG_H
#define ZFLYEMSYNAPSEANNOTATIONDIALOG_H

#include <QDialog>

#include "dvid/zdvidannotation.h"

class ZDvidSynapse;

namespace Ui {
class ZFlyEmSynapseAnnotationDialog;
}

class ZJsonObject;

class ZFlyEmSynapseAnnotationDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZFlyEmSynapseAnnotationDialog(QWidget *parent = 0);
  ~ZFlyEmSynapseAnnotationDialog();

  bool hasConfidence() const;
//  double getConfidence() const;
  std::string getConfidenceStr() const;

  void setConfidence(std::string c);

  ZJsonObject getPropJson() const;

  QString getAnnotation() const;

  void setAnnotation(const QString &annotation);

  enum EOption {
    OPTION_TBAR, OPTION_PSD
  };

  void setOption(ZDvidAnnotation::EKind kind);

  void set(const ZDvidSynapse &synapse);

protected slots:
  void updateAnnotationWidget();

protected:
  void paintEvent(QPaintEvent *);

private:
  int getConfidenceIndex(const std::string &cstr) const;

private:
  Ui::ZFlyEmSynapseAnnotationDialog *ui;

//  double m_confidence = 1.0;
  std::string m_confidenceStr;
//  bool m_hasConfidence = false;
};

#endif // ZFLYEMSYNAPSEANNOTATIONDIALOG_H
