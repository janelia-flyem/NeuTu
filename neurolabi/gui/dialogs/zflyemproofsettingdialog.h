#ifndef ZFLYEMPROOFSETTINGDIALOG_H
#define ZFLYEMPROOFSETTINGDIALOG_H

#include <QDialog>

namespace Ui {
class ZFlyEmProofSettingDialog;
}

class ZFlyEmProofDoc;

class ZFlyEmProofSettingDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZFlyEmProofSettingDialog(QWidget *parent = 0);
  ~ZFlyEmProofSettingDialog();

  int getGrayscaleCenterCutWidth() const;
  int getGrayscaleCenterCutHeight() const;

  int getSegmentationCenterCutWidth() const;
  int getSegmentationCenterCutHeight() const;

  bool showingFullSkeleton() const;

  void applySettings(ZFlyEmProofDoc *doc) const;

private:
  void initGrayscaleCenterCut();
  void initSegmentationCenterCut();

private:
  Ui::ZFlyEmProofSettingDialog *ui;
};

#endif // ZFLYEMPROOFSETTINGDIALOG_H
