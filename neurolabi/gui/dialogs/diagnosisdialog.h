#ifndef DIAGNOSISDIALOG_H
#define DIAGNOSISDIALOG_H

#include <QDialog>

class QTextBrowser;

namespace Ui {
class DiagnosisDialog;
}

class DiagnosisDialog : public QDialog
{
  Q_OBJECT

public:
  explicit DiagnosisDialog(QWidget *parent = 0);
  ~DiagnosisDialog();

  void scrollToBottom();
  void setSystemInfo(const QString &str);
  void setSystemInfo(const QStringList &info);

public slots:
  void scrollToBottom(int index);

private:
  void loadErrorFile();
  void loadWarnFile();
  void loadInfoFile();
  static void LoadFile(const std::string &filePath, QTextBrowser *browser);

private:
  Ui::DiagnosisDialog *ui;
};

#endif // DIAGNOSISDIALOG_H
