#ifndef ZFLYEMSYNAPSEANNOTATIONDIALOG_H
#define ZFLYEMSYNAPSEANNOTATIONDIALOG_H

#include <QDialog>

#include "dvid/zdvidannotation.h"

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

  double getConfidence() const;

  void setConfidence(double c);

  ZJsonObject getPropJson() const;

  QString getAnnotation() const;

  void setAnnotation(const QString &annotation);

  enum EOption {
    OPTION_TBAR, OPTION_PSD
  };

  void setOption(ZDvidAnnotation::EKind kind);

protected slots:
  void updateAnnotationWidget();

protected:
  void paintEvent(QPaintEvent *);

private:
  int getConfidenceIndex(double c) const;

private:
  Ui::ZFlyEmSynapseAnnotationDialog *ui;

  double m_confidence;
};

#endif // ZFLYEMSYNAPSEANNOTATIONDIALOG_H
