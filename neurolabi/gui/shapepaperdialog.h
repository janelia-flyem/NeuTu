#ifndef SHAPEPAPERDIALOG_H
#define SHAPEPAPERDIALOG_H

#include <QDialog>
#include <QString>
#include <QPointer>
#include <QVector>
#include "zparameter.h"
#include "zstringparameter.h"

class ZFlyEmDataFrame;
class MainWindow;

namespace Ui {
class ShapePaperDialog;
}

class ShapePaperDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ShapePaperDialog(QWidget *parent = 0);
  ~ShapePaperDialog();

  QString getSparseObjectDir() const;
  QString getObjectStackDir() const;
  QString getDataBundlePath() const;
  QString getSimmatPath() const;
  QString getDendrogramPath() const;

  MainWindow* getMainWindow();

  void dump(const QString &str, bool appending = false);

private slots:
  void on_configPushButton_clicked();

  void on_sparseObjectPushButton_clicked();

  void on_dataBundlePushButton_clicked();

  void detachFrame();

  void on_simmatPushButton_clicked();

  void on_dendrogramPushButton_clicked();

  void on_pushButton_5_clicked();

private:
  Ui::ShapePaperDialog *ui;

  ZFlyEmDataFrame *m_frame;

  ZStringParameter *m_objectStackDir;
  ZStringParameter *m_sparseObjectDir;
  ZStringParameter *m_resultDir;
  ZStringParameter *m_bundleDir;
};

#endif // SHAPEPAPERDIALOG_H
