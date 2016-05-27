#ifndef ZFLYEMSYNAPSEANNOTATIONDIALOG_H
#define ZFLYEMSYNAPSEANNOTATIONDIALOG_H

#include <QDialog>

namespace Ui {
class ZFlyEmSynapseAnnotationDialog;
}

class ZFlyEmSynapseAnnotationDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZFlyEmSynapseAnnotationDialog(QWidget *parent = 0);
  ~ZFlyEmSynapseAnnotationDialog();

  double getConfidence() const;

  void setConfidence(double c);

private:
  Ui::ZFlyEmSynapseAnnotationDialog *ui;
};

#endif // ZFLYEMSYNAPSEANNOTATIONDIALOG_H
