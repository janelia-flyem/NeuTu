#ifndef FLYEMPROJECTDIALOG_H
#define FLYEMPROJECTDIALOG_H

#include <QDialog>
#include "zstackdocreader.h"
#include "neutube.h"
#include "dvid/zdvidtarget.h"
#include "zprogressable.h"

class MainWindow;
class ZDvidDialog;
class QMenu;
class ZStackFrame;
class ZProgressReporter;

class FlyEmProjectDialog : public QDialog/*, public ZProgressable*/
{
  Q_OBJECT
public:
  explicit FlyEmProjectDialog(QWidget *parent = 0);
  ~FlyEmProjectDialog();

  MainWindow* getMainWindow();

  //virtual void connectSignalSlot();
  //virtual void createMenu();
  virtual void dump(const QString& str, bool appending = false);
  virtual void showInfo(const QString &str, bool appending = false);

  ZStackFrame *newDataFrame(ZStackDocReader &reader);

  inline void setDocTag(NeuTube::Document::ETag tag) {
    m_docTag = tag;
  }

  void setDvidDialog(ZDvidDialog *dvidDlg);
  void setDvidTargetD(const ZDvidTarget &dvidTarget);

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  void setProgressSignalSlot();
  void notifyProgressAdvanced(double dp);
  void notifyProgressStarted();
  void notifyProgressEnded();

  void setProgressReporter(ZProgressReporter *reporter);

signals:
  void newDocReady(ZStackDocReader *m_docReader, bool readyForPaint);
  void progressAdvanced(double dp);
  void progressStarted();
  void progressEnded();

public slots:
  virtual void consumeNewDoc(ZStackDocReader *m_docReader, bool readyForPaint);

  void advanceProgressSlot(double dp);
  void startProgressSlot();
  void endProgressSlot();

protected:
  ZDvidTarget m_dvidTarget;
  //ZStackDocReader m_docReader;
  ZDvidDialog *m_dvidDlg;
  QMenu *m_mainMenu;
  NeuTube::Document::ETag m_docTag;
  ZProgressReporter *m_progressReporter;
  //ZProgressable m_progressManager;
};

#endif // FLYEMPROJECTDIALOG_H
