#ifndef ZFLYEMBODYSCREENSHOTDIALOG_H
#define ZFLYEMBODYSCREENSHOTDIALOG_H

#include <cstdint>

#include <QDialog>

namespace Ui {
class ZFlyEmBodyScreenshotDialog;
}

class ZFlyEmBodyScreenshotDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZFlyEmBodyScreenshotDialog(QWidget *parent = 0);
  ~ZFlyEmBodyScreenshotDialog();

public:
  int getFrameWidth();
  int getFrameHeight();
  void setFrameWidth(int width);
  void setFrameHeight(int height);
  QString getInputPath() const;
  QString getOutputPath() const;
  void setInputPath(const QString &path);
  void setOutputPath(const QString &path);
  std::vector<uint64_t> getBodyIdArray() const;

private slots:
  void connectSignalSlot();
  void setInput();
  void setOutput();

private:
  Ui::ZFlyEmBodyScreenshotDialog *ui;
};

#endif // ZFLYEMBODYSCREENSHOTDIALOG_H
