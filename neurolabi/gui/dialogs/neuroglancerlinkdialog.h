#ifndef NEUROGLANCERLINKDIALOG_H
#define NEUROGLANCERLINKDIALOG_H

#include <QDialog>

namespace Ui {
class NeuroglancerLinkDialog;
}

class ZDvidEnv;
class QCheckBox;

class NeuroglancerLinkDialog : public QDialog
{
  Q_OBJECT

public:
  explicit NeuroglancerLinkDialog(QWidget *parent = nullptr);
  ~NeuroglancerLinkDialog();

  void init(const ZDvidEnv &env);

private:
  void initCheckbox(QCheckBox *box, bool enabled);

private:
  Ui::NeuroglancerLinkDialog *ui;
};

#endif // NEUROGLANCERLINKDIALOG_H
