#ifndef ZFLYEMBODYCOMPARISONDIALOG_H
#define ZFLYEMBODYCOMPARISONDIALOG_H

#include <QDialog>

namespace Ui {
class ZFlyEmBodyComparisonDialog;
}

class ZIntPoint;

class ZFlyEmBodyComparisonDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZFlyEmBodyComparisonDialog(QWidget *parent = 0);
  ~ZFlyEmBodyComparisonDialog();

  std::string getUuid() const;
  void setUuidList(const QStringList &stringList);
  void setUuidList(const std::vector<std::string> &stringList);
  void setCurrentUuidIndex(int index);

  /*!
   * \brief Get segmentation name
   */
  std::string getSegmentation() const;

  bool usingCustomSegmentation() const;
  bool usingDefaultSegmentation() const;
  bool usingSameSegmentation() const;

  ZIntPoint getPosition() const;

  void clearPosition();

private slots:
  void updateSegInfo();
  void connectSignalSlot();
  void pastePosition();

private:
  Ui::ZFlyEmBodyComparisonDialog *ui;
};

#endif // ZFLYEMBODYCOMPARISONDIALOG_H
