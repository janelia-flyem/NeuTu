#ifndef ZSYNAPSEPROPERTYDIALOG_H
#define ZSYNAPSEPROPERTYDIALOG_H

#include <QDialog>

namespace Ui {
class ZSynapsePropertyDialog;
}

class ZSynapsePropertyDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZSynapsePropertyDialog(QWidget *parent = nullptr);
  ~ZSynapsePropertyDialog();

  void setRadius(double pre, double post);

public slots:
  void notifyRadiusChanged();

signals:
  void synapseRadiusChanged(double pre, double post);

private:
  Ui::ZSynapsePropertyDialog *ui;
};

#endif // ZSYNAPSEPROPERTYDIALOG_H
