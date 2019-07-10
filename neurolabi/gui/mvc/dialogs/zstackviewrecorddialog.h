#ifndef ZSTACKVIEWRECORDDIALOG_H
#define ZSTACKVIEWRECORDDIALOG_H

#include <QDialog>

class ZStackViewRecorder;

namespace Ui {
class ZStackViewRecordDialog;
}

class ZStackViewRecordDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZStackViewRecordDialog(QWidget *parent = 0);
  ~ZStackViewRecordDialog();

  QString getFullPrefix() const;
  bool isAuto() const;

  void configureRecorder(ZStackViewRecorder *recorder);

private slots:
  void setOutput();

private:
  void setOutputPath(const QString &path);

private:
  Ui::ZStackViewRecordDialog *ui;
};

#endif // ZSTACKVIEWRECORDDIALOG_H
