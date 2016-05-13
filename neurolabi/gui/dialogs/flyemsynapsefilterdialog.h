#ifndef FLYEMSYNAPSEFILTERDIALOG_H
#define FLYEMSYNAPSEFILTERDIALOG_H

#include <QDialog>

namespace Ui {
class FlyEmSynapseFilterDialog;
}

class FlyEmSynapseFilterDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlyEmSynapseFilterDialog(QWidget *parent = 0);
  ~FlyEmSynapseFilterDialog();

private:
  Ui::FlyEmSynapseFilterDialog *ui;
};

#endif // FLYEMSYNAPSEFILTERDIALOG_H
